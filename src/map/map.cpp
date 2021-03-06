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
//      (c) Copyright 1998-2020 by Lutz Sammer, Vladi Shabanski,
//                                 Francois Beerten and Andrettin
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

#include "map/map.h"

#include "database/defines.h"
//Wyrmgus start
#include "editor.h"
#include "game.h" // for the SaveGameLoading variable
//Wyrmgus end
#include "iolib.h"
#include "map/map_layer.h"
#include "map/map_template.h"
#include "map/site.h"
#include "map/terrain_feature.h"
#include "map/terrain_type.h"
#include "map/tileset.h"
#include "plane.h"
#include "player.h"
//Wyrmgus start
#include "province.h"
#include "quest.h"
#include "settings.h"
#include "sound/sound_server.h"
#include "species.h"
//Wyrmgus end
#include "time/calendar.h"
#include "time/season.h"
#include "time/season_schedule.h"
#include "time/time_of_day.h"
#include "time/time_of_day_schedule.h"
//Wyrmgus start
#include "translate.h"
//Wyrmgus end
#include "ui/ui.h"
#include "unit/unit.h"
//Wyrmgus start
#include "unit/unit_find.h"
//Wyrmgus end
#include "unit/unit_manager.h"
#include "unit/unit_type_type.h"
//Wyrmgus start
#include "upgrade/upgrade.h"
//Wyrmgus end
#include "util/container_util.h"
#include "util/size_util.h"
#include "util/vector_random_util.h"
#include "util/vector_util.h"
#include "version.h"
#include "video.h"
#include "world.h"

#ifdef USE_OAML
#include <oaml.h>

extern oamlApi *oaml;
extern bool enableOAML;
#endif

CMap CMap::Map; //the current map
int FlagRevealMap; //flag must reveal the map
int ReplayRevealMap; //reveal Map is replay
int ForestRegeneration; //forest regeneration
char CurrentMapPath[1024]; //path of the current map

/*----------------------------------------------------------------------------
--  Visible and explored handling
----------------------------------------------------------------------------*/

/**
**  Marks seen tile -- used mainly for the Fog Of War
**
**  @param mf  MapField-position.
*/
//Wyrmgus start
//void CMap::MarkSeenTile(CMapField &mf)
void CMap::MarkSeenTile(CMapField &mf, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	const unsigned int tile = mf.getGraphicTile();
//	const unsigned int seentile = mf.playerInfo.SeenTile;
	//Wyrmgus end

	//  Nothing changed? Seeing already the correct tile.
	//Wyrmgus start
//	if (tile == seentile) {
	if (mf.IsSeenTileCorrect()) {
	//Wyrmgus end
		return;
	}
	//Wyrmgus start
//	mf.playerInfo.SeenTile = tile;
	mf.UpdateSeenTile();
	//Wyrmgus end

#ifdef MINIMAP_UPDATE
	//rb - GRRRRRRRRRRRR
	//Wyrmgus start
//	const unsigned int index = &mf - Map.Fields;
//	const int y = index / Info.MapWidth;
//	const int x = index - (y * Info.MapWidth);
	const CMapLayer *map_layer = Map.MapLayers[z];
	const unsigned int index = &mf - map_layer->Fields;
	const int y = index / map_layer->GetWidth();
	const int x = index - (y * map_layer->GetWidth());
	//Wyrmgus end
	const Vec2i pos = {x, y}
#endif

	//Wyrmgus start
	/*
	if (this->Tileset->TileTypeTable.empty() == false) {
#ifndef MINIMAP_UPDATE
		//rb - GRRRRRRRRRRRR
		const unsigned int index = &mf - Map.Fields;
		const int y = index / Info.MapWidth;
		const int x = index - (y * Info.MapWidth);
		const Vec2i pos(x, y);
#endif

		//  Handle wood changes. FIXME: check if for growing wood correct?
		if (tile == this->Tileset->getRemovedTreeTile()) {
			FixNeighbors(MapFieldForest, 1, pos);
		} else if (seentile == this->Tileset->getRemovedTreeTile()) {
			FixTile(MapFieldForest, 1, pos);
		} else if (mf.ForestOnMap()) {
			FixTile(MapFieldForest, 1, pos);
			FixNeighbors(MapFieldForest, 1, pos);

			// Handle rock changes.
		} else if (tile == Tileset->getRemovedRockTile()) {
			FixNeighbors(MapFieldRocks, 1, pos);
		} else if (seentile == Tileset->getRemovedRockTile()) {
			FixTile(MapFieldRocks, 1, pos);
		} else if (mf.RockOnMap()) {
			FixTile(MapFieldRocks, 1, pos);
			FixNeighbors(MapFieldRocks, 1, pos);

			//  Handle Walls changes.
		} else if (this->Tileset->isAWallTile(tile)
				   || this->Tileset->isAWallTile(seentile)) {
		//Wyrmgus end
			MapFixSeenWallTile(pos);
			MapFixSeenWallNeighbors(pos);
		}
	}
	*/
	//Wyrmgus end

#ifdef MINIMAP_UPDATE
	//Wyrmgus start
//	UI.Minimap.UpdateXY(pos);
	UI.Minimap.UpdateXY(pos, z);
	//Wyrmgus end
#endif
}

/**
**  Reveal the entire map.
*/
//Wyrmgus start
//void CMap::Reveal()
void CMap::Reveal(bool only_person_players)
//Wyrmgus end
{
	//  Mark every explored tile as visible. 1 turns into 2.
	//Wyrmgus start
	/*
	for (int i = 0; i != this->Info.MapWidth * this->Info.MapHeight; ++i) {
		CMapField &mf = *this->Field(i);
		CMapFieldPlayerInfo &playerInfo = mf.playerInfo;
		for (int p = 0; p < PlayerMax; ++p) {
			//Wyrmgus start
//			playerInfo.Visible[p] = std::max<unsigned short>(1, playerInfo.Visible[p]);
			if (Players[p].Type == PlayerPerson || !only_person_players) {
				playerInfo.Visible[p] = std::max<unsigned short>(1, playerInfo.Visible[p]);
			}
			//Wyrmgus end
		}
		MarkSeenTile(mf);
	}
	*/
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		for (int i = 0; i != this->Info.MapWidths[z] * this->Info.MapHeights[z]; ++i) {
			CMapField &mf = *this->Field(i, z);
			CMapFieldPlayerInfo &playerInfo = mf.playerInfo;
			for (int p = 0; p < PlayerMax; ++p) {
				if (CPlayer::Players[p]->Type == PlayerPerson || !only_person_players) {
					playerInfo.Visible[p] = std::max<unsigned short>(1, playerInfo.Visible[p]);
				}
			}
			MarkSeenTile(mf, z);
		}
	}
	//Wyrmgus end
	//  Global seen recount. Simple and effective.
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		//  Reveal neutral buildings. Gold mines:)
		if (unit.Player->Type == PlayerNeutral) {
			for (int p = 0; p < PlayerMax; ++p) {
				//Wyrmgus start
//				if (CPlayer::Players[p]->Type != PlayerNobody && (!(unit.Seen.ByPlayer & (1 << p)))) {
				if (CPlayer::Players[p]->Type != PlayerNobody && (CPlayer::Players[p]->Type == PlayerPerson || !only_person_players) && (!(unit.Seen.ByPlayer & (1 << p)))) {
				//Wyrmgus end
					UnitGoesOutOfFog(unit, *CPlayer::Players[p]);
					UnitGoesUnderFog(unit, *CPlayer::Players[p]);
				}
			}
		}
		UnitCountSeen(unit);
	}
}

/*----------------------------------------------------------------------------
--  Map queries
----------------------------------------------------------------------------*/

Vec2i CMap::map_pixel_pos_to_tile_pos(const PixelPos &mapPos) const
{
	const Vec2i tilePos(mapPos.x / stratagus::defines::get()->get_tile_width(), mapPos.y / stratagus::defines::get()->get_tile_height());

	return tilePos;
}

Vec2i CMap::scaled_map_pixel_pos_to_tile_pos(const PixelPos &mapPos) const
{
	return this->map_pixel_pos_to_tile_pos(mapPos / stratagus::defines::get()->get_scale_factor());
}

PixelPos CMap::tile_pos_to_map_pixel_pos_top_left(const Vec2i &tilePos) const
{
	PixelPos mapPixelPos(tilePos.x * stratagus::defines::get()->get_tile_width(), tilePos.y * stratagus::defines::get()->get_tile_height());

	return mapPixelPos;
}

PixelPos CMap::tile_pos_to_scaled_map_pixel_pos_top_left(const Vec2i &tilePos) const
{
	return this->tile_pos_to_map_pixel_pos_top_left(tilePos) * stratagus::defines::get()->get_scale_factor();
}

PixelPos CMap::tile_pos_to_map_pixel_pos_center(const Vec2i &tilePos) const
{
	return this->tile_pos_to_map_pixel_pos_top_left(tilePos) + stratagus::size::to_point(stratagus::defines::get()->get_tile_size()) / 2;
}

PixelPos CMap::tile_pos_to_scaled_map_pixel_pos_center(const Vec2i &tilePos) const
{
	return this->tile_pos_to_scaled_map_pixel_pos_top_left(tilePos) + stratagus::size::to_point(stratagus::defines::get()->get_scaled_tile_size()) / 2;
}

//Wyrmgus start
stratagus::terrain_type *CMap::GetTileTerrain(const Vec2i &pos, const bool overlay, const int z) const
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		return nullptr;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	return mf.GetTerrain(overlay);
}

stratagus::terrain_type *CMap::GetTileTopTerrain(const Vec2i &pos, const bool seen, const int z, const bool ignore_destroyed) const
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		return nullptr;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	return mf.GetTopTerrain();
}

int CMap::GetTileLandmass(const Vec2i &pos, int z) const
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		return 0;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	return mf.Landmass;
}

Vec2i CMap::GenerateUnitLocation(const stratagus::unit_type *unit_type, const stratagus::faction *faction, const Vec2i &min_pos, const Vec2i &max_pos, const int z) const
{
	if (SaveGameLoading) {
		return Vec2i(-1, -1);
	}
	
	CPlayer *player = GetFactionPlayer(faction);
	
	Vec2i random_pos(-1, -1);
	
	std::vector<stratagus::terrain_type *> allowed_terrains;
	if (unit_type->BoolFlag[FAUNA_INDEX].value && unit_type->get_species() != nullptr) { //if the unit is a fauna one, it has to start on terrain it is native to
		for (size_t i = 0; i < unit_type->get_species()->Terrains.size(); ++i) {
			allowed_terrains.push_back(unit_type->get_species()->Terrains[i]);
		}
	}
	
	for (size_t i = 0; i < unit_type->SpawnUnits.size(); ++i) {
		stratagus::unit_type *spawned_type = unit_type->SpawnUnits[i];
		if (spawned_type->BoolFlag[FAUNA_INDEX].value && spawned_type->get_species()) {
			for (size_t j = 0; j < spawned_type->get_species()->Terrains.size(); ++j) {
				allowed_terrains.push_back(spawned_type->get_species()->Terrains[j]);
			}
		}
	}

	std::vector<Vec2i> potential_positions;
	for (int x = min_pos.x; x <= (max_pos.x - (unit_type->get_tile_width() - 1)); ++x) {
		for (int y = min_pos.y; y <= (max_pos.y - (unit_type->get_tile_height() - 1)); ++y) {
			potential_positions.push_back(Vec2i(x, y));
		}
	}
	
	while (!potential_positions.empty()) {
		random_pos = potential_positions[SyncRand(potential_positions.size())];
		stratagus::vector::remove(potential_positions, random_pos);
		
		if (!this->Info.IsPointOnMap(random_pos, z) || (this->is_point_in_a_subtemplate_area(random_pos, z) && GameCycle == 0)) {
			continue;
		}
		
		if (allowed_terrains.size() > 0 && std::find(allowed_terrains.begin(), allowed_terrains.end(), GetTileTopTerrain(random_pos, false, z)) == allowed_terrains.end()) { //if the unit is a fauna one, it has to start on terrain it is native to
			continue;
		}
		
		std::vector<CUnit *> table;
		if (player != nullptr) {
			Select(random_pos - Vec2i(32, 32), random_pos + Vec2i(unit_type->get_tile_width() - 1, unit_type->get_tile_height() - 1) + Vec2i(32, 32), table, z, MakeAndPredicate(HasNotSamePlayerAs(*player), HasNotSamePlayerAs(*CPlayer::Players[PlayerNumNeutral])));
		} else if (!unit_type->GivesResource) {
			if (unit_type->BoolFlag[NEUTRAL_HOSTILE_INDEX].value || unit_type->BoolFlag[PREDATOR_INDEX].value || (unit_type->BoolFlag[PEOPLEAVERSION_INDEX].value && (unit_type->UnitType == UnitTypeType::Fly || unit_type->UnitType == UnitTypeType::Space))) {
				Select(random_pos - Vec2i(16, 16), random_pos + Vec2i(unit_type->get_tile_width() - 1, unit_type->get_tile_height() - 1) + Vec2i(16, 16), table, z, MakeOrPredicate(HasNotSamePlayerAs(*CPlayer::Players[PlayerNumNeutral]), HasSameTypeAs(*settlement_site_unit_type)));
			} else {
				Select(random_pos - Vec2i(8, 8), random_pos + Vec2i(unit_type->get_tile_width() - 1, unit_type->get_tile_height() - 1) + Vec2i(8, 8), table, z, HasNotSamePlayerAs(*CPlayer::Players[PlayerNumNeutral]));
			}
		} else if (unit_type->GivesResource && !unit_type->BoolFlag[BUILDING_INDEX].value) { //for non-building resources (i.e. wood piles), place them within a certain distance of player units, to prevent them from blocking the way
			Select(random_pos - Vec2i(4, 4), random_pos + Vec2i(unit_type->get_tile_width() - 1, unit_type->get_tile_height() - 1) + Vec2i(4, 4), table, z, HasNotSamePlayerAs(*CPlayer::Players[PlayerNumNeutral]));
		}
		
		if (!table.empty()) {
			continue;
		}

		bool passable_surroundings = true; //check if the unit won't be placed next to unpassable terrain
		for (int x = random_pos.x - 1; x < random_pos.x + unit_type->get_tile_width() + 1; ++x) {
			for (int y = random_pos.y - 1; y < random_pos.y + unit_type->get_tile_height() + 1; ++y) {
				if (Map.Info.IsPointOnMap(x, y, z) && Map.Field(x, y, z)->CheckMask(MapFieldUnpassable)) {
					passable_surroundings = false;
					break;
				}
			}
			if (!passable_surroundings) {
				break;
			}
		}
		if (!passable_surroundings) {
			continue;
		}

		if (UnitTypeCanBeAt(*unit_type, random_pos, z) && (!unit_type->BoolFlag[BUILDING_INDEX].value || CanBuildUnitType(nullptr, *unit_type, random_pos, 0, true, z))) {
			return random_pos;
		}
	}
	
	return Vec2i(-1, -1);
}
//Wyrmgus end

/**
**  Wall on map tile.
**
**  @param pos  map tile position.
**
**  @return    True if wall, false otherwise.
*/
bool CMap::WallOnMap(const Vec2i &pos, int z) const
{
	Assert(Map.Info.IsPointOnMap(pos, z));
	return Field(pos, z)->isAWall();
}

//Wyrmgus start
bool CMap::CurrentTerrainCanBeAt(const Vec2i &pos, bool overlay, int z)
{
	CMapField &mf = *this->Field(pos, z);
	stratagus::terrain_type *terrain = nullptr;
	
	if (overlay) {
		terrain = mf.OverlayTerrain;
	} else {
		terrain = mf.Terrain;
	}
	
	if (!terrain) {
		return true;
	}
	
	if (terrain->allows_single()) {
		return true;
	}

	std::vector<int> transition_directions;
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
						
					stratagus::terrain_type *adjacent_terrain = this->GetTileTerrain(adjacent_pos, overlay, z);
					if (overlay && adjacent_terrain && this->Field(adjacent_pos, z)->OverlayTerrainDestroyed) {
						adjacent_terrain = nullptr;
					}
					if (terrain != adjacent_terrain) { // also happens if terrain is null, so that i.e. tree transitions display correctly when adjacent to tiles without overlays
						transition_directions.push_back(GetDirectionFromOffset(x_offset, y_offset));
					}
				}
			}
		}
	}
	
	if (std::find(transition_directions.begin(), transition_directions.end(), North) != transition_directions.end() && std::find(transition_directions.begin(), transition_directions.end(), South) != transition_directions.end()) {
		return false;
	} else if (std::find(transition_directions.begin(), transition_directions.end(), West) != transition_directions.end() && std::find(transition_directions.begin(), transition_directions.end(), East) != transition_directions.end()) {
		return false;
	}

	return true;
}

bool CMap::TileBordersTerrain(const Vec2i &pos, const stratagus::terrain_type *terrain_type, const int z) const
{
	bool overlay = terrain_type != nullptr ? terrain_type->is_overlay() : false;

	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}

			if (this->GetTileTopTerrain(adjacent_pos, overlay, z) == terrain_type) {
				return true;
			}
		}
	}

	return false;
}

/**
**	@brief	Get whether a given tile borders only tiles with the same terrain as itself
**
**	@param	pos					The tile's position
**	@param	new_terrain_type	The potential new terrain type for the tile
**	@param	z					The tile's map layer
**
**	@return	True if the tile borders only tiles with the same terrain as itself, false otherwise
*/
bool CMap::TileBordersOnlySameTerrain(const Vec2i &pos, const stratagus::terrain_type *new_terrain_type, const int z) const
{
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			if (this->is_point_in_a_subtemplate_area(pos, z) && !this->is_point_in_a_subtemplate_area(adjacent_pos, z)) {
				continue;
			}
			stratagus::terrain_type *top_terrain = GetTileTopTerrain(pos, false, z);
			stratagus::terrain_type *adjacent_top_terrain = GetTileTopTerrain(adjacent_pos, false, z);
			if (!new_terrain_type->is_overlay()) {
				if (
					adjacent_top_terrain
					&& adjacent_top_terrain != top_terrain
					&& (!stratagus::vector::contains(top_terrain->get_inner_border_terrain_types(), adjacent_top_terrain) || !stratagus::vector::contains(new_terrain_type->get_inner_border_terrain_types(), adjacent_top_terrain))
					&& adjacent_top_terrain != new_terrain_type
				) {
					return false;
				}
			} else {
				if (
					adjacent_top_terrain
					&& adjacent_top_terrain != top_terrain
					&& !stratagus::vector::contains(top_terrain->get_base_terrain_types(), adjacent_top_terrain) && !stratagus::vector::contains(adjacent_top_terrain->get_base_terrain_types(), top_terrain)
					&& adjacent_top_terrain != new_terrain_type
				) {
					return false;
				}
			}
		}
	}
		
	return true;
}

bool CMap::TileBordersFlag(const Vec2i &pos, int z, int flag, bool reverse) const
{
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			CMapField &mf = *Map.Field(adjacent_pos, z);
			
			if ((!reverse && mf.CheckMask(flag)) || (reverse && !mf.CheckMask(flag))) {
				return true;
			}
		}
	}
		
	return false;
}

bool CMap::tile_borders_other_settlement_territory(const QPoint &pos, const int z) const
{
	stratagus::site *tile_settlement = this->Field(pos, z)->get_settlement();

	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			const QPoint adjacent_pos(pos.x() + sub_x, pos.y() + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}

			stratagus::site *adjacent_tile_settlement = this->Field(adjacent_pos, z)->get_settlement();
			if (tile_settlement != adjacent_tile_settlement) {
				return true;
			}
		}
	}

	return false;
}

bool CMap::tile_borders_other_player_territory(const QPoint &pos, const int z, const int range) const
{
	const CPlayer *tile_owner = this->Field(pos, z)->get_owner();

	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			const QPoint adjacent_pos(pos.x() + sub_x, pos.y() + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}

			const CPlayer *adjacent_tile_owner = this->Field(adjacent_pos, z)->get_owner();
			if (tile_owner != adjacent_tile_owner) {
				return true;
			}

			if (range >= 1 && this->tile_borders_other_player_territory(adjacent_pos, z, range - 1)) {
				return true;
			}
		}
	}

	return false;
}

bool CMap::TileBordersBuilding(const Vec2i &pos, int z)
{
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			CMapField &mf = *Map.Field(adjacent_pos, z);
			
			if (mf.CheckMask(MapFieldBuilding)) {
				return true;
			}
		}
	}
		
	return false;
}

bool CMap::TileBordersPathway(const Vec2i &pos, int z, bool only_railroad)
{
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			CMapField &mf = *Map.Field(adjacent_pos, z);
			
			if (
				(!only_railroad && mf.CheckMask(MapFieldRoad))
				|| mf.CheckMask(MapFieldRailroad)
			) {
				return true;
			}
		}
	}
		
	return false;
}

bool CMap::TileBordersUnit(const Vec2i &pos, int z)
{
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			CMapField &mf = *Map.Field(adjacent_pos, z);
			
			const CUnitCache &cache = mf.UnitCache;
			for (size_t i = 0; i != cache.size(); ++i) {
				CUnit &unit = *cache[i];
				if (unit.IsAliveOnMap()) {
					return true;
				}
			}
		}
	}
		
	return false;
}

/**
**	@brief	Get whether the given tile has any bordering terrains which are incompatible with a given terrain type
**
**	@param	pos					The tile's position
**	@param	new_terrain_type	The terrain type to check
**	@param	z					The tile's map layer
**
**	@return	True if the tile borders only tiles with the same terrain as itself, false otherwise
*/
bool CMap::TileBordersTerrainIncompatibleWithTerrain(const Vec2i &pos, const stratagus::terrain_type *terrain_type, const int z) const
{
	if (!terrain_type || !terrain_type->is_overlay()) {
		return false;
	}
	
	stratagus::terrain_type *tile_terrain = this->GetTileTerrain(pos, false, z);
	
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			
			stratagus::terrain_type *adjacent_terrain = this->GetTileTerrain(adjacent_pos, false, z);
			
			if (adjacent_terrain == nullptr) {
				continue;
			}

			if (tile_terrain == adjacent_terrain) {
				continue;
			}
			
			if (terrain_type->is_overlay()) {
				if ( //if the terrain type is an overlay one, the adjacent tile terrain is incompatible with it if it both cannot be a base terrain for the overlay terrain type, and it "expands into" the tile (that is, the tile has the adjacent terrain as an inner border terrain)
					stratagus::vector::contains(tile_terrain->get_inner_border_terrain_types(), adjacent_terrain)
					&& !stratagus::vector::contains(terrain_type->get_base_terrain_types(), adjacent_terrain)
				) {
					return true;
				}
			} else {
				//if the terrain type is not an overlay one, the adjacent tile terrain is incompatible with it if it cannot border the terrain type
				if (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), adjacent_terrain) == terrain_type->BorderTerrains.end()) {
					return true;
				}
			}
		}
	}
		
	return false;
}

bool CMap::TileBordersTerrainIncompatibleWithTerrainPair(const Vec2i &pos, const stratagus::terrain_type *terrain_type, const stratagus::terrain_type *overlay_terrain_type, const int z) const
{
	if (!terrain_type) {
		return false;
	}

	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);

			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}

			stratagus::terrain_type *adjacent_terrain = this->GetTileTerrain(adjacent_pos, false, z);

			if (adjacent_terrain == nullptr) {
				continue;
			}

			if (terrain_type == adjacent_terrain) {
				continue;
			}

			//the adjacent tile terrain is incompatible with the non-overlay terrain type if it cannot border the terrain type
			if (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), adjacent_terrain) == terrain_type->BorderTerrains.end()) {
				return true;
			}

			if (overlay_terrain_type != nullptr) {
				if ( //if the terrain type is an overlay one, the adjacent tile terrain is incompatible with it if it both cannot be a base terrain for the overlay terrain type, and it "expands into" the tile (that is, the tile has the adjacent terrain as an inner border terrain)
					stratagus::vector::contains(terrain_type->get_inner_border_terrain_types(), adjacent_terrain)
					&& !stratagus::vector::contains(overlay_terrain_type->get_base_terrain_types(), adjacent_terrain)
					) {
					return true;
				}
			}
		}
	}

	return false;
}

/**
**	@brief	Get whether a tile has units that are incompatible with a given terrain type
**
**	@param	pos				The tile's position
**	@param	terrain_type	The terrain type
**	@param	z				The tile's map layer
**
**	@return	Whether the tile has units that are incompatible with the given terrain type
*/
bool CMap::TileHasUnitsIncompatibleWithTerrain(const Vec2i &pos, const stratagus::terrain_type *terrain_type, const int z)
{
	CMapField &mf = *Map.Field(pos, z);
	
	const CUnitCache &cache = mf.UnitCache;
	for (size_t i = 0; i != cache.size(); ++i) {
		const CUnit &unit = *cache[i];
		if (unit.IsAliveOnMap() && (terrain_type->Flags & unit.Type->MovementMask) != 0) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether a given tile is in a subtemplate area
**
**	@param	pos				The tile's position
**	@param	z				The tile's map layer
**	@param	subtemplate		Optional subtemplate argument, if not null then will only return true if the point is in that specific subtemplate area; if it is null, then true will be returned if the point is in any subtemplate area
**
**	@return	True if the tile is in a subtemplate area, or false otherwise
*/
bool CMap::is_point_in_a_subtemplate_area(const Vec2i &pos, const int z, const stratagus::map_template *subtemplate) const
{
	for (size_t i = 0; i < this->MapLayers[z]->subtemplate_areas.size(); ++i) {
		if (subtemplate && subtemplate != std::get<2>(this->MapLayers[z]->subtemplate_areas[i])) {
			continue;
		}
		
		Vec2i min_pos = std::get<0>(this->MapLayers[z]->subtemplate_areas[i]);
		Vec2i max_pos = std::get<1>(this->MapLayers[z]->subtemplate_areas[i]);
		if (pos.x >= min_pos.x && pos.y >= min_pos.y && pos.x <= max_pos.x && pos.y <= max_pos.y) {
			return true;
		}
	}

	return false;
}

bool CMap::is_subtemplate_on_map(const stratagus::map_template *subtemplate) const
{
	const QPoint subtemplate_pos = this->get_subtemplate_pos(subtemplate);
	return subtemplate_pos.x() != -1 && subtemplate_pos.y() != -1;
}

std::pair<Vec2i, Vec2i> CMap::get_subtemplate_rect(const stratagus::map_template *subtemplate) const
{
	if (!subtemplate) {
		return std::make_pair(Vec2i(-1, -1), Vec2i(-1, -1));
	}

	const stratagus::map_template *main_template = subtemplate->GetTopMapTemplate();
	if (main_template && subtemplate != main_template) {
		const int z = GetMapLayer(main_template->get_plane() ? main_template->get_plane()->Ident : "", main_template->get_world() ? main_template->get_world()->get_identifier() : "");
		if (z != -1) {
			for (size_t i = 0; i < this->MapLayers[z]->subtemplate_areas.size(); ++i) {
				if (subtemplate == std::get<2>(this->MapLayers[z]->subtemplate_areas[i])) {
					return std::make_pair(std::get<0>(CMap::Map.MapLayers[z]->subtemplate_areas[i]), std::get<1>(CMap::Map.MapLayers[z]->subtemplate_areas[i]));
				}
			}
		}
	}

	return std::make_pair(Vec2i(-1, -1), Vec2i(-1, -1));
}

/**
**	@brief	Get the applied map position of a given subtemplate
**
**	@param	subtemplate		The subtemplate
**
**	@return	The subtemplate's position if found, or (-1, -1) otherwise
*/
Vec2i CMap::get_subtemplate_pos(const stratagus::map_template *subtemplate) const
{
	std::pair<Vec2i, Vec2i> subtemplate_rect = this->get_subtemplate_rect(subtemplate);
	return subtemplate_rect.first;
}

Vec2i CMap::get_subtemplate_center_pos(const stratagus::map_template *subtemplate) const
{
	std::pair<Vec2i, Vec2i> subtemplate_rect = this->get_subtemplate_rect(subtemplate);

	const Vec2i &start_pos = subtemplate_rect.first;
	const Vec2i &end_pos = subtemplate_rect.second;

	return start_pos + ((end_pos - start_pos) / 2);
}

/**
**	@brief	Get the applied end map position of a given subtemplate
**
**	@param	subtemplate		The subtemplate
**
**	@return	The subtemplate's end position if found, or (-1, -1) otherwise
*/
Vec2i CMap::get_subtemplate_end_pos(const stratagus::map_template *subtemplate) const
{
	std::pair<Vec2i, Vec2i> subtemplate_rect = this->get_subtemplate_rect(subtemplate);
	return subtemplate_rect.second;
}

/**
**	@brief	Get the applied map layer of a given subtemplate
**
**	@param	subtemplate		The subtemplate
**
**	@return	The subtemplate's map layer if found, or null otherwise
*/
CMapLayer *CMap::get_subtemplate_map_layer(const stratagus::map_template *subtemplate) const
{
	if (!subtemplate) {
		return nullptr;
	}
	
	const stratagus::map_template *main_template = subtemplate->GetTopMapTemplate();
	if (main_template && subtemplate != main_template) {
		const int z = GetMapLayer(main_template->get_plane() ? main_template->get_plane()->Ident : "", main_template->get_world() ? main_template->get_world()->get_identifier() : "");
		if (z != -1) {
			for (size_t i = 0; i < this->MapLayers[z]->subtemplate_areas.size(); ++i) {
				if (subtemplate == std::get<2>(this->MapLayers[z]->subtemplate_areas[i])) {
					return this->MapLayers[z];
				}
			}
		}
	}
	
	return nullptr;
}

/**
**	@brief	Get the map layer connectors in a given map template
**
**	@param	subtemplate		The subtemplate
**
**	@return	A list of the connector units
*/
std::vector<CUnit *> CMap::get_map_template_layer_connectors(const stratagus::map_template *map_template) const
{
	std::vector<CUnit *> layer_connectors;
	
	if (!map_template) {
		return layer_connectors;
	}
	
	const stratagus::map_template *main_template = map_template->GetTopMapTemplate();
	if (main_template) {
		const bool is_main_template = main_template == map_template;
		const int z = GetMapLayer(main_template->get_plane() ? main_template->get_plane()->Ident : "", main_template->get_world() ? main_template->get_world()->get_identifier() : "");
		if (z != -1) {
			for (size_t i = 0; i < this->MapLayers[z]->LayerConnectors.size(); ++i) {
				CUnit *connector_unit = this->MapLayers[z]->LayerConnectors[i];
				const Vec2i unit_pos = connector_unit->get_center_tile_pos();
				
				if (is_main_template && this->is_point_in_a_subtemplate_area(unit_pos, z)) {
					continue;
				} else if (!is_main_template && !this->is_point_in_a_subtemplate_area(unit_pos, z, map_template)) {
					continue;
				}

				layer_connectors.push_back(connector_unit);
			}
		}
	}
	
	return layer_connectors;
}

/**
**	@brief	Get whether a given tile is adjacent to non-subtemplate area tiles
**
**	@param	pos		The tile's position
**	@param	z		The tile's map layer
**
**	@return	True if the tile is adjacent to a non-subtemplate area tile, or false otherwise
*/
bool CMap::is_point_adjacent_to_non_subtemplate_area(const Vec2i &pos, const int z) const
{
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset == 0 && y_offset == 0) {
				continue;
			}
			
			Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
			
			if (Map.Info.IsPointOnMap(adjacent_pos, z) && !this->is_point_in_a_subtemplate_area(adjacent_pos, z)) {
				return true;
			}
		}
	}
	
	return false;
}

bool CMap::is_rect_in_settlement(const QRect &rect, const int z, const stratagus::site *settlement)
{
	for (int x = rect.x(); x <= rect.right(); ++x) {
		for (int y = rect.y(); y <= rect.bottom(); ++y) {
			const QPoint tile_pos(x, y);

			if (!Map.Info.IsPointOnMap(tile_pos, z)) {
				return false;
			}

			const CMapField *tile = this->Field(tile_pos, z);

			//doesn't return false for tiles with no settlement
			if (tile->get_settlement() == nullptr) {
				continue;
			}

			if (tile->get_settlement() != settlement) {
				return false;
			}
		}
	}

	return true;
}

void CMap::SetCurrentPlane(stratagus::plane *plane)
{
	if (UI.CurrentMapLayer->plane == plane) {
		return;
	}
	
	int map_layer = -1;
	
	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		if (Map.MapLayers[z]->plane == plane) {
			map_layer = z;
			break;
		}
	}
	
	if (map_layer == -1) {
		for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
			if (Map.MapLayers[z]->plane == plane) {
				map_layer = z;
				break;
			}
		}
	}
	
	if (map_layer != -1) {
		ChangeCurrentMapLayer(map_layer);
	}
}

void CMap::SetCurrentWorld(stratagus::world *world)
{
	if (UI.CurrentMapLayer->world == world) {
		return;
	}
	
	int map_layer = -1;
	
	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		if (Map.MapLayers[z]->world == world) {
			map_layer = z;
			break;
		}
	}
	
	if (map_layer == -1) {
		for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
			if (Map.MapLayers[z]->world == world) {
				map_layer = z;
				break;
			}
		}
	}
	
	if (map_layer != -1) {
		ChangeCurrentMapLayer(map_layer);
	}
}

stratagus::plane *CMap::GetCurrentPlane() const
{
	if (UI.CurrentMapLayer) {
		return UI.CurrentMapLayer->plane;
	} else {
		return nullptr;
	}
}

stratagus::world *CMap::GetCurrentWorld() const
{
	if (UI.CurrentMapLayer) {
		return UI.CurrentMapLayer->world;
	} else {
		return nullptr;
	}
}
//Wyrmgus end

/**
**  Can move to this point, applying mask.
**
**  @param pos   map tile position.
**  @param mask  Mask for movement to apply.
**
**  @return      True if could be entered, false otherwise.
*/
bool CheckedCanMoveToMask(const Vec2i &pos, int mask, int z)
{
	return CMap::Map.Info.IsPointOnMap(pos, z) && CanMoveToMask(pos, mask, z);
}

/**
**  Can a unit of unit-type be placed at this point.
**
**  @param type  unit-type to be checked.
**  @param pos   map tile position.
**
**  @return      True if could be entered, false otherwise.
*/
bool UnitTypeCanBeAt(const stratagus::unit_type &type, const Vec2i &pos, int z)
{
	const int mask = type.MovementMask;
	unsigned int index = pos.y * CMap::Map.Info.MapWidths[z];

	for (int addy = 0; addy < type.get_tile_height(); ++addy) {
		for (int addx = 0; addx < type.get_tile_width(); ++addx) {
			if (CMap::Map.Info.IsPointOnMap(pos.x + addx, pos.y + addy, z) == false) {
				return false;
			}

			const CMapField *tile = CMap::Map.Field(pos.x + addx + index, z);
			if (tile->CheckMask(mask) == true || (tile->Terrain == nullptr && stratagus::game::get()->get_current_campaign() != nullptr)) {
				return false;
			}
			
		}
		index += CMap::Map.Info.MapWidths[z];
	}
	return true;
}

/**
**  Can a unit be placed to this point.
**
**  @param unit  unit to be checked.
**  @param pos   map tile position.
**
**  @return      True if could be placeded, false otherwise.
*/
//Wyrmgus start
//bool UnitCanBeAt(const CUnit &unit, const Vec2i &pos)
bool UnitCanBeAt(const CUnit &unit, const Vec2i &pos, int z)
//Wyrmgus end
{
	Assert(unit.Type);
	if (unit.Type->BoolFlag[NONSOLID_INDEX].value) {
		return true;
	}
	//Wyrmgus start
//	return UnitTypeCanBeAt(*unit.Type, pos);
	return UnitTypeCanBeAt(*unit.Type, pos, z);
	//Wyrmgus end
}

/**
**  Fixes initially the wood and seen tiles.
*/
void PreprocessMap()
{
	ShowLoadProgress("%s", _("Initializing Map"));
		
	//Wyrmgus start
	/*
	for (int ix = 0; ix < Map.Info.MapWidth; ++ix) {
		for (int iy = 0; iy < Map.Info.MapHeight; ++iy) {
			CMapField &mf = *Map.Field(ix, iy);
			mf.playerInfo.SeenTile = mf.getGraphicTile();
		}
	}
	*/

	for (size_t z = 0; z < CMap::Map.MapLayers.size(); ++z) {
		for (int ix = 0; ix < CMap::Map.Info.MapWidths[z]; ++ix) {
			for (int iy = 0; iy < CMap::Map.Info.MapHeights[z]; ++iy) {
				const QPoint tile_pos(ix, iy);
				CMapField &mf = *CMap::Map.Field(tile_pos, z);
				CMap::Map.calculate_tile_solid_tile(tile_pos, false, z);
				if (mf.OverlayTerrain != nullptr) {
					CMap::Map.calculate_tile_solid_tile(tile_pos, true, z);
				}
				CMap::Map.CalculateTileTransitions(tile_pos, false, z);
				CMap::Map.CalculateTileTransitions(tile_pos, true, z);
			}
		}

		//settlement territories need to be generated after tile transitions are calculated, so that the coast map field has been set
		CMap::Map.generate_settlement_territories(z);

		for (int ix = 0; ix < CMap::Map.Info.MapWidths[z]; ++ix) {
			for (int iy = 0; iy < CMap::Map.Info.MapHeights[z]; ++iy) {
				const QPoint tile_pos(ix, iy);
				CMapField &mf = *CMap::Map.Field(tile_pos, z);
				CMap::Map.CalculateTileLandmass(tile_pos, z);
				CMap::Map.CalculateTileOwnershipTransition(tile_pos, z);
				CMap::Map.calculate_tile_terrain_feature(tile_pos, z);
				mf.UpdateSeenTile();
				UI.Minimap.UpdateXY(tile_pos, z);
				UI.Minimap.update_territory_xy(tile_pos, z);
				if (mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
					CMap::Map.MarkSeenTile(mf, z);
				}
			}
		}
	}
	//Wyrmgus end
	//Wyrmgus start
	/*
	// it is required for fixing the wood that all tiles are marked as seen!
	if (Map.Tileset->TileTypeTable.empty() == false) {
		Vec2i pos;
		for (pos.x = 0; pos.x < Map.Info.MapWidth; ++pos.x) {
			for (pos.y = 0; pos.y < Map.Info.MapHeight; ++pos.y) {
				MapFixWallTile(pos);
				MapFixSeenWallTile(pos);
			}
		}
	}
	*/
	//Wyrmgus end
}

//Wyrmgus start
int GetMapLayer(const std::string &plane_ident, const std::string &world_ident)
{
	stratagus::plane *plane = stratagus::plane::try_get(plane_ident);
	stratagus::world *world = stratagus::world::try_get(world_ident);

	for (size_t z = 0; z < CMap::Map.MapLayers.size(); ++z) {
		if (CMap::Map.MapLayers[z]->plane == plane && CMap::Map.MapLayers[z]->world == world) {
			return z;
		}
	}
	
	return -1;
}

/**
**	@brief	Change the map layer currently being displayed to the previous one
*/
void ChangeToPreviousMapLayer()
{
	if (!UI.PreviousMapLayer) {
		return;
	}
	
	ChangeCurrentMapLayer(UI.PreviousMapLayer->ID);
}

/**
**	@brief	Change the map layer currently being displayed
**
**	@param	z	The map layer
*/
void ChangeCurrentMapLayer(const int z)
{
	if (z < 0 || z >= static_cast<int>(CMap::Map.MapLayers.size()) || UI.CurrentMapLayer->ID == z) {
		return;
	}
	
	Vec2i new_viewport_map_pos(UI.SelectedViewport->MapPos.x * CMap::Map.Info.MapWidths[z] / UI.CurrentMapLayer->get_width(), UI.SelectedViewport->MapPos.y * CMap::Map.Info.MapHeights[z] / UI.CurrentMapLayer->get_height());
	
	UI.PreviousMapLayer = UI.CurrentMapLayer;
	UI.CurrentMapLayer = CMap::Map.MapLayers[z];
	UI.Minimap.UpdateCache = true;
	UI.SelectedViewport->Set(new_viewport_map_pos, stratagus::size::to_point(stratagus::defines::get()->get_scaled_tile_size()) / 2);
}

/**
**	@brief	Set the current time of day for a particular map layer
**
**	@param	time_of_day_ident	The time of day's string identifier
**	@param	z					The map layer
*/
void SetTimeOfDay(const std::string &time_of_day_ident, int z)
{
	if (time_of_day_ident.empty()) {
		CMap::Map.MapLayers[z]->SetTimeOfDay(nullptr);
		CMap::Map.MapLayers[z]->RemainingTimeOfDayHours = 0;
	} else {
		CTimeOfDaySchedule *schedule = CMap::Map.MapLayers[z]->TimeOfDaySchedule;
		if (schedule) {
			for (size_t i = 0; i < schedule->ScheduledTimesOfDay.size(); ++i) {
				CScheduledTimeOfDay *time_of_day = schedule->ScheduledTimesOfDay[i];
				if (time_of_day->TimeOfDay->get_identifier() == time_of_day_ident)  {
					CMap::Map.MapLayers[z]->SetTimeOfDay(time_of_day);
					CMap::Map.MapLayers[z]->RemainingTimeOfDayHours = time_of_day->GetHours(CMap::Map.MapLayers[z]->GetSeason());
					break;
				}
			}
		}
	}
}

/**
**	@brief	Set the time of day schedule for a particular map layer
**
**	@param	time_of_day_schedule_ident	The time of day schedule's string identifier
**	@param	z							The map layer
*/
void SetTimeOfDaySchedule(const std::string &time_of_day_schedule_ident, const int z)
{
	if (z >= static_cast<int>(CMap::Map.MapLayers.size())) {
		fprintf(stderr, "Error in CMap::SetTimeOfDaySchedule: the given map layer index (%d) is not valid given the map layer quantity (%d).\n", z, CMap::Map.MapLayers.size());
		return;
	}

	if (time_of_day_schedule_ident.empty()) {
		CMap::Map.MapLayers[z]->TimeOfDaySchedule = nullptr;
		CMap::Map.MapLayers[z]->SetTimeOfDay(nullptr);
		CMap::Map.MapLayers[z]->RemainingTimeOfDayHours = 0;
	} else {
		CTimeOfDaySchedule *schedule = CTimeOfDaySchedule::GetTimeOfDaySchedule(time_of_day_schedule_ident);
		if (schedule) {
			CMap::Map.MapLayers[z]->TimeOfDaySchedule = schedule;
			CMap::Map.MapLayers[z]->SetTimeOfDay(schedule->ScheduledTimesOfDay.front());
			CMap::Map.MapLayers[z]->RemainingTimeOfDayHours = CMap::Map.MapLayers[z]->TimeOfDay->GetHours(CMap::Map.MapLayers[z]->GetSeason());
		}
	}
}

/**
**	@brief	Set the current season for a particular map layer
**
**	@param	season_ident		The season's string identifier
**	@param	z					The map layer
*/
void SetSeason(const std::string &season_ident, int z)
{
	if (season_ident.empty()) {
		CMap::Map.MapLayers[z]->SetSeason(nullptr);
		CMap::Map.MapLayers[z]->RemainingSeasonHours = 0;
	} else {
		CSeasonSchedule *schedule = CMap::Map.MapLayers[z]->SeasonSchedule;
		if (schedule) {
			for (size_t i = 0; i < schedule->ScheduledSeasons.size(); ++i) {
				CScheduledSeason *season = schedule->ScheduledSeasons[i];
				if (season->Season->get_identifier() == season_ident)  {
					CMap::Map.MapLayers[z]->SetSeason(season);
					CMap::Map.MapLayers[z]->RemainingSeasonHours = season->Hours;
					break;
				}
			}
		}
	}
}

/**
**	@brief	Set the season schedule for a particular map layer
**
**	@param	season_schedule_ident		The season schedule's string identifier
**	@param	z							The map layer
*/
void SetSeasonSchedule(const std::string &season_schedule_ident, int z)
{
	if (season_schedule_ident.empty()) {
		CMap::Map.MapLayers[z]->SeasonSchedule = nullptr;
		CMap::Map.MapLayers[z]->SetSeason(nullptr);
		CMap::Map.MapLayers[z]->RemainingSeasonHours = 0;
	} else {
		CSeasonSchedule *schedule = CSeasonSchedule::GetSeasonSchedule(season_schedule_ident);
		if (schedule) {
			CMap::Map.MapLayers[z]->SeasonSchedule = schedule;
			CMap::Map.MapLayers[z]->SetSeason(schedule->ScheduledSeasons.front());
			CMap::Map.MapLayers[z]->RemainingSeasonHours = CMap::Map.MapLayers[z]->Season->Hours;
		}
	}
}
//Wyrmgus end

/**
**	@brief	Get whether a given coordinate is a valid point on the map
**
**	@param	x	The x coordinate
**	@param	y	The y coordinate
**	@param	z	The map layer
**
**	@return	True if the coordinate is valid, false otherwise
*/
bool CMapInfo::IsPointOnMap(const int x, const int y, const int z) const
{
	return (z >= 0 && z < (int) MapWidths.size() && z < (int) MapHeights.size() && x >= 0 && y >= 0 && x < MapWidths[z] && y < MapHeights[z]);
}

/**
**	@brief	Get whether a given coordinate is a valid point on the map
**
**	@param	pos	The coordinate position
**	@param	z	The map layer
**
**	@return	True if the coordinate is valid, false otherwise
*/
bool CMapInfo::IsPointOnMap(const Vec2i &pos, const int z) const
{
	return IsPointOnMap(pos.x, pos.y, z);
}

/**
**	@brief	Get whether a given coordinate is a valid point on the map
**
**	@param	x			The x coordinate
**	@param	y			The y coordinate
**	@param	map_layer	The map layer
**
**	@return	True if the coordinate is valid, false otherwise
*/
bool CMapInfo::IsPointOnMap(const int x, const int y, const CMapLayer *map_layer) const
{
	return (map_layer && x >= 0 && y >= 0 && x < map_layer->get_width() && y < map_layer->get_height());
}

/**
**	@brief	Get whether a given coordinate is a valid point on the map
**
**	@param	pos			The coordinate position
**	@param	map_layer	The map layer
**
**	@return	True if the coordinate is valid, false otherwise
*/
bool CMapInfo::IsPointOnMap(const Vec2i &pos, const CMapLayer *map_layer) const
{
	return IsPointOnMap(pos.x, pos.y, map_layer);
}

/**
**	@brief	Clear CMapInfo
*/
void CMapInfo::Clear()
{
	this->Description.clear();
	this->Filename.clear();
	this->MapWidth = this->MapHeight = 0;
	//Wyrmgus start
	this->MapWidths.clear();
	this->MapHeights.clear();
	//Wyrmgus end
	memset(this->PlayerSide, 0, sizeof(this->PlayerSide));
	memset(this->PlayerType, 0, sizeof(this->PlayerType));
	this->MapUID = 0;
}

CMap::CMap() : NoFogOfWar(false), TileGraphic(nullptr), Landmasses(0)
{
	Tileset = new CTileset;
}

CMap::~CMap()
{
	delete Tileset;
}

unsigned int CMap::getIndex(int x, int y, int z) const
{
	return x + y * this->Info.MapWidths[z];
}

unsigned int CMap::getIndex(const Vec2i &pos, int z) const
{
	return getIndex(pos.x, pos.y, z);
}

/**
**	@brief	Get the map field at a given location
**
**	@param	index	The index of the map field
**	@param	z		The map layer of the map field
**
**	@return	The map field
*/
CMapField *CMap::Field(const unsigned int index, const int z) const
{
	return this->MapLayers[z]->Field(index);
}

/**
**	@brief	Get the map field at a given location
**
**	@param	x	The x coordinate of the map field
**	@param	y	The y coordinate of the map field
**	@param	z	The map layer of the map field
**
**	@return	The map field
*/
CMapField *CMap::Field(const int x, const int y, const int z) const
{
	return this->MapLayers[z]->Field(x, y);
}

/**
**	@brief	Allocate and initialize map table
*/
void CMap::Create()
{
	Assert(this->MapLayers.size() == 0);

	CMapLayer *map_layer = new CMapLayer(this->Info.MapWidth, this->Info.MapHeight);
	map_layer->ID = this->MapLayers.size();
	this->MapLayers.push_back(map_layer);
	this->Info.MapWidths.push_back(this->Info.MapWidth);
	this->Info.MapHeights.push_back(this->Info.MapHeight);
	
	if (Editor.Running == EditorNotRunning) {
		map_layer->SeasonSchedule = CSeasonSchedule::DefaultSeasonSchedule;
		map_layer->SetSeasonByHours(CDate::CurrentTotalHours);
		
		if (!GameSettings.Inside && !GameSettings.NoTimeOfDay) {
			map_layer->TimeOfDaySchedule = CTimeOfDaySchedule::DefaultTimeOfDaySchedule;
			map_layer->SetTimeOfDayByHours(CDate::CurrentTotalHours);
		} else {
			map_layer->TimeOfDaySchedule = nullptr;
			map_layer->SetTimeOfDay(nullptr); // make indoors have no time of day setting until it is possible to make light sources change their surrounding "time of day" // indoors it is always dark (maybe would be better to allow a special setting to have bright indoor places?
		}
	}
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::Init()
{
	this->InitFogOfWar();
}

/**
**  Cleanup the map module.
*/
void CMap::Clean()
{
	UI.CurrentMapLayer = nullptr;
	UI.PreviousMapLayer = nullptr;
	this->Landmasses = 0;

	//Wyrmgus start
	this->ClearMapLayers();
	this->BorderLandmasses.clear();
	this->site_units.clear();
	//Wyrmgus end

	// Tileset freed by Tileset?

	this->Info.Clear();
	this->NoFogOfWar = false;
	this->Tileset->clear();
	this->TileModelsFileName.clear();
	CGraphic::Free(this->TileGraphic);
	this->TileGraphic = nullptr;

	FlagRevealMap = 0;
	ReplayRevealMap = 0;

	UI.Minimap.Destroy();
}

void CMap::ClearMapLayers()
{
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		delete this->MapLayers[z];
	}
	this->MapLayers.clear();
}

/**
** Save the complete map.
**
** @param file Output file.
*/
void CMap::Save(CFile &file) const
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: map\n");
	file.printf("LoadTileModels(\"%s\")\n\n", this->TileModelsFileName.c_str());
	file.printf("StratagusMap(\n");
	file.printf("  \"version\", \"%s\",\n", VERSION);
	file.printf("  \"description\", \"%s\",\n", this->Info.Description.c_str());
	file.printf("  \"the-map\", {\n");
	file.printf("  \"size\", {%d, %d},\n", this->Info.MapWidth, this->Info.MapHeight);
	file.printf("  \"%s\",\n", this->NoFogOfWar ? "no-fog-of-war" : "fog-of-war");
	file.printf("  \"filename\", \"%s\",\n", this->Info.Filename.c_str());
	//Wyrmgus start
	file.printf("  \"extra-map-layers\", {\n");
	for (size_t z = 1; z < this->MapLayers.size(); ++z) {
		file.printf("  {%d, %d},\n", this->Info.MapWidths[z], this->Info.MapHeights[z]);
	}
	file.printf("  },\n");
	file.printf("  \"time-of-day\", {\n");
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {\"%s\", %d, %d},\n", this->MapLayers[z]->TimeOfDaySchedule ? this->MapLayers[z]->TimeOfDaySchedule->Ident.c_str() : "", this->MapLayers[z]->TimeOfDay ? this->MapLayers[z]->TimeOfDay->ID : 0, this->MapLayers[z]->RemainingTimeOfDayHours);
	}
	file.printf("  },\n");
	file.printf("  \"season\", {\n");
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {\"%s\", %d, %d},\n", this->MapLayers[z]->SeasonSchedule ? this->MapLayers[z]->SeasonSchedule->Ident.c_str() : "", this->MapLayers[z]->Season ? this->MapLayers[z]->Season->ID : 0, this->MapLayers[z]->RemainingSeasonHours);
	}
	file.printf("  },\n");
	file.printf("  \"layer-references\", {\n");
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {\"%s\", \"%s\"},\n", this->MapLayers[z]->plane ? this->MapLayers[z]->plane->Ident.c_str() : "", this->MapLayers[z]->world ? this->MapLayers[z]->world->Ident.c_str() : "");
	}
	file.printf("  },\n");
	file.printf("  \"landmasses\", {\n");
	for (int i = 1; i <= this->Landmasses; ++i) {
		file.printf("  {");
		for (size_t j = 0; j < this->BorderLandmasses[i].size(); ++j) {
			file.printf("%d, ", this->BorderLandmasses[i][j]);
		}
		file.printf("},\n");
	}
	file.printf("  },\n");
	//Wyrmgus end

	file.printf("  \"map-fields\", {\n");
	//Wyrmgus start
	/*
	for (int h = 0; h < this->Info.MapHeight; ++h) {
		file.printf("  -- %d\n", h);
		for (int w = 0; w < this->Info.MapWidth; ++w) {
			const CMapField &mf = *this->Field(w, h);

			mf.Save(file);
			if (w & 1) {
				file.printf(",\n");
			} else {
				file.printf(", ");
			}
		}
	}
	*/
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {\n");
		for (int h = 0; h < this->Info.MapHeights[z]; ++h) {
			file.printf("  -- %d\n", h);
			for (int w = 0; w < this->Info.MapWidths[z]; ++w) {
				const CMapField &mf = *this->Field(w, h, z);

				mf.Save(file);
				if (w & 1) {
					file.printf(",\n");
				} else {
					file.printf(", ");
				}
			}
		}
		file.printf("  },\n");
	}
	//Wyrmgus end
	file.printf("}})\n");
}

/*----------------------------------------------------------------------------
-- Map Tile Update Functions
----------------------------------------------------------------------------*/

/**
**  Correct the seen wood field, depending on the surrounding.
**
**  @param type  type of tile to update
**  @param seen  1 if updating seen value, 0 for real
**  @param pos   Map tile-position.
*/
//Wyrmgus start
/*
void CMap::FixTile(unsigned short type, int seen, const Vec2i &pos)
{
	Assert(type == MapFieldForest || type == MapFieldRocks);

	//  Outside of map or no wood.
	if (!Info.IsPointOnMap(pos)) {
		return;
	}
	unsigned int index = getIndex(pos);
	CMapField &mf = *this->Field(index);

	if (!((type == MapFieldForest && Tileset->isAWoodTile(mf.playerInfo.SeenTile))
		  || (type == MapFieldRocks && Tileset->isARockTile(mf.playerInfo.SeenTile)))) {
		if (seen) {
			return;
		}
	}

	if (!seen && !(mf.getFlag() & type)) {
		return;
	}

	// Select Table to lookup
	int removedtile;
	int flags;
	if (type == MapFieldForest) {
		removedtile = this->Tileset->getRemovedTreeTile();
		flags = (MapFieldForest | MapFieldUnpassable);
	} else { // (type == MapFieldRocks)
		removedtile = this->Tileset->getRemovedRockTile();
		flags = (MapFieldRocks | MapFieldUnpassable);
	}
	//  Find out what each tile has with respect to wood, or grass.
	int ttup;
	int ttdown;
	int ttleft;
	int ttright;

	if (pos.y - 1 < 0) {
		ttup = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf - this->Info.MapWidth);
		ttup = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	if (pos.x + 1 >= this->Info.MapWidth) {
		ttright = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf + 1);
		ttright = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	if (pos.y + 1 >= this->Info.MapHeight) {
		ttdown = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf + this->Info.MapWidth);
		ttdown = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	if (pos.x - 1 < 0) {
		ttleft = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf - 1);
		ttleft = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	int tile = this->Tileset->getTileBySurrounding(type, ttup, ttright, ttdown, ttleft);

	//Update seen tile.
	if (tile == -1) { // No valid wood remove it.
		if (seen) {
			mf.playerInfo.SeenTile = removedtile;
			this->FixNeighbors(type, seen, pos);
		} else {
			mf.setGraphicTile(removedtile);
			mf.Flags &= ~flags;
			mf.Value = 0;
			UI.Minimap.UpdateXY(pos);
		}
	} else if (seen && this->Tileset->isEquivalentTile(tile, mf.playerInfo.SeenTile)) { //Same Type
		return;
	} else {
		if (seen) {
			mf.playerInfo.SeenTile = tile;
		} else {
			mf.setGraphicTile(tile);
		}
	}

	//maybe isExplored
	if (mf.playerInfo.IsExplored(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		if (!seen) {
			MarkSeenTile(mf);
		}
	}
}
*/
//Wyrmgus end

/**
**  Correct the surrounding fields.
**
**  @param type  Tiletype of tile to adjust
**  @param seen  1 if updating seen value, 0 for real
**  @param pos   Map tile-position.
*/
//Wyrmgus start
/*
void CMap::FixNeighbors(unsigned short type, int seen, const Vec2i &pos)
{
	const Vec2i offset[] = {Vec2i(1, 0), Vec2i(-1, 0), Vec2i(0, 1), Vec2i(0, -1),
							Vec2i(-1, -1), Vec2i(-1, 1), Vec2i(1, -1), Vec2i(1, 1)
						   };

	for (unsigned int i = 0; i < sizeof(offset) / sizeof(*offset); ++i) {
		FixTile(type, seen, pos + offset[i]);
	}
}
*/
//Wyrmgus end

//Wyrmgus start
void CMap::SetTileTerrain(const Vec2i &pos, stratagus::terrain_type *terrain, int z)
{
	if (!terrain) {
		return;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	stratagus::terrain_type *old_terrain = this->GetTileTerrain(pos, terrain->is_overlay(), z);
	
	if (terrain->is_overlay()) {
		if (mf.OverlayTerrain == terrain) {
			return;
		}
	} else {
		if (mf.Terrain == terrain) {
			return;
		}
	}
	
	mf.SetTerrain(terrain);
	
	if (terrain->is_overlay()) {
		//remove decorations if the overlay terrain has changed
		std::vector<CUnit *> table;
		Select(pos, pos, table, z);
		for (size_t i = 0; i != table.size(); ++i) {
			if (table[i] && table[i]->IsAlive() && table[i]->Type->UnitType == UnitTypeType::Land && table[i]->Type->BoolFlag[DECORATION_INDEX].value) {
				if (Editor.Running == EditorNotRunning) {
					LetUnitDie(*table[i]);
				} else {
					EditorActionRemoveUnit(*table[i], false);
				}
			}
		}
	}
	
	//recalculate transitions and solid tiles for both non-overlay and overlay, since setting one may have changed the other
	this->calculate_tile_solid_tile(pos, false, z);
	if (mf.OverlayTerrain != nullptr) {
		this->calculate_tile_solid_tile(pos, true, z);
	}
	this->CalculateTileTransitions(pos, false, z); 
	this->CalculateTileTransitions(pos, true, z);
	this->calculate_tile_terrain_feature(pos, z);
	
	if (mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
		MarkSeenTile(mf, z);
	}
	UI.Minimap.UpdateXY(pos, z);
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
					
					if (terrain->is_overlay() && adjacent_mf.OverlayTerrain != terrain && Editor.Running == EditorNotRunning) {
						continue;
					}
					
					this->CalculateTileTransitions(adjacent_pos, false, z);
					this->CalculateTileTransitions(adjacent_pos, true, z);
					
					if (adjacent_mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
						MarkSeenTile(adjacent_mf, z);
					}
					UI.Minimap.UpdateXY(adjacent_pos, z);
				}
			}
		}
	}
}

//Wyrmgus start
//void CMap::RemoveTileOverlayTerrain(const Vec2i &pos)
void CMap::RemoveTileOverlayTerrain(const Vec2i &pos, int z)
//Wyrmgus end
{
	CMapField &mf = *this->Field(pos, z);
	
	if (!mf.OverlayTerrain) {
		return;
	}
	
	stratagus::terrain_type *old_terrain = mf.OverlayTerrain;
	
	mf.RemoveOverlayTerrain();
	
	this->CalculateTileTransitions(pos, true, z);
	this->calculate_tile_terrain_feature(pos, z);
	
	if (mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
		MarkSeenTile(mf, z);
	}
	UI.Minimap.UpdateXY(pos, z);
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
					
					this->CalculateTileTransitions(adjacent_pos, true, z);
					
					if (adjacent_mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
						MarkSeenTile(adjacent_mf, z);
					}
					UI.Minimap.UpdateXY(adjacent_pos, z);
				}
			}
		}
	}
}

void CMap::SetOverlayTerrainDestroyed(const Vec2i &pos, bool destroyed, int z)
{
	CMapLayer *map_layer = this->MapLayers[z];
	
	if (!map_layer) {
		return;
	}
	
	CMapField &mf = *map_layer->Field(pos);
	
	if (!mf.OverlayTerrain || mf.OverlayTerrainDestroyed == destroyed) {
		return;
	}
	
	mf.SetOverlayTerrainDestroyed(destroyed);
	
	if (destroyed) {
		if (mf.OverlayTerrain->Flags & MapFieldForest) {
			mf.Flags &= ~(MapFieldForest | MapFieldUnpassable);
			mf.Flags |= MapFieldStumps;
			map_layer->DestroyedForestTiles.push_back(pos);
		} else if (mf.OverlayTerrain->Flags & MapFieldRocks) {
			mf.Flags &= ~(MapFieldRocks | MapFieldUnpassable);
			mf.Flags |= MapFieldGravel;
		} else if (mf.OverlayTerrain->Flags & MapFieldWall) {
			mf.Flags &= ~(MapFieldWall | MapFieldUnpassable);
			mf.Flags |= MapFieldGravel;
			if (mf.Flags & MapFieldUnderground) {
				mf.Flags &= ~(MapFieldAirUnpassable);
			}
		}
		mf.Value = 0;
	} else {
		if (mf.Flags & MapFieldStumps) { //if is a cleared tree tile regrowing trees
			mf.Flags &= ~(MapFieldStumps);
			mf.Flags |= MapFieldForest | MapFieldUnpassable;
			mf.Value = stratagus::resource::get_all()[WoodCost]->DefaultAmount;
		}
	}
	
	if (destroyed) {
		if (mf.OverlayTerrain->get_destroyed_tiles().size() > 0) {
			mf.OverlaySolidTile = mf.OverlayTerrain->get_destroyed_tiles()[SyncRand(mf.OverlayTerrain->get_destroyed_tiles().size())];
		}
	} else {
		this->calculate_tile_solid_tile(pos, true, z);
	}

	this->CalculateTileTransitions(pos, true, z);
	
	if (mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
		MarkSeenTile(mf, z);
	}
	UI.Minimap.UpdateXY(pos, z);
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
					
					if (adjacent_mf.OverlayTerrain != mf.OverlayTerrain) {
						continue;
					}
					
					this->CalculateTileTransitions(adjacent_pos, true, z);
					
					if (adjacent_mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
						MarkSeenTile(adjacent_mf, z);
					}
					UI.Minimap.UpdateXY(adjacent_pos, z);
				}
			}
		}
	}
}

void CMap::SetOverlayTerrainDamaged(const Vec2i &pos, bool damaged, int z)
{
	CMapField &mf = *this->Field(pos, z);
	
	if (!mf.OverlayTerrain || mf.OverlayTerrainDamaged == damaged) {
		return;
	}
	
	mf.SetOverlayTerrainDamaged(damaged);
	
	if (damaged) {
		if (mf.OverlayTerrain->get_damaged_tiles().size() > 0) {
			mf.OverlaySolidTile = mf.OverlayTerrain->get_damaged_tiles()[SyncRand(mf.OverlayTerrain->get_damaged_tiles().size())];
		}
	} else {
		this->calculate_tile_solid_tile(pos, true, z);
	}

	this->CalculateTileTransitions(pos, true, z);
	
	if (mf.playerInfo.IsTeamVisible(*CPlayer::GetThisPlayer())) {
		MarkSeenTile(mf, z);
	}
	UI.Minimap.UpdateXY(pos, z);
}

static stratagus::tile_transition_type GetTransitionType(std::vector<int> &adjacent_directions, bool allow_single = false)
{
	if (adjacent_directions.size() == 0) {
		return stratagus::tile_transition_type::none;
	}
	
	stratagus::tile_transition_type transition_type = stratagus::tile_transition_type::none;

	if (allow_single && stratagus::vector::contains(adjacent_directions, North) && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, West) && stratagus::vector::contains(adjacent_directions, East)) {
		transition_type = stratagus::tile_transition_type::single;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, West) && stratagus::vector::contains(adjacent_directions, East)) {
		transition_type = stratagus::tile_transition_type::north_single;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, West) && stratagus::vector::contains(adjacent_directions, East)) {
		transition_type = stratagus::tile_transition_type::south_single;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, North) && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::west_single;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, North) && stratagus::vector::contains(adjacent_directions, South) && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, East)) {
		transition_type = stratagus::tile_transition_type::east_single;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, North) && stratagus::vector::contains(adjacent_directions, South) && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::north_south;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, West) && stratagus::vector::contains(adjacent_directions, East) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::west_east;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest) && stratagus::vector::contains(adjacent_directions, Southeast)) {
		transition_type = stratagus::tile_transition_type::north_southwest_inner_southeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest)) {
		transition_type = stratagus::tile_transition_type::north_southwest_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southeast)) {
		transition_type = stratagus::tile_transition_type::north_southeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, Northwest) && stratagus::vector::contains(adjacent_directions, Northeast)) {
		transition_type = stratagus::tile_transition_type::south_northwest_inner_northeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, Northwest)) {
		transition_type = stratagus::tile_transition_type::south_northwest_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, Northeast)) {
		transition_type = stratagus::tile_transition_type::south_northeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Northeast) && stratagus::vector::contains(adjacent_directions, Southeast)) {
		transition_type = stratagus::tile_transition_type::west_northeast_inner_southeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Northeast)) {
		transition_type = stratagus::tile_transition_type::west_northeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southeast)) {
		transition_type = stratagus::tile_transition_type::west_southeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, East) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Northwest) && stratagus::vector::contains(adjacent_directions, Southwest)) {
		transition_type = stratagus::tile_transition_type::east_northwest_inner_southwest_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, East) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Northwest)) {
		transition_type = stratagus::tile_transition_type::east_northwest_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, East) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest)) {
		transition_type = stratagus::tile_transition_type::east_southwest_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southeast)) {
		transition_type = stratagus::tile_transition_type::northwest_outer_southeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, East) && stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest)) {
		transition_type = stratagus::tile_transition_type::northeast_outer_southwest_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, Northeast)) {
		transition_type = stratagus::tile_transition_type::southwest_outer_northeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, East) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, South) && stratagus::vector::contains(adjacent_directions, Northwest)) {
		transition_type = stratagus::tile_transition_type::southeast_outer_northwest_inner;
	} else if (stratagus::vector::contains(adjacent_directions, North) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::north;
	} else if (stratagus::vector::contains(adjacent_directions, South) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::south;
	} else if (stratagus::vector::contains(adjacent_directions, West) && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::west;
	} else if (stratagus::vector::contains(adjacent_directions, East) && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::east;
	} else if ((stratagus::vector::contains(adjacent_directions, North) || stratagus::vector::contains(adjacent_directions, West)) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_outer;
	} else if ((stratagus::vector::contains(adjacent_directions, North) || stratagus::vector::contains(adjacent_directions, East)) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northeast_outer;
	} else if ((stratagus::vector::contains(adjacent_directions, South) || stratagus::vector::contains(adjacent_directions, West)) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::southwest_outer;
	} else if ((stratagus::vector::contains(adjacent_directions, South) || stratagus::vector::contains(adjacent_directions, East)) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::southeast_outer;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, Northwest) && stratagus::vector::contains(adjacent_directions, Southeast) && stratagus::vector::contains(adjacent_directions, Northeast) && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_northeast_southwest_southeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, Northwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Northeast) && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_northeast_southwest_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, Northwest) && stratagus::vector::contains(adjacent_directions, Southeast) && stratagus::vector::contains(adjacent_directions, Northeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_northeast_southeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, Northwest) && stratagus::vector::contains(adjacent_directions, Southeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_southwest_southeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southeast) && stratagus::vector::contains(adjacent_directions, Northeast) && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northeast_southwest_southeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, Northwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Northeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_northeast_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::southwest_southeast_inner;
	} else if (allow_single && stratagus::vector::contains(adjacent_directions, Northwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_southwest_inner;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && stratagus::vector::contains(adjacent_directions, Southeast) && stratagus::vector::contains(adjacent_directions, Northeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northeast_southeast_inner;
	} else if (stratagus::vector::contains(adjacent_directions, Northwest) && stratagus::vector::contains(adjacent_directions, Southeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_southeast_inner;
	} else if (stratagus::vector::contains(adjacent_directions, Northeast) && stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northeast_southwest_inner;
	} else if (stratagus::vector::contains(adjacent_directions, Northwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northwest_inner;
	} else if (stratagus::vector::contains(adjacent_directions, Northeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::northeast_inner;
	} else if (stratagus::vector::contains(adjacent_directions, Southwest) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::southwest_inner;
	} else if (stratagus::vector::contains(adjacent_directions, Southeast) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = stratagus::tile_transition_type::southeast_inner;
	}

	return transition_type;
}

void CMap::calculate_tile_solid_tile(const QPoint &pos, const bool overlay, const int z)
{
	CMapField *tile = this->Field(pos, z);

	const stratagus::terrain_type *terrain_type = nullptr;
	int solid_tile = 0;

	if (overlay) {
		terrain_type = tile->OverlayTerrain;
	} else {
		terrain_type = tile->Terrain;
	}

	if (terrain_type->has_tiled_background()) {
		const CPlayerColorGraphic *terrain_graphics = terrain_type->get_graphics();
		const int solid_tile_frame_x = pos.x() % terrain_graphics->get_frames_per_row();
		const int solid_tile_frame_y = pos.y() % terrain_graphics->get_frames_per_column();
		solid_tile = terrain_graphics->get_frame_index(QPoint(solid_tile_frame_x, solid_tile_frame_y));
	} else {
		if (!terrain_type->get_solid_tiles().empty()) {
			solid_tile = terrain_type->get_solid_tiles()[SyncRand(terrain_type->get_solid_tiles().size())];
		}
	}

	if (overlay) {
		tile->OverlaySolidTile = solid_tile;
	} else {
		tile->SolidTile = solid_tile;
	}
}

void CMap::CalculateTileTransitions(const Vec2i &pos, bool overlay, int z)
{
	CMapField &mf = *this->Field(pos, z);
	stratagus::terrain_type *terrain = nullptr;
	if (overlay) {
		terrain = mf.OverlayTerrain;
		mf.OverlayTransitionTiles.clear();
	} else {
		terrain = mf.Terrain;
		mf.TransitionTiles.clear();
	}
	
	if (!terrain || (overlay && mf.OverlayTerrainDestroyed)) {
		return;
	}
	
	int terrain_id = terrain->ID;
	
	std::map<int, std::vector<int>> adjacent_terrain_directions;
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					stratagus::terrain_type *adjacent_terrain = this->GetTileTerrain(adjacent_pos, overlay, z);
					if (overlay && adjacent_terrain && this->Field(adjacent_pos, z)->OverlayTerrainDestroyed) {
						adjacent_terrain = nullptr;
					}
					if (adjacent_terrain && terrain != adjacent_terrain) {
						if (stratagus::vector::contains(terrain->get_inner_border_terrain_types(), adjacent_terrain)) {
							adjacent_terrain_directions[adjacent_terrain->ID].push_back(GetDirectionFromOffset(x_offset, y_offset));
						} else if (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), adjacent_terrain) == terrain->BorderTerrains.end()) { //if the two terrain types can't border, look for a third terrain type which can border both, and which treats both as outer border terrains, and then use for transitions between both tiles
							for (const stratagus::terrain_type *border_terrain : terrain->BorderTerrains) {
								if (stratagus::vector::contains(terrain->get_inner_border_terrain_types(), border_terrain) && stratagus::vector::contains(adjacent_terrain->get_inner_border_terrain_types(), border_terrain)) {
									adjacent_terrain_directions[border_terrain->ID].push_back(GetDirectionFromOffset(x_offset, y_offset));
									break;
								}
							}
						}
					}
					if (!adjacent_terrain || (overlay && terrain != adjacent_terrain && std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), adjacent_terrain) == terrain->BorderTerrains.end())) { // happens if terrain is null or if it is an overlay tile which doesn't have a border with this one, so that i.e. tree transitions display correctly when adjacent to tiles without overlays
						adjacent_terrain_directions[stratagus::terrain_type::get_all().size()].push_back(GetDirectionFromOffset(x_offset, y_offset));
					}
				}
			}
		}
	}
	
	for (std::map<int, std::vector<int>>::iterator iterator = adjacent_terrain_directions.begin(); iterator != adjacent_terrain_directions.end(); ++iterator) {
		int adjacent_terrain_id = iterator->first;
		stratagus::terrain_type *adjacent_terrain = adjacent_terrain_id < (int) stratagus::terrain_type::get_all().size() ? stratagus::terrain_type::get_all()[adjacent_terrain_id] : nullptr;
		const stratagus::tile_transition_type transition_type = GetTransitionType(iterator->second, terrain->allows_single());
		
		if (transition_type != stratagus::tile_transition_type::none) {
			bool found_transition = false;
			
			if (!overlay) {
				if (adjacent_terrain != nullptr) {
					const std::vector<int> &transition_tiles = terrain->get_transition_tiles(adjacent_terrain, transition_type);
					if (!transition_tiles.empty()) {
						mf.TransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(terrain, transition_tiles[SyncRand(transition_tiles.size())]));
						found_transition = true;
					} else {
						const std::vector<int> &adjacent_transition_tiles = adjacent_terrain->get_adjacent_transition_tiles(terrain, transition_type);
						if (!adjacent_transition_tiles.empty()) {
							mf.TransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(adjacent_terrain, adjacent_transition_tiles[SyncRand(adjacent_transition_tiles.size())]));
							found_transition = true;
						} else {
							const std::vector<int> &adjacent_transition_tiles = adjacent_terrain->get_adjacent_transition_tiles(nullptr, transition_type);
							if (!adjacent_transition_tiles.empty()) {
								mf.TransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(adjacent_terrain, adjacent_transition_tiles[SyncRand(adjacent_transition_tiles.size())]));
								found_transition = true;
							}
						}
					}
				} else {
					const std::vector<int> &transition_tiles = terrain->get_transition_tiles(nullptr, transition_type);
					if (!transition_tiles.empty()) {
						mf.TransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(terrain, transition_tiles[SyncRand(transition_tiles.size())]));
					}
				}
			} else {
				if (adjacent_terrain != nullptr) {
					const std::vector<int> &transition_tiles = terrain->get_transition_tiles(adjacent_terrain, transition_type);
					if (!transition_tiles.empty()) {
						mf.OverlayTransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(terrain, transition_tiles[SyncRand(transition_tiles.size())]));
						found_transition = true;
					} else {
						const std::vector<int> &adjacent_transition_tiles = adjacent_terrain->get_transition_tiles(terrain, transition_type);
						if (!adjacent_transition_tiles.empty()) {
							mf.OverlayTransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(adjacent_terrain, adjacent_transition_tiles[SyncRand(adjacent_transition_tiles.size())]));
							found_transition = true;
						} else {
							const std::vector<int> &adjacent_transition_tiles = adjacent_terrain->get_transition_tiles(nullptr, transition_type);
							if (!adjacent_transition_tiles.empty()) {
								mf.OverlayTransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(adjacent_terrain, adjacent_transition_tiles[SyncRand(adjacent_transition_tiles.size())]));
								found_transition = true;
							}
						}
					}
				} else {
					const std::vector<int> &transition_tiles = terrain->get_transition_tiles(nullptr, transition_type);
					if (!transition_tiles.empty()) {
						mf.OverlayTransitionTiles.push_back(std::pair<stratagus::terrain_type *, int>(terrain, transition_tiles[SyncRand(transition_tiles.size())]));
					}
				}
				
				if ((mf.Flags & MapFieldWaterAllowed) && (!adjacent_terrain || !(adjacent_terrain->Flags & MapFieldWaterAllowed))) { //if this is a water tile adjacent to a non-water tile, replace the water flag with a coast one
					mf.Flags &= ~(MapFieldWaterAllowed);
					mf.Flags |= MapFieldCoastAllowed;
				}
			}
			
			if (adjacent_terrain && found_transition) {
				for (size_t i = 0; i != iterator->second.size(); ++i) {
					adjacent_terrain_directions[stratagus::terrain_type::get_all().size()].erase(std::remove(adjacent_terrain_directions[stratagus::terrain_type::get_all().size()].begin(), adjacent_terrain_directions[stratagus::terrain_type::get_all().size()].end(), iterator->second[i]), adjacent_terrain_directions[stratagus::terrain_type::get_all().size()].end());
				}
			}
		}
	}
	
	//sort the transitions so that they will be displayed in the correct order
	if (overlay) {
		bool swapped = true;
		for (int passes = 0; passes < (int) mf.OverlayTransitionTiles.size() && swapped; ++passes) {
			swapped = false;
			for (int i = 0; i < ((int) mf.OverlayTransitionTiles.size()) - 1; ++i) {
				bool change_order = false;
				if (stratagus::vector::contains(mf.OverlayTransitionTiles[i + 1].first->get_inner_border_terrain_types(), mf.OverlayTransitionTiles[i].first)) {
					std::pair<stratagus::terrain_type *, int> temp_transition = mf.OverlayTransitionTiles[i];
					mf.OverlayTransitionTiles[i] = mf.OverlayTransitionTiles[i + 1];
					mf.OverlayTransitionTiles[i + 1] = temp_transition;
					swapped = true;
				}
			}
		}
	} else {
		bool swapped = true;
		for (int passes = 0; passes < (int) mf.TransitionTiles.size() && swapped; ++passes) {
			swapped = false;
			for (int i = 0; i < ((int) mf.TransitionTiles.size()) - 1; ++i) {
				bool change_order = false;
				if (stratagus::vector::contains(mf.TransitionTiles[i + 1].first->get_inner_border_terrain_types(), mf.TransitionTiles[i].first)) {
					std::pair<stratagus::terrain_type *, int> temp_transition = mf.TransitionTiles[i];
					mf.TransitionTiles[i] = mf.TransitionTiles[i + 1];
					mf.TransitionTiles[i + 1] = temp_transition;
					swapped = true;
				}
			}
		}
	}
}

void CMap::CalculateTileLandmass(const Vec2i &pos, int z)
{
	if (!this->Info.IsPointOnMap(pos, z)) {
		return;
	}
	
	if (Editor.Running != EditorNotRunning) { //no need to assign landmasses while in the editor
		return;
	}
	
	CMapField &mf = *this->Field(pos, z);

	if (mf.Landmass != 0) {
		return; //already calculated
	}
	
	const bool is_space = mf.Flags & MapFieldSpace;

	if (is_space) {
		return; //no landmass for space tiles
	}

	const bool is_water = (mf.Flags & MapFieldWaterAllowed) || (mf.Flags & MapFieldCoastAllowed);

	//doesn't have a landmass ID, and hasn't inherited one from another tile yet, so add a new one
	mf.Landmass = this->Landmasses + 1;
	this->Landmasses += 1;
	this->BorderLandmasses.resize(this->Landmasses + 1);
	//now, spread the new landmass ID to neighboring land tiles
	std::vector<Vec2i> landmass_tiles;
	landmass_tiles.push_back(pos);
	//calculate the landmass of any neighboring land tiles with no set landmass as well
	for (size_t i = 0; i < landmass_tiles.size(); ++i) {
		for (int x_offset = -1; x_offset <= 1; ++x_offset) {
			for (int y_offset = -1; y_offset <= 1; ++y_offset) {
				if (x_offset != 0 || y_offset != 0) {
					Vec2i adjacent_pos(landmass_tiles[i].x + x_offset, landmass_tiles[i].y + y_offset);
					if (this->Info.IsPointOnMap(adjacent_pos, z)) {
						CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
						const bool adjacent_is_space = adjacent_mf.Flags & MapFieldSpace;

						if (adjacent_is_space) {
							continue;
						}

						const bool adjacent_is_water = (adjacent_mf.Flags & MapFieldWaterAllowed) || (adjacent_mf.Flags & MapFieldCoastAllowed);
									
						if (adjacent_is_water == is_water && adjacent_mf.Landmass == 0) {
							adjacent_mf.Landmass = mf.Landmass;
							landmass_tiles.push_back(adjacent_pos);
						} else if (adjacent_is_water != is_water && adjacent_mf.Landmass != 0 && std::find(this->BorderLandmasses[mf.Landmass].begin(), this->BorderLandmasses[mf.Landmass].end(), adjacent_mf.Landmass) == this->BorderLandmasses[mf.Landmass].end()) {
							this->BorderLandmasses[mf.Landmass].push_back(adjacent_mf.Landmass);
							this->BorderLandmasses[adjacent_mf.Landmass].push_back(mf.Landmass);
						}
					}
				}
			}
		}
	}
}

void CMap::calculate_tile_terrain_feature(const Vec2i &pos, int z)
{
	if (!this->Info.IsPointOnMap(pos, z)) {
		return;
	}
	
	if (Editor.Running != EditorNotRunning) { //no need to assign terrain features while in the editor
		return;
	}
	
	CMapField &mf = *this->Field(pos, z);

	if (mf.get_terrain_feature() != nullptr) {
		return; //already has a terrain feature
	}

	//if any adjacent tile has the same top terrain as this one, and has a terrain feature, then use that
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if ((x_offset != 0 || y_offset != 0) && !(x_offset != 0 && y_offset != 0)) { //only directly adjacent tiles (no diagonal ones, and not the same tile)
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);

					if (adjacent_mf.get_terrain_feature() != nullptr && adjacent_mf.get_terrain_feature()->get_terrain_type() == GetTileTopTerrain(pos, false, z)) {
						mf.set_terrain_feature(adjacent_mf.get_terrain_feature());
						return;
					}
				}
			}
		}
	}
}

void CMap::CalculateTileOwnershipTransition(const Vec2i &pos, int z)
{
	if (!this->Info.IsPointOnMap(pos, z)) {
		return;
	}
	
	if (Editor.Running != EditorNotRunning) { //no need to assign ownership transitions while in the editor
		return;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	mf.set_ownership_border_tile(-1);

	if (mf.get_owner() == nullptr) {
		return;
	}
	
	std::vector<int> adjacent_directions;
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
					if (adjacent_mf.get_owner() != mf.get_owner()) {
						adjacent_directions.push_back(GetDirectionFromOffset(x_offset, y_offset));
					}
				}
			}
		}
	}
	
	const stratagus::tile_transition_type transition_type = GetTransitionType(adjacent_directions, true);

	if (transition_type != stratagus::tile_transition_type::none) {
		const std::vector<int> &transition_tiles = stratagus::defines::get()->get_border_terrain_type()->get_transition_tiles(nullptr, transition_type);
		if (!transition_tiles.empty()) {
			mf.set_ownership_border_tile(stratagus::vector::get_random(transition_tiles));
		}
	}
}

void CMap::AdjustMap()
{
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		Vec2i map_start_pos(0, 0);
		Vec2i map_end(this->Info.MapWidths[z], this->Info.MapHeights[z]);
		
		this->AdjustTileMapIrregularities(false, map_start_pos, map_end, z);
		this->AdjustTileMapIrregularities(true, map_start_pos, map_end, z);
		this->AdjustTileMapTransitions(map_start_pos, map_end, z);
		this->AdjustTileMapIrregularities(false, map_start_pos, map_end, z);
		this->AdjustTileMapIrregularities(true, map_start_pos, map_end, z);
	}
}

void CMap::AdjustTileMapIrregularities(const bool overlay, const Vec2i &min_pos, const Vec2i &max_pos, const int z)
{
	bool no_irregularities_found = false;
	int try_count = 0;
	const int max_try_count = 100;
	while (!no_irregularities_found && try_count < max_try_count) {
		no_irregularities_found = true;
		try_count++;

		for (int x = min_pos.x; x < max_pos.x; ++x) {
			for (int y = min_pos.y; y < max_pos.y; ++y) {
				CMapField &mf = *this->Field(x, y, z);
				stratagus::terrain_type *terrain = overlay ? mf.OverlayTerrain : mf.Terrain;
				if (!terrain || terrain->allows_single()) {
					continue;
				}
				std::vector<stratagus::terrain_type *> acceptable_adjacent_tile_types;
				acceptable_adjacent_tile_types.push_back(terrain);
				stratagus::vector::merge(acceptable_adjacent_tile_types, terrain->get_outer_border_terrain_types());
				
				int horizontal_adjacent_tiles = 0;
				int vertical_adjacent_tiles = 0;
				int nw_quadrant_adjacent_tiles = 0; //should be 4 if the wrong tile types are present in X-1,Y; X-1,Y-1; X,Y-1; and X+1,Y+1
				int ne_quadrant_adjacent_tiles = 0;
				int sw_quadrant_adjacent_tiles = 0;
				int se_quadrant_adjacent_tiles = 0;
				
				if ((x - 1) >= 0 && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), this->GetTileTerrain(Vec2i(x - 1, y), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					horizontal_adjacent_tiles += 1;
					nw_quadrant_adjacent_tiles += 1;
					sw_quadrant_adjacent_tiles += 1;
				}
				if ((x + 1) < this->Info.MapWidths[z] && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), this->GetTileTerrain(Vec2i(x + 1, y), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					horizontal_adjacent_tiles += 1;
					ne_quadrant_adjacent_tiles += 1;
					se_quadrant_adjacent_tiles += 1;
				}
				
				if ((y - 1) >= 0 && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), this->GetTileTerrain(Vec2i(x, y - 1), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					vertical_adjacent_tiles += 1;
					nw_quadrant_adjacent_tiles += 1;
					ne_quadrant_adjacent_tiles += 1;
				}
				if ((y + 1) < this->Info.MapHeights[z] && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), this->GetTileTerrain(Vec2i(x, y + 1), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					vertical_adjacent_tiles += 1;
					sw_quadrant_adjacent_tiles += 1;
					se_quadrant_adjacent_tiles += 1;
				}

				if ((x - 1) >= 0 && (y - 1) >= 0 && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), this->GetTileTerrain(Vec2i(x - 1, y - 1), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					nw_quadrant_adjacent_tiles += 1;
					se_quadrant_adjacent_tiles += 1;
				}
				if ((x - 1) >= 0 && (y + 1) < this->Info.MapHeights[z] && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), GetTileTerrain(Vec2i(x - 1, y + 1), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					sw_quadrant_adjacent_tiles += 1;
					ne_quadrant_adjacent_tiles += 1;
				}
				if ((x + 1) < this->Info.MapWidths[z] && (y - 1) >= 0 && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), GetTileTerrain(Vec2i(x + 1, y - 1), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					ne_quadrant_adjacent_tiles += 1;
					sw_quadrant_adjacent_tiles += 1;
				}
				if ((x + 1) < this->Info.MapWidths[z] && (y + 1) < this->Info.MapHeights[z] && std::find(acceptable_adjacent_tile_types.begin(), acceptable_adjacent_tile_types.end(), GetTileTerrain(Vec2i(x + 1, y + 1), overlay, z)) == acceptable_adjacent_tile_types.end()) {
					se_quadrant_adjacent_tiles += 1;
					nw_quadrant_adjacent_tiles += 1;
				}
				
				
				if (horizontal_adjacent_tiles >= 2 || vertical_adjacent_tiles >= 2 || nw_quadrant_adjacent_tiles >= 4 || ne_quadrant_adjacent_tiles >= 4 || sw_quadrant_adjacent_tiles >= 4 || se_quadrant_adjacent_tiles >= 4) {
					if (overlay) {
						mf.RemoveOverlayTerrain();
					} else {
						std::map<stratagus::terrain_type *, int> best_terrain_scores;

						for (int sub_x = -1; sub_x <= 1; ++sub_x) {
							for (int sub_y = -1; sub_y <= 1; ++sub_y) {
								if ((x + sub_x) < min_pos.x || (x + sub_x) >= max_pos.x || (y + sub_y) < min_pos.y || (y + sub_y) >= max_pos.y || (sub_x == 0 && sub_y == 0)) {
									continue;
								}
								stratagus::terrain_type *tile_terrain = GetTileTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
								if (mf.Terrain != tile_terrain) {
									best_terrain_scores[tile_terrain]++;
								}
							}
						}

						stratagus::terrain_type *best_terrain = nullptr;
						int best_score = 0;
						for (const auto &score_pair : best_terrain_scores) {
							const int score = score_pair.second;
							if (score > best_score) {
								best_score = score;
								best_terrain = score_pair.first;
							}
						}

						mf.SetTerrain(best_terrain);
					}
					no_irregularities_found = false;
				}
			}
		}
	}
}

void CMap::AdjustTileMapTransitions(const Vec2i &min_pos, const Vec2i &max_pos, int z)
{
	for (int x = min_pos.x; x < max_pos.x; ++x) {
		for (int y = min_pos.y; y < max_pos.y; ++y) {
			CMapField &mf = *this->Field(x, y, z);

			for (int sub_x = -1; sub_x <= 1; ++sub_x) {
				for (int sub_y = -1; sub_y <= 1; ++sub_y) {
					if ((x + sub_x) < min_pos.x || (x + sub_x) >= max_pos.x || (y + sub_y) < min_pos.y || (y + sub_y) >= max_pos.y || (sub_x == 0 && sub_y == 0)) {
						continue;
					}
					stratagus::terrain_type *tile_terrain = GetTileTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
					stratagus::terrain_type *tile_top_terrain = GetTileTopTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
					if (
						mf.Terrain != tile_terrain
						&& tile_top_terrain->is_overlay()
						&& tile_top_terrain != mf.OverlayTerrain
						&& !stratagus::vector::contains(tile_terrain->get_outer_border_terrain_types(), mf.Terrain)
						&& !stratagus::vector::contains(tile_top_terrain->get_base_terrain_types(), mf.Terrain)
					) {
						mf.SetTerrain(tile_terrain);
					}
				}
			}
		}
	}

	for (int x = min_pos.x; x < max_pos.x; ++x) {
		for (int y = min_pos.y; y < max_pos.y; ++y) {
			CMapField &mf = *this->Field(x, y, z);

			for (int sub_x = -1; sub_x <= 1; ++sub_x) {
				for (int sub_y = -1; sub_y <= 1; ++sub_y) {
					if ((x + sub_x) < min_pos.x || (x + sub_x) >= max_pos.x || (y + sub_y) < min_pos.y || (y + sub_y) >= max_pos.y || (sub_x == 0 && sub_y == 0)) {
						continue;
					}
					stratagus::terrain_type *tile_terrain = GetTileTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
					if (mf.Terrain != tile_terrain && !stratagus::vector::contains(mf.Terrain->BorderTerrains, tile_terrain)) {
						for (stratagus::terrain_type *border_terrain : mf.Terrain->BorderTerrains) {
							if (stratagus::vector::contains(border_terrain->BorderTerrains, mf.Terrain) && stratagus::vector::contains(border_terrain->BorderTerrains, tile_terrain)) {
								mf.SetTerrain(border_terrain);
								break;
							}
						}
					}
				}
			}
		}
	}
}

/**
**	@brief	Generate a given terrain on the map
**
**	@param	generated_terrain	The terrain generation characteristics
**	@param	min_pos				The minimum position in the map to generate the terrain on
**	@param	max_pos				The maximum position in the map to generate the terrain on
**	@param	preserve_coastline	Whether to avoid changing the coastline during terrain generation
**	@param	z					The map layer to generate the terrain on
*/
void CMap::GenerateTerrain(const std::unique_ptr<stratagus::generated_terrain> &generated_terrain, const Vec2i &min_pos, const Vec2i &max_pos, const bool preserve_coastline, const int z)
{
	if (SaveGameLoading) {
		return;
	}
	
	stratagus::terrain_type *terrain_type = generated_terrain->TerrainType;
	const int seed_count = generated_terrain->SeedCount;
	const int max_tile_quantity = (max_pos.x + 1 - min_pos.x) * (max_pos.y + 1 - min_pos.y) * generated_terrain->MaxPercent / 100;
	int tile_quantity = 0;
	
	Vec2i random_pos(0, 0);
	int count = seed_count;
	
	std::vector<Vec2i> seeds;
	
	if (generated_terrain->UseExistingAsSeeds) { //use existing tiles of the given terrain as seeds for the terrain generation
		for (int x = min_pos.x; x <= max_pos.x; ++x) {
			for (int y = min_pos.y; y <= max_pos.y; ++y) {
				const Vec2i tile_pos(x, y);
				const CMapField *tile = this->Field(x, y, z);
				
				if (max_tile_quantity != 0 && tile->GetTopTerrain() == terrain_type) {
					tile_quantity++;
				}
				
				if (!generated_terrain->CanUseTileAsSeed(tile)) {
					continue;
				}
				
				if (this->is_point_in_a_subtemplate_area(tile_pos, z)) {
					continue;
				}
				
				seeds.push_back(tile_pos);
			}
		}
	}
	
	if (generated_terrain->UseSubtemplateBordersAsSeeds) {
		for (size_t i = 0; i < this->MapLayers[z]->subtemplate_areas.size(); ++i) {
			const Vec2i subtemplate_min_pos = std::get<0>(this->MapLayers[z]->subtemplate_areas[i]);
			const Vec2i subtemplate_max_pos = std::get<1>(this->MapLayers[z]->subtemplate_areas[i]);
			
			for (int x = subtemplate_min_pos.x; x <= subtemplate_max_pos.x; ++x) {
				for (int y = subtemplate_min_pos.y; y <= subtemplate_max_pos.y; ++y) {
					const Vec2i tile_pos(x, y);
					const CMapField *tile = this->Field(x, y, z);
					
					if (!generated_terrain->CanUseTileAsSeed(tile)) {
						continue;
					}
					
					if (!this->is_point_adjacent_to_non_subtemplate_area(tile_pos, z)) {
						continue;
					}
					
					seeds.push_back(tile_pos);
				}
			}
		}
	}
	
	std::vector<Vec2i> potential_positions;
	for (int x = min_pos.x; x <= max_pos.x; ++x) {
		for (int y = min_pos.y; y <= max_pos.y; ++y) {
			potential_positions.push_back(Vec2i(x, y));
		}
	}
	
	// create initial seeds
	while (count > 0 && !potential_positions.empty()) {
		if (max_tile_quantity != 0 && tile_quantity >= max_tile_quantity) {
			break;
		}
		
		random_pos = potential_positions[SyncRand(potential_positions.size())];
		potential_positions.erase(std::remove(potential_positions.begin(), potential_positions.end(), random_pos), potential_positions.end());
		
		if (!this->Info.IsPointOnMap(random_pos, z) || this->is_point_in_a_subtemplate_area(random_pos, z)) {
			continue;
		}
		
		stratagus::terrain_type *tile_terrain = this->GetTileTerrain(random_pos, false, z);
		
		if (!generated_terrain->CanGenerateOnTile(this->Field(random_pos, z))) {
			continue;
		}
		
		if (
			(
				(
					!terrain_type->is_overlay()
					&& ((tile_terrain == terrain_type && GetTileTopTerrain(random_pos, false, z)->is_overlay()) || (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), tile_terrain) != terrain_type->BorderTerrains.end() && this->TileBordersOnlySameTerrain(random_pos, terrain_type, z)))
				)
				|| (
					terrain_type->is_overlay()
					&& stratagus::vector::contains(terrain_type->get_base_terrain_types(), tile_terrain) && this->TileBordersOnlySameTerrain(random_pos, terrain_type, z)
					&& (!GetTileTopTerrain(random_pos, false, z)->is_overlay() || GetTileTopTerrain(random_pos, false, z) == terrain_type)
				)
			)
			&& (!preserve_coastline || (terrain_type->Flags & MapFieldWaterAllowed) == (tile_terrain->Flags & MapFieldWaterAllowed))
			&& !this->TileHasUnitsIncompatibleWithTerrain(random_pos, terrain_type, z)
			&& (!(terrain_type->Flags & MapFieldUnpassable) || !this->TileBordersUnit(random_pos, z)) // if the terrain is unpassable, don't expand to spots adjacent to units
		) {
			std::vector<Vec2i> adjacent_positions;
			for (int sub_x = -1; sub_x <= 1; sub_x += 2) { // +2 so that only diagonals are used
				for (int sub_y = -1; sub_y <= 1; sub_y += 2) {
					Vec2i diagonal_pos(random_pos.x + sub_x, random_pos.y + sub_y);
					Vec2i vertical_pos(random_pos.x, random_pos.y + sub_y);
					Vec2i horizontal_pos(random_pos.x + sub_x, random_pos.y);
					if (!this->Info.IsPointOnMap(diagonal_pos, z)) {
						continue;
					}
					
					stratagus::terrain_type *diagonal_tile_terrain = this->GetTileTerrain(diagonal_pos, false, z);
					stratagus::terrain_type *vertical_tile_terrain = this->GetTileTerrain(vertical_pos, false, z);
					stratagus::terrain_type *horizontal_tile_terrain = this->GetTileTerrain(horizontal_pos, false, z);
					
					if (
						!generated_terrain->CanGenerateOnTile(this->Field(diagonal_pos, z))
						|| !generated_terrain->CanGenerateOnTile(this->Field(vertical_pos, z))
						|| !generated_terrain->CanGenerateOnTile(this->Field(horizontal_pos, z))
					) {
						continue;
					}
		
					if (
						(
							(
								!terrain_type->is_overlay()
								&& ((diagonal_tile_terrain == terrain_type && GetTileTopTerrain(diagonal_pos, false, z)->is_overlay()) || (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), diagonal_tile_terrain) != terrain_type->BorderTerrains.end() && this->TileBordersOnlySameTerrain(diagonal_pos, terrain_type, z)))
								&& ((vertical_tile_terrain == terrain_type && GetTileTopTerrain(vertical_pos, false, z)->is_overlay()) || (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), vertical_tile_terrain) != terrain_type->BorderTerrains.end() && this->TileBordersOnlySameTerrain(vertical_pos, terrain_type, z)))
								&& ((horizontal_tile_terrain == terrain_type && GetTileTopTerrain(horizontal_pos, false, z)->is_overlay()) || (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), horizontal_tile_terrain) != terrain_type->BorderTerrains.end() && this->TileBordersOnlySameTerrain(horizontal_pos, terrain_type, z)))
							)
							|| (
								terrain_type->is_overlay()
								&& stratagus::vector::contains(terrain_type->get_base_terrain_types(), diagonal_tile_terrain) && this->TileBordersOnlySameTerrain(diagonal_pos, terrain_type, z)
								&& stratagus::vector::contains(terrain_type->get_base_terrain_types(), vertical_tile_terrain) && this->TileBordersOnlySameTerrain(vertical_pos, terrain_type, z)
								&& stratagus::vector::contains(terrain_type->get_base_terrain_types(), horizontal_tile_terrain) && this->TileBordersOnlySameTerrain(horizontal_pos, terrain_type, z)
								&& (!GetTileTopTerrain(diagonal_pos, false, z)->is_overlay() || GetTileTopTerrain(diagonal_pos, false, z) == terrain_type) && (!GetTileTopTerrain(vertical_pos, false, z)->is_overlay() || GetTileTopTerrain(vertical_pos, false, z) == terrain_type) && (!GetTileTopTerrain(horizontal_pos, false, z)->is_overlay() || GetTileTopTerrain(horizontal_pos, false, z) == terrain_type)
							)
						)
						&& (!preserve_coastline || ((terrain_type->Flags & MapFieldWaterAllowed) == (diagonal_tile_terrain->Flags & MapFieldWaterAllowed) && (terrain_type->Flags & MapFieldWaterAllowed) == (vertical_tile_terrain->Flags & MapFieldWaterAllowed) && (terrain_type->Flags & MapFieldWaterAllowed) == (horizontal_tile_terrain->Flags & MapFieldWaterAllowed)))
						&& !this->TileHasUnitsIncompatibleWithTerrain(diagonal_pos, terrain_type, z) && !this->TileHasUnitsIncompatibleWithTerrain(vertical_pos, terrain_type, z) && !this->TileHasUnitsIncompatibleWithTerrain(horizontal_pos, terrain_type, z)
						&& (!(terrain_type->Flags & MapFieldUnpassable) || (!this->TileBordersUnit(diagonal_pos, z) && !this->TileBordersUnit(vertical_pos, z) && !this->TileBordersUnit(horizontal_pos, z))) // if the terrain is unpassable, don't expand to spots adjacent to buildings
						&& !this->is_point_in_a_subtemplate_area(diagonal_pos, z) && !this->is_point_in_a_subtemplate_area(vertical_pos, z) && !this->is_point_in_a_subtemplate_area(horizontal_pos, z)
					) {
						adjacent_positions.push_back(diagonal_pos);
					}
				}
			}
			
			if (adjacent_positions.size() > 0) {
				Vec2i adjacent_pos = adjacent_positions[SyncRand(adjacent_positions.size())];
				if (!terrain_type->is_overlay()) {
					this->Field(random_pos, z)->RemoveOverlayTerrain();
					this->Field(adjacent_pos, z)->RemoveOverlayTerrain();
					this->Field(Vec2i(random_pos.x, adjacent_pos.y), z)->RemoveOverlayTerrain();
					this->Field(Vec2i(adjacent_pos.x, random_pos.y), z)->RemoveOverlayTerrain();
				}
				this->Field(random_pos, z)->SetTerrain(terrain_type);
				this->Field(adjacent_pos, z)->SetTerrain(terrain_type);
				this->Field(Vec2i(random_pos.x, adjacent_pos.y), z)->SetTerrain(terrain_type);
				this->Field(Vec2i(adjacent_pos.x, random_pos.y), z)->SetTerrain(terrain_type);
				count -= 1;
				seeds.push_back(random_pos);
				seeds.push_back(adjacent_pos);
				seeds.push_back(Vec2i(random_pos.x, adjacent_pos.y));
				seeds.push_back(Vec2i(adjacent_pos.x, random_pos.y));
				
				tile_quantity += 4;
			}
		}
	}
	
	//expand seeds
	for (size_t i = 0; i < seeds.size(); ++i) {
		Vec2i seed_pos = seeds[i];
		
		if (max_tile_quantity != 0 && tile_quantity >= max_tile_quantity) {
			break;
		}
		
		const int random_number = SyncRand(100);
		if (random_number >= generated_terrain->ExpansionChance) {
			continue;
		}
		
		std::vector<Vec2i> adjacent_positions;
		for (int sub_x = -1; sub_x <= 1; sub_x += 2) { // +2 so that only diagonals are used
			for (int sub_y = -1; sub_y <= 1; sub_y += 2) {
				Vec2i diagonal_pos(seed_pos.x + sub_x, seed_pos.y + sub_y);
				Vec2i vertical_pos(seed_pos.x, seed_pos.y + sub_y);
				Vec2i horizontal_pos(seed_pos.x + sub_x, seed_pos.y);
				if (!this->Info.IsPointOnMap(diagonal_pos, z) || diagonal_pos.x < min_pos.x || diagonal_pos.y < min_pos.y || diagonal_pos.x > max_pos.x || diagonal_pos.y > max_pos.y) {
					continue;
				}

				if ( //must either be able to generate on the tiles, or they must already have the generated terrain type
					!generated_terrain->CanTileBePartOfExpansion(this->Field(diagonal_pos, z))
					|| !generated_terrain->CanTileBePartOfExpansion(this->Field(vertical_pos, z))
					|| !generated_terrain->CanTileBePartOfExpansion(this->Field(horizontal_pos, z))
				) {
					continue;
				}
		
				stratagus::terrain_type *diagonal_tile_terrain = this->GetTileTerrain(diagonal_pos, false, z);
				stratagus::terrain_type *vertical_tile_terrain = this->GetTileTerrain(vertical_pos, false, z);
				stratagus::terrain_type *horizontal_tile_terrain = this->GetTileTerrain(horizontal_pos, false, z);
				stratagus::terrain_type *diagonal_tile_top_terrain = this->GetTileTopTerrain(diagonal_pos, false, z);
				stratagus::terrain_type *vertical_tile_top_terrain = this->GetTileTopTerrain(vertical_pos, false, z);
				stratagus::terrain_type *horizontal_tile_top_terrain = this->GetTileTopTerrain(horizontal_pos, false, z);
				
				if (!terrain_type->is_overlay()) {
					if (diagonal_tile_terrain != terrain_type && (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), diagonal_tile_terrain) == terrain_type->BorderTerrains.end() || this->TileBordersTerrainIncompatibleWithTerrain(diagonal_pos, terrain_type, z))) {
						continue;
					}
					if (vertical_tile_terrain != terrain_type && (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), vertical_tile_terrain) == terrain_type->BorderTerrains.end() || this->TileBordersTerrainIncompatibleWithTerrain(vertical_pos, terrain_type, z))) {
						continue;
					}
					if (horizontal_tile_terrain != terrain_type && (std::find(terrain_type->BorderTerrains.begin(), terrain_type->BorderTerrains.end(), horizontal_tile_terrain) == terrain_type->BorderTerrains.end() || this->TileBordersTerrainIncompatibleWithTerrain(horizontal_pos, terrain_type, z))) {
						continue;
					}
				} else {
					if ((!stratagus::vector::contains(terrain_type->get_base_terrain_types(), diagonal_tile_terrain) || this->TileBordersTerrainIncompatibleWithTerrain(diagonal_pos, terrain_type, z)) && GetTileTerrain(diagonal_pos, terrain_type->is_overlay(), z) != terrain_type) {
						continue;
					}
					if ((!stratagus::vector::contains(terrain_type->get_base_terrain_types(), vertical_tile_terrain) || this->TileBordersTerrainIncompatibleWithTerrain(vertical_pos, terrain_type, z)) && GetTileTerrain(vertical_pos, terrain_type->is_overlay(), z) != terrain_type) {
						continue;
					}
					if ((!stratagus::vector::contains(terrain_type->get_base_terrain_types(), horizontal_tile_terrain) || this->TileBordersTerrainIncompatibleWithTerrain(horizontal_pos, terrain_type, z)) && GetTileTerrain(horizontal_pos, terrain_type->is_overlay(), z) != terrain_type) {
						continue;
					}
				}
				
				if (diagonal_tile_top_terrain == terrain_type && vertical_tile_top_terrain == terrain_type && horizontal_tile_top_terrain == terrain_type) { //at least one of the tiles being expanded to must be different from the terrain type
					continue;
				}
				
				//tiles within a subtemplate area can only be used as seeds, they cannot be modified themselves
				if (
					(this->is_point_in_a_subtemplate_area(diagonal_pos, z) && !generated_terrain->CanUseTileAsSeed(this->Field(diagonal_pos, z)))
					|| (this->is_point_in_a_subtemplate_area(vertical_pos, z) && !generated_terrain->CanUseTileAsSeed(this->Field(vertical_pos, z)))
					|| (this->is_point_in_a_subtemplate_area(horizontal_pos, z) && !generated_terrain->CanUseTileAsSeed(this->Field(horizontal_pos, z)))
				) {
					continue;
				}
				
				if (
					preserve_coastline
					&& (
						(terrain_type->Flags & MapFieldWaterAllowed) != (diagonal_tile_terrain->Flags & MapFieldWaterAllowed)
						|| (terrain_type->Flags & MapFieldWaterAllowed) != (vertical_tile_terrain->Flags & MapFieldWaterAllowed)
						|| (terrain_type->Flags & MapFieldWaterAllowed) != (horizontal_tile_terrain->Flags & MapFieldWaterAllowed)
					)
				) {
					continue;
				}
				
				if (this->TileHasUnitsIncompatibleWithTerrain(diagonal_pos, terrain_type, z) || this->TileHasUnitsIncompatibleWithTerrain(vertical_pos, terrain_type, z) || this->TileHasUnitsIncompatibleWithTerrain(horizontal_pos, terrain_type, z)) {
					continue;
				}
				
				if ( // if the terrain is unpassable, don't expand to spots adjacent to buildings
					(terrain_type->Flags & MapFieldUnpassable) && (this->TileBordersUnit(diagonal_pos, z) || this->TileBordersUnit(vertical_pos, z) || this->TileBordersUnit(horizontal_pos, z))
				) {
					continue;
				}
				
				//tiles with no terrain could nevertheless have units that were placed there already, e.g. due to units in a subtemplate being placed in a location where something is already present (e.g. units with a settlement set as their location, or resource units generated near the player's starting location); as such, we need to check if the terrain is compatible with those units
				if (this->TileHasUnitsIncompatibleWithTerrain(diagonal_pos, terrain_type, z) || this->TileHasUnitsIncompatibleWithTerrain(vertical_pos, terrain_type, z) || this->TileHasUnitsIncompatibleWithTerrain(horizontal_pos, terrain_type, z)) {
					continue;
				}

				adjacent_positions.push_back(diagonal_pos);
			}
		}
		
		if (adjacent_positions.size() > 0) {
			Vec2i adjacent_pos = adjacent_positions[SyncRand(adjacent_positions.size())];
			Vec2i adjacent_pos_horizontal(adjacent_pos.x, seed_pos.y);
			Vec2i adjacent_pos_vertical(seed_pos.x, adjacent_pos.y);
			
			if (!this->is_point_in_a_subtemplate_area(adjacent_pos, z) && this->GetTileTopTerrain(adjacent_pos, false, z) != terrain_type && (this->GetTileTerrain(adjacent_pos, terrain_type->is_overlay(), z) != terrain_type || generated_terrain->CanRemoveTileOverlayTerrain(this->Field(adjacent_pos, z)))) {
				if (!terrain_type->is_overlay() && generated_terrain->CanRemoveTileOverlayTerrain(this->Field(adjacent_pos, z))) {
					this->Field(adjacent_pos, z)->RemoveOverlayTerrain();
				}

				if (this->GetTileTerrain(adjacent_pos, terrain_type->is_overlay(), z) != terrain_type) {
					this->Field(adjacent_pos, z)->SetTerrain(terrain_type);
				}
				
				seeds.push_back(adjacent_pos);
				
				if (this->GetTileTopTerrain(adjacent_pos, false, z) == terrain_type) {
					tile_quantity++;
				}
			}
			
			if (!this->is_point_in_a_subtemplate_area(adjacent_pos_horizontal, z) && this->GetTileTopTerrain(adjacent_pos_horizontal, false, z) != terrain_type && (this->GetTileTerrain(adjacent_pos_horizontal, terrain_type->is_overlay(), z) != terrain_type || generated_terrain->CanRemoveTileOverlayTerrain(this->Field(adjacent_pos_horizontal, z)))) {
				if (!terrain_type->is_overlay() && generated_terrain->CanRemoveTileOverlayTerrain(this->Field(adjacent_pos_horizontal, z))) {
					this->Field(adjacent_pos_horizontal, z)->RemoveOverlayTerrain();
				}
				
				if (this->GetTileTerrain(adjacent_pos_horizontal, terrain_type->is_overlay(), z) != terrain_type) {
					this->Field(adjacent_pos_horizontal, z)->SetTerrain(terrain_type);
				}
				
				seeds.push_back(adjacent_pos_horizontal);
				
				if (this->GetTileTopTerrain(adjacent_pos_horizontal, false, z) == terrain_type) {
					tile_quantity++;
				}
			}
			
			if (!this->is_point_in_a_subtemplate_area(adjacent_pos_vertical, z) && this->GetTileTopTerrain(adjacent_pos_vertical, false, z) != terrain_type && (this->GetTileTerrain(adjacent_pos_vertical, terrain_type->is_overlay(), z) != terrain_type || generated_terrain->CanRemoveTileOverlayTerrain(this->Field(adjacent_pos_vertical, z)))) {
				if (!terrain_type->is_overlay() && generated_terrain->CanRemoveTileOverlayTerrain(this->Field(adjacent_pos_vertical, z))) {
					this->Field(adjacent_pos_vertical, z)->RemoveOverlayTerrain();
				}
				
				if (this->GetTileTerrain(adjacent_pos_vertical, terrain_type->is_overlay(), z) != terrain_type) {
					this->Field(adjacent_pos_vertical, z)->SetTerrain(terrain_type);
				}
				
				seeds.push_back(adjacent_pos_vertical);
				
				if (this->GetTileTopTerrain(adjacent_pos_vertical, false, z) == terrain_type) {
					tile_quantity++;
				}
			}
		}
	}
}

bool CMap::CanTileBePartOfMissingTerrainGeneration(const CMapField *tile, const stratagus::terrain_type *terrain_type, const stratagus::terrain_type *overlay_terrain_type) const
{
	if (tile->GetTopTerrain() == nullptr) {
		return true;
	}

	if (tile->Terrain == terrain_type && (tile->OverlayTerrain == overlay_terrain_type || overlay_terrain_type == nullptr)) {
		return true;
	}

	return false;
}

void CMap::generate_missing_terrain(const Vec2i &min_pos, const Vec2i &max_pos, const int z)
{
	if (SaveGameLoading) {
		return;
	}

	std::vector<Vec2i> seeds;

	//use tiles that have a terrain as seeds for the terrain generation
	bool has_tile_with_missing_terrain = false;
	for (int x = min_pos.x; x <= max_pos.x; ++x) {
		for (int y = min_pos.y; y <= max_pos.y; ++y) {
			const QPoint tile_pos(x, y);
			const CMapField *tile = this->Field(x, y, z);

			if (tile->GetTopTerrain() == nullptr) {
				has_tile_with_missing_terrain = true;
				continue;
			}

			if (!this->TileBordersTerrain(tile_pos, nullptr, z)) {
				continue; //the seed must border a tile with null terrain
			}

			seeds.push_back(tile_pos);
		}
	}

	if (!has_tile_with_missing_terrain) {
		return;
	}

	//expand seeds
	while (!seeds.empty()) {
		Vec2i seed_pos = seeds[SyncRand(seeds.size())];
		stratagus::vector::remove(seeds, seed_pos);

		const CMapField *seed_tile = this->Field(seed_pos, z);

		stratagus::terrain_type *terrain_type = seed_tile->Terrain;
		stratagus::terrain_type *overlay_terrain_type = seed_tile->OverlayTerrain;
		const stratagus::terrain_feature *terrain_feature = seed_tile->get_terrain_feature();

		if (overlay_terrain_type != nullptr) {
			if (
				(overlay_terrain_type->Flags & MapFieldWall)
				|| (overlay_terrain_type->Flags & MapFieldRoad)
				|| (overlay_terrain_type->Flags & MapFieldRailroad)
			) {
				overlay_terrain_type = nullptr; //don't expand overlay terrain to tiles with empty terrain if the overlay is a wall or pathway
			}
		}

		std::vector<Vec2i> adjacent_positions;
		for (int sub_x = -1; sub_x <= 1; sub_x += 2) { // +2 so that only diagonals are used
			for (int sub_y = -1; sub_y <= 1; sub_y += 2) {
				Vec2i diagonal_pos(seed_pos.x + sub_x, seed_pos.y + sub_y);
				Vec2i vertical_pos(seed_pos.x, seed_pos.y + sub_y);
				Vec2i horizontal_pos(seed_pos.x + sub_x, seed_pos.y);
				if (!this->Info.IsPointOnMap(diagonal_pos, z) || diagonal_pos.x < min_pos.x || diagonal_pos.y < min_pos.y || diagonal_pos.x > max_pos.x || diagonal_pos.y > max_pos.y) {
					continue;
				}

				if ( //must either be able to generate on the tiles, or they must already have the generated terrain type
					!this->CanTileBePartOfMissingTerrainGeneration(this->Field(diagonal_pos, z), terrain_type, overlay_terrain_type)
					|| !this->CanTileBePartOfMissingTerrainGeneration(this->Field(vertical_pos, z), terrain_type, overlay_terrain_type)
					|| !this->CanTileBePartOfMissingTerrainGeneration(this->Field(horizontal_pos, z), terrain_type, overlay_terrain_type)
					) {
					continue;
				}

				stratagus::terrain_type *diagonal_tile_top_terrain = this->GetTileTopTerrain(diagonal_pos, false, z);
				stratagus::terrain_type *vertical_tile_top_terrain = this->GetTileTopTerrain(vertical_pos, false, z);
				stratagus::terrain_type *horizontal_tile_top_terrain = this->GetTileTopTerrain(horizontal_pos, false, z);

				if (diagonal_tile_top_terrain == nullptr && this->TileBordersTerrainIncompatibleWithTerrainPair(diagonal_pos, terrain_type, overlay_terrain_type, z)) {
					continue;
				}
				if (vertical_tile_top_terrain == nullptr && this->TileBordersTerrainIncompatibleWithTerrainPair(vertical_pos, terrain_type, overlay_terrain_type, z)) {
					continue;
				}
				if (horizontal_tile_top_terrain == nullptr && this->TileBordersTerrainIncompatibleWithTerrainPair(horizontal_pos, terrain_type, overlay_terrain_type, z)) {
					continue;
				}

				if (diagonal_tile_top_terrain != nullptr && vertical_tile_top_terrain != nullptr && horizontal_tile_top_terrain != nullptr) { //at least one of the tiles being expanded to must have null terrain
					continue;
				}

				if (overlay_terrain_type != nullptr) {
					if (this->TileHasUnitsIncompatibleWithTerrain(diagonal_pos, overlay_terrain_type, z) || this->TileHasUnitsIncompatibleWithTerrain(vertical_pos, overlay_terrain_type, z) || this->TileHasUnitsIncompatibleWithTerrain(horizontal_pos, overlay_terrain_type, z)) {
						continue;
					}
				}

				adjacent_positions.push_back(diagonal_pos);
			}
		}

		if (adjacent_positions.size() > 0) {
			if (adjacent_positions.size() > 1) {
				seeds.push_back(seed_pos); //push the seed back again for another try, since it may be able to generate further terrain in the future
			}

			Vec2i adjacent_pos = adjacent_positions[SyncRand(adjacent_positions.size())];
			Vec2i adjacent_pos_horizontal(adjacent_pos.x, seed_pos.y);
			Vec2i adjacent_pos_vertical(seed_pos.x, adjacent_pos.y);

			if (this->GetTileTopTerrain(adjacent_pos, false, z) == nullptr) {
				this->Field(adjacent_pos, z)->SetTerrain(terrain_type);
				this->Field(adjacent_pos, z)->SetTerrain(overlay_terrain_type);
				if (terrain_feature != nullptr) {
					this->Field(adjacent_pos, z)->set_terrain_feature(terrain_feature);
				}
				seeds.push_back(adjacent_pos);
			}

			if (this->GetTileTopTerrain(adjacent_pos_horizontal, false, z) == nullptr) {
				this->Field(adjacent_pos_horizontal, z)->SetTerrain(terrain_type);
				this->Field(adjacent_pos_horizontal, z)->SetTerrain(overlay_terrain_type);
				if (terrain_feature != nullptr) {
					this->Field(adjacent_pos_horizontal, z)->set_terrain_feature(terrain_feature);
				}
				seeds.push_back(adjacent_pos_horizontal);
			}

			if (this->GetTileTopTerrain(adjacent_pos_vertical, false, z) == nullptr) {
				this->Field(adjacent_pos_vertical, z)->SetTerrain(terrain_type);
				this->Field(adjacent_pos_vertical, z)->SetTerrain(overlay_terrain_type);
				if (terrain_feature != nullptr) {
					this->Field(adjacent_pos_vertical, z)->set_terrain_feature(terrain_feature);
				}
				seeds.push_back(adjacent_pos_vertical);
			}
		}
	}

	//set the terrain of the remaining tiles without any to their most-neighbored terrain/overlay terrain pair
	for (int x = min_pos.x; x <= max_pos.x; ++x) {
		for (int y = min_pos.y; y <= max_pos.y; ++y) {
			const Vec2i tile_pos(x, y);

			CMapField *tile = this->Field(x, y, z);

			if (tile->GetTopTerrain() != nullptr) {
				continue;
			}

			std::map<std::pair<stratagus::terrain_type *, stratagus::terrain_type *>, int> terrain_type_pair_neighbor_count;

			for (int x_offset = -1; x_offset <= 1; ++x_offset) {
				for (int y_offset = -1; y_offset <= 1; ++y_offset) {
					if (x_offset == 0 && y_offset == 0) {
						continue;
					}

					Vec2i adjacent_pos(tile_pos.x + x_offset, tile_pos.y + y_offset);

					if (!this->Info.IsPointOnMap(adjacent_pos, z)) {
						continue;
					}

					const CMapField *adjacent_tile = this->Field(adjacent_pos, z);
					stratagus::terrain_type *adjacent_terrain_type = adjacent_tile->GetTerrain(false);
					stratagus::terrain_type *adjacent_overlay_terrain_type = adjacent_tile->GetTerrain(true);

					if (adjacent_terrain_type == nullptr) {
						continue;
					}

					std::pair<stratagus::terrain_type *, stratagus::terrain_type *> terrain_type_pair(adjacent_terrain_type, adjacent_overlay_terrain_type);

					auto find_iterator = terrain_type_pair_neighbor_count.find(terrain_type_pair);
					if (find_iterator == terrain_type_pair_neighbor_count.end()) {
						terrain_type_pair_neighbor_count[terrain_type_pair] = 1;
					} else {
						find_iterator->second++;
					}
				}
			}

			std::pair<stratagus::terrain_type *, stratagus::terrain_type *> best_terrain_type_pair(nullptr, nullptr);
			int best_terrain_type_neighbor_count = 0;
			for (const auto &element : terrain_type_pair_neighbor_count) {
				if (element.second > best_terrain_type_neighbor_count) {
					best_terrain_type_pair = element.first;
					best_terrain_type_neighbor_count = element.second;
				}
			}

			//set the terrain and overlay terrain to the same as the most-neighbored one
			tile->SetTerrain(best_terrain_type_pair.first);
			tile->SetTerrain(best_terrain_type_pair.second);
		}
	}
}

void CMap::generate_settlement_territories(const int z)
{
	if (SaveGameLoading) {
		return;
	}

	stratagus::point_set seeds;

	for (int x = 0; x < this->Info.MapWidths[z]; ++x) {
		for (int y = 0; y < this->Info.MapHeights[z]; ++y) {
			QPoint tile_pos(x, y);

			if (this->Field(tile_pos, z)->get_settlement() == nullptr) {
				continue;
			}

			if (!this->tile_borders_other_settlement_territory(tile_pos, z)) {
				continue;
			}

			seeds.insert(std::move(tile_pos));
		}
	}

	seeds = this->expand_settlement_territories(stratagus::container::to_vector(seeds), z, (MapFieldUnpassable | MapFieldCoastAllowed | MapFieldSpace), MapFieldUnderground);
	seeds = this->expand_settlement_territories(stratagus::container::to_vector(seeds), z, (MapFieldCoastAllowed | MapFieldSpace), MapFieldUnderground);
	seeds = this->expand_settlement_territories(stratagus::container::to_vector(seeds), z, MapFieldSpace, MapFieldUnderground);
	seeds = this->expand_settlement_territories(stratagus::container::to_vector(seeds), z, MapFieldSpace);
	this->expand_settlement_territories(stratagus::container::to_vector(seeds), z);

	//set the settlement of the remaining tiles without any to their most-neighbored settlement
	for (int x = 0; x < this->Info.MapWidths[z]; ++x) {
		for (int y = 0; y < this->Info.MapHeights[z]; ++y) {
			const QPoint tile_pos(x, y);

			CMapField *tile = this->Field(x, y, z);

			if (tile->get_settlement() != nullptr) {
				continue;
			}

			std::map<stratagus::site *, int> settlement_neighbor_count;

			for (int x_offset = -1; x_offset <= 1; ++x_offset) {
				for (int y_offset = -1; y_offset <= 1; ++y_offset) {
					if (x_offset == 0 && y_offset == 0) {
						continue;
					}

					QPoint adjacent_pos(tile_pos.x() + x_offset, tile_pos.y() + y_offset);

					if (!this->Info.IsPointOnMap(adjacent_pos, z)) {
						continue;
					}

					const CMapField *adjacent_tile = this->Field(adjacent_pos, z);
					stratagus::site *adjacent_settlement = adjacent_tile->get_settlement();

					if (adjacent_settlement == nullptr) {
						continue;
					}

					settlement_neighbor_count[adjacent_settlement]++;
				}
			}

			stratagus::site *best_settlement = nullptr;
			int best_settlement_neighbor_count = 0;
			for (const auto &kv_pair : settlement_neighbor_count) {
				if (kv_pair.second > best_settlement_neighbor_count) {
					best_settlement = kv_pair.first;
					best_settlement_neighbor_count = kv_pair.second;
				}
			}

			//set the settlement to the same as the most-neighbored one
			tile->set_settlement(best_settlement);
		}
	}

	this->calculate_settlement_territory_border_tiles(z);
}

stratagus::point_set CMap::expand_settlement_territories(std::vector<QPoint> &&seeds, const int z, const int block_flags, const int same_flags)
{
	//the seeds blocked by the block flags are stored, and then returned by the function
	stratagus::point_set blocked_seeds;

	//expand seeds
	while (!seeds.empty()) {
		const QPoint seed_pos = seeds[SyncRand(seeds.size())];
		stratagus::vector::remove(seeds, seed_pos);

		const CMapField *seed_tile = this->Field(seed_pos, z);

		//tiles with a block flag can be expanded to, but they can't serve as a basis for further expansion
		if (seed_tile->CheckMask(block_flags)) {
			blocked_seeds.insert(seed_pos);
			continue;
		}

		stratagus::site *settlement = seed_tile->get_settlement();
		const CMapField *settlement_tile = this->Field(settlement->get_site_unit()->get_center_tile_pos(), z);

		std::vector<QPoint> adjacent_positions;
		for (int sub_x = -1; sub_x <= 1; sub_x += 2) { // +2 so that only diagonals are used
			for (int sub_y = -1; sub_y <= 1; sub_y += 2) {
				const QPoint diagonal_pos(seed_pos.x() + sub_x, seed_pos.y() + sub_y);
				const QPoint vertical_pos(seed_pos.x(), seed_pos.y() + sub_y);
				const QPoint horizontal_pos(seed_pos.x() + sub_x, seed_pos.y());

				if (!this->Info.IsPointOnMap(diagonal_pos, z)) {
					continue;
				}

				CMapField *diagonal_tile = this->Field(diagonal_pos, z);
				CMapField *vertical_tile = this->Field(vertical_pos, z);
				CMapField *horizontal_tile = this->Field(horizontal_pos, z);

				if ( //the tiles must either have no settlement, or have the settlement we want to assign
					(diagonal_tile->get_settlement() != nullptr && diagonal_tile->get_settlement() != settlement)
					|| (vertical_tile->get_settlement() != nullptr && vertical_tile->get_settlement() != settlement)
					|| (horizontal_tile->get_settlement() != nullptr && horizontal_tile->get_settlement() != settlement)
					) {
					continue;
				}

				if (diagonal_tile->get_settlement() != nullptr && vertical_tile->get_settlement() != nullptr && horizontal_tile->get_settlement() != nullptr) { //at least one of the tiles being expanded to must have no assigned settlement
					continue;
				}

				//the same flags function similarly to the block flags, but block only if the tile does not contain the same same_flags as the settlement's original tile, and they block expansion to the tile itself, not just expansion from it
				if ((diagonal_tile->Flags & same_flags) != (settlement_tile->Flags & same_flags) || (vertical_tile->Flags & same_flags) != (settlement_tile->Flags & same_flags) || (horizontal_tile->Flags & same_flags) != (settlement_tile->Flags & same_flags)) {
					blocked_seeds.insert(seed_pos);
					continue;
				}

				adjacent_positions.push_back(diagonal_pos);
			}
		}

		if (adjacent_positions.size() > 0) {
			if (adjacent_positions.size() > 1) {
				seeds.push_back(seed_pos); //push the seed back again for another try, since it may be able to generate further terrain in the future
			}

			QPoint adjacent_pos = adjacent_positions[SyncRand(adjacent_positions.size())];
			QPoint adjacent_pos_horizontal(adjacent_pos.x(), seed_pos.y());
			QPoint adjacent_pos_vertical(seed_pos.x(), adjacent_pos.y());

			this->Field(adjacent_pos, z)->set_settlement(settlement);
			this->Field(adjacent_pos_horizontal, z)->set_settlement(settlement);
			this->Field(adjacent_pos_vertical, z)->set_settlement(settlement);

			seeds.push_back(adjacent_pos);
			seeds.push_back(adjacent_pos_horizontal);
			seeds.push_back(adjacent_pos_vertical);
		}
	}

	return blocked_seeds;
}

void CMap::calculate_settlement_territory_border_tiles(const int z)
{
	for (const CUnit *site_unit : this->site_units) {
		if (site_unit->MapLayer->ID != z) {
			continue;
		}

		site_unit->settlement->clear_border_tiles();
	}

	for (int x = 0; x < this->Info.MapWidths[z]; ++x) {
		for (int y = 0; y < this->Info.MapHeights[z]; ++y) {
			const QPoint tile_pos(x, y);
			if (this->tile_borders_other_settlement_territory(tile_pos, z)) {
				const CMapField *tile = this->Field(x, y, z);
				stratagus::site *settlement = tile->get_settlement();
				if (settlement != nullptr) {
					settlement->add_border_tile(tile_pos);
				}
			}
		}
	}
}

void CMap::GenerateNeutralUnits(stratagus::unit_type *unit_type, int quantity, const Vec2i &min_pos, const Vec2i &max_pos, bool grouped, int z)
{
	if (SaveGameLoading) {
		return;
	}
	
	Vec2i unit_pos(-1, -1);
	
	for (int i = 0; i < quantity; ++i) {
		if (i == 0 || !grouped) {
			unit_pos = GenerateUnitLocation(unit_type, nullptr, min_pos, max_pos, z);
		}
		if (!this->Info.IsPointOnMap(unit_pos, z)) {
			continue;
		}
		if (unit_type->GivesResource) {
			CUnit *unit = CreateResourceUnit(unit_pos, *unit_type, z);
		} else {
			CUnit *unit = CreateUnit(unit_pos, *unit_type, CPlayer::Players[PlayerNumNeutral], z, unit_type->BoolFlag[BUILDING_INDEX].value && unit_type->get_tile_width() > 1 && unit_type->get_tile_height() > 1);
		}
	}
}
//Wyrmgus end

//Wyrmgus start
void CMap::ClearOverlayTile(const Vec2i &pos, int z)
{
	CMapField &mf = *this->Field(pos, z);

	if (!mf.OverlayTerrain) {
		return;
	}
	
	this->SetOverlayTerrainDestroyed(pos, true, z);

	//remove decorations if a wall, tree or rock was removed from the tile
	std::vector<CUnit *> table;
	Select(pos, pos, table, z);
	for (size_t i = 0; i != table.size(); ++i) {
		if (table[i]->Type->UnitType == UnitTypeType::Land && table[i]->Type->BoolFlag[DECORATION_INDEX].value) {
			if (Editor.Running == EditorNotRunning) {
				LetUnitDie(*table[i]);			
			} else {
				EditorActionRemoveUnit(*table[i], false);
			}
		}
	}

	//check if any further tile should be removed with the clearing of this one
	if (!mf.OverlayTerrain->allows_single()) {
		for (int x_offset = -1; x_offset <= 1; ++x_offset) {
			for (int y_offset = -1; y_offset <= 1; ++y_offset) {
				if (x_offset != 0 || y_offset != 0) {
					Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
					if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
						CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
						
						if (adjacent_mf.OverlayTerrain == mf.OverlayTerrain && !adjacent_mf.OverlayTerrainDestroyed && !this->CurrentTerrainCanBeAt(adjacent_pos, true, z)) {
							this->ClearOverlayTile(adjacent_pos, z);
						}
					}
				}
			}
		}
	}
}
//Wyrmgus end

//Wyrmgus start
/*
/// Remove wood from the map.
void CMap::ClearWoodTile(const Vec2i &pos)
{
	CMapField &mf = *this->Field(pos);

	mf.setGraphicTile(this->Tileset->getRemovedTreeTile());
	mf.Flags &= ~(MapFieldForest | MapFieldUnpassable);
	mf.Value = 0;

	UI.Minimap.UpdateXY(pos);
	FixNeighbors(MapFieldForest, 0, pos);

	//maybe isExplored
	if (mf.playerInfo.IsExplored(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		MarkSeenTile(mf);
	}
}

/// Remove rock from the map.
void CMap::ClearRockTile(const Vec2i &pos)
{
	CMapField &mf = *this->Field(pos);

	mf.setGraphicTile(this->Tileset->getRemovedRockTile());
	mf.Flags &= ~(MapFieldRocks | MapFieldUnpassable);
	mf.Value = 0;
	
	UI.Minimap.UpdateXY(pos);
	FixNeighbors(MapFieldRocks, 0, pos);

	//maybe isExplored
	if (mf.playerInfo.IsExplored(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		MarkSeenTile(mf);
	}
}
*/
//Wyrmgus end

/**
**	@brief	Regenerate forest.
*/
void CMap::RegenerateForest()
{
	if (!ForestRegeneration) {
		return;
	}

	for (CMapLayer *map_layer : this->MapLayers) {
		map_layer->RegenerateForest();
	}
}


/**
**  Load the map presentation
**
**  @param mapname  map filename
*/
void LoadStratagusMapInfo(const std::string &mapname)
{
	// Set the default map setup by replacing .smp with .sms
	size_t loc = mapname.find(".smp");
	if (loc != std::string::npos) {
		CMap::Map.Info.Filename = mapname;
		CMap::Map.Info.Filename.replace(loc, 4, ".sms");
	}

	const std::string filename = LibraryFileName(mapname.c_str());
	LuaLoadFile(filename);
}
