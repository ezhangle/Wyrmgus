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

#include "character.h" //for MaxCharacterTitles
#include "civilization_base.h"
#include "database/data_type.h"
#include "faction_tier.h"
#include "gender.h"
#include "player.h" //for certain enums
#include "time/date.h"

class CAiBuildingTemplate;
class CCurrency;
class CForceTemplate;
class CLanguage;
class CUpgrade;
enum class ForceType;
struct lua_State;

int CclDefineCivilization(lua_State *l);

namespace stratagus {

class calendar;
class civilization_group;
class civilization_supergroup;
class deity;
class quest;
class species;
class unit_class;
class upgrade_class;
enum class government_type;

class civilization final : public civilization_base, public data_type<civilization>
{
	Q_OBJECT

	Q_PROPERTY(stratagus::civilization* parent_civilization MEMBER parent_civilization READ get_parent_civilization)
	Q_PROPERTY(stratagus::civilization_group* group MEMBER group READ get_group)
	Q_PROPERTY(bool visible MEMBER visible READ is_visible)
	Q_PROPERTY(bool playable MEMBER playable READ is_playable)
	Q_PROPERTY(QString interface READ get_interface_qstring)
	Q_PROPERTY(QString default_color READ get_default_color_qstring)
	Q_PROPERTY(CUpgrade* upgrade MEMBER upgrade READ get_upgrade)
	Q_PROPERTY(QStringList ship_names READ get_ship_names_qstring_list)
	Q_PROPERTY(QVariantList acquired_upgrades READ get_acquired_upgrades_qstring_list)

public:
	static constexpr const char *class_identifier = "civilization";
	static constexpr const char *database_folder = "civilizations";

	static civilization *add(const std::string &identifier, const stratagus::module *module)
	{
		civilization *civilization = data_type::add(identifier, module);
		civilization->ID = civilization::get_all().size() - 1;
		return civilization;
	}

	explicit civilization(const std::string &identifier) : civilization_base(identifier)
	{
	}

	~civilization();

	virtual void process_sml_property(const sml_property &property) override;
	virtual void process_sml_scope(const sml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	virtual void reset_history() override
	{
		this->acquired_upgrades.clear();
	}

	civilization *get_parent_civilization() const
	{
		return this->parent_civilization;
	}
	
	stratagus::species *get_species() const
	{
		if (civilization_base::get_species() != nullptr) {
			return civilization_base::get_species();
		}

		if (this->get_parent_civilization() != nullptr) {
			return this->get_parent_civilization()->get_species();
		}

		return nullptr;
	}
	
	civilization_group *get_group() const
	{
		return this->group;
	}

	civilization_supergroup *get_supergroup() const;

	int GetUpgradePriority(const CUpgrade *upgrade) const;
	int GetForceTypeWeight(const ForceType force_type) const;

	const std::string &get_interface() const
	{
		return this->interface;
	}

	QString get_interface_qstring() const
	{
		return QString::fromStdString(this->interface);
	}

	Q_INVOKABLE void set_interface(const std::string &interface)
	{
		this->interface = interface;
	}

	const std::string &get_default_color() const
	{
		return this->default_color;
	}

	QString get_default_color_qstring() const
	{
		return QString::fromStdString(this->default_color);
	}

	Q_INVOKABLE void set_default_color(const std::string &default_color)
	{
		this->default_color = default_color;
	}

	CUpgrade *get_upgrade() const
	{
		return this->upgrade;
	}

	calendar *get_calendar() const;
	CCurrency *GetCurrency() const;

	bool is_visible() const
	{
		return this->visible;
	}

	bool is_playable() const
	{
		return this->playable;
	}

	const std::vector<civilization *> &get_develops_from() const
	{
		return this->develops_from;
	}

	const std::vector<civilization *> &get_develops_to() const
	{
		return this->develops_to;
	}

	cursor *get_cursor(const cursor_type type) const;

	void set_cursor(const cursor_type type, cursor *cursor)
	{
		if (this->cursors.contains(type)) {
			throw std::runtime_error("Another cursor is already registered for type \"" + std::to_string(static_cast<int>(type)) + "\".");
		}

		this->cursors[type] = cursor;
	}

	std::string_view get_title_name(const government_type government_type, const faction_tier tier) const;
	void process_title_name_scope(const sml_data &scope);
	std::string_view get_character_title_name(const int title_type, const int faction_type, const government_type government_type, const faction_tier tier, const gender gender) const;
	void process_character_title_name_scope(const sml_data &scope);
	void process_character_title_name_scope(const int title_type, const sml_data &scope);
	void process_character_title_name_scope(const int title_type, const int faction_type, const sml_data &scope);
	void process_character_title_name_scope(const int title_type, const int faction_type, const government_type government_type, const sml_data &scope);

	std::vector<CForceTemplate *> GetForceTemplates(const ForceType force_type) const;
	std::vector<CAiBuildingTemplate *> GetAiBuildingTemplates() const;

	const std::map<gender, std::vector<std::string>> &get_personal_names() const;
	const std::vector<std::string> &get_personal_names(const gender gender) const;
	const std::vector<std::string> &get_surnames() const;
	const std::vector<std::string> &get_unit_class_names(const unit_class *unit_class) const;
	const std::vector<std::string> &get_ship_names() const;

	unit_type *get_class_unit_type(const unit_class *unit_class) const;

	void set_class_unit_type(const unit_class *unit_class, unit_type *unit_type)
	{
		if (unit_type == nullptr) {
			this->class_unit_types.erase(unit_class);
			return;
		}

		this->class_unit_types[unit_class] = unit_type;
	}

	void remove_class_unit_type(unit_type *unit_type)
	{
		for (std::map<const unit_class *, stratagus::unit_type *>::reverse_iterator iterator = this->class_unit_types.rbegin(); iterator != this->class_unit_types.rend(); ++iterator) {
			if (iterator->second == unit_type) {
				this->class_unit_types.erase(iterator->first);
			}
		}
	}

	CUpgrade *get_class_upgrade(const upgrade_class *upgrade_class) const;

	void set_class_upgrade(const upgrade_class *upgrade_class, CUpgrade *upgrade)
	{
		if (upgrade == nullptr) {
			this->class_upgrades.erase(upgrade_class);
			return;
		}

		this->class_upgrades[upgrade_class] = upgrade;
	}

	void remove_class_upgrade(CUpgrade *upgrade)
	{
		for (std::map<const upgrade_class *, CUpgrade *>::reverse_iterator iterator = this->class_upgrades.rbegin(); iterator != this->class_upgrades.rend(); ++iterator) {
			if (iterator->second == upgrade) {
				this->class_upgrades.erase(iterator->first);
			}
		}
	}

	const std::vector<CFiller> &get_ui_fillers() const
	{
		if (!this->ui_fillers.empty()) {
			return this->ui_fillers;
		}

		if (this->get_parent_civilization() != nullptr) {
			return this->get_parent_civilization()->get_ui_fillers();
		}

		return this->ui_fillers;
	}

	const std::vector<character *> &get_characters() const
	{
		return this->characters;
	}

	void add_character(character *character)
	{
		this->characters.push_back(character);
	}

	const std::vector<CUpgrade *> &get_acquired_upgrades() const
	{
		return this->acquired_upgrades;
	}

	QVariantList get_acquired_upgrades_qstring_list() const;

	Q_INVOKABLE void add_acquired_upgrade(CUpgrade *upgrade)
	{
		this->acquired_upgrades.push_back(upgrade);
	}

	Q_INVOKABLE void remove_acquired_upgrade(CUpgrade *upgrade);

	int ID = -1;
private:
	civilization *parent_civilization = nullptr;
	civilization_group *group = nullptr;
public:
	std::string Adjective;			/// adjective pertaining to the civilization
private:
	std::string interface; //the string identifier for the civilization's interface
	std::string default_color; //name of the civilization's default color (used for the encyclopedia, tech tree, etc.)
	CUpgrade *upgrade = nullptr;
public:
	unit_sound_set UnitSounds;			/// sounds for unit events
	CLanguage *Language = nullptr;	/// the language used by the civilization
private:
	calendar *calendar = nullptr;	/// the calendar used by the civilization
public:
	CCurrency *Currency = nullptr;	/// the currency used by the civilization
private:
	bool visible = true; //whether the civilization is visible e.g. in the map editor
	bool playable = true; //civilizations are playable by default
	std::vector<civilization *> develops_from; //from which civilizations this civilization develops
	std::vector<civilization *> develops_to; //to which civilizations this civilization develops
	std::map<cursor_type, cursor *> cursors;
public:
	std::vector<quest *> Quests;	/// quests belonging to this civilization
	std::map<const CUpgrade *, int> UpgradePriorities;		/// Priority for each upgrade
	std::map<ForceType, std::vector<CForceTemplate *>> ForceTemplates;	/// Force templates, mapped to each force type
	std::map<ForceType, int> ForceTypeWeights;	/// Weights for each force type
	std::vector<CAiBuildingTemplate *> AiBuildingTemplates;	/// AI building templates
public:
	std::vector<std::string> ProvinceNames;		/// Province names for the civilization
private:
	std::map<const unit_class *, unit_type *> class_unit_types; //the unit type slot of a particular class for the civilization
	std::map<const upgrade_class *, CUpgrade *> class_upgrades; //the upgrade slot of a particular class for the civilization
	std::vector<CFiller> ui_fillers;
	std::vector<character *> characters;
public:
	std::vector<deity *> Deities;
	std::vector<site *> sites; //sites used for this civilization if a randomly-generated one is required
private:
	std::map<government_type, std::map<faction_tier, std::string>> title_names;
	std::map<int, std::map<int, std::map<government_type, std::map<faction_tier, std::map<gender, std::string>>>>> character_title_names;
	std::vector<CUpgrade *> acquired_upgrades;
public:
	std::map<std::string, std::map<CDate, bool>> HistoricalUpgrades;	/// historical upgrades of the faction, with the date of change

	friend int ::CclDefineCivilization(lua_State *l);
};

}