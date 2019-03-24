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
/**@name config.cpp - The config source file. */
//
//      (c) Copyright 2018-2019 by Andrettin
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

#include "config.h"

#include "age.h"
#include "animation/animation.h"
#include "character.h"
#include "config_operator.h"
#include "config_property.h"
#include "dynasty.h"
#include "economy/currency.h"
#include "game/game.h"
#include "game/trigger.h"
#include "hair_color.h"
#include "iocompat.h"
#include "iolib.h"
#include "language/language.h"
#include "language/word.h"
#include "literary_text.h"
#include "map/map_template.h"
#include "map/site.h"
#include "map/terrain_type.h"
#include "missile/missile_type.h"
#include "player_color.h"
#include "quest/campaign.h"
#include "religion/deity.h"
#include "religion/deity_domain.h"
#include "religion/pantheon.h"
#include "school_of_magic.h"
#include "skin_color.h"
#include "sound/sound.h"
#include "species/species.h"
#include "species/species_category.h"
#include "species/species_category_rank.h"
#include "spell/spells.h"
#include "time/calendar.h"
#include "time/season.h"
#include "time/season_schedule.h"
#include "time/time_of_day.h"
#include "time/time_of_day_schedule.h"
#include "time/timeline.h"
#include "ui/button_action.h"
#include "ui/button_level.h"
#include "ui/icon.h"
#include "unit/historical_unit.h"
#include "unit/unit_type.h"
#include "util.h"
#include "world/plane.h"
#include "world/world.h"

#include <fstream>

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**	@brief	Parse a configuration data file
**
**	@param	filepath	The path to the file holding the config data
**	@param	define_only	Whether the elements in the configuration data should only be defined with their ident, without their properties being processed
*/
void CConfigData::ParseConfigData(const std::string &filepath, const bool define_only)
{
	std::vector<std::string> data;
	std::vector<CConfigData *> output;
	
	if (!CanAccessFile(filepath.c_str())) {
		fprintf(stderr, "File \"%s\" not found.\n", filepath.c_str());
	}
	
	std::ifstream text_stream(filepath);
	std::string line;
	
	while (std::getline(text_stream, line)) {
		CConfigData::ParseLine(line, data);
	}
	
	if (data.empty()) {
		fprintf(stderr, "Could not get string data for config file \"%s\".\n", filepath.c_str());
		return;
	}
	
	CConfigData *config_data = nullptr;
	std::string key;
	CConfigOperator property_operator = CConfigOperator::None;
	std::string value;
	for (size_t i = 0; i < data.size(); ++i) {
		std::string str = data[i];
		if (str.size() >= 3 && str.front() == '[' && str[1] != '/' && str.back() == ']' && str.find(' ') == std::string::npos) { //opens a tag; exclude whitespace for the unlikely case when a text string (e.g. encyclopedia description) starts with '[' and ends with ']' due to BBCode
			std::string tag_name = str;
			tag_name = FindAndReplaceString(tag_name, "[", "");
			tag_name = FindAndReplaceString(tag_name, "]", "");
			CConfigData *new_config_data = new CConfigData(tag_name);
			if (config_data) {
				new_config_data->Parent = config_data;
			}
			config_data = new_config_data;
		} else if (str.size() >= 3 && str.front() == '[' && str[1] == '/' && str.back() == ']' && str.find(' ') == std::string::npos) { //closes a tag
			std::string tag_name = str;
			tag_name = FindAndReplaceString(tag_name, "[/", "");
			tag_name = FindAndReplaceString(tag_name, "]", "");
			if (config_data) { //closes current tag
				if (tag_name == config_data->Tag) {
					if (config_data->Parent == nullptr) {
						output.push_back(config_data);
						config_data = nullptr;
					} else {
						CConfigData *parent_config_data = config_data->Parent;
						parent_config_data->Sections.push_back(config_data);
						config_data = parent_config_data;
					}
				} else {
					fprintf(stderr, "Error parsing config file \"%s\": Tried closing tag \"%s\" while the open tag was \"%s\".\n", filepath.c_str(), tag_name.c_str(), config_data->Tag.c_str());
				}
			} else {
				fprintf(stderr, "Error parsing config file \"%s\": Tried closing a tag (\"%s\") before any tag had been opened.\n", filepath.c_str(), tag_name.c_str());
			}
		} else if (key.empty()) { //key
			if (config_data) {
				key = str;
			} else {
				fprintf(stderr, "Error parsing config file \"%s\": Tried defining key \"%s\" before any tag had been opened.\n", filepath.c_str(), str.c_str());
			}
		} else if (!key.empty() && property_operator == CConfigOperator::None) { //operator
			if (config_data) {
				if (str == "=") {
					property_operator = CConfigOperator::Assignment;
				} else if (str == "+=") {
					property_operator = CConfigOperator::Addition;
				} else if (str == "-=") {
					property_operator = CConfigOperator::Subtraction;
				} else {
					fprintf(stderr, "Error parsing config file \"%s\": Tried using operator \"%s\" for key \"%s\", but it is not a valid operator.\n", filepath.c_str(), str.c_str(), key.c_str());
				}
			} else {
				fprintf(stderr, "Error parsing config file \"%s\": Tried using operator \"%s\" for key \"%s\" without any tag being opened.\n", filepath.c_str(), str.c_str(), key.c_str());
			}
		} else if (property_operator != CConfigOperator::None) { //value
			if (config_data) {
				std::string value = str;
				if (key == "ident") {
					config_data->Ident = value;
				} else {
					config_data->Properties.push_back(CConfigProperty(key, property_operator, value));
				}
				key.clear();
				property_operator = CConfigOperator::None;
			} else {
				fprintf(stderr, "Error parsing config file \"%s\": Tried assigning value \"%s\" to key \"%s\" without any tag being opened.\n", filepath.c_str(), str.c_str(), key.c_str());
			}
		}
	}
	
	if (output.empty()) {
		fprintf(stderr, "Could not parse output for config file \"%s\".\n", filepath.c_str());
		return;
	}
	
	ProcessConfigData(output, define_only);
}

/**
**	@brief	Parse a line in a configuration data file
**
**	@param	line	The line to be parsed
**	@param	data	The vector holding the data file's output
*/
void CConfigData::ParseLine(const std::string &line, std::vector<std::string> &data)
{
	bool opened_quotation_marks = false;
	bool escaped = false;
	std::string current_string;
	
	for (const char &character : line) {
		if (!escaped) {
			if (character == '\"') {
				opened_quotation_marks = !opened_quotation_marks;
				continue;
			} else if (character == '\\') {
				escaped = true; //escape character, so that e.g. newlines can be properly added to text
				continue;
			}
		}
		
		if (!opened_quotation_marks) {
			if (character == '#') {
				break; //ignore what is written after the comment symbol ('#'), as well as the symbol itself, unless it occurs within quotes
			}
			
			//whitespace, carriage returns and etc. separate tokens, if they occur outside of quotes
			if (character == ' ' || character == '\t' || character == '\r' || character == '\n') {
				if (!current_string.empty()) {
					data.push_back(current_string);
					current_string.clear();
				}
				
				continue;
			}
		}
		
	
		if (escaped) {
			escaped = false;
			
			if (character == 'n') {
				current_string += '\n';
				continue;
			} else if (character == 't') {
				current_string += '\t';
				continue;
			} else if (character == 'r') {
				current_string += '\r';
				continue;
			} else if (character == '\"') {
				current_string += '\"';
				continue;
			} else if (character == '\\') {
				current_string += '\\';
				continue;
			}
		}
		
		current_string += character;
	}
	
	if (!current_string.empty()) {
		data.push_back(current_string);
	}
}

/**
**	@brief	Process data provided by a configuration file
**
**	@param	config_data_list	The list of configuration data
**	@param	define_only			Whether the elements in the configuration data should only be defined with their ident, without their properties being processed
*/
void CConfigData::ProcessConfigData(const std::vector<CConfigData *> &config_data_list, const bool define_only)
{
	for (size_t i = 0; i < config_data_list.size(); ++i) {
		CConfigData *config_data = config_data_list[i];
		std::string ident = config_data->Ident;
		ident = FindAndReplaceString(ident, "_", "-");
		
		if (ident.empty() && config_data->Tag != "button") {
			fprintf(stderr, "String identifier is empty for config data belonging to tag \"%s\".\n", config_data->Tag.c_str());
			continue;
		}
		
		if (config_data->Tag == "age") {
			CAge *age = CAge::GetOrAdd(ident);
			if (!define_only) {
				age->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "animations") {
			CAnimations *animations = AnimationsByIdent(ident);
			if (!animations) {
				animations = new CAnimations;
				AnimationMap[ident] = animations;
				animations->Ident = ident;
			}
			if (!define_only) {
				animations->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "button") {
			if (!define_only) {
				ButtonAction::ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "button_level") {
			CButtonLevel *button_level = CButtonLevel::GetOrAddButtonLevel(ident);
			if (!define_only) {
				button_level->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "calendar") {
			CCalendar *calendar = CCalendar::GetOrAddCalendar(ident);
			if (!define_only) {
				calendar->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "campaign") {
			CCampaign *campaign = CCampaign::GetOrAdd(ident);
			if (!define_only) {
				campaign->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "character") {
			CCharacter *character = nullptr;
			if (LoadingHistory) {
				//only load the history for characters that are already in the character database
				character = CCharacter::GetCharacter(ident);
			} else {
				character = CCharacter::GetOrAddCharacter(ident);
			}
			if (!character) {
				continue;
			}
			if (!define_only) {
				character->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "currency") {
			CCurrency *currency = CCurrency::GetOrAdd(ident);
			if (!define_only) {
				currency->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "deity") {
			CDeity *deity = CDeity::GetOrAddDeity(ident);
			if (!define_only) {
				deity->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "deity_domain") {
			CDeityDomain *deity_domain = CDeityDomain::GetOrAddDeityDomain(ident);
			if (!define_only) {
				deity_domain->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "dynasty") {
			CDynasty *dynasty = CDynasty::GetOrAdd(ident);
			if (!define_only) {
				dynasty->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "hair_color") {
			CHairColor *hair_color = CHairColor::GetOrAdd(ident);
			if (!define_only) {
				hair_color->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "historical_unit") {
			CHistoricalUnit *historical_unit = CHistoricalUnit::GetOrAdd(ident);
			if (!define_only) {
				historical_unit->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "icon") {
			CIcon *icon = CIcon::GetOrAdd(ident);
			if (!define_only) {
				icon->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "language") {
			CLanguage *language = CLanguage::GetOrAdd(ident);
			if (!define_only) {
				language->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "literary_text") {
			CLiteraryText *literary_text = CLiteraryText::GetOrAdd(ident);
			if (!define_only) {
				literary_text->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "map_template") {
			CMapTemplate *map_template = CMapTemplate::GetOrAdd(ident);
			if (!define_only) {
				map_template->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "missile_type") {
			MissileType *missile_type = MissileTypeByIdent(ident);
			if (!missile_type) {
				missile_type = NewMissileTypeSlot(ident);
			}
			if (!define_only) {
				missile_type->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "pantheon") {
			CPantheon *pantheon = CPantheon::GetOrAdd(ident);
			if (!define_only) {
				pantheon->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "plane") {
			CPlane *plane = CPlane::GetOrAddPlane(ident);
			if (!define_only) {
				plane->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "player_color") {
			CPlayerColor *player_color = CPlayerColor::GetOrAdd(ident);
			if (!define_only) {
				player_color->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "school_of_magic") {
			CSchoolOfMagic *school_of_magic = CSchoolOfMagic::GetOrAddSchoolOfMagic(ident);
			if (!define_only) {
				school_of_magic->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "season") {
			CSeason *season = CSeason::GetOrAdd(ident);
			if (!define_only) {
				season->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "season_schedule") {
			CSeasonSchedule *season_schedule = CSeasonSchedule::GetOrAdd(ident);
			if (!define_only) {
				season_schedule->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "site") {
			CSite *site = CSite::GetOrAdd(ident);
			if (!define_only) {
				site->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "skin_color") {
			CSkinColor *skin_color = CSkinColor::GetOrAdd(ident);
			if (!define_only) {
				skin_color->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "sound") {
			if (!define_only) {
				CSound::ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "species") {
			CSpecies *species = CSpecies::GetOrAdd(ident);
			if (!define_only) {
				species->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "species_category") {
			CSpeciesCategory *species_category = CSpeciesCategory::GetOrAdd(ident);
			if (!define_only) {
				species_category->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "species_category_rank") {
			CSpeciesCategoryRank *species_category_rank = CSpeciesCategoryRank::GetOrAdd(ident);
			if (!define_only) {
				species_category_rank->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "spell") {
			CSpell *spell = CSpell::GetOrAddSpell(ident);
			if (!define_only) {
				spell->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "terrain_type") {
			CTerrainType *terrain_type = CTerrainType::GetOrAddTerrainType(ident);
			if (!define_only) {
				terrain_type->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "timeline") {
			CTimeline *timeline = CTimeline::GetOrAdd(ident);
			if (!define_only) {
				timeline->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "time_of_day") {
			CTimeOfDay *time_of_day = CTimeOfDay::GetOrAdd(ident);
			if (!define_only) {
				time_of_day->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "time_of_day_schedule") {
			CTimeOfDaySchedule *time_of_day_schedule = CTimeOfDaySchedule::GetOrAdd(ident);
			if (!define_only) {
				time_of_day_schedule->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "trigger") {
			CTrigger *trigger = CTrigger::GetOrAddTrigger(ident);
			if (!define_only) {
				trigger->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "unit_type") {
			CUnitType *unit_type = UnitTypeByIdent(ident);
			if (!unit_type) {
				unit_type = NewUnitTypeSlot(ident);
			}
			if (!define_only) {
				unit_type->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "upgrade") {
			CUpgrade *upgrade = CUpgrade::New(ident);
			if (!define_only) {
				upgrade->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "word") {
			CWord *word = CWord::GetOrAdd(ident);
			if (!define_only) {
				word->ProcessConfigData(config_data);
			}
		} else if (config_data->Tag == "world") {
			CWorld *world = CWorld::GetOrAddWorld(ident);
			if (!define_only) {
				world->ProcessConfigData(config_data);
			}
		} else {
			fprintf(stderr, "Invalid data type: \"%s\".\n", config_data->Tag.c_str());
		}
	}
}

/**
**	@brief	Process color configuration data
**
**	@return	The color
*/
Color CConfigData::ProcessColor() const
{
	Color color;
	
	for (const CConfigProperty &property : this->Properties) {
		if (property.Operator != CConfigOperator::Assignment) {
			fprintf(stderr, "Wrong operator enumeration index for property \"%s\": %i.\n", property.Key.c_str(), property.Operator);
			continue;
		}
		
		if (property.Key == "red") {
			color.r = std::stoi(property.Value) / 255.0;
		} else if (property.Key == "green") {
			color.g = std::stoi(property.Value) / 255.0;
		} else if (property.Key == "blue") {
			color.b = std::stoi(property.Value) / 255.0;
		} else if (property.Key == "alpha") {
			color.a = std::stoi(property.Value) / 255.0;
		} else {
			fprintf(stderr, "Invalid color property: \"%s\".\n", property.Key.c_str());
		}
	}
	
	return color;
}
