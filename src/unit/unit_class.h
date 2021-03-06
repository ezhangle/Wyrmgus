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
//      (c) Copyright 2019-2020 by Andrettin
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

namespace stratagus {

class unit_type;

class unit_class final : public named_data_entry, public data_type<unit_class>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "unit_class";
	static constexpr const char *database_folder = "unit_classes";

	static unit_class *add(const std::string &identifier, const stratagus::module *module)
	{
		unit_class *unit_class = data_type::add(identifier, module);
		unit_class->index = unit_class::get_all().size() - 1;
		return unit_class;
	}

	unit_class(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	int get_index() const
	{
		return this->index;
	}

	const std::vector<unit_type *> &get_unit_types() const
	{
		return this->unit_types;
	}

	bool has_unit_type(unit_type *unit_type) const;

	void add_unit_type(unit_type *unit_type)
	{
		this->unit_types.push_back(unit_type);
	}

	void remove_unit_type(unit_type *unit_type);

private:
	int index = -1;
	std::vector<unit_type *> unit_types;
};

}