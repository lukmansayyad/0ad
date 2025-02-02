/* Copyright (C) 2020 Wildfire Games.
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

#include "lib/self_test.h"

#include "graphics/Terrain.h"

#include "graphics/Patch.h"
#include "graphics/RenderableObject.h"
#include "maths/Fixed.h"
#include "maths/FixedVector3D.h"

#include <sstream>
#include <vector>

class TestTerrain : public CxxTest::TestSuite
{
	void SetVertex(CTerrain& terrain, ssize_t i, ssize_t j, u16 height)
	{
		terrain.GetHeightMap()[j*terrain.GetVerticesPerSide() + i] = height;
		terrain.MakeDirty(RENDERDATA_UPDATE_VERTICES);
	}

	u16 GetVertex(CTerrain& terrain, ssize_t i, ssize_t j)
	{
		return terrain.GetHeightMap()[j*terrain.GetVerticesPerSide() + i];
	}

	void Set45Slope(CTerrain& terrain)
	{
		SetVertex(terrain, 0, 0, 100);
		SetVertex(terrain, 0, 1, 100);
		SetVertex(terrain, 0, 2, 100);
		SetVertex(terrain, 1, 0, 100 + TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 1, 1, 100 + TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 1, 2, 100 + TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 2, 0, 100 + 2*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 2, 1, 100 + 2*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 2, 2, 100 + 2*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 3, 0, 100 + 2*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 3, 1, 100 + 2*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 3, 2, 100 + 2*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);

	}

	void SetHighPlateau(CTerrain& terrain, int height)
	{
		SetVertex(terrain, 4, 0, 100 + height*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 4, 1, 100 + height*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 4, 2, 100 + height*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 5, 0, 100 + height*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 5, 1, 100 + height*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
		SetVertex(terrain, 5, 2, 100 + height*TERRAIN_TILE_SIZE*HEIGHT_UNITS_PER_METRE);
	}

public:
	void test_GetExactGroundLevel()
	{
		CTerrain terrain;
		terrain.Initialize(4, NULL);
		Set45Slope(terrain);
		SetHighPlateau(terrain, 20);

		float ground;

		ground = terrain.GetExactGroundLevel(0.f, 1.5f*TERRAIN_TILE_SIZE);
		TS_ASSERT_DELTA(ground, 100.f/HEIGHT_UNITS_PER_METRE, 0.01f);

		ground = terrain.GetExactGroundLevel(0.5f*TERRAIN_TILE_SIZE, 1.5f*TERRAIN_TILE_SIZE);
		TS_ASSERT_DELTA(ground, 100.f/HEIGHT_UNITS_PER_METRE+0.5f*TERRAIN_TILE_SIZE, 0.01f);

		ground = terrain.GetExactGroundLevel(1.5f*TERRAIN_TILE_SIZE, 1.5f*TERRAIN_TILE_SIZE);
		TS_ASSERT_DELTA(ground, 100.f/HEIGHT_UNITS_PER_METRE+1.5f*TERRAIN_TILE_SIZE, 0.01f);

		ground = terrain.GetExactGroundLevel(2.5f*TERRAIN_TILE_SIZE, 1.5f*TERRAIN_TILE_SIZE);
		TS_ASSERT_DELTA(ground, 100.f/HEIGHT_UNITS_PER_METRE+2.f*TERRAIN_TILE_SIZE, 0.01f);

		ground = terrain.GetExactGroundLevel(3.5f*TERRAIN_TILE_SIZE, 1.5f*TERRAIN_TILE_SIZE);
		TS_ASSERT_DELTA(ground, 100.f/HEIGHT_UNITS_PER_METRE+11.f*TERRAIN_TILE_SIZE, 0.01f);

		ground = terrain.GetExactGroundLevel(4.5f*TERRAIN_TILE_SIZE, 1.5f*TERRAIN_TILE_SIZE);
		TS_ASSERT_DELTA(ground, 100.f/HEIGHT_UNITS_PER_METRE+20.f*TERRAIN_TILE_SIZE, 0.01f);
	}

	void test_GetExactGroundLevelFixed()
	{
		CTerrain terrain;
		terrain.Initialize(4, NULL);
		Set45Slope(terrain);
		SetHighPlateau(terrain, 20);

		const double maxDelta = 0.0001;

		fixed ground;

		ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(0.f), fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE));
		TS_ASSERT_DELTA(ground.ToDouble(), 100.0/HEIGHT_UNITS_PER_METRE, maxDelta);

		ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(0.5f*TERRAIN_TILE_SIZE), fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE));
		TS_ASSERT_DELTA(ground.ToDouble(), 100.0/HEIGHT_UNITS_PER_METRE+0.5*TERRAIN_TILE_SIZE, maxDelta);

		ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE), fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE));
		TS_ASSERT_DELTA(ground.ToDouble(), 100.0/HEIGHT_UNITS_PER_METRE+1.5*TERRAIN_TILE_SIZE, maxDelta);

		ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(2.5f*TERRAIN_TILE_SIZE), fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE));
		TS_ASSERT_DELTA(ground.ToDouble(), 100.0/HEIGHT_UNITS_PER_METRE+2.0*TERRAIN_TILE_SIZE, maxDelta);

		ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(3.5f*TERRAIN_TILE_SIZE), fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE));
		TS_ASSERT_DELTA(ground.ToDouble(), 100.0/HEIGHT_UNITS_PER_METRE+11.0*TERRAIN_TILE_SIZE, maxDelta);

		ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(4.5f*TERRAIN_TILE_SIZE), fixed::FromFloat(1.5f*TERRAIN_TILE_SIZE));
		TS_ASSERT_DELTA(ground.ToDouble(), 100.0/HEIGHT_UNITS_PER_METRE+20.0*TERRAIN_TILE_SIZE, maxDelta);
	}

	void test_GetExactGroundLevelFixed_max()
	{
		CTerrain terrain;
		terrain.Initialize(4, NULL);
		SetVertex(terrain, 0, 0, 65535);
		SetVertex(terrain, 0, 1, 65535);
		SetVertex(terrain, 1, 0, 65535);
		SetVertex(terrain, 1, 1, 65535);

		const double maxDelta = 0.024;

		int p = 255;
		for (int zi = 0; zi < p; ++zi)
		{
			for (int xi = 0; xi < p; ++xi)
			{
				fixed ground = terrain.GetExactGroundLevelFixed(fixed::FromFloat(xi/(float)p*TERRAIN_TILE_SIZE), fixed::FromFloat(zi/(float)p*TERRAIN_TILE_SIZE));
				TS_ASSERT_DELTA(ground.ToDouble(), 65535.0/HEIGHT_UNITS_PER_METRE, maxDelta);
			}
		}
	}

	void test_CalcNormal()
	{
		CTerrain terrain;
		terrain.Initialize(4, NULL);
		Set45Slope(terrain);

		CVector3D vec;

		terrain.CalcNormal(1, 1, vec);
		TS_ASSERT_DELTA(vec.X, -1.f/sqrt(2.f), 0.01f);
		TS_ASSERT_DELTA(vec.Y, 1.f/sqrt(2.f), 0.01f);
		TS_ASSERT_EQUALS(vec.Z, 0.f);

		terrain.CalcNormal(2, 1, vec);
		TS_ASSERT_DELTA(vec.X, (-1.f/sqrt(2.f)) / sqrt(2.f+sqrt(2.f)), 0.01f);
		TS_ASSERT_DELTA(vec.Y, (1.f+1.f/sqrt(2.f)) / sqrt(2.f+sqrt(2.f)), 0.01f);
		TS_ASSERT_EQUALS(vec.Z, 0);

		terrain.CalcNormal(5, 1, vec);
		TS_ASSERT_EQUALS(vec.X, 0.f);
		TS_ASSERT_EQUALS(vec.Y, 1.f);
		TS_ASSERT_EQUALS(vec.Z, 0.f);
	}

	void test_CalcNormalFixed()
	{
		CTerrain terrain;
		terrain.Initialize(4, NULL);
		Set45Slope(terrain);

		CFixedVector3D vec;

		terrain.CalcNormalFixed(1, 1, vec);
		TS_ASSERT_DELTA(vec.X.ToFloat(), -1.f/sqrt(2.f), 0.01f);
		TS_ASSERT_DELTA(vec.Y.ToFloat(), 1.f/sqrt(2.f), 0.01f);
		TS_ASSERT_EQUALS(vec.Z.ToFloat(), 0.f);

		terrain.CalcNormalFixed(2, 1, vec);
		TS_ASSERT_DELTA(vec.X.ToFloat(), (-1.f/sqrt(2.f)) / sqrt(2.f+sqrt(2.f)), 0.01f);
		TS_ASSERT_DELTA(vec.Y.ToFloat(), (1.f+1.f/sqrt(2.f)) / sqrt(2.f+sqrt(2.f)), 0.01f);
		TS_ASSERT_EQUALS(vec.Z.ToFloat(), 0);

		terrain.CalcNormalFixed(5, 1, vec);
		TS_ASSERT_EQUALS(vec.X.ToFloat(), 0.f);
		TS_ASSERT_EQUALS(vec.Y.ToFloat(), 1.f);
		TS_ASSERT_EQUALS(vec.Z.ToFloat(), 0.f);
	}

	void test_Resize()
	{
		// We do resize by size in patches, so it doesn't make sense to
		// fill each vertex with a different value. Instead we use a single
		// value per a patch.
		struct ResizeTestCase
		{
			ssize_t horizontalOffset, verticalOffset;
			std::vector<std::vector<u16>> sourcePatches;
			std::vector<std::vector<u16>> expectedPatches;
		};
		const ResizeTestCase testCases[] = {
			// Without offset.
			{
				0, 0,
				{
				{42}
				},
				{
				{42}
				}
			},
			{
				0, 0,
				{
				{1, 2},
				{3, 4}
				},
				{
				{1, 2},
				{3, 4}
				}
			},
			{
				0, 0,
				{
				{1, 2},
				{3, 4}
				},
				{
				{1, 1, 2, 2},
				{1, 1, 2, 2},
				{3, 3, 4, 4},
				{3, 3, 4, 4}
				}
			},
			{
				0, 0,
				{
				{ 1,  2 , 3,  4},
				{ 5,  6 , 7,  8},
				{ 9, 10, 11, 12},
				{13, 14, 15, 16}
				},
				{
				{ 6,  7},
				{10, 11},
				}
			},
			// With offset.
			{
				-2, -2,
				{
				{1, 2},
				{3, 4}
				},
				{
				{0, 0},
				{0, 0}
				}
			},
			{
				-2, 0,
				{
				{1, 2},
				{3, 4}
				},
				{
				{0, 0},
				{0, 0}
				}
			},
			{
				4, 4,
				{
				{1, 2},
				{3, 4}
				},
				{
				{0, 0},
				{0, 0}
				}
			},
			{
				1, 1,
				{
				{ 1,  2 , 3,  4},
				{ 5,  6 , 7,  8},
				{ 9, 10, 11, 12},
				{13, 14, 15, 16}
				},
				{
				{ 6 , 7,  8, 8},
				{10, 11, 12, 12},
				{14, 15, 16, 16},
				{14, 15, 16, 16}
				}
			},
			{
				1, 1,
				{
				{1, 2},
				{3, 4}
				},
				{
				{4, 4},
				{4, 4}
				}
			},
			{
				-2, 0,
				{
				{ 1,  2 , 3,  4},
				{ 5,  6 , 7,  8},
				{ 9, 10, 11, 12},
				{13, 14, 15, 16}
				},
				{
				{5, 5},
				{9, 9}
				}
			},
			{
				2, -2,
				{
				{ 1,  2 , 3,  4},
				{ 5,  6 , 7,  8},
				{ 9, 10, 11, 12},
				{13, 14, 15, 16}
				},
				{
				{4, 4},
				{4, 4}
				}
			},
			{
				3, -1,
				{
				{ 1,  2 , 3,  4},
				{ 5,  6 , 7,  8},
				{ 9, 10, 11, 12},
				{13, 14, 15, 16}
				},
				{
				{0, 0},
				{0, 0}
				}
			},
			{
				-2, -1,
				{
				{1, 2},
				{3, 4}
				},
				{
				{1, 1, 1, 1},
				{1, 1, 1, 1},
				{1, 1, 1, 1},
				{3, 3, 3, 3}
				}
			},
		};

		for (const ResizeTestCase& testCase : testCases)
		{
			const ssize_t sourceSize = testCase.sourcePatches.size();
			const ssize_t expectedSize = testCase.expectedPatches.size();

			TS_ASSERT_LESS_THAN(0, sourceSize);
			TS_ASSERT_LESS_THAN(0, expectedSize);

			const ssize_t sourceMapSize = sourceSize * PATCH_SIZE + 1;
			const ssize_t expectedMapSize = expectedSize * PATCH_SIZE + 1;

			CTerrain terrain;
			{
				std::vector<u16> heightmap(sourceMapSize * sourceMapSize);
				for (ssize_t jTile = 0; jTile < sourceSize; ++jTile)
				{
					TS_ASSERT_EQUALS(sourceSize, testCase.sourcePatches[jTile].size());
					for (ssize_t iTile = 0; iTile < sourceSize; ++iTile)
					{
						for (ssize_t j = 0; j < PATCH_SIZE; ++j)
							for (ssize_t i = 0; i < PATCH_SIZE; ++i)
							{
								const ssize_t idx =
									(jTile * PATCH_SIZE + j) * sourceMapSize +
									 iTile * PATCH_SIZE + i;
								heightmap[idx] = testCase.sourcePatches[jTile][iTile];
							}
					}
				}
				terrain.Initialize(sourceSize, heightmap.data());
			}

			terrain.ResizeAndOffset(expectedSize, testCase.horizontalOffset, testCase.verticalOffset);

			TS_ASSERT_EQUALS(expectedMapSize, terrain.GetVerticesPerSide());
			TS_ASSERT_EQUALS(expectedSize, terrain.GetPatchesPerSide());

			for (ssize_t jTile = 0; jTile < expectedSize; ++jTile)
			{
				TS_ASSERT_EQUALS(
					expectedSize, testCase.expectedPatches[jTile].size());
				for (ssize_t iTile = 0; iTile < expectedSize; ++iTile)
				{
					for (ssize_t j = 0; j < PATCH_SIZE; ++j)
						for (ssize_t i = 0; i < PATCH_SIZE; ++i)
						{
							// The whole patch should have the same height,
							// since we resize by patches.
							if (GetVertex(terrain, iTile * PATCH_SIZE,
							                       jTile * PATCH_SIZE) ==
							    GetVertex(terrain, iTile * PATCH_SIZE + i,
							                       jTile * PATCH_SIZE + j))
								continue;
							TS_FAIL("The whole patch should have the same height");
							std::stringstream ss;
							ss << "iTile=" << iTile << " jTile=" << jTile
							   << " i=" << i << " j=" << j;
							TS_WARN(ss.str());
							ss.str(std::string());
							ss << "found=" << GetVertex(terrain, iTile * PATCH_SIZE + i,
							                                     jTile * PATCH_SIZE + j)
							   << " expected=" << GetVertex(terrain, iTile * PATCH_SIZE,
							                                         jTile * PATCH_SIZE);
							TS_WARN(ss.str());
							return;
						}

					if (testCase.expectedPatches[jTile][iTile] ==
						GetVertex(terrain, iTile * PATCH_SIZE,
						                   jTile * PATCH_SIZE))
						continue;
					std::stringstream ss;
					ss << "The patch has wrong height"
					   << " (i=" << iTile << " j=" << jTile << "):"
					   << " found=" << GetVertex(terrain, iTile * PATCH_SIZE,
					                                      jTile * PATCH_SIZE)
					   << " expected=" << testCase.expectedPatches[jTile][iTile];
					TS_FAIL(ss.str());
					ss.str(std::string());
					ss << "Terrain (" << terrain.GetPatchesPerSide()
					   << "x" << terrain.GetPatchesPerSide() << "):";
					TS_WARN(ss.str());
					for (ssize_t jj = 0; jj < expectedSize; ++jj)
					{
						ss.str(std::string());
						ss << "[";
						for (ssize_t ii = 0; ii < expectedSize; ++ii)
						{
							if (ii)
								ss << ", ";
							ss << GetVertex(terrain, ii * PATCH_SIZE,
							                         jj * PATCH_SIZE);
						}
						ss << "]";
						TS_WARN(ss.str());
					}
					return;
				}
			}
		}
	}
};
