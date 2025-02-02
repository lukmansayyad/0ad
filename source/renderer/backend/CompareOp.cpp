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

#include "CompareOp.h"

namespace Renderer
{

namespace Backend
{

CompareOp ParseCompareOp(const CStr& str)
{
	// TODO: it might make sense to use upper case in XML for consistency.
#define CASE(NAME, VALUE) if (str == NAME) return CompareOp::VALUE
	CASE("never", NEVER);
	CASE("less", LESS);
	CASE("equal", EQUAL);
	CASE("lequal", LESS_OR_EQUAL);
	CASE("greater", GREATER);
	CASE("notequal", NOT_EQUAL);
	CASE("gequal", GREATER_OR_EQUAL);
	CASE("always", ALWAYS);
#undef CASE
	debug_warn("Invalid compare op");
	return CompareOp::NEVER;
}

} // namespace Backend

} // namespace Renderer
