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
//      (c) Copyright 2018-2020 by Andrettin
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

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "data_type.h"
#include "time/date.h"
#include "vec2i.h"

class CFaction;
class CPlayer;
class CRegion;
class CUnit;
class CUnitType;
class CUniqueItem;
struct lua_State;

int CclDefineSite(lua_State *l);

namespace stratagus {

class civilization;
class map_template;
class unit_class;

class site : public named_data_entry, public data_type<site>, public CDataType
{
	Q_OBJECT

	Q_PROPERTY(bool major MEMBER major READ is_major)
	Q_PROPERTY(stratagus::map_template* map_template MEMBER map_template READ get_map_template)
	Q_PROPERTY(QPoint pos MEMBER pos READ get_pos)

public:
	static constexpr const char *class_identifier = "site";
	static constexpr const char *database_folder = "sites";

	site(const std::string &identifier) : named_data_entry(identifier), CDataType(identifier)
	{
	}

	virtual void ProcessConfigData(const CConfigData *config_data) override;
	virtual void initialize() override;

	std::string GetCulturalName(const civilization *civilization) const;

	bool is_major() const
	{
		return this->major;
	}

	map_template *get_map_template() const
	{
		return this->map_template;
	}

	const QPoint &get_pos() const
	{
		return this->pos;
	}

	CUnit *get_site_unit() const
	{
		return this->site_unit;
	}

	void set_site_unit(CUnit *unit);

	CPlayer *get_owner() const
	{
		return this->owner;
	}

	void set_owner(CPlayer *player);

	void add_border_tile(const QPoint &tile_pos)
	{
		this->border_tiles.push_back(tile_pos);
	}

	void clear_border_tiles()
	{
		this->border_tiles.clear();
	}

	void update_border_tile_graphics();

private:
	bool major = false; /// Whether the site is a major one; major sites have settlement sites, and as such can have town halls
	QPoint pos = QPoint(-1, -1); /// Position of the site in its map template
	map_template *map_template = nullptr; /// Map template where this site is located
	CPlayer *owner = nullptr;
	CUnit *site_unit = nullptr;									/// Unit which represents this site
public:
	std::vector<CRegion *> Regions;								/// Regions where this site is located
	std::vector<CFaction *> Cores;						/// Factions which have this site as a core
	std::map<const civilization *, std::string> CulturalNames;	/// Names for the site for each different culture/civilization
	std::map<CDate, const CFaction *> HistoricalOwners;				/// Historical owners of the site
	std::map<CDate, int> HistoricalPopulation;					/// Historical population
	std::vector<std::tuple<CDate, CDate, const CUnitType *, int, const CFaction *>> HistoricalUnits;	/// Historical quantity of a particular unit type (number of people for units representing a person)
	std::vector<std::tuple<CDate, CDate, const unit_class *, CUniqueItem *, const CFaction *>> HistoricalBuildings; /// Historical buildings, with start and end date
	std::vector<std::tuple<CDate, CDate, const CUnitType *, CUniqueItem *, int>> HistoricalResources; /// Historical resources, with start and end date; the integer at the end is the resource quantity
private:
	std::vector<QPoint> border_tiles; //the tiles for this settlement which border the territory of another settlement

	friend int ::CclDefineSite(lua_State *l);
};

}
