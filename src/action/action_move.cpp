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
/**@name action_move.cpp - The move action. */
//
//      (c) Copyright 1998-2020 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "stratagus.h"

#include "action/action_move.h"

#include "ai.h"
#include "animation.h"
//Wyrmgus start
#include "commands.h"
//Wyrmgus end
#include "database/defines.h"
#include "iolib.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/tileset.h"
#include "pathfinder.h"
#include "script.h"
#include "settings.h"
#include "sound/sound.h"
#include "sound/unit_sound_type.h"
#include "ui/interface.h"
#include "ui/ui.h"
#include "unit/unit.h"
#include "unit/unit_find.h"
#include "unit/unit_type.h"
#include "unit/unit_type_type.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//Wyrmgus start
///* static */ COrder *COrder::NewActionMove(const Vec2i &pos)
/* static */ COrder *COrder::NewActionMove(const Vec2i &pos, int z)
//Wyrmgus end
{
	Assert(CMap::Map.Info.IsPointOnMap(pos, z));

	COrder_Move *order = new COrder_Move;

	order->goalPos = pos;
	//Wyrmgus start
	order->MapLayer = z;
	//Wyrmgus end
	return order;
}

/* virtual */ void COrder_Move::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-move\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"range\", %d,", this->Range);
	//Wyrmgus start
//	file.printf(" \"tile\", {%d, %d}", this->goalPos.x, this->goalPos.y);
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);
	file.printf(" \"map-layer\", %d", this->MapLayer);
	//Wyrmgus end

	file.printf("}");
}

/* virtual */ bool COrder_Move::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "range")) {
		++j;
		this->Range = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos.x , &this->goalPos.y);
		lua_pop(l, 1);
	//Wyrmgus start
	} else if (!strcmp(value, "map-layer")) {
		++j;
		this->MapLayer = LuaToNumber(l, -1, j + 1);
	//Wyrmgus end
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Move::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Move::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	if (this->MapLayer != UI.CurrentMapLayer->ID) {
		return lastScreenPos;
	}

	const PixelPos targetPos = vp.TilePosToScreen_Center(this->goalPos);

	if (Preference.ShowPathlines) {
		Video.FillCircleClip(ColorGreen, lastScreenPos, 2);
		Video.DrawLineClip(ColorGreen, lastScreenPos, targetPos);
		Video.FillCircleClip(ColorGreen, targetPos, 3);
	}
	
	return targetPos;
}

/* virtual */ void COrder_Move::UpdatePathFinderData(PathFinderInput &input)
{
	const Vec2i tileSize(0, 0);
	input.SetGoal(this->goalPos, tileSize, this->MapLayer);

	int distance = this->Range;
	input.SetMaxRange(distance);
	input.SetMinRange(0);
}

/**
**  Unit moves! Generic function called from other actions.
**
**  @param unit  Pointer to unit.
**
**  @return      >0 remaining path length, 0 wait for path, -1
**               reached goal, -2 can't reach the goal.
*/
int DoActionMove(CUnit &unit)
{
	//Wyrmgus start
	CMapField &mf = *unit.MapLayer->Field(unit.tilePos);
	if (unit.Type->BoolFlag[BRIDGE_INDEX].value && unit.CanMove()) { // if is a raft, don't move if any unit over it is still moving
		std::vector<CUnit *> table;
		Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
		for (size_t i = 0; i != table.size(); ++i) {
			if (!table[i]->Removed && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land) {
				if (table[i]->Moving) {
					unit.Wait = 1;
					unit.Moving = 0;
					unit.StepCount = 0;
					return PF_WAIT;
				}
			}
		}
	} else if ((mf.Flags & MapFieldBridge) && !unit.Type->BoolFlag[BRIDGE_INDEX].value && unit.Type->UnitType == UnitTypeType::Land) { //if the unit is a land unit over a raft, don't move if the raft is still moving
		std::vector<CUnit *> table;
		Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
		for (size_t i = 0; i != table.size(); ++i) {
			if (!table[i]->Removed && table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->CanMove()) {
				if (table[i]->Moving) {
					unit.Wait = 1;
					unit.Moving = 0;
					unit.StepCount = 0;
					return PF_WAIT;
				}
			}
		}
	}
	//Wyrmgus end
		
	Vec2i posd; // movement in tile.
	int d;

	Assert(unit.CanMove());

	if (!unit.Moving && (unit.GetAnimations()->Move.get() != unit.Anim.CurrAnim || !unit.Anim.Wait)) {
		Assert(!unit.Anim.Unbreakable);

		// FIXME: So units flying up and down are not affected.
		unit.pixel_offset = QPoint(0, 0);
		//Wyrmgus start
		if (unit.Type->BoolFlag[BRIDGE_INDEX].value) { // if is a raft, move everything on top of it as it moves
			std::vector<CUnit *> table;
			Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
			for (size_t i = 0; i != table.size(); ++i) {
				if (!table[i]->Removed && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land) {
					table[i]->pixel_offset = QPoint(0, 0);
				}
			}
		}
		//Wyrmgus end

		UnmarkUnitFieldFlags(unit);
		d = NextPathElement(unit, &posd.x, &posd.y);
		MarkUnitFieldFlags(unit);
		switch (d) {
			case PF_UNREACHABLE: // Can't reach, stop
				if (unit.Player->AiEnabled) {
					AiCanNotMove(unit);
				}
				unit.Moving = 0;
				unit.StepCount = 0;
				return d;
			case PF_REACHED: // Reached goal, stop
				unit.Moving = 0;
				return d;
			case PF_WAIT: // No path, wait
				unit.Wait = 10;

				unit.Moving = 0;
				unit.StepCount = 0;
				return d;
			default: // On the way moving
				unit.Moving = 1;
				break;
		}
		
		if (unit.Type->UnitType == UnitTypeType::Naval) { // Boat (un)docking?
			//Wyrmgus start
//			const CMapField &mf_cur = *Map.Field(unit.Offset);
//			const CMapField &mf_next = *Map.Field(unit.tilePos + posd);
			const CMapField &mf_cur = *unit.MapLayer->Field(unit.Offset);
			const CMapField &mf_next = *unit.MapLayer->Field(unit.tilePos + posd);
			//Wyrmgus end

			if (mf_cur.WaterOnMap() && mf_next.CoastOnMap()) {
				PlayUnitSound(unit, stratagus::unit_sound_type::docking);
			} else if (mf_cur.CoastOnMap() && mf_next.WaterOnMap()) {
				PlayUnitSound(unit, stratagus::unit_sound_type::docking); // undocking
			}
		}
		Vec2i pos = unit.tilePos + posd;
		//Wyrmgus start
		if (unit.Type->BoolFlag[BRIDGE_INDEX].value) { // if is a raft, move everything on top of it as it moves
			std::vector<CUnit *> table;
			Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
			for (size_t i = 0; i != table.size(); ++i) {
				if (!table[i]->Removed && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land) {
					table[i]->MoveToXY(pos, table[i]->MapLayer->ID);
					table[i]->pixel_offset.setX(-posd.x * stratagus::defines::get()->get_tile_width());
					table[i]->pixel_offset.setY(-posd.y * stratagus::defines::get()->get_tile_height());
					UnitHeadingFromDeltaXY(*table[i], posd);
				}
			}
		}
		//Wyrmgus end
		//Wyrmgus start
//		unit.MoveToXY(pos);
		unit.MoveToXY(pos, unit.MapLayer->ID);
		unit.StepCount++;
		unit.StepCount = std::min(unit.StepCount, (unsigned char) 10);
		//Wyrmgus end
		//Wyrmgus start
		PlayUnitSound(unit, stratagus::unit_sound_type::step);
		//Wyrmgus end

		// Remove unit from the current selection
		//Wyrmgus start
//		if (unit.Selected && !Map.Field(pos)->playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
		if (unit.Selected && !unit.MapLayer->Field(pos)->playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
		//Wyrmgus end
			if (IsOnlySelected(unit)) { //  Remove building cursor
				CancelBuildingMode();
			}
			if (!ReplayRevealMap) {
				UnSelectUnit(unit);
				SelectionChanged();
			}
		}

		unit.pixel_offset.setX(-posd.x * stratagus::defines::get()->get_tile_width());
		unit.pixel_offset.setY(-posd.y * stratagus::defines::get()->get_tile_height());
		unit.Frame = unit.Type->StillFrame;
		UnitHeadingFromDeltaXY(unit, posd);
	} else {
		posd.x = Heading2X[unit.Direction / NextDirection];
		posd.y = Heading2Y[unit.Direction / NextDirection];
		d = unit.pathFinderData->output.Length + 1;
	}

	unit.pathFinderData->output.Cycles++;// reset have to be manualy controlled by caller.
	int move = UnitShowAnimationScaled(unit, unit.GetAnimations()->Move.get(), DefaultTileMovementCost);

	unit.pixel_offset += QPoint(posd.x * move, posd.y * move);
	
	//Wyrmgus start
	if (unit.Type->BoolFlag[BRIDGE_INDEX].value) { // if is a raft, move everything on top of it as it moves
		std::vector<CUnit *> table;
		Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
		for (size_t i = 0; i != table.size(); ++i) {
			if (!table[i]->Removed && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land) {
				table[i]->pixel_offset += QPoint(posd.x * move, posd.y * move);
			}
		}
	}
	//Wyrmgus end
	
	//Wyrmgus start
	if (abs(unit.get_pixel_offset().x()) > (stratagus::defines::get()->get_tile_width() * 2) || abs(unit.get_pixel_offset().y()) > (stratagus::defines::get()->get_tile_height() * 2)) {
		unit.pixel_offset = QPoint(0, 0);
#ifdef DEBUG
		fprintf(stderr, "Error in DoActionMove: unit's pixel movement was too big.\n");
#endif
		
		if (unit.Type->BoolFlag[BRIDGE_INDEX].value) { // if is a raft, move everything on top of it as it moves
			std::vector<CUnit *> table;
			Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
			for (size_t i = 0; i != table.size(); ++i) {
				if (!table[i]->Removed && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land) {
					table[i]->pixel_offset = QPoint(0, 0);
				}
			}
		}
	}
	//Wyrmgus end

	// Finished move animation, set Moving to 0 so we recalculate the path
	// next frame
	// FIXME: this is broken for subtile movement
	if (!unit.Anim.Unbreakable && !unit.get_pixel_offset().x() && !unit.get_pixel_offset().y()) {
		unit.Moving = 0;
	}

	return d;
}


/* virtual */ void COrder_Move::Execute(CUnit &unit)
{
	Assert(unit.CanMove());

	if (unit.Wait) {
		if (!unit.Waiting) {
			unit.Waiting = 1;
			unit.WaitBackup = unit.Anim;
		}
		UnitShowAnimation(unit, unit.GetAnimations()->Still.get());
		unit.Wait--;
		return;
	}
	if (unit.Waiting) {
		unit.Anim = unit.WaitBackup;
		unit.Waiting = 0;
	}
	// FIXME: (mr-russ) Make a reachable goal here with GoalReachable ...

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			// Some tries to reach the goal
			this->Range++;
			break;
		case PF_REACHED:
			//Wyrmgus start
			if (this->Range >= 1) {
				if ((unit.MapLayer->Field(unit.tilePos)->Flags & MapFieldBridge) && !unit.Type->BoolFlag[BRIDGE_INDEX].value && unit.Type->UnitType == UnitTypeType::Land) { //if the unit is a land unit over a raft
					std::vector<CUnit *> table;
					Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
					for (size_t i = 0; i != table.size(); ++i) {
						if (!table[i]->Removed && table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->CanMove()) {
							if (table[i]->CurrentAction() == UnitAction::Still) {
								CommandStopUnit(*table[i]);
								CommandMove(*table[i], this->goalPos, FlushCommands, this->MapLayer);
							}
							return;
						}
					}
				} else if (unit.Type->BoolFlag[BRIDGE_INDEX].value && unit.CanMove()) { // if is a raft
					std::vector<CUnit *> table;
					Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
					for (size_t i = 0; i != table.size(); ++i) {
						if (!table[i]->Removed && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land && table[i]->CanMove()) {
							if (table[i]->CurrentAction() == UnitAction::Still) {
								CommandStopUnit(*table[i]);
								CommandMove(*table[i], this->goalPos, FlushCommands, this->MapLayer);
							}
							return;
						}
					}
				}
			}
			//Wyrmgus end
			this->Finished = true;
			break;
		default:
			break;
	}
}
