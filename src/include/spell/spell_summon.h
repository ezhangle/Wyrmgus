//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
//
//      (c) Copyright 1999-2012 by Vladi Belperchinov-Shabanski,
//                                 Joris DAUPHIN, and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

#pragma once

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "spells.h"

class Spell_Summon : public SpellActionType
{
public:
	Spell_Summon() : SpellActionType(1), UnitType(nullptr), TTL(0),
		RequireCorpse(false), JoinToAiForce(false) {};
	virtual void ProcessConfigData(const CConfigData *config_data) override {}
	virtual int Cast(CUnit &caster, const CSpell &spell,
					 CUnit *target, const Vec2i &goalPos, int z, int modifier);
	virtual void Parse(lua_State *l, int startIndex, int endIndex);

private:
	stratagus::unit_type *UnitType;    /// Type of unit to be summoned.
	int TTL;                /// Time to live for summoned unit. 0 means infinite
	int RequireCorpse;      /// Corpse consumed while summoning.
	bool JoinToAiForce;     /// if true, captured unit is joined into caster's AI force, if available
};
