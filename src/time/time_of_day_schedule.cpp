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
/**@name time_of_day_schedule.cpp - The time of day schedule source file. */
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

#include "time/time_of_day_schedule.h"

#include "config.h"
#include "time/season.h"
#include "time/time_of_day.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CTimeOfDaySchedule *CTimeOfDaySchedule::DefaultTimeOfDaySchedule = nullptr;
	
/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**	@brief	Destructor
*/
CTimeOfDaySchedule::~CTimeOfDaySchedule()
{
	for (CScheduledTimeOfDay *scheduled_time_of_day : this->ScheduledTimesOfDay) {
		delete scheduled_time_of_day;
	}
}

/**
**	@brief	Process data provided by a configuration file
**
**	@param	config_data	The configuration data
*/
void CTimeOfDaySchedule::ProcessConfigData(const CConfigData *config_data)
{
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->Name = value;
		} else if (key == "default_schedule") {
			const bool is_default_schedule = StringToBool(value);
			if (is_default_schedule) {
				CTimeOfDaySchedule::DefaultTimeOfDaySchedule = this;
			}
		} else {
			fprintf(stderr, "Invalid time of day schedule property: \"%s\".\n", key.c_str());
		}
	}
	
	for (const CConfigData *child_config_data : config_data->Children) {
		if (child_config_data->Tag == "scheduled_time_of_day") {
			CScheduledTimeOfDay *scheduled_time_of_day = new CScheduledTimeOfDay;
			scheduled_time_of_day->ID = this->ScheduledTimesOfDay.size();
			scheduled_time_of_day->Schedule = this;
			this->ScheduledTimesOfDay.push_back(scheduled_time_of_day);
			
			scheduled_time_of_day->ProcessConfigData(child_config_data);
		} else {
			fprintf(stderr, "Invalid time of day schedule property: \"%s\".\n", child_config_data->Tag.c_str());
		}
	}
	
	this->CalculateHourMultiplier();
}

/**
**	@brief	Process data provided by a configuration file
**
**	@param	config_data	The configuration data
*/
void CScheduledTimeOfDay::ProcessConfigData(const CConfigData *config_data)
{
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "time_of_day") {
			value = FindAndReplaceString(value, "_", "-");
			this->TimeOfDay = CTimeOfDay::GetTimeOfDay(value);
		} else if (key == "hours") {
			this->Schedule->TotalHours -= this->Hours; //remove old amount of hours from the schedule, if it has already been defined before
			this->Hours = std::stoi(value);
			this->Schedule->TotalHours += this->Hours;
		} else {
			fprintf(stderr, "Invalid scheduled time of day property: \"%s\".\n", key.c_str());
		}
	}
	
	for (const CConfigData *child_config_data : config_data->Children) {
		if (child_config_data->Tag == "season_hours") {
			CSeason *season = nullptr;
			int season_hours = 0;
			
			for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
				std::string key = child_config_data->Properties[j].first;
				std::string value = child_config_data->Properties[j].second;
				
				if (key == "season") {
					value = FindAndReplaceString(value, "_", "-");
					season = CSeason::Get(value);
				} else if (key == "hours") {
					season_hours = std::stoi(value);
				} else {
					fprintf(stderr, "Invalid season hours for scheduled time of day property: \"%s\".\n", key.c_str());
				}
			}
			
			if (!season) {
				fprintf(stderr, "Season hours for scheduled time of day has no valid season.\n");
				continue;
			}
			
			if (season_hours <= 0) {
				fprintf(stderr, "Season hours for scheduled time of day has no valid amount of hours.\n");
				continue;
			}
			
			this->SeasonHours[season] = season_hours;
		} else {
			fprintf(stderr, "Invalid scheduled time of day property: \"%s\".\n", child_config_data->Tag.c_str());
		}
	}
	
	if (!this->TimeOfDay) {
		fprintf(stderr, "Scheduled time of day has no valid time of day.\n");
	}
	
	if (this->Hours <= 0) {
		fprintf(stderr, "Scheduled time of day has no valid amount of hours.\n");
	}
}

/**
**	@brief	Get the amount of hours the scheduled time of day lasts
**
**	@param	season	Optional parameter; if given, gets the amount of hours for the given season, if defined
**
**	@return	The amount of hours
*/
int CScheduledTimeOfDay::GetHours(const CSeason *season) const
{
	if (season) {
		std::map<const CSeason *, int>::const_iterator find_iterator = this->SeasonHours.find(season);
		
		if (find_iterator != this->SeasonHours.end()) {
			return find_iterator->second;
		}
	}
	
	return this->Hours;
}
