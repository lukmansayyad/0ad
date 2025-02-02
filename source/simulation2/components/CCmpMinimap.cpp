/* Copyright (C) 2022 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "simulation2/system/Component.h"
#include "ICmpMinimap.h"

#include "simulation2/components/ICmpPlayerManager.h"
#include "simulation2/components/ICmpPlayer.h"
#include "simulation2/components/ICmpOwnership.h"
#include "simulation2/MessageTypes.h"

#include "graphics/Color.h"

class CCmpMinimap final : public ICmpMinimap
{
public:
	static void ClassInit(CComponentManager& componentManager)
	{
		componentManager.SubscribeToMessageType(MT_Deserialized);
		componentManager.SubscribeToMessageType(MT_PositionChanged);
		componentManager.SubscribeToMessageType(MT_OwnershipChanged);
		componentManager.SubscribeToMessageType(MT_PlayerColorChanged);
		componentManager.SubscribeToMessageType(MT_MinimapPing);
	}

	DEFAULT_COMPONENT_ALLOCATOR(Minimap)

	// Template state:

	bool m_UsePlayerColor;

	u8 m_R, m_G, m_B; // static template state if m_UsePlayerColor false; dynamic state if true

	// Dynamic state:

	bool m_Active;
	entity_pos_t m_X, m_Z; // cache the latest position for more efficient rendering; only valid when m_Active true

	// Not serialized (based on renderer timing):
	// TODO: eventually ping state should be serialized and tied into simulation time, but currently lag causes too many problems
	double m_PingEndTime;
	bool m_IsPinging;

	bool m_HasIcon = false;
	std::string m_IconPath;
	float m_IconSize = 16.0f;

	static std::string GetSchema()
	{
		return
			"<element name='Type'>"
				"<choice>"
					"<value>food</value>"
					"<value>wood</value>"
					"<value>stone</value>"
					"<value>metal</value>"
					"<value>structure</value>"
					"<value>unit</value>"
					"<value>support</value>"
					"<value>hero</value>"
				"</choice>"
			"</element>"
			"<optional>"
				"<element name='Color'>"
					"<attribute name='r'>"
						"<data type='integer'><param name='minInclusive'>0</param><param name='maxInclusive'>255</param></data>"
					"</attribute>"
					"<attribute name='g'>"
						"<data type='integer'><param name='minInclusive'>0</param><param name='maxInclusive'>255</param></data>"
					"</attribute>"
					"<attribute name='b'>"
						"<data type='integer'><param name='minInclusive'>0</param><param name='maxInclusive'>255</param></data>"
					"</attribute>"
				"</element>"
			"</optional>"
			"<optional>"
				"<element name='Icon' a:help='Icon texture that should be displayed on a minimap. Filepath relative to art/textures/ui/session/icons/minimap/.'>"
					"<attribute name='size'>"
						"<data type='float'><param name='minExclusive'>0</param></data>"
					"</attribute>"
					"<text/>"
				"</element>"
			"</optional>";
	}

	void Init(const CParamNode& paramNode) override
	{
		m_Active = true;
		m_IsPinging = false;
		m_PingEndTime = 0.0;
		m_HasIcon = false;

		const CParamNode& color = paramNode.GetChild("Color");
		if (color.IsOk())
		{
			m_UsePlayerColor = false;
			m_R = (u8)color.GetChild("@r").ToInt();
			m_G = (u8)color.GetChild("@g").ToInt();
			m_B = (u8)color.GetChild("@b").ToInt();
		}
		else
		{
			m_UsePlayerColor = true;
			// Choose a bogus color which will get replaced once we have an owner
			m_R = 255;
			m_G = 0;
			m_B = 255;
		}

		const CParamNode& iconNode = paramNode.GetChild("Icon");
		if (iconNode.IsOk())
		{
			const CParamNode& iconSizeNode = iconNode.GetChild("@size");
			if (iconSizeNode.IsOk())
			{
				m_HasIcon = true;
				m_IconPath = "art/textures/ui/session/icons/minimap/" + iconNode.ToString();
				m_IconSize = iconSizeNode.ToFloat();
			}
		}
	}

	void Deinit() override
	{
	}

	template<typename S>
	void SerializeCommon(S& serialize)
	{
		serialize.Bool("active", m_Active);

		if (m_Active)
		{
			serialize.NumberFixed_Unbounded("x", m_X);
			serialize.NumberFixed_Unbounded("z", m_Z);
		}
	}

	void Serialize(ISerializer& serialize) override
	{
		SerializeCommon(serialize);
	}

	void Deserialize(const CParamNode& paramNode, IDeserializer& deserialize) override
	{
		Init(paramNode);

		SerializeCommon(deserialize);
	}

	void HandleMessage(const CMessage& msg, bool UNUSED(global)) override
	{
		switch (msg.GetType())
		{
		case MT_PositionChanged:
		{
			const CMessagePositionChanged& data = static_cast<const CMessagePositionChanged&> (msg);

			if (data.inWorld)
			{
				m_Active = true;
				m_X = data.x;
				m_Z = data.z;
			}
			else
			{
				m_Active = false;
			}

			break;
		}
		case MT_Deserialized:
		case MT_OwnershipChanged:
		case MT_PlayerColorChanged:
		{
			UpdateColor();
			break;
		}
		case MT_MinimapPing:
		{
			CmpPtr<ICmpOwnership> cmpOwnership(GetSimContext(), GetEntityId());
			if (!cmpOwnership || cmpOwnership->GetOwner() != (player_id_t)GetSimContext().GetCurrentDisplayedPlayer())
				break;

			// This depends on the viewing player, so don't alter the synchronized simulation state
			m_IsPinging = true;
			m_PingEndTime = 0.0;

			break;
		}
		}
	}

	bool GetRenderData(u8& r, u8& g, u8& b, entity_pos_t& x, entity_pos_t& z) const override
	{
		if (!m_Active)
			return false;

		r = m_R;
		g = m_G;
		b = m_B;
		x = m_X;
		z = m_Z;
		return true;
	}

	bool CheckPing(double currentTime, double pingDuration) override
	{
		if (!m_Active || !m_IsPinging)
			return false;

		// We're currently pinging
		if (m_PingEndTime == 0.0)
			m_PingEndTime = currentTime + pingDuration;
		else if (currentTime > m_PingEndTime)
		{
			m_IsPinging = false;
			m_PingEndTime = 0;
		}

		return m_IsPinging;
	}

	void UpdateColor() override
	{
		if (!m_UsePlayerColor)
			return;

		CmpPtr<ICmpOwnership> cmpOwnership(GetEntityHandle());
		if (!cmpOwnership)
			return;

		player_id_t owner = cmpOwnership->GetOwner();
		if (owner == INVALID_PLAYER)
			return;

		CmpPtr<ICmpPlayerManager> cmpPlayerManager(GetSystemEntity());
		if (!cmpPlayerManager)
			return;

		CmpPtr<ICmpPlayer> cmpPlayer(GetSimContext(), cmpPlayerManager->GetPlayerByID(owner));
		if (!cmpPlayer)
			return;

		CColor color = cmpPlayer->GetDisplayedColor();
		m_R = (u8) (color.r * 255);
		m_G = (u8) (color.g * 255);
		m_B = (u8) (color.b * 255);
	}

	bool HasIcon() override
	{
		return m_HasIcon;
	}

	std::string GetIconPath() override
	{
		return m_IconPath;
	}

	float GetIconSize() override
	{
		return m_IconSize;
	}
};

REGISTER_COMPONENT_TYPE(Minimap)
