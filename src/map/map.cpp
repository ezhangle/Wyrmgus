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
/**@name map.cpp - The map source file. */
//
//      (c) Copyright 1998-2018 by Lutz Sammer, Vladi Shabanski,
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "map/map.h"

//Wyrmgus start
#include <fstream>
//Wyrmgus end

#include "calendar/calendar.h"
//Wyrmgus start
#include "editor.h"
#include "game.h" // for the SaveGameLoading variable
//Wyrmgus end
#include "iolib.h"
#include "map/map_layer.h"
#include "map/map_template.h"
#include "map/terrain_type.h"
#include "map/tileset.h"
#include "plane.h"
#include "player.h"
//Wyrmgus start
#include "province.h"
#include "quest.h"
#include "settings.h"
#include "sound_server.h"
//Wyrmgus end
//Wyrmgus start
#include "translate.h"
//Wyrmgus end
#include "ui/ui.h"
#include "unit.h"
//Wyrmgus start
#include "unit_find.h"
//Wyrmgus end
#include "unit_manager.h"
//Wyrmgus start
#include "upgrade.h"
//Wyrmgus end
#include "version.h"
#include "video.h"
#include "world.h"

#ifdef USE_OAML
#include <oaml.h>

extern oamlApi *oaml;
extern bool enableOAML;
#endif

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

//Wyrmgus start
std::vector<CSite *> Sites;
std::map<std::string, CSite *> SiteIdentToPointer;
std::vector<CTerrainFeature *> TerrainFeatures;
std::map<std::string, CTerrainFeature *> TerrainFeatureIdentToPointer;
std::map<std::tuple<int, int, int>, int> TerrainFeatureColorToIndex;
//Wyrmgus end
CMap Map;                   /// The current map
int FlagRevealMap;          /// Flag must reveal the map
int ReplayRevealMap;        /// Reveal Map is replay
int ForestRegeneration;     /// Forest regeneration
char CurrentMapPath[1024];  /// Path of the current map

//Wyrmgus start
/**
**  Get a site
*/
CSite *GetSite(std::string site_ident)
{
	if (site_ident.empty()) {
		return nullptr;
	}
	
	std::map<std::string, CSite *>::const_iterator find_iterator = SiteIdentToPointer.find(site_ident);
	
	if (find_iterator != SiteIdentToPointer.end()) {
		return find_iterator->second;
	}
	
	return nullptr;
}

/**
**  Get a terrain feature
*/
CTerrainFeature *GetTerrainFeature(std::string terrain_feature_ident)
{
	if (terrain_feature_ident.empty()) {
		return nullptr;
	}
	
	std::map<std::string, CTerrainFeature *>::const_iterator find_iterator = TerrainFeatureIdentToPointer.find(terrain_feature_ident);
	
	if (find_iterator != TerrainFeatureIdentToPointer.end()) {
		return find_iterator->second;
	}
	
	return nullptr;
}

std::string GetDegreeLevelNameById(int degree_level)
{
	if (degree_level == ExtremelyHighDegreeLevel) {
		return "extremely-high";
	} else if (degree_level == VeryHighDegreeLevel) {
		return "very-high";
	} else if (degree_level == HighDegreeLevel) {
		return "high";
	} else if (degree_level == MediumDegreeLevel) {
		return "medium";
	} else if (degree_level == LowDegreeLevel) {
		return "low";
	} else if (degree_level == VeryLowDegreeLevel) {
		return "very-low";
	}
	return "";
}

int GetDegreeLevelIdByName(std::string degree_level)
{
	if (degree_level == "extremely-high") {
		return ExtremelyHighDegreeLevel;
	} else if (degree_level == "very-high") {
		return VeryHighDegreeLevel;
	} else if (degree_level == "high") {
		return HighDegreeLevel;
	} else if (degree_level == "medium") {
		return MediumDegreeLevel;
	} else if (degree_level == "low") {
		return LowDegreeLevel;
	} else if (degree_level == "very-low") {
		return VeryLowDegreeLevel;
	} else {
		return -1;
	}
}

/**
**  Get a site's cultural name.
*/
std::string CSite::GetCulturalName(int civilization)
{
	if (civilization != -1 && this->CulturalNames.find(civilization) != this->CulturalNames.end()) {
		return this->CulturalNames[civilization];
	} else {
		return this->Name;
	}
}
//Wyrmgus end

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
	const unsigned int index = &mf - Map.MapLayers[z]->Fields;
	const int y = index / Info.MapWidths[z];
	const int x = index - (y * Info.MapWidths[z]);
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
				if (Players[p].Type == PlayerPerson || !only_person_players) {
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
//				if (Players[p].Type != PlayerNobody && (!(unit.Seen.ByPlayer & (1 << p)))) {
				if (Players[p].Type != PlayerNobody && (Players[p].Type == PlayerPerson || !only_person_players) && (!(unit.Seen.ByPlayer & (1 << p)))) {
				//Wyrmgus end
					UnitGoesOutOfFog(unit, Players[p]);
					UnitGoesUnderFog(unit, Players[p]);
				}
			}
		}
		UnitCountSeen(unit);
	}
}

/*----------------------------------------------------------------------------
--  Map queries
----------------------------------------------------------------------------*/

Vec2i CMap::MapPixelPosToTilePos(const PixelPos &mapPos, const int map_layer) const
{
	const Vec2i tilePos(mapPos.x / GetMapLayerPixelTileSize(map_layer).x, mapPos.y / GetMapLayerPixelTileSize(map_layer).y);

	return tilePos;
}

PixelPos CMap::TilePosToMapPixelPos_TopLeft(const Vec2i &tilePos, const CMapLayer *map_layer) const
{
	PixelPos mapPixelPos(tilePos.x * GetMapLayerPixelTileSize(map_layer ? map_layer->ID : -1).x, tilePos.y * GetMapLayerPixelTileSize(map_layer ? map_layer->ID : -1).y);

	return mapPixelPos;
}

PixelPos CMap::TilePosToMapPixelPos_Center(const Vec2i &tilePos, const CMapLayer *map_layer) const
{
	return TilePosToMapPixelPos_TopLeft(tilePos, map_layer) + GetMapLayerPixelTileSize(map_layer ? map_layer->ID : -1) / 2;
}

//Wyrmgus start
CTerrainType *CMap::GetTileTerrain(const Vec2i &pos, bool overlay, int z) const
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		return nullptr;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	if (overlay) {
		return mf.OverlayTerrain;
	} else {
		return mf.Terrain;
	}
}

CTerrainType *CMap::GetTileTopTerrain(const Vec2i &pos, bool seen, int z, bool ignore_destroyed) const
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		return nullptr;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	if (!seen) {
		if (mf.OverlayTerrain && (!ignore_destroyed || !mf.OverlayTerrainDestroyed)) {
			return mf.OverlayTerrain;
		} else {
			return mf.Terrain;
		}
	} else {
		if (mf.playerInfo.SeenOverlayTerrain) {
			return mf.playerInfo.SeenOverlayTerrain;
		} else {
			return mf.playerInfo.SeenTerrain;
		}
	}
}

int CMap::GetTileLandmass(const Vec2i &pos, int z) const
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		return 0;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	return mf.Landmass;
}

Vec2i CMap::GenerateUnitLocation(const CUnitType *unit_type, CFaction *faction, Vec2i min_pos, Vec2i max_pos, int z) const
{
	if (SaveGameLoading) {
		return Vec2i(-1, -1);
	}
	
	CPlayer *player = GetFactionPlayer(faction);
	if (!unit_type->BoolFlag[TOWNHALL_INDEX].value && player != nullptr && player->StartMapLayer == z && player->StartPos.x >= min_pos.x && player->StartPos.x <= max_pos.x && player->StartPos.y >= min_pos.y && player->StartPos.y <= max_pos.y) {
		return player->StartPos;
	}
	
	Vec2i random_pos(-1, -1);
	
	std::vector<CTerrainType *> allowed_terrains;
	if (unit_type->BoolFlag[FAUNA_INDEX].value && unit_type->Species) { //if the unit is a fauna one, it has to start on terrain it is native to
		for (size_t i = 0; i < unit_type->Species->Terrains.size(); ++i) {
			allowed_terrains.push_back(unit_type->Species->Terrains[i]);
		}
	}
	
	for (size_t i = 0; i < unit_type->SpawnUnits.size(); ++i) {
		CUnitType *spawned_type = unit_type->SpawnUnits[i];
		if (spawned_type->BoolFlag[FAUNA_INDEX].value && spawned_type->Species) {
			for (size_t j = 0; j < spawned_type->Species->Terrains.size(); ++j) {
				allowed_terrains.push_back(spawned_type->Species->Terrains[j]);
			}
		}
	}

	int while_count = 0;
	
	while (while_count < 100) {
		random_pos.x = SyncRand(max_pos.x - (unit_type->TileSize.x - 1) - min_pos.x + 1) + min_pos.x;
		random_pos.y = SyncRand(max_pos.y - (unit_type->TileSize.y - 1) - min_pos.y + 1) + min_pos.y;
		
		if (!this->Info.IsPointOnMap(random_pos, z) || (this->IsPointInASubtemplateArea(random_pos, z) && GameCycle == 0)) {
			continue;
		}
		
		if (allowed_terrains.size() > 0 && std::find(allowed_terrains.begin(), allowed_terrains.end(), GetTileTopTerrain(random_pos, false, z)) == allowed_terrains.end()) { //if the unit is a fauna one, it has to start on terrain it is native to
			while_count += 1;
			continue;
		}
		
		std::vector<CUnit *> table;
		if (player != nullptr) {
			Select(random_pos - Vec2i(32, 32), random_pos + Vec2i(unit_type->TileSize.x - 1, unit_type->TileSize.y - 1) + Vec2i(32, 32), table, z, MakeAndPredicate(HasNotSamePlayerAs(*player), HasNotSamePlayerAs(Players[PlayerNumNeutral])));
		} else if (!unit_type->GivesResource) {
			if (unit_type->BoolFlag[PREDATOR_INDEX].value || (unit_type->BoolFlag[PEOPLEAVERSION_INDEX].value && unit_type->UnitType == UnitTypeFly)) {
				Select(random_pos - Vec2i(16, 16), random_pos + Vec2i(unit_type->TileSize.x - 1, unit_type->TileSize.y - 1) + Vec2i(16, 16), table, z, MakeOrPredicate(HasNotSamePlayerAs(Players[PlayerNumNeutral]), HasSameTypeAs(*SettlementSiteUnitType)));
			} else {
				Select(random_pos - Vec2i(8, 8), random_pos + Vec2i(unit_type->TileSize.x - 1, unit_type->TileSize.y - 1) + Vec2i(8, 8), table, z, HasNotSamePlayerAs(Players[PlayerNumNeutral]));
			}
		} else if (unit_type->GivesResource && !unit_type->BoolFlag[BUILDING_INDEX].value) { //for non-building resources (i.e. wood piles), place them within a certain distance of player units, to prevent them from blocking the way
			Select(random_pos - Vec2i(4, 4), random_pos + Vec2i(unit_type->TileSize.x - 1, unit_type->TileSize.y - 1) + Vec2i(4, 4), table, z, HasNotSamePlayerAs(Players[PlayerNumNeutral]));
		}
		
		if (table.size() == 0) {
			bool passable_surroundings = true; //check if the unit won't be placed next to unpassable terrain
			for (int x = random_pos.x - 1; x < random_pos.x + unit_type->TileSize.x + 1; ++x) {
				for (int y = random_pos.y - 1; y < random_pos.y + unit_type->TileSize.y + 1; ++y) {
					if (Map.Info.IsPointOnMap(x, y, z) && Map.Field(x, y, z)->CheckMask(MapFieldUnpassable)) {
						passable_surroundings = false;
					}
				}
			}
			if (passable_surroundings && UnitTypeCanBeAt(*unit_type, random_pos, z) && (!unit_type->BoolFlag[BUILDING_INDEX].value || CanBuildUnitType(nullptr, *unit_type, random_pos, 0, true, z))) {
				return random_pos;
			}
		}
		
		while_count += 1;
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
//Wyrmgus start
//bool CMap::WallOnMap(const Vec2i &pos) const
bool CMap::WallOnMap(const Vec2i &pos, int z) const
//Wyrmgus end
{
	//Wyrmgus start
//	Assert(Map.Info.IsPointOnMap(pos));
//	return Field(pos)->isAWall();
	Assert(Map.Info.IsPointOnMap(pos, z));
	return Field(pos, z)->isAWall();
	//Wyrmgus end
}

//Wyrmgus start
bool CMap::CurrentTerrainCanBeAt(const Vec2i &pos, bool overlay, int z)
{
	CMapField &mf = *this->Field(pos, z);
	CTerrainType *terrain = nullptr;
	
	if (overlay) {
		terrain = mf.OverlayTerrain;
	} else {
		terrain = mf.Terrain;
	}
	
	if (!terrain) {
		return true;
	}
	
	if (terrain->AllowSingle) {
		return true;
	}

	std::vector<int> transition_directions;
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
						
					CTerrainType *adjacent_terrain = this->GetTileTerrain(adjacent_pos, overlay, z);
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

bool CMap::TileBordersOnlySameTerrain(const Vec2i &pos, CTerrainType *new_terrain, int z)
{
	for (int sub_x = -1; sub_x <= 1; ++sub_x) {
		for (int sub_y = -1; sub_y <= 1; ++sub_y) {
			Vec2i adjacent_pos(pos.x + sub_x, pos.y + sub_y);
			if (!this->Info.IsPointOnMap(adjacent_pos, z) || (sub_x == 0 && sub_y == 0)) {
				continue;
			}
			if (this->IsPointInASubtemplateArea(pos, z) && !this->IsPointInASubtemplateArea(adjacent_pos, z)) {
				continue;
			}
			CTerrainType *top_terrain = GetTileTopTerrain(pos, false, z);
			CTerrainType *adjacent_top_terrain = GetTileTopTerrain(adjacent_pos, false, z);
			if (!new_terrain->Overlay) {
				if (
					adjacent_top_terrain
					&& adjacent_top_terrain != top_terrain
					&& (std::find(top_terrain->InnerBorderTerrains.begin(), top_terrain->InnerBorderTerrains.end(), adjacent_top_terrain) == top_terrain->InnerBorderTerrains.end() || std::find(new_terrain->InnerBorderTerrains.begin(), new_terrain->InnerBorderTerrains.end(), adjacent_top_terrain) == new_terrain->InnerBorderTerrains.end())
					&& adjacent_top_terrain != new_terrain
				) {
					return false;
				}
			} else {
				if (
					adjacent_top_terrain
					&& adjacent_top_terrain != top_terrain
					&& std::find(top_terrain->BaseTerrainTypes.begin(), top_terrain->BaseTerrainTypes.end(), adjacent_top_terrain) == top_terrain->BaseTerrainTypes.end() && std::find(adjacent_top_terrain->BaseTerrainTypes.begin(), adjacent_top_terrain->BaseTerrainTypes.end(), top_terrain) == adjacent_top_terrain->BaseTerrainTypes.end()
					&& adjacent_top_terrain != new_terrain
				) {
					return false;
				}
			}
		}
	}
		
	return true;
}

bool CMap::TileBordersFlag(const Vec2i &pos, int z, int flag, bool reverse)
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

bool CMap::TileHasUnitsIncompatibleWithTerrain(const Vec2i &pos, CTerrainType *terrain, int z)
{
	CMapField &mf = *Map.Field(pos, z);
	
	const CUnitCache &cache = mf.UnitCache;
	for (size_t i = 0; i != cache.size(); ++i) {
		CUnit &unit = *cache[i];
		if (unit.IsAliveOnMap() && (terrain->Flags & unit.Type->MovementMask) != 0) {
			return true;
		}
	}

	return false;
}

bool CMap::IsPointInASubtemplateArea(const Vec2i &pos, int z) const
{
	for (size_t i = 0; i < this->MapLayers[z]->SubtemplateAreas.size(); ++i) {
		Vec2i min_pos = std::get<0>(this->MapLayers[z]->SubtemplateAreas[i]);
		Vec2i max_pos = std::get<1>(this->MapLayers[z]->SubtemplateAreas[i]);
		if (pos.x >= min_pos.x && pos.y >= min_pos.y && pos.x <= max_pos.x && pos.y <= max_pos.y) {
			return true;
		}
	}

	return false;
}

bool CMap::IsLayerUnderground(int z) const
{
	if (GameSettings.Inside) {
		return true;
	}
	
	if (this->MapLayers[z]->SurfaceLayer > 0) {
		return true;
	}

	return false;
}

void CMap::SetCurrentPlane(CPlane *plane)
{
	if (UI.CurrentMapLayer->Plane == plane) {
		return;
	}
	
	int map_layer = -1;
	
	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		if (Map.MapLayers[z]->Plane == plane && Map.MapLayers[z]->SurfaceLayer == this->GetCurrentSurfaceLayer()) {
			map_layer = z;
			break;
		}
	}
	
	if (map_layer == -1) {
		for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
			if (Map.MapLayers[z]->Plane == plane) {
				map_layer = z;
				break;
			}
		}
	}
	
	if (map_layer != -1) {
		ChangeCurrentMapLayer(map_layer);
	}
}

void CMap::SetCurrentWorld(CWorld *world)
{
	if (UI.CurrentMapLayer->World == world) {
		return;
	}
	
	int map_layer = -1;
	
	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		if (Map.MapLayers[z]->World == world && Map.MapLayers[z]->SurfaceLayer == this->GetCurrentSurfaceLayer()) {
			map_layer = z;
			break;
		}
	}
	
	if (map_layer == -1) {
		for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
			if (Map.MapLayers[z]->World == world) {
				map_layer = z;
				break;
			}
		}
	}
	
	if (map_layer != -1) {
		ChangeCurrentMapLayer(map_layer);
	}
}

void CMap::SetCurrentSurfaceLayer(int surface_layer)
{
	if (UI.CurrentMapLayer->SurfaceLayer == surface_layer) {
		return;
	}
	
	int map_layer = -1;
	
	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		if (Map.MapLayers[z]->Plane == this->GetCurrentPlane() && Map.MapLayers[z]->World == this->GetCurrentWorld() && Map.MapLayers[z]->SurfaceLayer == surface_layer) {
			map_layer = z;
			break;
		}
	}
	
	if (map_layer != -1) {
		ChangeCurrentMapLayer(map_layer);
	}
}

CPlane *CMap::GetCurrentPlane() const
{
	if (UI.CurrentMapLayer) {
		return UI.CurrentMapLayer->Plane;
	} else {
		return nullptr;
	}
}

CWorld *CMap::GetCurrentWorld() const
{
	if (UI.CurrentMapLayer) {
		return UI.CurrentMapLayer->World;
	} else {
		return nullptr;
	}
}

int CMap::GetCurrentSurfaceLayer() const
{
	if (UI.CurrentMapLayer) {
		return UI.CurrentMapLayer->SurfaceLayer;
	} else {
		return 0;
	}
}

PixelSize CMap::GetCurrentPixelTileSize() const
{
	if (UI.CurrentMapLayer) {
		return UI.CurrentMapLayer->PixelTileSize;
	} else {
		return PixelSize(32, 32);
	}
}

PixelSize CMap::GetMapLayerPixelTileSize(int map_layer) const
{
	if (map_layer >= 0 && map_layer < (int) Map.MapLayers.size()) {
		return Map.MapLayers[map_layer]->PixelTileSize;
	} else {
		return PixelSize(32, 32);
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
//Wyrmgus start
//bool CheckedCanMoveToMask(const Vec2i &pos, int mask)
bool CheckedCanMoveToMask(const Vec2i &pos, int mask, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	return Map.Info.IsPointOnMap(pos) && CanMoveToMask(pos, mask);
	return Map.Info.IsPointOnMap(pos, z) && CanMoveToMask(pos, mask, z);
	//Wyrmgus end
}

/**
**  Can a unit of unit-type be placed at this point.
**
**  @param type  unit-type to be checked.
**  @param pos   map tile position.
**
**  @return      True if could be entered, false otherwise.
*/
//Wyrmgus start
//bool UnitTypeCanBeAt(const CUnitType &type, const Vec2i &pos)
bool UnitTypeCanBeAt(const CUnitType &type, const Vec2i &pos, int z)
//Wyrmgus end
{
	const int mask = type.MovementMask;
	//Wyrmgus start
//	unsigned int index = pos.y * Map.Info.MapWidth;
	unsigned int index = pos.y * Map.Info.MapWidths[z];
	//Wyrmgus end

	for (int addy = 0; addy < type.TileSize.y; ++addy) {
		for (int addx = 0; addx < type.TileSize.x; ++addx) {
			//Wyrmgus start
			/*
			if (Map.Info.IsPointOnMap(pos.x + addx, pos.y + addy) == false
				|| Map.Field(pos.x + addx + index)->CheckMask(mask) == true) {
			*/
			if (Map.Info.IsPointOnMap(pos.x + addx, pos.y + addy, z) == false
				|| Map.Field(pos.x + addx + index, z)->CheckMask(mask) == true) {
			//Wyrmgus end
				return false;
			}
		}
		//Wyrmgus start
//		index += Map.Info.MapWidth;
		index += Map.Info.MapWidths[z];
		//Wyrmgus end
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
	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		for (int ix = 0; ix < Map.Info.MapWidths[z]; ++ix) {
			for (int iy = 0; iy < Map.Info.MapHeights[z]; ++iy) {
				CMapField &mf = *Map.Field(ix, iy, z);
				Map.CalculateTileTransitions(Vec2i(ix, iy), false, z);
				Map.CalculateTileTransitions(Vec2i(ix, iy), true, z);
				Map.CalculateTileLandmass(Vec2i(ix, iy), z);
				Map.CalculateTileOwnership(Vec2i(ix, iy), z);
				Map.CalculateTileTerrainFeature(Vec2i(ix, iy), z);
				mf.UpdateSeenTile();
				UI.Minimap.UpdateXY(Vec2i(ix, iy), z);
				if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
					Map.MarkSeenTile(mf, z);
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
int GetMapLayer(std::string plane_ident, std::string world_ident, int surface_layer)
{
	CPlane *plane = CPlane::GetPlane(plane_ident);
	CWorld *world = CWorld::GetWorld(world_ident);

	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		if (Map.MapLayers[z]->Plane == plane && Map.MapLayers[z]->World == world && Map.MapLayers[z]->SurfaceLayer == surface_layer) {
			return z;
		}
	}
	
	return -1;
}

int GetSubtemplateStartX(std::string subtemplate_ident)
{
	CMapTemplate *subtemplate = CMapTemplate::GetMapTemplate(subtemplate_ident);
	
	if (!subtemplate) {
		return -1;
	}

	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		for (size_t i = 0; i < Map.MapLayers[z]->SubtemplateAreas.size(); ++i) {
			Vec2i min_pos = std::get<0>(Map.MapLayers[z]->SubtemplateAreas[i]);
			if (subtemplate == std::get<2>(Map.MapLayers[z]->SubtemplateAreas[i])) {
				return min_pos.x;
			}
		}
	}

	return -1;
}

int GetSubtemplateStartY(std::string subtemplate_ident)
{
	CMapTemplate *subtemplate = CMapTemplate::GetMapTemplate(subtemplate_ident);
	
	if (!subtemplate) {
		return -1;
	}

	for (size_t z = 0; z < Map.MapLayers.size(); ++z) {
		for (size_t i = 0; i < Map.MapLayers[z]->SubtemplateAreas.size(); ++i) {
			Vec2i min_pos = std::get<0>(Map.MapLayers[z]->SubtemplateAreas[i]);
			if (subtemplate == std::get<2>(Map.MapLayers[z]->SubtemplateAreas[i])) {
				return min_pos.y;
			}
		}
	}

	return -1;
}

/**
**	@brief	Change the map layer currently being displayed to the previous
**
**	@param	z	The map layer
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
	if (z < 0 || z >= (int) Map.MapLayers.size() || UI.CurrentMapLayer->ID == z) {
		return;
	}
	
	Vec2i new_viewport_map_pos(UI.SelectedViewport->MapPos.x * Map.Info.MapWidths[z] / Map.Info.MapWidths[UI.CurrentMapLayer->ID], UI.SelectedViewport->MapPos.y * Map.Info.MapHeights[z] / Map.Info.MapHeights[UI.CurrentMapLayer->ID]);
	
	UI.PreviousMapLayer = UI.CurrentMapLayer;
	UI.CurrentMapLayer = Map.MapLayers[z];
	UI.Minimap.UpdateCache = true;
	UI.SelectedViewport->Set(new_viewport_map_pos, Map.GetCurrentPixelTileSize() / 2);
	UpdateSurfaceLayerButtons();
}

void SetTimeOfDay(int time_of_day, int z)
{
	Map.MapLayers[z]->TimeOfDay = time_of_day;
}
//Wyrmgus end

/**
**  Clear CMapInfo.
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

CMap::CMap() : NoFogOfWar(false), TileGraphic(nullptr), Landmasses(0), BorderTerrain(nullptr)
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

CMapField *CMap::Field(unsigned int index, int z) const
{
	return &this->MapLayers[z]->Fields[index];
}

/**
**	@brief	Get the MapField at location x, y
*/
CMapField *CMap::Field(int x, int y, int z) const
{
	return &this->MapLayers[z]->Fields[x + y * this->Info.MapWidths[z]];
}

CMapField *CMap::Field(const Vec2i &pos, int z) const
{
	return Field(pos.x, pos.y, z);
}

/**
**	@brief	Allocate and initialize map table
*/
void CMap::Create()
{
	Assert(this->MapLayers.size() == 0);

	CMapLayer *map_layer = new CMapLayer;
	map_layer->ID = this->MapLayers.size();
	map_layer->Fields = new CMapField[this->Info.MapWidth * this->Info.MapHeight];
	if (Editor.Running == EditorNotRunning) {
		if (!GameSettings.Inside && !GameSettings.NoTimeOfDay) {
			map_layer->TimeOfDay = CCalendar::GetTimeOfDay(CDate::CurrentTotalHours, DefaultHoursPerDay);
		} else {
			map_layer->TimeOfDay = NoTimeOfDay; // make indoors have no time of day setting until it is possible to make light sources change their surrounding "time of day" // indoors it is always dark (maybe would be better to allow a special setting to have bright indoor places?
		}
		map_layer->Season = CCalendar::GetSeason(CDate::CurrentTotalHours / DefaultHoursPerDay, DefaultDaysPerYear);
	}
	this->MapLayers.push_back(map_layer);
	this->Info.MapWidths.push_back(this->Info.MapWidth);
	this->Info.MapHeights.push_back(this->Info.MapHeight);
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::Init()
{
	for (std::map<PixelSize, CGraphic *>::iterator iterator = this->FogGraphics.begin(); iterator != this->FogGraphics.end(); ++iterator) {
		InitFogOfWar(iterator->first);
	}
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
	this->SiteUnits.clear();
	//Wyrmgus end

	// Tileset freed by Tileset?

	this->Info.Clear();
	//Wyrmgus start
//	this->Fields = nullptr;
	//Wyrmgus end
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
		file.printf("  {%d},\n", this->MapLayers[z]->TimeOfDay);
	}
	file.printf("  },\n");
	file.printf("  \"season\", {\n");
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {%d},\n", this->MapLayers[z]->Season);
	}
	file.printf("  },\n");
	file.printf("  \"pixel-tile-size\", {\n");
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {%d, %d},\n", this->MapLayers[z]->PixelTileSize.x, this->MapLayers[z]->PixelTileSize.y);
	}
	file.printf("  },\n");
	file.printf("  \"layer-references\", {\n");
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		file.printf("  {\"%s\", \"%s\", %d},\n", this->MapLayers[z]->Plane ? this->MapLayers[z]->Plane->Ident.c_str() : "", this->MapLayers[z]->World ? this->MapLayers[z]->World->Ident.c_str() : "", this->MapLayers[z]->SurfaceLayer);
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
void CMap::SetTileTerrain(const Vec2i &pos, CTerrainType *terrain, int z)
{
	if (!terrain) {
		return;
	}
	
	CMapField &mf = *this->Field(pos, z);
	
	CTerrainType *old_terrain = this->GetTileTerrain(pos, terrain->Overlay, z);
	
	if (terrain->Overlay) {
		if (mf.OverlayTerrain == terrain) {
			return;
		}
	} else {
		if (mf.Terrain == terrain) {
			return;
		}
	}
	
	mf.SetTerrain(terrain);
	
	if (terrain->Overlay) {
		//remove decorations if the overlay terrain has changed
		std::vector<CUnit *> table;
		Select(pos, pos, table, z);
		for (size_t i = 0; i != table.size(); ++i) {
			if (table[i] && table[i]->IsAlive() && table[i]->Type->UnitType == UnitTypeLand && table[i]->Type->BoolFlag[DECORATION_INDEX].value) {
				if (Editor.Running == EditorNotRunning) {
					LetUnitDie(*table[i]);
				} else {
					EditorActionRemoveUnit(*table[i], false);
				}
			}
		}
	}
	
	this->CalculateTileTransitions(pos, false, z); //recalculate both, since one may have changed the other
	this->CalculateTileTransitions(pos, true, z);
	this->CalculateTileTerrainFeature(pos, z);
	
	if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
		MarkSeenTile(mf, z);
	}
	UI.Minimap.UpdateXY(pos, z);
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
					
					if (terrain->Overlay && adjacent_mf.OverlayTerrain != terrain && Editor.Running == EditorNotRunning) {
						continue;
					}
					
					this->CalculateTileTransitions(adjacent_pos, false, z);
					this->CalculateTileTransitions(adjacent_pos, true, z);
					
					if (adjacent_mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
						MarkSeenTile(adjacent_mf, z);
					}
					UI.Minimap.UpdateXY(adjacent_pos, z);
				}
			}
		}
	}
	
	if (terrain->Overlay) {
		if ((terrain->Flags & MapFieldUnpassable) || (old_terrain && (old_terrain->Flags & MapFieldUnpassable))) {
			Map.CalculateTileOwnership(pos, z);
			
			for (int x_offset = -16; x_offset <= 16; ++x_offset) {
				for (int y_offset = -16; y_offset <= 16; ++y_offset) {
					if (x_offset != 0 || y_offset != 0) {
						Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
						if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
							Map.CalculateTileOwnership(adjacent_pos, z);
						}
					}
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
	
	CTerrainType *old_terrain = mf.OverlayTerrain;
	
	mf.RemoveOverlayTerrain();
	
	this->CalculateTileTransitions(pos, true, z);
	this->CalculateTileTerrainFeature(pos, z);
	
	if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
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
					
					if (adjacent_mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
						MarkSeenTile(adjacent_mf, z);
					}
					UI.Minimap.UpdateXY(adjacent_pos, z);
				}
			}
		}
	}
	
	if (old_terrain->Flags & MapFieldUnpassable) {
		Map.CalculateTileOwnership(pos, z);
			
		for (int x_offset = -16; x_offset <= 16; ++x_offset) {
			for (int y_offset = -16; y_offset <= 16; ++y_offset) {
				if (x_offset != 0 || y_offset != 0) {
					Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
					if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
						Map.CalculateTileOwnership(adjacent_pos, z);
					}
				}
			}
		}
	}
}

void CMap::SetOverlayTerrainDestroyed(const Vec2i &pos, bool destroyed, int z)
{
	CMapField &mf = *this->Field(pos, z);
	
	if (!mf.OverlayTerrain || mf.OverlayTerrainDestroyed == destroyed) {
		return;
	}
	
	mf.SetOverlayTerrainDestroyed(destroyed);
	
	if (destroyed) {
		if (mf.OverlayTerrain->Flags & MapFieldForest) {
			mf.Flags &= ~(MapFieldForest | MapFieldUnpassable);
			mf.Flags |= MapFieldStumps;
		} else if (mf.OverlayTerrain->Flags & MapFieldRocks) {
			mf.Flags &= ~(MapFieldRocks | MapFieldUnpassable);
			mf.Flags |= MapFieldGravel;
		} else if (mf.OverlayTerrain->Flags & MapFieldWall) {
			mf.Flags &= ~(MapFieldWall | MapFieldUnpassable);
			if (GameSettings.Inside) {
				mf.Flags &= ~(MapFieldAirUnpassable);
			}
			mf.Flags |= MapFieldGravel;
		}
		mf.Value = 0;
	} else {
		if (mf.Flags & MapFieldStumps) { //if is a cleared tree tile regrowing trees
			mf.Flags &= ~(MapFieldStumps);
			mf.Flags |= MapFieldForest | MapFieldUnpassable;
			mf.Value = Resources[WoodCost].DefaultAmount;
		}
	}
	
	this->CalculateTileTransitions(pos, true, z);
	
	if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
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
					
					if (adjacent_mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
						MarkSeenTile(adjacent_mf, z);
					}
					UI.Minimap.UpdateXY(adjacent_pos, z);
				}
			}
		}
	}
	
	if (mf.OverlayTerrain->Flags & MapFieldUnpassable) {
		Map.CalculateTileOwnership(pos, z);
			
		for (int x_offset = -16; x_offset <= 16; ++x_offset) {
			for (int y_offset = -16; y_offset <= 16; ++y_offset) {
				if (x_offset != 0 || y_offset != 0) {
					Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
					if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
						Map.CalculateTileOwnership(adjacent_pos, z);
					}
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
	
	this->CalculateTileTransitions(pos, true, z);
	
	if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
		MarkSeenTile(mf, z);
	}
	UI.Minimap.UpdateXY(pos, z);
}

static int GetTransitionType(std::vector<int> &adjacent_directions, bool allow_single = false)
{
	if (adjacent_directions.size() == 0) {
		return -1;
	}
	
	int transition_type = -1;

	if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end()) {
		transition_type = SingleTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end()) {
		transition_type = NorthSingleTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end()) {
		transition_type = SouthSingleTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = WestSingleTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end()) {
		transition_type = EastSingleTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthSouthTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end()) {
		transition_type = WestEastTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end()) {
		transition_type = NorthSouthwestInnerSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end()) {
		transition_type = NorthSouthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end()) {
		transition_type = NorthSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end()) {
		transition_type = SouthNorthwestInnerNortheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end()) {
		transition_type = SouthNorthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end()) {
		transition_type = SouthNortheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end()) {
		transition_type = WestNortheastInnerSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end()) {
		transition_type = WestNortheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end()) {
		transition_type = WestSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end()) {
		transition_type = EastNorthwestInnerSouthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end()) {
		transition_type = EastNorthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end()) {
		transition_type = EastSouthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end()) {
		transition_type = NorthwestOuterSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end()) {
		transition_type = NortheastOuterSouthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end()) {
		transition_type = SouthwestOuterNortheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end()) {
		transition_type = SoutheastOuterNorthwestInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = SouthTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end()) {
		transition_type = WestTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end()) {
		transition_type = EastTransitionType;
	} else if ((std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() || std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end()) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end()) {
		transition_type = NorthwestOuterTransitionType;
	} else if ((std::find(adjacent_directions.begin(), adjacent_directions.end(), North) != adjacent_directions.end() || std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end()) && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end()) {
		transition_type = NortheastOuterTransitionType;
	} else if ((std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() || std::find(adjacent_directions.begin(), adjacent_directions.end(), West) != adjacent_directions.end()) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end()) {
		transition_type = SouthwestOuterTransitionType;
	} else if ((std::find(adjacent_directions.begin(), adjacent_directions.end(), South) != adjacent_directions.end() || std::find(adjacent_directions.begin(), adjacent_directions.end(), East) != adjacent_directions.end()) && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end()) {
		transition_type = SoutheastOuterTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestNortheastSouthwestSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestNortheastSouthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestNortheastSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestSouthwestSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NortheastSouthwestSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestNortheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = SouthwestSoutheastInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestSouthwestInnerTransitionType;
	} else if (allow_single && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NortheastSoutheastInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestSoutheastInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NortheastSouthwestInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), Northwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NorthwestInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), Northeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = NortheastInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), Southwest) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = SouthwestInnerTransitionType;
	} else if (std::find(adjacent_directions.begin(), adjacent_directions.end(), Southeast) != adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), North) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), South) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), West) == adjacent_directions.end() && std::find(adjacent_directions.begin(), adjacent_directions.end(), East) == adjacent_directions.end()) {
		transition_type = SoutheastInnerTransitionType;
	}

	return transition_type;
}

void CMap::CalculateTileTransitions(const Vec2i &pos, bool overlay, int z)
{
	CMapField &mf = *this->Field(pos, z);
	CTerrainType *terrain = nullptr;
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
					CTerrainType *adjacent_terrain = this->GetTileTerrain(adjacent_pos, overlay, z);
					if (overlay && adjacent_terrain && this->Field(adjacent_pos, z)->OverlayTerrainDestroyed) {
						adjacent_terrain = nullptr;
					}
					if (adjacent_terrain && terrain != adjacent_terrain) {
						if (std::find(terrain->InnerBorderTerrains.begin(), terrain->InnerBorderTerrains.end(), adjacent_terrain) != terrain->InnerBorderTerrains.end()) {
							adjacent_terrain_directions[adjacent_terrain->ID].push_back(GetDirectionFromOffset(x_offset, y_offset));
						} else if (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), adjacent_terrain) == terrain->BorderTerrains.end()) { //if the two terrain types can't border, look for a third terrain type which can border both, and which treats both as outer border terrains, and then use for transitions between both tiles
							for (size_t i = 0; i < terrain->BorderTerrains.size(); ++i) {
								CTerrainType *border_terrain = terrain->BorderTerrains[i];
								if (std::find(terrain->InnerBorderTerrains.begin(), terrain->InnerBorderTerrains.end(), border_terrain) != terrain->InnerBorderTerrains.end() && std::find(adjacent_terrain->InnerBorderTerrains.begin(), adjacent_terrain->InnerBorderTerrains.end(), border_terrain) != adjacent_terrain->InnerBorderTerrains.end()) {
									adjacent_terrain_directions[border_terrain->ID].push_back(GetDirectionFromOffset(x_offset, y_offset));
									break;
								}
							}
						}
					}
					if (!adjacent_terrain || (overlay && terrain != adjacent_terrain && std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), adjacent_terrain) == terrain->BorderTerrains.end())) { // happens if terrain is null or if it is an overlay tile which doesn't have a border with this one, so that i.e. tree transitions display correctly when adjacent to tiles without overlays
						adjacent_terrain_directions[CTerrainType::TerrainTypes.size()].push_back(GetDirectionFromOffset(x_offset, y_offset));
					}
				}
			}
		}
	}
	
	for (std::map<int, std::vector<int>>::iterator iterator = adjacent_terrain_directions.begin(); iterator != adjacent_terrain_directions.end(); ++iterator) {
		int adjacent_terrain_id = iterator->first;
		CTerrainType *adjacent_terrain = adjacent_terrain_id < (int) CTerrainType::TerrainTypes.size() ? CTerrainType::TerrainTypes[adjacent_terrain_id] : nullptr;
		int transition_type = GetTransitionType(iterator->second, terrain->AllowSingle);
		
		if (transition_type != -1) {
			bool found_transition = false;
			
			if (!overlay) {
				if (adjacent_terrain) {
					if (terrain->TransitionTiles[std::tuple<int, int>(adjacent_terrain_id, transition_type)].size() > 0) {
						mf.TransitionTiles.push_back(std::pair<CTerrainType *, int>(terrain, terrain->TransitionTiles[std::tuple<int, int>(adjacent_terrain_id, transition_type)][SyncRand(terrain->TransitionTiles[std::tuple<int, int>(adjacent_terrain_id, transition_type)].size())]));
						found_transition = true;
					} else if (adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(terrain_id, transition_type)].size() > 0) {
						mf.TransitionTiles.push_back(std::pair<CTerrainType *, int>(adjacent_terrain, adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(terrain_id, transition_type)][SyncRand(adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(terrain_id, transition_type)].size())]));
						found_transition = true;
					} else if (adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(-1, transition_type)].size() > 0) {
						mf.TransitionTiles.push_back(std::pair<CTerrainType *, int>(adjacent_terrain, adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(-1, transition_type)][SyncRand(adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(-1, transition_type)].size())]));
						found_transition = true;
					}
				} else {
					if (terrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)].size() > 0) {
						mf.TransitionTiles.push_back(std::pair<CTerrainType *, int>(terrain, terrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)][SyncRand(terrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)].size())]));
					}
				}
			} else {
				if (adjacent_terrain) {
					if (adjacent_terrain && terrain->TransitionTiles[std::tuple<int, int>(adjacent_terrain_id, transition_type)].size() > 0) {
						mf.OverlayTransitionTiles.push_back(std::pair<CTerrainType *, int>(terrain, terrain->TransitionTiles[std::tuple<int, int>(adjacent_terrain_id, transition_type)][SyncRand(terrain->TransitionTiles[std::tuple<int, int>(adjacent_terrain_id, transition_type)].size())]));
						found_transition = true;
					} else if (adjacent_terrain && adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(terrain_id, transition_type)].size() > 0) {
						mf.OverlayTransitionTiles.push_back(std::pair<CTerrainType *, int>(adjacent_terrain, adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(terrain_id, transition_type)][SyncRand(adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(terrain_id, transition_type)].size())]));
						found_transition = true;
					} else if (adjacent_terrain && adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(-1, transition_type)].size() > 0) {
						mf.OverlayTransitionTiles.push_back(std::pair<CTerrainType *, int>(adjacent_terrain, adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(-1, transition_type)][SyncRand(adjacent_terrain->AdjacentTransitionTiles[std::tuple<int, int>(-1, transition_type)].size())]));
						found_transition = true;
					}
				} else {
					if (terrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)].size() > 0) {
						mf.OverlayTransitionTiles.push_back(std::pair<CTerrainType *, int>(terrain, terrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)][SyncRand(terrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)].size())]));
					}
				}
				
				if ((mf.Flags & MapFieldWaterAllowed) && (!adjacent_terrain || !(adjacent_terrain->Flags & MapFieldWaterAllowed))) { //if this is a water tile adjacent to a non-water tile, replace the water flag with a coast one
					mf.Flags &= ~(MapFieldWaterAllowed);
					mf.Flags |= MapFieldCoastAllowed;
				}
			}
			
			if (adjacent_terrain && found_transition) {
				for (size_t i = 0; i != iterator->second.size(); ++i) {
					adjacent_terrain_directions[CTerrainType::TerrainTypes.size()].erase(std::remove(adjacent_terrain_directions[CTerrainType::TerrainTypes.size()].begin(), adjacent_terrain_directions[CTerrainType::TerrainTypes.size()].end(), iterator->second[i]), adjacent_terrain_directions[CTerrainType::TerrainTypes.size()].end());
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
				if (std::find(mf.OverlayTransitionTiles[i + 1].first->InnerBorderTerrains.begin(), mf.OverlayTransitionTiles[i + 1].first->InnerBorderTerrains.end(), mf.OverlayTransitionTiles[i].first) != mf.OverlayTransitionTiles[i + 1].first->InnerBorderTerrains.end()) {
					std::pair<CTerrainType *, int> temp_transition = mf.OverlayTransitionTiles[i];
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
				if (std::find(mf.TransitionTiles[i + 1].first->InnerBorderTerrains.begin(), mf.TransitionTiles[i + 1].first->InnerBorderTerrains.end(), mf.TransitionTiles[i].first) != mf.TransitionTiles[i + 1].first->InnerBorderTerrains.end()) {
					std::pair<CTerrainType *, int> temp_transition = mf.TransitionTiles[i];
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
	
	bool is_water = (mf.Flags & MapFieldWaterAllowed) || (mf.Flags & MapFieldCoastAllowed);

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
						bool adjacent_is_water = (adjacent_mf.Flags & MapFieldWaterAllowed) || (adjacent_mf.Flags & MapFieldCoastAllowed);
									
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

void CMap::CalculateTileTerrainFeature(const Vec2i &pos, int z)
{
	if (!this->Info.IsPointOnMap(pos, z)) {
		return;
	}
	
	if (Editor.Running != EditorNotRunning) { //no need to assign terrain features while in the editor
		return;
	}
	
	CMapField &mf = *this->Field(pos, z);

	if (mf.TerrainFeature) {
		return; //already has a terrain feature
	}

	//if any adjacent tile the same top terrain as this one, and has a terrain feature, then use that
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if ((x_offset != 0 || y_offset != 0) && !(x_offset != 0 && y_offset != 0)) { //only directly adjacent tiles (no diagonal ones, and not the same tile)
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);

					if (adjacent_mf.TerrainFeature && adjacent_mf.TerrainFeature->TerrainType == GetTileTopTerrain(pos, false, z)) {
						mf.TerrainFeature = adjacent_mf.TerrainFeature;
						return;
					}
				}
			}
		}
	}
}

void CMap::CalculateTileOwnership(const Vec2i &pos, int z)
{
	if (!this->Info.IsPointOnMap(pos, z)) {
		return;
	}

	CMapField &mf = *this->Field(pos, z);
	
	int new_owner = -1;
	bool must_have_no_owner = false;
	
	if (mf.Flags & MapFieldBuilding) { //make sure the place a building is located is set to be owned by its player; this is necessary for scenarios, since when they start buildings could be on another player's territory (i.e. if a farm starts next to a town hall)
		const CUnitCache &cache = mf.UnitCache;
		for (size_t i = 0; i != cache.size(); ++i) {
			CUnit *unit = cache[i];
			if (!unit) {
				fprintf(stderr, "Error in CMap::CalculateTileOwnership (pos %d, %d): a unit in the tile's unit cache is null.\n", pos.x, pos.y);
			}
			if (unit->IsAliveOnMap() && unit->Type->BoolFlag[BUILDING_INDEX].value) {
				if (unit->Variable[OWNERSHIPINFLUENCERANGE_INDEX].Value && unit->Player->Index != PlayerNumNeutral) {
					new_owner = unit->Player->Index;
					break;
				} else if (unit->Player->Index == PlayerNumNeutral && (unit->Type == SettlementSiteUnitType || (unit->Type->GivesResource && !unit->Type->BoolFlag[CANHARVEST_INDEX].value))) { //there cannot be an owner for the tile of a (neutral) settlement site or deposit, otherwise players might not be able to build over them
					must_have_no_owner = true;
					break;
				}
			}
		}
	}

	if (new_owner == -1 && !must_have_no_owner) { //if no building is on the tile, set it to the first unit to have influence on it, if that isn't blocked by an obstacle
		std::vector<unsigned long> obstacle_flags;
		obstacle_flags.push_back(MapFieldCoastAllowed);
		obstacle_flags.push_back(MapFieldUnpassable);

		std::vector<CUnit *> table;
		Select(pos - Vec2i(16, 16), pos + Vec2i(16, 16), table, z);
		for (size_t i = 0; i != table.size(); ++i) {
			CUnit *unit = table[i];
			if (!unit) {
				fprintf(stderr, "Error in CMap::CalculateTileOwnership (pos %d, %d): a unit within the tile's range is null.\n", pos.x, pos.y);
			}
			if (unit->IsAliveOnMap() && unit->Variable[OWNERSHIPINFLUENCERANGE_INDEX].Value > 0 && unit->MapDistanceTo(pos, z) <= unit->Variable[OWNERSHIPINFLUENCERANGE_INDEX].Value) {
				bool obstacle_check = true;
				for (size_t j = 0; j < obstacle_flags.size(); ++j) {
					bool obstacle_subcheck = false;
					for (int x = 0; x < unit->Type->TileSize.x; ++x) {
						for (int y = 0; y < unit->Type->TileSize.y; ++y) {
							if (CheckObstaclesBetweenTiles(unit->tilePos + Vec2i(x, y), pos, obstacle_flags[j], z, 0, nullptr, unit->Player->Index)) { //the obstacle must be avoidable from at least one of the unit's tiles
								obstacle_subcheck = true;
								break;
							}
						}
						if (obstacle_subcheck) {
							break;
						}
					}
					if (!obstacle_subcheck) {
						obstacle_check = false;
						break;
					}
				}
				if (!obstacle_check) {
					continue;
				}
				new_owner = unit->Player->Index;
				break;
			}
		}
	}
	
	if (new_owner != mf.Owner) {
		mf.Owner = new_owner;
		
		this->CalculateTileOwnershipTransition(pos, z);
		
		for (int x_offset = -1; x_offset <= 1; ++x_offset) {
			for (int y_offset = -1; y_offset <= 1; ++y_offset) {
				if (x_offset != 0 || y_offset != 0) {
					Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
					if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
						CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
							
						this->CalculateTileOwnershipTransition(adjacent_pos, z);
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
	
	mf.OwnershipBorderTile = -1;

	if (mf.Owner == -1) {
		return;
	}
	
	std::vector<int> adjacent_directions;
	
	for (int x_offset = -1; x_offset <= 1; ++x_offset) {
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			if (x_offset != 0 || y_offset != 0) {
				Vec2i adjacent_pos(pos.x + x_offset, pos.y + y_offset);
				if (Map.Info.IsPointOnMap(adjacent_pos, z)) {
					CMapField &adjacent_mf = *this->Field(adjacent_pos, z);
					if (adjacent_mf.Owner != mf.Owner) {
						adjacent_directions.push_back(GetDirectionFromOffset(x_offset, y_offset));
					}
				}
			}
		}
	}
	
	int transition_type = GetTransitionType(adjacent_directions, true);
	
	if (transition_type != -1) {
		if (Map.BorderTerrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)].size() > 0) {
			mf.OwnershipBorderTile = Map.BorderTerrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)][SyncRand(Map.BorderTerrain->TransitionTiles[std::tuple<int, int>(-1, transition_type)].size())];
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

void CMap::AdjustTileMapIrregularities(bool overlay, const Vec2i &min_pos, const Vec2i &max_pos, int z)
{
	bool no_irregularities_found = false;
	while (!no_irregularities_found) {
		no_irregularities_found = true;
		for (int x = min_pos.x; x < max_pos.x; ++x) {
			for (int y = min_pos.y; y < max_pos.y; ++y) {
				CMapField &mf = *this->Field(x, y, z);
				CTerrainType *terrain = overlay ? mf.OverlayTerrain : mf.Terrain;
				if (!terrain || terrain->AllowSingle) {
					continue;
				}
				std::vector<CTerrainType *> acceptable_adjacent_tile_types;
				acceptable_adjacent_tile_types.push_back(terrain);
				for (size_t i = 0; i < terrain->OuterBorderTerrains.size(); ++i) {
					acceptable_adjacent_tile_types.push_back(terrain->OuterBorderTerrains[i]);
				}
				
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
						bool changed_terrain = false;
						for (int sub_x = -1; sub_x <= 1; ++sub_x) {
							for (int sub_y = -1; sub_y <= 1; ++sub_y) {
								if ((x + sub_x) < min_pos.x || (x + sub_x) >= max_pos.x || (y + sub_y) < min_pos.y || (y + sub_y) >= max_pos.y || (sub_x == 0 && sub_y == 0)) {
									continue;
								}
								CTerrainType *tile_terrain = GetTileTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
								if (mf.Terrain != tile_terrain) {
									if (std::find(mf.Terrain->InnerBorderTerrains.begin(), mf.Terrain->InnerBorderTerrains.end(), tile_terrain) != mf.Terrain->InnerBorderTerrains.end()) {
										mf.SetTerrain(tile_terrain);
										changed_terrain = true;
										break;
									}
								}
							}
							if (changed_terrain) {
								break;
							}
						}
						if (!changed_terrain && terrain->InnerBorderTerrains.size() > 0) {
							mf.SetTerrain(terrain->InnerBorderTerrains[0]);
						}
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
					CTerrainType *tile_terrain = GetTileTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
					CTerrainType *tile_top_terrain = GetTileTopTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
					if (
						mf.Terrain != tile_terrain
						&& tile_top_terrain->Overlay
						&& tile_top_terrain != mf.OverlayTerrain
						&& std::find(tile_terrain->OuterBorderTerrains.begin(), tile_terrain->OuterBorderTerrains.end(), mf.Terrain) == tile_terrain->OuterBorderTerrains.end()
						&& std::find(tile_top_terrain->BaseTerrainTypes.begin(), tile_top_terrain->BaseTerrainTypes.end(), mf.Terrain) == tile_top_terrain->BaseTerrainTypes.end()
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
					CTerrainType *tile_terrain = GetTileTerrain(Vec2i(x + sub_x, y + sub_y), false, z);
					if (mf.Terrain != tile_terrain && std::find(mf.Terrain->BorderTerrains.begin(), mf.Terrain->BorderTerrains.end(), tile_terrain) == mf.Terrain->BorderTerrains.end()) {
						for (size_t i = 0; i < mf.Terrain->BorderTerrains.size(); ++i) {
							CTerrainType *border_terrain = mf.Terrain->BorderTerrains[i];
							if (std::find(border_terrain->BorderTerrains.begin(), border_terrain->BorderTerrains.end(), mf.Terrain) != border_terrain->BorderTerrains.end() && std::find(border_terrain->BorderTerrains.begin(), border_terrain->BorderTerrains.end(), tile_terrain) != border_terrain->BorderTerrains.end()) {
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

void CMap::GenerateTerrain(CTerrainType *terrain, int seed_number, int expansion_number, const Vec2i &min_pos, const Vec2i &max_pos, bool preserve_coastline, int z)
{
	if (SaveGameLoading) {
		return;
	}
	
	Vec2i random_pos(0, 0);
	int count = seed_number;
	int while_count = 0;
	
	// create initial seeds
	while (count > 0 && while_count < seed_number * 100) {
		random_pos.x = SyncRand(max_pos.x - min_pos.x + 1) + min_pos.x;
		random_pos.y = SyncRand(max_pos.y - min_pos.y + 1) + min_pos.y;
		
		if (!this->Info.IsPointOnMap(random_pos, z) || this->IsPointInASubtemplateArea(random_pos, z)) {
			continue;
		}
		
		CTerrainType *tile_terrain = GetTileTerrain(random_pos, false, z);
		
		if (
			(
				(
					!terrain->Overlay
					&& ((tile_terrain == terrain && GetTileTopTerrain(random_pos, false, z)->Overlay) || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(random_pos, terrain, z)))
				)
				|| (
					terrain->Overlay
					&& std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(random_pos, terrain, z)
					&& (!GetTileTopTerrain(random_pos, false, z)->Overlay || GetTileTopTerrain(random_pos, false, z) == terrain)
				)
			)
			&& (!preserve_coastline || (terrain->Flags & MapFieldWaterAllowed) == (tile_terrain->Flags & MapFieldWaterAllowed))
			&& !this->TileHasUnitsIncompatibleWithTerrain(random_pos, terrain, z)
			&& (!(terrain->Flags & MapFieldUnpassable) || !this->TileBordersUnit(random_pos, z)) // if the terrain is unpassable, don't expand to spots adjacent to units
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
					
					CTerrainType *diagonal_tile_terrain = GetTileTerrain(diagonal_pos, false, z);
					CTerrainType *vertical_tile_terrain = GetTileTerrain(vertical_pos, false, z);
					CTerrainType *horizontal_tile_terrain = GetTileTerrain(horizontal_pos, false, z);
					
					if (
						(
							(
								!terrain->Overlay
								&& ((diagonal_tile_terrain == terrain && GetTileTopTerrain(diagonal_pos, false, z)->Overlay) || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), diagonal_tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(diagonal_pos, terrain, z)))
								&& ((vertical_tile_terrain == terrain && GetTileTopTerrain(vertical_pos, false, z)->Overlay) || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), vertical_tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(vertical_pos, terrain, z)))
								&& ((horizontal_tile_terrain == terrain && GetTileTopTerrain(horizontal_pos, false, z)->Overlay) || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), horizontal_tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(horizontal_pos, terrain, z)))
							)
							|| (
								terrain->Overlay
								&& std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), diagonal_tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(diagonal_pos, terrain, z)
								&& std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), vertical_tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(vertical_pos, terrain, z)
								&& std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), horizontal_tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(horizontal_pos, terrain, z)
								&& (!GetTileTopTerrain(diagonal_pos, false, z)->Overlay || GetTileTopTerrain(diagonal_pos, false, z) == terrain) && (!GetTileTopTerrain(vertical_pos, false, z)->Overlay || GetTileTopTerrain(vertical_pos, false, z) == terrain) && (!GetTileTopTerrain(horizontal_pos, false, z)->Overlay || GetTileTopTerrain(horizontal_pos, false, z) == terrain)
							)
						)
						&& (!preserve_coastline || ((terrain->Flags & MapFieldWaterAllowed) == (diagonal_tile_terrain->Flags & MapFieldWaterAllowed) && (terrain->Flags & MapFieldWaterAllowed) == (vertical_tile_terrain->Flags & MapFieldWaterAllowed) && (terrain->Flags & MapFieldWaterAllowed) == (horizontal_tile_terrain->Flags & MapFieldWaterAllowed)))
						&& !this->TileHasUnitsIncompatibleWithTerrain(diagonal_pos, terrain, z) && !this->TileHasUnitsIncompatibleWithTerrain(vertical_pos, terrain, z) && !this->TileHasUnitsIncompatibleWithTerrain(horizontal_pos, terrain, z)
						&& (!(terrain->Flags & MapFieldUnpassable) || (!this->TileBordersUnit(diagonal_pos, z) && !this->TileBordersUnit(vertical_pos, z) && !this->TileBordersUnit(horizontal_pos, z))) // if the terrain is unpassable, don't expand to spots adjacent to buildings
						&& !this->IsPointInASubtemplateArea(diagonal_pos, z) && !this->IsPointInASubtemplateArea(vertical_pos, z) && !this->IsPointInASubtemplateArea(horizontal_pos, z)
					) {
						adjacent_positions.push_back(diagonal_pos);
					}
				}
			}
			
			if (adjacent_positions.size() > 0) {
				Vec2i adjacent_pos = adjacent_positions[SyncRand(adjacent_positions.size())];
				if (!terrain->Overlay) {
					this->Field(random_pos, z)->RemoveOverlayTerrain();
					this->Field(adjacent_pos, z)->RemoveOverlayTerrain();
					this->Field(Vec2i(random_pos.x, adjacent_pos.y), z)->RemoveOverlayTerrain();
					this->Field(Vec2i(adjacent_pos.x, random_pos.y), z)->RemoveOverlayTerrain();
				}
				this->Field(random_pos, z)->SetTerrain(terrain);
				this->Field(adjacent_pos, z)->SetTerrain(terrain);
				this->Field(Vec2i(random_pos.x, adjacent_pos.y), z)->SetTerrain(terrain);
				this->Field(Vec2i(adjacent_pos.x, random_pos.y), z)->SetTerrain(terrain);
				count -= 1;
			}
		}
		
		while_count += 1;
	}
	
	// expand seeds
	count = expansion_number;
	while_count = 0;
	
	while (count > 0 && while_count < expansion_number * 100) {
		random_pos.x = SyncRand(max_pos.x - min_pos.x + 1) + min_pos.x;
		random_pos.y = SyncRand(max_pos.y - min_pos.y + 1) + min_pos.y;
		
		if (
			this->Info.IsPointOnMap(random_pos, z)
			&& GetTileTopTerrain(random_pos, false, z) == terrain
			&& (!terrain->Overlay || this->TileBordersOnlySameTerrain(random_pos, terrain, z))
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
					
					CTerrainType *diagonal_tile_terrain = GetTileTerrain(diagonal_pos, false, z);
					CTerrainType *vertical_tile_terrain = GetTileTerrain(vertical_pos, false, z);
					CTerrainType *horizontal_tile_terrain = GetTileTerrain(horizontal_pos, false, z);
					
					if (
						(
							(
								!terrain->Overlay
								&& (diagonal_tile_terrain == terrain || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), diagonal_tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(diagonal_pos, terrain, z)))
								&& (vertical_tile_terrain == terrain || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), vertical_tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(vertical_pos, terrain, z)))
								&& (horizontal_tile_terrain == terrain || (std::find(terrain->BorderTerrains.begin(), terrain->BorderTerrains.end(), horizontal_tile_terrain) != terrain->BorderTerrains.end() && this->TileBordersOnlySameTerrain(horizontal_pos, terrain, z)))
								&& (diagonal_tile_terrain != terrain || vertical_tile_terrain != terrain || horizontal_tile_terrain != terrain || GetTileTopTerrain(diagonal_pos, false, z)->Overlay || GetTileTopTerrain(vertical_pos, false, z)->Overlay || GetTileTopTerrain(horizontal_pos, false, z)->Overlay)
							)
							|| (
								terrain->Overlay
								&& ((std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), diagonal_tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(diagonal_pos, terrain, z)) || GetTileTerrain(diagonal_pos, terrain->Overlay, z) == terrain)
								&& ((std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), vertical_tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(vertical_pos, terrain, z)) || GetTileTerrain(vertical_pos, terrain->Overlay, z) == terrain)
								&& ((std::find(terrain->BaseTerrainTypes.begin(), terrain->BaseTerrainTypes.end(), horizontal_tile_terrain) != terrain->BaseTerrainTypes.end() && this->TileBordersOnlySameTerrain(horizontal_pos, terrain, z)) || GetTileTerrain(horizontal_pos, terrain->Overlay, z) == terrain)
								&& (GetTileTerrain(diagonal_pos, terrain->Overlay, z) != terrain || GetTileTerrain(vertical_pos, terrain->Overlay, z) != terrain || GetTileTerrain(horizontal_pos, terrain->Overlay, z) != terrain)
								&& (!GetTileTopTerrain(diagonal_pos, false, z)->Overlay || GetTileTopTerrain(diagonal_pos, false, z) == terrain) && (!GetTileTopTerrain(vertical_pos, false, z)->Overlay || GetTileTopTerrain(vertical_pos, false, z) == terrain) && (!GetTileTopTerrain(horizontal_pos, false, z)->Overlay || GetTileTopTerrain(horizontal_pos, false, z) == terrain) // don't expand into tiles with overlays if the terrain is an overlay terrain itself
							)
						)
						&& (!preserve_coastline || ((terrain->Flags & MapFieldWaterAllowed) == (diagonal_tile_terrain->Flags & MapFieldWaterAllowed) && (terrain->Flags & MapFieldWaterAllowed) == (vertical_tile_terrain->Flags & MapFieldWaterAllowed) && (terrain->Flags & MapFieldWaterAllowed) == (horizontal_tile_terrain->Flags & MapFieldWaterAllowed)))
						&& !this->TileHasUnitsIncompatibleWithTerrain(diagonal_pos, terrain, z) && !this->TileHasUnitsIncompatibleWithTerrain(vertical_pos, terrain, z) && !this->TileHasUnitsIncompatibleWithTerrain(horizontal_pos, terrain, z)
						&& (!(terrain->Flags & MapFieldUnpassable) || (!this->TileBordersUnit(diagonal_pos, z) && !this->TileBordersUnit(vertical_pos, z) && !this->TileBordersUnit(horizontal_pos, z))) // if the terrain is unpassable, don't expand to spots adjacent to buildings
						&& (!this->IsPointInASubtemplateArea(diagonal_pos, z) || GetTileTerrain(diagonal_pos, terrain->Overlay, z) == terrain) && (!this->IsPointInASubtemplateArea(vertical_pos, z) || GetTileTerrain(vertical_pos, terrain->Overlay, z) == terrain) && (!this->IsPointInASubtemplateArea(horizontal_pos, z) || GetTileTerrain(horizontal_pos, terrain->Overlay, z) == terrain)
					) {
						adjacent_positions.push_back(diagonal_pos);
					}
				}
			}
			
			if (adjacent_positions.size() > 0) {
				Vec2i adjacent_pos = adjacent_positions[SyncRand(adjacent_positions.size())];
				if (!terrain->Overlay) {
					this->Field(adjacent_pos, z)->RemoveOverlayTerrain();
					this->Field(Vec2i(random_pos.x, adjacent_pos.y), z)->RemoveOverlayTerrain();
					this->Field(Vec2i(adjacent_pos.x, random_pos.y), z)->RemoveOverlayTerrain();
				}
				this->Field(adjacent_pos, z)->SetTerrain(terrain);
				this->Field(Vec2i(random_pos.x, adjacent_pos.y), z)->SetTerrain(terrain);
				this->Field(Vec2i(adjacent_pos.x, random_pos.y), z)->SetTerrain(terrain);
				count -= 1;
			}
		}
		
		while_count += 1;
	}
}

void CMap::GenerateNeutralUnits(CUnitType *unit_type, int quantity, const Vec2i &min_pos, const Vec2i &max_pos, bool grouped, int z)
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
			CUnit *unit = CreateUnit(unit_pos, *unit_type, &Players[PlayerNumNeutral], z, unit_type->BoolFlag[BUILDING_INDEX].value && unit_type->TileSize.x > 1 && unit_type->TileSize.y > 1);
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
		if (table[i]->Type->UnitType == UnitTypeLand && table[i]->Type->BoolFlag[DECORATION_INDEX].value) {
			if (Editor.Running == EditorNotRunning) {
				LetUnitDie(*table[i]);			
			} else {
				EditorActionRemoveUnit(*table[i], false);
			}
		}
	}

	//check if any further tile should be removed with the clearing of this one
	if (!mf.OverlayTerrain->AllowSingle) {
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
**  Regenerate forest.
**
**  @param pos  Map tile pos
*/
//Wyrmgus start
//void CMap::RegenerateForestTile(const Vec2i &pos)
void CMap::RegenerateForestTile(const Vec2i &pos, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	Assert(Map.Info.IsPointOnMap(pos));
//	CMapField &mf = *this->Field(pos);
	Assert(Map.Info.IsPointOnMap(pos, z));
	CMapField &mf = *this->Field(pos, z);
	//Wyrmgus end

	//Wyrmgus start
//	if (mf.getGraphicTile() != this->Tileset->getRemovedTreeTile()) {
	if ((mf.OverlayTerrain && !mf.OverlayTerrainDestroyed) || !(mf.getFlag() & MapFieldStumps)) {
	//Wyrmgus end
		return;
	}

	//  Increment each value of no wood.
	//  If grown up, place new wood.
	//  FIXME: a better looking result would be fine
	//    Allow general updates to any tiletype that regrows

	//Wyrmgus start
	/*
	const unsigned int occupedFlag = (MapFieldWall | MapFieldUnpassable | MapFieldLandUnit | MapFieldBuilding);
	++mf.Value;
	if (mf.Value < ForestRegeneration) {
		return;
	}
	mf.Value = ForestRegeneration;
	if ((mf.Flags & occupedFlag) || pos.y == 0) {
		return;
	}
	*/
	
	const unsigned int permanent_occupied_flag = (MapFieldWall | MapFieldUnpassable | MapFieldBuilding);
	const unsigned int occupedFlag = (MapFieldWall | MapFieldUnpassable | MapFieldLandUnit | MapFieldBuilding | MapFieldItem); // don't regrow forests under items
	
	if ((mf.Flags & permanent_occupied_flag)) { //if the tree tile is occupied by buildings and the like, reset the regeneration process
		mf.Value = 0;
		return;
	}

	if ((mf.Flags & occupedFlag) || pos.y == 0) {
		return;
	}
	
	++mf.Value;
	if (mf.Value < ForestRegeneration) {
		return;
	}
	mf.Value = ForestRegeneration;
	//Wyrmgus end
	
	//Wyrmgus start
//	const Vec2i offset(0, -1);
//	CMapField &topMf = *(&mf - this->Info.MapWidth);

	for (int x_offset = -1; x_offset <= 1; x_offset+=2) { //increment by 2 to avoid instances where it is 0
		for (int y_offset = -1; y_offset <= 1; y_offset+=2) {
			const Vec2i verticalOffset(0, y_offset);
			CMapField &verticalMf = *this->Field(pos + verticalOffset, z);
			const Vec2i horizontalOffset(x_offset, 0);
			CMapField &horizontalMf = *this->Field(pos + horizontalOffset, z);
			const Vec2i diagonalOffset(x_offset, y_offset);
			CMapField &diagonalMf = *this->Field(pos + diagonalOffset, z);
			
			if (
				this->Info.IsPointOnMap(pos + verticalOffset, z)
				&& ((verticalMf.OverlayTerrain && verticalMf.OverlayTerrainDestroyed && (verticalMf.getFlag() & MapFieldStumps) && verticalMf.Value >= ForestRegeneration && !(verticalMf.Flags & occupedFlag)) || (verticalMf.getFlag() & MapFieldForest))
				&& this->Info.IsPointOnMap(pos + diagonalOffset, z)
				&& ((diagonalMf.OverlayTerrain && diagonalMf.OverlayTerrainDestroyed && (diagonalMf.getFlag() & MapFieldStumps) && diagonalMf.Value >= ForestRegeneration && !(diagonalMf.Flags & occupedFlag)) || (diagonalMf.getFlag() & MapFieldForest))
				&& this->Info.IsPointOnMap(pos + horizontalOffset, z)
				&& ((horizontalMf.OverlayTerrain && horizontalMf.OverlayTerrainDestroyed && (horizontalMf.getFlag() & MapFieldStumps) && horizontalMf.Value >= ForestRegeneration && !(horizontalMf.Flags & occupedFlag)) || (horizontalMf.getFlag() & MapFieldForest))
			) {
				DebugPrint("Real place wood\n");
				this->SetOverlayTerrainDestroyed(pos + verticalOffset, false, z);
				this->SetOverlayTerrainDestroyed(pos + diagonalOffset, false, z);
				this->SetOverlayTerrainDestroyed(pos + horizontalOffset, false, z);
				this->SetOverlayTerrainDestroyed(pos, false, z);
				
				return;
			}
		}
	}

	/*
	if (topMf.getGraphicTile() == this->Tileset->getRemovedTreeTile()
		&& topMf.Value >= ForestRegeneration
		&& !(topMf.Flags & occupedFlag)) {
		DebugPrint("Real place wood\n");
		topMf.setTileIndex(*Map.Tileset, Map.Tileset->getTopOneTreeTile(), 0);
		topMf.setGraphicTile(Map.Tileset->getTopOneTreeTile());
		topMf.playerInfo.SeenTile = topMf.getGraphicTile();
		topMf.Value = 0;
		topMf.Flags |= MapFieldForest | MapFieldUnpassable;
		UI.Minimap.UpdateSeenXY(pos + offset);
		UI.Minimap.UpdateXY(pos + offset);
		
		mf.setTileIndex(*Map.Tileset, Map.Tileset->getBottomOneTreeTile(), 0);
		mf.setGraphicTile(Map.Tileset->getBottomOneTreeTile());
		mf.playerInfo.SeenTile = mf.getGraphicTile();
		mf.Value = 0;
		mf.Flags |= MapFieldForest | MapFieldUnpassable;
		UI.Minimap.UpdateSeenXY(pos);
		UI.Minimap.UpdateXY(pos);
		
		if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
			MarkSeenTile(mf);
		}
		if (Map.Field(pos + offset)->playerInfo.IsTeamVisible(*ThisPlayer)) {
			MarkSeenTile(topMf);
		}
		FixNeighbors(MapFieldForest, 0, pos + offset);
		FixNeighbors(MapFieldForest, 0, pos);
	}
	*/
}

/**
**  Regenerate forest.
*/
void CMap::RegenerateForest()
{
	if (!ForestRegeneration) {
		return;
	}
	Vec2i pos;
	//Wyrmgus start
	/*
	for (pos.y = 0; pos.y < Info.MapHeight; ++pos.y) {
		for (pos.x = 0; pos.x < Info.MapWidth; ++pos.x) {
			RegenerateForestTile(pos);
		}
	}
	*/
	for (size_t z = 0; z < this->MapLayers.size(); ++z) {
		for (pos.y = 0; pos.y < Info.MapHeights[z]; ++pos.y) {
			for (pos.x = 0; pos.x < Info.MapWidths[z]; ++pos.x) {
				RegenerateForestTile(pos, z);
			}
		}
	}
	//Wyrmgus end
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
		Map.Info.Filename = mapname;
		Map.Info.Filename.replace(loc, 4, ".sms");
	}

	const std::string filename = LibraryFileName(mapname.c_str());
	LuaLoadFile(filename);
}

//@}
