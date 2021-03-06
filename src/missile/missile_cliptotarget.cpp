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
/**@name missile_cliptotarget.cpp - The missile ClipToTarget. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "missile.h"

#include "database/defines.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "unit/unit.h"

/**
**  Missile remains clipped to target's current goal and plays his animation once
*/
void MissileClipToTarget::Action()
{
	this->Wait = this->Type->get_sleep();

	if (this->TargetUnit != nullptr) {
		this->position.x = this->TargetUnit->tilePos.x * stratagus::defines::get()->get_tile_width() + this->TargetUnit->get_pixel_offset().x();
		this->position.y = this->TargetUnit->tilePos.y * stratagus::defines::get()->get_tile_height() + this->TargetUnit->get_pixel_offset().y();
	}

	if (this->NextMissileFrame(1, 0)) {
		if (this->SourceUnit && this->SourceUnit->IsAliveOnMap() && this->TargetUnit->IsAliveOnMap()) {
			this->MissileHit();
		}
		this->TTL = 0;
	}
}
