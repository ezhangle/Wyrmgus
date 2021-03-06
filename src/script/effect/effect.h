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

class CPlayer;

namespace stratagus {

class dialogue;
class sml_data;
class sml_property;
enum class sml_operator;

//a scripted effect
class effect
{
public:
	static std::unique_ptr<effect> from_sml_property(const sml_property &property);
	static std::unique_ptr<effect> from_sml_scope(const sml_data &scope);

	explicit effect(const sml_operator effect_operator);

	virtual const std::string &get_class_identifier() const = 0;

	virtual void process_sml_property(const sml_property &property);
	virtual void process_sml_scope(const sml_data &scope);

	void do_effect(CPlayer *player) const;

	virtual void do_assignment_effect(CPlayer *player) const
	{
		Q_UNUSED(player)

		throw std::runtime_error("The assignment operator is not supported for \"" + this->get_class_identifier() + "\" effects.");
	}

	virtual void do_addition_effect(CPlayer *player) const
	{
		Q_UNUSED(player)

		throw std::runtime_error("The addition operator is not supported for \"" + this->get_class_identifier() + "\" effects.");
	}

	virtual void do_subtraction_effect(CPlayer *player) const
	{
		Q_UNUSED(player)

		throw std::runtime_error("The subtraction operator is not supported for \"" + this->get_class_identifier() + "\" effects.");
	}

	std::string get_string() const;

	virtual std::string get_assignment_string() const
	{
		throw std::runtime_error("The assignment operator is not supported for \"" + this->get_class_identifier() + "\" effects.");
	}

	virtual std::string get_addition_string() const
	{
		throw std::runtime_error("The addition operator is not supported for \"" + this->get_class_identifier() + "\" effects.");
	}

	virtual std::string get_subtraction_string() const
	{
		throw std::runtime_error("The subtraction operator is not supported for \"" + this->get_class_identifier() + "\" effects.");
	}

	virtual bool is_hidden() const
	{
		return false;
	}

private:
	sml_operator effect_operator;
};

}
