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

#include "unit/unit.h"

#include "achievement.h"
#include "action/action_attack.h"
//Wyrmgus start
#include "action/action_resource.h"
#include "action/action_upgradeto.h"
//Wyrmgus end
#include "actions.h"
#include "ai.h"
//Wyrmgus start
#include "ai/ai_local.h" //for using AiHelpers
//Wyrmgus end
#include "animation.h"
#include "character.h"
#include "civilization.h"
#include "commands.h"
#include "construct.h"
#include "database/defines.h"
#include "faction.h"
#include "game.h"
#include "editor.h"
//Wyrmgus start
#include "grand_strategy.h"
//Wyrmgus end
//Wyrmgus start
#include "item.h"
//Wyrmgus end
#include "item_slot.h"
#include "luacallback.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/site.h"
#include "map/tileset.h"
#include "missile.h"
#include "network.h"
#include "objective_type.h"
#include "pathfinder.h"
#include "plane.h"
#include "player.h"
//Wyrmgus start
#include "quest.h"
//Wyrmgus end
#include "religion/deity.h"
#include "script.h"
#include "sound/sound.h"
#include "sound/sound_server.h"
#include "sound/unitsound.h"
#include "sound/unit_sound_type.h"
#include "spells.h"
#include "time/time_of_day.h"
#include "translate.h"
#include "ui/button.h"
#include "ui/interface.h"
#include "ui/ui.h"
#include "unit/unit_find.h"
#include "unit/unit_manager.h"
#include "unit/unit_type.h"
#include "unit/unit_type_type.h"
#include "unit/unit_type_variation.h"
#include "upgrade/dependency.h"
#include "upgrade/upgrade.h"
#include "upgrade/upgrade_modifier.h"
//Wyrmgus start
#include "util/util.h"
//Wyrmgus end
#include "util/vector_util.h"
#include "video.h"

/**
**  @class CUnit unit.h
**
**  \#include "unit/unit.h"
**
**  Everything belonging to a unit. FIXME: rearrange for less memory.
**
**  This class contains all information about a unit in game.
**  A unit could be anything: a man, a vehicle, a ship, or a building.
**  Currently only a tile, a unit, or a missile could be placed on the map.
**
**  The unit structure members:
**
**  CUnit::Refs
**
**  The reference counter of the unit. If the pointer to the unit
**  is stored the counter must be incremented and if this reference
**  is destroyed the counter must be decremented. Alternative it
**  would be possible to implement a garbage collector for this.
**
**  CUnit::Slot
**
**  This is the unique slot number. It is not possible that two
**  units have the same slot number at the same time. The slot
**  numbers are reused.
**  This field could be accessed by the macro UnitNumber(Unit *).
**
**  CUnit::UnitSlot
**
**  This is the index into #Units[], where the unit pointer is
**  stored.  #Units[] is a table of all units currently active in
**  game. This pointer is only needed to speed up, the remove of
**  the unit pointer from #Units[], it didn't must be searched in
**  the table.
**
**  CUnit::PlayerSlot
**
**  The index into Player::Units[], where the unit pointer is
**  stored. Player::Units[] is a table of all units currently
**  belonging to a player. This pointer is only needed to speed
**  up, the remove of the unit pointer from Player::Units[].
**
**  CUnit::Container
**
**  Pointer to the unit containing it, or null if the unit is
**  free. This points to the transporter for units on board, or to
**  the building for peasants inside(when they are mining).
**
**  CUnit::UnitInside
**
**  Pointer to the last unit added inside. Order doesn't really
**  matter. All units inside are kept in a circular linked list.
**  This is null if there are no units inside. Multiple levels
**  of inclusion are allowed, though not very useful right now
**
**  CUnit::NextContained, CUnit::PrevContained
**
**  The next and previous element in the curent container. Bogus
**  values allowed for units not contained.
**
**  CUnit::InsideCount
**
**  The number of units inside the container.
**
**  CUnit::BoardCount
**
**  The number of units transported inside the container. This
**  does not include for instance stuff like harvesters returning
**  cargo.
**
**  CUnit::tilePos
**
**  The tile map coordinates of the unit.
**  0,0 is the upper left on the map.
**
**  CUnit::Type
**
**  Pointer to the unit-type (::UnitType). The unit-type contains
**  all information that all units of the same type shares.
**  (Animations, Name, Stats, ...)
**
**  CUnit::SeenType
**  Pointer to the unit-type that this unit was, when last seen.
**  Currently only used by buildings.
**
**  CUnit::Player
**
**  Pointer to the owner of this unit (::Player). An unit could
**  only be owned by one player.
**
**  CUnit::Stats
**
**  Pointer to the current status (::UnitStats) of a unit. The
**  units of the same player and the same type could share the same
**  stats. The status contains all values which could be different
**  for each player. This f.e. the upgradeable abilities of an
**  unit.  (CUnit::Stats::SightRange, CUnit::Stats::Armor,
**  CUnit::Stats::HitPoints, ...)
**
**  CUnit::CurrentSightRange
**
**  Current sight range of a unit, this changes when a unit enters
**  a transporter or building or exits one of these.
**
**  CUnit::Colors
**
**  Player colors of the unit. Contains the hardware dependent
**  pixel values for the player colors (palette index #208-#211).
**  Setup from the global palette. This is a pointer.
**  @note Index #208-#211 are various SHADES of the team color
**  (#208 is brightest shade, #211 is darkest shade) .... these
**  numbers are NOT red=#208, blue=#209, etc
**
**  CUnit::IX CUnit::IY
**
**  Coordinate displacement in pixels or coordinates inside a tile.
**  Currently only !=0, if the unit is moving from one tile to
**  another (0-32 and for ships/flyers 0-64).
**
**  CUnit::Frame
**
**  Current graphic image of the animation sequence. The high bit
**  (128) is used to flip this image horizontal (x direction).
**  This also limits the number of different frames/image to 126.
**
**  CUnit::SeenFrame
**
**  Graphic image (see CUnit::Frame) what the player on this
**  computer has last seen. If UnitNotSeen the player haven't seen
**  this unit yet.
**
**  CUnit::Direction
**
**  Contains the binary angle (0-255) in which the direction the
**  unit looks. 0, 32, 64, 128, 160, 192, 224, 256 corresponds to
**  0?, 45?, 90?, 135?, 180?, 225?, 270?, 315?, 360? or north,
**  north-east, east, south-east, south, south-west, west,
**  north-west, north. Currently only 8 directions are used, this
**  is more for the future.
**
**  CUnit::Attacked
**
**  Last cycle the unit was attacked. 0 means never.
**
**  CUnit::Burning
**
**  If Burning is non-zero, the unit is burning.
**
**  CUnit::VisCount[PlayerMax]
**
**              Used to keep track of visible units on the map, it counts the
**              Number of seen tiles for each player. This is only modified
**              in UnitsMarkSeen and UnitsUnmarkSeen, from fow.
**              We keep track of visilibty for each player, and combine with
**              Shared vision ONLY when querying and such.
**
**  CUnit::SeenByPlayer
**
**              This is a bitmask of 1 and 0 values. SeenByPlayer & (1<<p) is 0
**              If p never saw the unit and 1 if it did. This is important for
**              keeping track of dead units under fog. We only keep track of units
**              that are visible under fog with this.
**
**  CUnit::Destroyed
**
** @todo docu.
**  If you need more information, please send me an email or write it self.
**
**  CUnit::Removed
**
**  This flag means the unit is not active on map. This flag
**  have workers set if they are inside a building, units that are
**  on board of a transporter.
**
**  CUnit::Selected
**
**  Unit is selected. (So you can give it orders)
**
**  CUnit::UnderConstruction
**  Set when a building is under construction, and still using the
**  generic building animation.
**
**  CUnit::SeenUnderConstruction
**  Last seen state of construction.  Used to draw correct building
**  frame. See CUnit::UnderConstruction for more information.
**
**  CUnit::SeenState
**  The Seen State of the building.
**  01 The building in being built when last seen.
**  10 The building was been upgraded when last seen.
**
**  CUnit::Boarded
**
**  This is 1 if the unit is on board a transporter.
**
**
**  CUnit::XP
**
**  Number of XP of the unit.
**
**  CUnit::Kills
**
**  How many units have been killed by the unit.
**
**  CUnit::GroupId
**
**  Number of the group to that the unit belongs. This is the main
**  group showed on map, a unit can belong to many groups.
**
**  CUnit::LastGroup
**
**  Automatic group number, to reselect the same units. When the
**  user selects more than one unit all units is given the next
**  same number. (Used for ALT-CLICK)
**
**  CUnit::Value
**
**  This values hold the amount of resources in a resource or in
**  in a harvester.
**  @todo continue documentation
**
**  CUnit::Wait
**
**  The unit is forced too wait for that many cycles. Be careful,
**  setting this to 0 will lock the unit.
**
**  CUnit::State
**
**  Animation state, currently position in the animation script.
**  0 if an animation has just started, it should only be changed
**  inside of actions.
**
**  CUnit::Reset
**
**  @todo continue documentation
**
**  CUnit::Blink
**
**
**  CUnit::Moving
**
**
**  CUnit::RescuedFrom
**
**  Pointer to the original owner of a unit. It will be null if
**  the unit was not rescued.
**
**  CUnit::Orders
**
**  Contains all orders of the unit. Slot 0 is always used.
**
**  CUnit::SavedOrder
**
**  This order is executed, if the current order is finished.
**  This is used for attacking units, to return to the old
**  place or for patrolling units to return to patrol after
**  killing some enemies. Any new order given to the unit,
**  clears this saved order.
**
**  CUnit::NewOrder
**
**  This field is only used by buildings and this order is
**  assigned to any by this building new trained unit.
**  This is can be used to set the exit or gathering point of a
**  building.
**
**  CUnit::Goal
**
**  Generic goal pointer. Used by teleporters to point to circle of power.
**
**
** @todo continue documentation
**
*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

bool EnableTrainingQueue;                 /// Config: training queues enabled
bool EnableBuildingCapture = false;               /// Config: capture buildings enabled
bool RevealAttacker;                      /// Config: reveal attacker enabled
int ResourcesMultiBuildersMultiplier = 0; /// Config: spend resources for building with multiple workers

static unsigned long HelpMeLastCycle;     /// Last cycle HelpMe sound played
static int HelpMeLastX;                   /// Last X coordinate HelpMe sound played
static int HelpMeLastY;                   /// Last Y coordinate HelpMe sound played

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void RemoveUnitFromContainer(CUnit &unit);

extern int ExtraDeathIndex(const char *death);

/**
**  Increase a unit's reference count.
*/
void CUnit::RefsIncrease()
{
	Assert(Refs && !Destroyed);
	if (!SaveGameLoading) {
		++Refs;
	}
}

/**
**  Decrease a unit's reference count.
*/
void CUnit::RefsDecrease()
{
	Assert(Refs);
	if (!SaveGameLoading) {
		if (Destroyed) {
			if (!--Refs) {
				Release();
			}
		} else {
			--Refs;
			Assert(Refs);
		}
	}
}

void CUnit::Init()
{
	Refs = 0;
	ReleaseCycle = 0;
	PlayerSlot = static_cast<size_t>(-1);
	InsideCount = 0;
	BoardCount = 0;
	UnitInside = nullptr;
	Container = nullptr;
	NextContained = nullptr;
	PrevContained = nullptr;
	NextWorker = nullptr;

	Resource.Workers = nullptr;
	Resource.Assigned = 0;
	Resource.Active = 0;
	
	for (int i = 0; i < static_cast<int>(stratagus::item_slot::count); ++i) {
		this->EquippedItems[i].clear();
	}
	this->SoldUnits.clear();

	tilePos.x = 0;
	tilePos.y = 0;
	//Wyrmgus start
	RallyPointPos.x = -1;
	RallyPointPos.y = -1;
	MapLayer = nullptr;
	RallyPointMapLayer = nullptr;
	//Wyrmgus end
	Offset = 0;
	Type = nullptr;
	Player = nullptr;
	Stats = nullptr;
	//Wyrmgus start
	Character = nullptr;
	this->settlement = nullptr;
	Trait = nullptr;
	Prefix = nullptr;
	Suffix = nullptr;
	Spell = nullptr;
	Work = nullptr;
	Elixir = nullptr;
	Unique = nullptr;
	Bound = false;
	Identified = true;
	ConnectingDestination = nullptr;
	//Wyrmgus end
	CurrentSightRange = 0;

	pathFinderData = new PathFinderData;
	pathFinderData->input.SetUnit(*this);

	//Wyrmgus start
	Name.clear();
	ExtraName.clear();
	this->surname.clear();
	Variation = 0;
	memset(LayerVariation, -1, sizeof(LayerVariation));
	//Wyrmgus end
	this->pixel_offset = QPoint(0, 0);
	Frame = 0;
	Direction = 0;
	DamagedType = ANIMATIONS_DEATHTYPES;
	Attacked = 0;
	Burning = 0;
	Destroyed = 0;
	Removed = 0;
	Selected = 0;
	TeamSelected = 0;
	UnderConstruction = 0;
	Active = 0;
	Boarded = 0;
	RescuedFrom = nullptr;
	memset(VisCount, 0, sizeof(VisCount));
	memset(&Seen, 0, sizeof(Seen));
	this->Variable.clear();
	TTL = 0;
	Threshold = 0;
	GroupId = 0;
	LastGroup = 0;
	ResourcesHeld = 0;
	Wait = 0;
	Blink = 0;
	Moving = 0;
	ReCast = 0;
	CacheLock = 0;
	Summoned = 0;
	Waiting = 0;
	MineLow = 0;
	memset(&Anim, 0, sizeof(Anim));
	memset(&WaitBackup, 0, sizeof(WaitBackup));
	GivesResource = 0;
	CurrentResource = 0;
	StepCount = 0;
	Orders.clear();
	delete SavedOrder;
	SavedOrder = nullptr;
	delete NewOrder;
	NewOrder = nullptr;
	delete CriticalOrder;
	CriticalOrder = nullptr;
	AutoCastSpell = nullptr;
	SpellCoolDownTimers = nullptr;
	AutoRepair = 0;
	Goal = nullptr;
	IndividualUpgrades.clear();
}

/**
**  Release an unit.
**
**  The unit is only released, if all references are dropped.
*/
void CUnit::Release(bool final)
{
	if (Type == nullptr) {
		DebugPrint("unit already free\n");
		return;
	}
	//Wyrmgus start
	if (Orders.size() != 1) {
		fprintf(stderr, "Unit to be released has more than 1 order; Unit Type: \"%s\", Orders: %d, First Order Type: %d.\n", this->Type->Ident.c_str(), (int)Orders.size(), this->CurrentAction());
	}
	//Wyrmgus end
	Assert(Orders.size() == 1);
	// Must be removed before here
	Assert(Removed);

	// First release, remove from lists/tables.
	if (!Destroyed) {
		DebugPrint("%d: First release %d\n" _C_ Player->Index _C_ UnitNumber(*this));

		// Are more references remaining?
		Destroyed = 1; // mark as destroyed

		if (Container && !final) {
			if (Boarded) {
				Container->BoardCount--;
			}
			MapUnmarkUnitSight(*this);
			RemoveUnitFromContainer(*this);
		}

		while (Resource.Workers) {
			Resource.Workers->DeAssignWorkerFromMine(*this);
		}

		if (--Refs > 0) {
			return;
		}
	}

	Assert(!Refs);

	//
	// No more references remaining, but the network could have an order
	// on the way. We must wait a little time before we could free the
	// memory.
	//

	Type = nullptr;
	//Wyrmgus start
	Character = nullptr;
	if (this->settlement != nullptr && this->settlement->get_site_unit() == this) {
		this->settlement->set_site_unit(nullptr);
	}
	this->settlement = nullptr;
	Trait = nullptr;
	Prefix = nullptr;
	Suffix = nullptr;
	Spell = nullptr;
	Work = nullptr;
	Elixir = nullptr;
	Unique = nullptr;
	Bound = false;
	Identified = true;
	ConnectingDestination = nullptr;
	
	for (int i = 0; i < static_cast<int>(stratagus::item_slot::count); ++i) {
		this->EquippedItems[i].clear();
	}
	this->SoldUnits.clear();
	//Wyrmgus end

	delete pathFinderData;
	delete[] AutoCastSpell;
	delete[] SpellCoolDownTimers;
	this->Variable.clear();
	for (std::vector<COrder *>::iterator order = Orders.begin(); order != Orders.end(); ++order) {
		delete *order;
	}
	Orders.clear();

	// Remove the unit from the global units table.
	UnitManager.ReleaseUnit(this);
}

//Wyrmgus start
void CUnit::SetResourcesHeld(int quantity)
{
	this->ResourcesHeld = quantity;
	
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (
		variation
		&& (
			(variation->ResourceMin && this->ResourcesHeld < variation->ResourceMin)
			|| (variation->ResourceMax && this->ResourcesHeld > variation->ResourceMax)
		)
	) {
		this->ChooseVariation();
	}
}

void CUnit::ChangeResourcesHeld(int quantity)
{
	this->SetResourcesHeld(this->ResourcesHeld + quantity);
}

void CUnit::ReplaceOnTop(CUnit &replaced_unit)
{
	if (replaced_unit.Unique != nullptr) {
		this->SetUnique(replaced_unit.Unique);
	} else {
		if (replaced_unit.Prefix != nullptr) {
			this->SetPrefix(replaced_unit.Prefix);
		}
		if (replaced_unit.Suffix != nullptr) {
			this->SetSuffix(replaced_unit.Suffix);
		}
		if (replaced_unit.Spell != nullptr) {
			this->SetSpell(replaced_unit.Spell);
		}
	}
	if (replaced_unit.settlement != nullptr) {
		this->settlement = replaced_unit.settlement;
		if (this->Type->BoolFlag[TOWNHALL_INDEX].value) {
			this->settlement->set_site_unit(this);
			CMap::Map.site_units.erase(std::remove(CMap::Map.site_units.begin(), CMap::Map.site_units.end(), &replaced_unit), CMap::Map.site_units.end());
			CMap::Map.site_units.push_back(this);
		}
	}
	
	this->SetResourcesHeld(replaced_unit.ResourcesHeld); // We capture the value of what is beneath.
	this->Variable[GIVERESOURCE_INDEX].Value = replaced_unit.Variable[GIVERESOURCE_INDEX].Value;
	this->Variable[GIVERESOURCE_INDEX].Max = replaced_unit.Variable[GIVERESOURCE_INDEX].Max;
	this->Variable[GIVERESOURCE_INDEX].Enable = replaced_unit.Variable[GIVERESOURCE_INDEX].Enable;
	
	replaced_unit.Remove(nullptr); // Destroy building beneath
	UnitLost(replaced_unit);
	UnitClearOrders(replaced_unit);
	replaced_unit.Release();
}

void CUnit::ChangeExperience(int amount, int around_range)
{
	std::vector<CUnit *> table;
	if (around_range > 0) {
		SelectAroundUnit(*this, around_range, table, MakeAndPredicate(HasSamePlayerAs(*this->Player), IsNotBuildingType()));
	}
	
	amount /= 1 + table.size();

	if (this->Type->BoolFlag[ORGANIC_INDEX].value) {
		this->Variable[XP_INDEX].Max += amount;
		this->Variable[XP_INDEX].Value = this->Variable[XP_INDEX].Max;
		this->XPChanged();
	}

	if (around_range > 0) {
		for (size_t i = 0; i != table.size(); ++i) {
			if (table[i]->Type->BoolFlag[ORGANIC_INDEX].value) {
				table[i]->Variable[XP_INDEX].Max += amount;
				table[i]->Variable[XP_INDEX].Value = table[i]->Variable[XP_INDEX].Max;
				table[i]->XPChanged();
			}
		}
	}
}

void CUnit::IncreaseLevel(int level_quantity, bool automatic_learning)
{
	while (level_quantity > 0) {
		this->Variable[LEVEL_INDEX].Value += 1;
		if (this->Type->Stats[this->Player->Index].Variables[LEVEL_INDEX].Value < this->Variable[LEVEL_INDEX].Value) {
			if (GetAvailableLevelUpUpgrades(true) == 0 || (this->Variable[LEVEL_INDEX].Value - this->Type->Stats[this->Player->Index].Variables[LEVEL_INDEX].Value) > 1) {
				this->Variable[POINTS_INDEX].Max += 5 * (this->Variable[LEVEL_INDEX].Value + 1);
				this->Variable[POINTS_INDEX].Value += 5 * (this->Variable[LEVEL_INDEX].Value + 1);
			}
			
			this->Variable[LEVELUP_INDEX].Value += 1;
			this->Variable[LEVELUP_INDEX].Max = this->Variable[LEVELUP_INDEX].Value;
			// if there are no level-up upgrades available for the unit, increase its HP instead
			if (this->GetAvailableLevelUpUpgrades() < this->Variable[LEVELUP_INDEX].Value) {
				this->Variable[HP_INDEX].Max += 10;
				this->Variable[LEVELUP_INDEX].Value -= 1;
				this->Variable[LEVELUP_INDEX].Max = this->Variable[LEVELUP_INDEX].Value;
			}
		}
		this->Variable[HP_INDEX].Value = this->GetModifiedVariable(HP_INDEX, VariableMax);
		level_quantity -= 1;
	}
	
	UpdateXPRequired();
	
	bool upgrade_found = true;
	while (this->Variable[LEVELUP_INDEX].Value > 0 && upgrade_found && automatic_learning) {
		upgrade_found = false;

		if (((int) AiHelpers.ExperienceUpgrades.size()) > Type->Slot) {
			std::vector<stratagus::unit_type *> potential_upgrades;
			
			if ((this->Player->AiEnabled || this->Character == nullptr) && this->Type->BoolFlag[HARVESTER_INDEX].value && this->CurrentResource && AiHelpers.ExperienceUpgrades[Type->Slot].size() > 1) {
				//if is a harvester who is currently gathering, try to upgrade to a unit type which is best for harvesting the current resource
				unsigned int best_gathering_rate = 0;
				for (size_t i = 0; i != AiHelpers.ExperienceUpgrades[Type->Slot].size(); ++i) {
					stratagus::unit_type *experience_upgrade_type = AiHelpers.ExperienceUpgrades[Type->Slot][i];
					if (CheckDependencies(experience_upgrade_type, this, true)) {
						if (this->Character == nullptr || std::find(this->Character->ForbiddenUpgrades.begin(), this->Character->ForbiddenUpgrades.end(), experience_upgrade_type) == this->Character->ForbiddenUpgrades.end()) {
							if (!experience_upgrade_type->ResInfo[this->CurrentResource]) {
								continue;
							}
							unsigned int gathering_rate = experience_upgrade_type->GetResourceStep(this->CurrentResource, this->Player->Index);
							if (gathering_rate >= best_gathering_rate) {
								if (gathering_rate > best_gathering_rate) {
									best_gathering_rate = gathering_rate;
									potential_upgrades.clear();
								}
								potential_upgrades.push_back(experience_upgrade_type);
							}
						}
					}
				}
			} else if (this->Player->AiEnabled || (this->Character == nullptr && AiHelpers.ExperienceUpgrades[Type->Slot].size() == 1)) {
				for (size_t i = 0; i != AiHelpers.ExperienceUpgrades[Type->Slot].size(); ++i) {
					if (CheckDependencies(AiHelpers.ExperienceUpgrades[Type->Slot][i], this, true)) {
						if (this->Character == nullptr || std::find(this->Character->ForbiddenUpgrades.begin(), this->Character->ForbiddenUpgrades.end(), AiHelpers.ExperienceUpgrades[Type->Slot][i]) == this->Character->ForbiddenUpgrades.end()) {
							potential_upgrades.push_back(AiHelpers.ExperienceUpgrades[Type->Slot][i]);
						}
					}
				}
			}
			
			if (potential_upgrades.size() > 0) {
				this->Variable[LEVELUP_INDEX].Value -= 1;
				this->Variable[LEVELUP_INDEX].Max = this->Variable[LEVELUP_INDEX].Value;
				stratagus::unit_type *chosen_unit_type = potential_upgrades[SyncRand(potential_upgrades.size())];
				if (this->Player == CPlayer::GetThisPlayer()) {
					this->Player->Notify(NotifyGreen, this->tilePos, this->MapLayer->ID, _("%s has upgraded to %s!"), this->GetMessageName().c_str(), chosen_unit_type->get_name().c_str());
				}
				TransformUnitIntoType(*this, *chosen_unit_type);
				upgrade_found = true;
			}
		}
			
		if ((this->Player->AiEnabled || this->Character == nullptr) && this->Variable[LEVELUP_INDEX].Value) {
			if (((int) AiHelpers.LearnableAbilities.size()) > Type->Slot) {
				std::vector<CUpgrade *> potential_abilities;
				for (size_t i = 0; i != AiHelpers.LearnableAbilities[Type->Slot].size(); ++i) {
					if (CanLearnAbility(AiHelpers.LearnableAbilities[Type->Slot][i])) {
						potential_abilities.push_back(AiHelpers.LearnableAbilities[Type->Slot][i]);
					}
				}
				if (potential_abilities.size() > 0) {
					if (potential_abilities.size() == 1 || this->Player->AiEnabled) { //if can only acquire one particular ability, get it automatically
						CUpgrade *chosen_ability = potential_abilities[SyncRand(potential_abilities.size())];
						AbilityAcquire(*this, chosen_ability);
						upgrade_found = true;
						if (this->Player == CPlayer::GetThisPlayer()) {
							this->Player->Notify(NotifyGreen, this->tilePos, this->MapLayer->ID, _("%s has acquired the %s ability!"), this->GetMessageName().c_str(), chosen_ability->get_name().c_str());
						}
					}
				}
			}
		}
	}
	
	this->Variable[LEVELUP_INDEX].Enable = 1;
	
	this->Player->UpdateLevelUpUnits();
}

void CUnit::Retrain()
{
	//lose all abilities (the AbilityLost function also returns the level-ups to the unit)
	for (CUpgrade *upgrade : CUpgrade::get_all()) {
		if (this->GetIndividualUpgrade(upgrade)) {
			if (upgrade->is_ability() && std::find(this->Type->StartingAbilities.begin(), this->Type->StartingAbilities.end(), upgrade) == this->Type->StartingAbilities.end()) {
				AbilityLost(*this, upgrade, true);
			} else if (!strncmp(upgrade->Ident.c_str(), "upgrade-deity-", 14) && strncmp(upgrade->Ident.c_str(), "upgrade-deity-domain-", 21) && this->Character && this->Character->Custom) { //allow changing the deity for custom heroes
				IndividualUpgradeLost(*this, upgrade, true);
			}
		}
	}
	
	std::string unit_name = GetMessageName();
	
	//now, revert the unit's type to the level 1 one
	while (this->Type->Stats[this->Player->Index].Variables[LEVEL_INDEX].Value > 1) {
		bool found_previous_unit_type = false;
		for (stratagus::unit_type *unit_type : stratagus::unit_type::get_all()) {
			if (this->Character != nullptr && std::find(this->Character->ForbiddenUpgrades.begin(), this->Character->ForbiddenUpgrades.end(), unit_type) != this->Character->ForbiddenUpgrades.end()) {
				continue;
			}
			if (((int) AiHelpers.ExperienceUpgrades.size()) > unit_type->Slot) {
				for (size_t j = 0; j != AiHelpers.ExperienceUpgrades[unit_type->Slot].size(); ++j) {
					if (AiHelpers.ExperienceUpgrades[unit_type->Slot][j] == this->Type) {
						this->Variable[LEVELUP_INDEX].Value += 1;
						this->Variable[LEVELUP_INDEX].Max = this->Variable[LEVELUP_INDEX].Value;
						this->Variable[LEVELUP_INDEX].Enable = 1;
						TransformUnitIntoType(*this, *unit_type);
						if (!IsNetworkGame() && Character != nullptr) {	//save the unit-type experience upgrade for persistent characters
							if (Character->get_unit_type() != unit_type) {
								if (this->Player == CPlayer::GetThisPlayer()) {
									Character->set_unit_type(unit_type);
									SaveHero(Character);
									CAchievement::CheckAchievements();
								}
							}
						}
						found_previous_unit_type = true;
						break;
					}
				}
			}
			if (found_previous_unit_type) {
				break;
			}
		}
		if (!found_previous_unit_type) {
			break;
		}
	}
	
	if (this->Player == CPlayer::GetThisPlayer()) {
		this->Player->Notify(NotifyGreen, this->tilePos, this->MapLayer->ID, _("%s's level-up choices have been reset."), unit_name.c_str());
	}
}

void CUnit::HealingItemAutoUse()
{
	if (!HasInventory()) {
		return;
	}
	
	CUnit *uins = this->UnitInside;
	
	for (int i = 0; i < this->InsideCount; ++i, uins = uins->NextContained) {
		if (!uins->Type->BoolFlag[ITEM_INDEX].value || uins->Elixir) {
			continue;
		}
		
		if (!stratagus::is_consumable_item_class(uins->Type->get_item_class())) {
			continue;
		}
		
		if (uins->Variable[HITPOINTHEALING_INDEX].Value > 0) {
			if (
				uins->Variable[HITPOINTHEALING_INDEX].Value <= (this->GetModifiedVariable(HP_INDEX, VariableMax) - this->Variable[HP_INDEX].Value)
				|| (this->Variable[HP_INDEX].Value * 100 / this->GetModifiedVariable(HP_INDEX, VariableMax)) <= 20 // use a healing item if has less than 20% health
			) {
				if (!this->CriticalOrder) {
					this->CriticalOrder = COrder::NewActionUse(*uins);
				}
				break;
			}
		}
	}
}

void CUnit::SetCharacter(const std::string &character_ident, bool custom_hero)
{
	if (this->CurrentAction() == UnitAction::Die) {
		return;
	}
	
	if (this->Character != nullptr) {
		this->Player->Heroes.erase(std::remove(this->Player->Heroes.begin(), this->Player->Heroes.end(), this), this->Player->Heroes.end());
		
		this->Variable[HERO_INDEX].Max = this->Variable[HERO_INDEX].Value = this->Variable[HERO_INDEX].Enable = 0;
	}
	
	stratagus::character *character = nullptr;
	if (!custom_hero) {
		character = stratagus::character::get(character_ident);
	} else {
		character = GetCustomHero(character_ident);
	}
	
	if (character) {
		this->Character = character;
	} else {
		fprintf(stderr, "Character \"%s\" doesn't exist.\n", character_ident.c_str());
		return;
	}
		
	int old_mana_percent = 0;
	if (this->Variable[MANA_INDEX].Max > 0) {
		old_mana_percent = this->Variable[MANA_INDEX].Value * 100 / this->Variable[MANA_INDEX].Max;
	}
	
	this->Name = this->Character->get_name();
	this->ExtraName = this->Character->ExtraName;
	this->surname = this->Character->get_surname();
	
	if (this->Character->get_unit_type() != nullptr) {
		if (this->Character->get_unit_type() != this->Type) { //set type to that of the character
			TransformUnitIntoType(*this, *this->Character->get_unit_type());
		}
		
		this->Variable = this->Character->get_unit_type()->Stats[this->Player->Index].Variables;
	} else {
		fprintf(stderr, "Character \"%s\" has no unit type.\n", character_ident.c_str());
		return;
	}
	
	this->IndividualUpgrades.clear(); //reset the individual upgrades and then apply the character's
	this->Trait = nullptr;
	
	if (this->Type->get_civilization() != nullptr) {
		CUpgrade *civilization_upgrade = this->Type->get_civilization()->get_upgrade();
		if (civilization_upgrade != nullptr) {
			this->SetIndividualUpgrade(civilization_upgrade, 1);
		}
	}
	if (this->Type->Faction != -1 && !stratagus::faction::get_all()[this->Type->Faction]->FactionUpgrade.empty()) {
		CUpgrade *faction_upgrade = CUpgrade::try_get(stratagus::faction::get_all()[this->Type->Faction]->FactionUpgrade);
		if (faction_upgrade) {
			this->SetIndividualUpgrade(faction_upgrade, 1);
		}
	}

	if (this->Character->Trait != nullptr) { //set trait
		TraitAcquire(*this, this->Character->Trait);
	} else if (Editor.Running == EditorNotRunning && this->Type->Traits.size() > 0) {
		TraitAcquire(*this, this->Type->Traits[SyncRand(this->Type->Traits.size())]);
	}
	
	if (this->Character->Deity != nullptr && this->Character->Deity->CharacterUpgrade != nullptr) {
		IndividualUpgradeAcquire(*this, this->Character->Deity->CharacterUpgrade);
	}
	
	//load worshipped deities
	for (size_t i = 0; i < this->Character->Deities.size(); ++i) {
		CUpgrade *deity_upgrade = this->Character->Deities[i]->DeityUpgrade;
		if (deity_upgrade) {
			IndividualUpgradeAcquire(*this, deity_upgrade);
		}
	}
	
	for (const CUpgrade *ability_upgrade : this->Type->StartingAbilities) {
		if (CheckDependencies(ability_upgrade, this)) {
			IndividualUpgradeAcquire(*this, ability_upgrade);
		}
	}
	
	this->Variable[LEVEL_INDEX].Max = 100000; // because the code above sets the max level to the unit type stats' Level variable (which is the same as its value)
	if (this->Variable[LEVEL_INDEX].Value < this->Character->Level) {
		this->IncreaseLevel(this->Character->Level - this->Variable[LEVEL_INDEX].Value, false);
	}
	
	this->Variable[XP_INDEX].Enable = 1;
	this->Variable[XP_INDEX].Value = this->Variable[XPREQUIRED_INDEX].Value * this->Character->ExperiencePercent / 100;
	this->Variable[XP_INDEX].Max = this->Variable[XP_INDEX].Value;
	
	if (this->Variable[MANA_INDEX].Max > 0) {
		this->Variable[MANA_INDEX].Value = this->Variable[MANA_INDEX].Max * old_mana_percent / 100;
	}
			
	//load learned abilities
	std::vector<const CUpgrade *> abilities_to_remove;
	for (size_t i = 0; i < this->Character->Abilities.size(); ++i) {
		if (CanLearnAbility(this->Character->Abilities[i])) {
			AbilityAcquire(*this, this->Character->Abilities[i], false);
		} else { //can't learn the ability? something changed in the game's code, remove it from persistent data and allow the hero to repick the ability
			abilities_to_remove.push_back(this->Character->Abilities[i]);
		}
	}
	
	if (!abilities_to_remove.empty()) {
		for (size_t i = 0; i < abilities_to_remove.size(); ++i) {
			stratagus::vector::remove(this->Character->Abilities, abilities_to_remove[i]);
		}

		if (this->Player == CPlayer::GetThisPlayer()) {
			SaveHero(this->Character);
		}
	}
	
	//load read works
	for (size_t i = 0; i < this->Character->ReadWorks.size(); ++i) {
		ReadWork(this->Character->ReadWorks[i], false);
	}
	
	//load consumed elixirs
	for (size_t i = 0; i < this->Character->ConsumedElixirs.size(); ++i) {
		ConsumeElixir(this->Character->ConsumedElixirs[i], false);
	}
	
	//load items
	for (size_t i = 0; i < this->Character->Items.size(); ++i) {
		CUnit *item = MakeUnitAndPlace(this->tilePos, *this->Character->Items[i]->Type, CPlayer::Players[PlayerNumNeutral], this->MapLayer->ID);
		if (this->Character->Items[i]->Prefix != nullptr) {
			item->SetPrefix(this->Character->Items[i]->Prefix);
		}
		if (this->Character->Items[i]->Suffix != nullptr) {
			item->SetSuffix(this->Character->Items[i]->Suffix);
		}
		if (this->Character->Items[i]->Spell != nullptr) {
			item->SetSpell(this->Character->Items[i]->Spell);
		}
		if (this->Character->Items[i]->Work != nullptr) {
			item->SetWork(this->Character->Items[i]->Work);
		}
		if (this->Character->Items[i]->Elixir != nullptr) {
			item->SetElixir(this->Character->Items[i]->Elixir);
		}
		item->Unique = this->Character->Items[i]->Unique;
		if (!this->Character->Items[i]->Name.empty()) {
			item->Name = this->Character->Items[i]->Name;
		}
		item->Bound = this->Character->Items[i]->Bound;
		item->Identified = this->Character->Items[i]->Identified;
		item->Remove(this);
		if (this->Character->IsItemEquipped(this->Character->Items[i])) {
			EquipItem(*item, false);
		}
	}
	
	if (this->Character != nullptr) {
		this->Player->Heroes.push_back(this);
	}

	this->Variable[HERO_INDEX].Max = this->Variable[HERO_INDEX].Value = this->Variable[HERO_INDEX].Enable = 1;
	
	this->ChooseVariation(); //choose a new variation now
	for (int i = 0; i < MaxImageLayers; ++i) {
		ChooseVariation(nullptr, false, i);
	}
	this->UpdateButtonIcons();
	this->UpdateXPRequired();
}

bool CUnit::CheckTerrainForVariation(const stratagus::unit_type_variation *variation) const
{
	//if the variation has one or more terrain set as a precondition, then all tiles underneath the unit must match at least one of those terrains
	if (variation->Terrains.size() > 0) {
		if (!CMap::Map.Info.IsPointOnMap(this->tilePos, this->MapLayer)) {
			return false;
		}

		bool terrain_check = true;
		for (int x = 0; x < this->Type->get_tile_width(); ++x) {
			for (int y = 0; y < this->Type->get_tile_height(); ++y) {
				if (CMap::Map.Info.IsPointOnMap(this->tilePos + Vec2i(x, y), this->MapLayer)) {
					if (!stratagus::vector::contains(variation->Terrains, CMap::Map.GetTileTopTerrain(this->tilePos + Vec2i(x, y), false, this->MapLayer->ID, true))) {
						return false;
					}
				}
			}
		}
	}
	
	//if the variation has one or more terrains set as a forbidden precondition, then no tiles underneath the unit may match one of those terrains
	if (variation->TerrainsForbidden.size() > 0) {
		if (CMap::Map.Info.IsPointOnMap(this->tilePos, this->MapLayer)) {
			for (int x = 0; x < this->Type->get_tile_width(); ++x) {
				for (int y = 0; y < this->Type->get_tile_height(); ++y) {
					if (CMap::Map.Info.IsPointOnMap(this->tilePos + Vec2i(x, y), this->MapLayer)) {
						if (stratagus::vector::contains(variation->TerrainsForbidden, CMap::Map.GetTileTopTerrain(this->tilePos + Vec2i(x, y), false, this->MapLayer->ID, true))) {
							return false;
						}
					}
				}
			}
		}
	}
	
	return true;
}

bool CUnit::CheckSeasonForVariation(const stratagus::unit_type_variation *variation) const
{
	if (
		!variation->Seasons.empty()
		&& (!this->MapLayer || std::find(variation->Seasons.begin(), variation->Seasons.end(), this->MapLayer->GetSeason()) == variation->Seasons.end())
	) {
		return false;
	}
	
	if (
		!variation->ForbiddenSeasons.empty()
		&& this->MapLayer
		&& std::find(variation->ForbiddenSeasons.begin(), variation->ForbiddenSeasons.end(), this->MapLayer->GetSeason()) != variation->ForbiddenSeasons.end()
	) {
		return false;
	}
	
	return true;
}

void CUnit::ChooseVariation(const stratagus::unit_type *new_type, bool ignore_old_variation, int image_layer)
{
	std::string priority_variation;
	if (image_layer == -1) {
		if (this->Character != nullptr && !this->Character->get_variation().empty()) {
			priority_variation = this->Character->get_variation();
		} else if (this->GetVariation() != nullptr) {
			priority_variation = this->GetVariation()->get_identifier();
		}
	} else {
		if (image_layer == HairImageLayer && this->Character != nullptr && !this->Character->get_variation().empty()) {
			priority_variation = this->Character->get_variation();
		} else if (this->GetLayerVariation(image_layer)) {
			priority_variation = this->GetLayerVariation(image_layer)->get_identifier();
		}
	}
	
	std::vector<stratagus::unit_type_variation *> type_variations;
	const std::vector<std::unique_ptr<stratagus::unit_type_variation>> &variation_list = image_layer == -1 ? (new_type != nullptr ? new_type->get_variations() : this->Type->get_variations()) : (new_type != nullptr ? new_type->LayerVariations[image_layer] : this->Type->LayerVariations[image_layer]);
	
	bool found_similar = false;
	for (const auto &variation : variation_list) {
		if (variation->ResourceMin && this->ResourcesHeld < variation->ResourceMin) {
			continue;
		}
		if (variation->ResourceMax && this->ResourcesHeld > variation->ResourceMax) {
			continue;
		}
		
		if (!this->CheckSeasonForVariation(variation.get())) {
			continue;
		}
		
		if (!this->CheckTerrainForVariation(variation.get())) {
			continue;
		}
		
		bool upgrades_check = true;
		bool requires_weapon = false;
		bool found_weapon = false;
		bool requires_shield = false;
		bool found_shield = false;
		for (const CUpgrade *required_upgrade : variation->UpgradesRequired) {
			if (required_upgrade->is_weapon()) {
				requires_weapon = true;
				if (UpgradeIdentAllowed(*this->Player, required_upgrade->Ident.c_str()) == 'R' || this->GetIndividualUpgrade(required_upgrade)) {
					found_weapon = true;
				}
			} else if (required_upgrade->is_shield()) {
				requires_shield = true;
				if (UpgradeIdentAllowed(*this->Player, required_upgrade->Ident.c_str()) == 'R' || this->GetIndividualUpgrade(required_upgrade)) {
					found_shield = true;
				}
			} else if (UpgradeIdentAllowed(*this->Player, required_upgrade->Ident.c_str()) != 'R' && this->GetIndividualUpgrade(required_upgrade) == false) {
				upgrades_check = false;
				break;
			}
		}
		
		if (upgrades_check) {
			for (const CUpgrade *forbidden_upgrade : variation->UpgradesForbidden) {
				if (UpgradeIdentAllowed(*this->Player, forbidden_upgrade->Ident.c_str()) == 'R' || this->GetIndividualUpgrade(forbidden_upgrade)) {
					upgrades_check = false;
					break;
				}
			}
		}
		
		for (const stratagus::item_class item_class_not_equipped : variation->item_classes_not_equipped) {
			if (this->is_item_class_equipped(item_class_not_equipped)) {
				upgrades_check = false;
				break;
			}
		}
		for (size_t j = 0; j < variation->ItemsNotEquipped.size(); ++j) {
			if (this->IsItemTypeEquipped(variation->ItemsNotEquipped[j])) {
				upgrades_check = false;
				break;
			}
		}
		if (upgrades_check == false) {
			continue;
		}
		for (const stratagus::item_class item_class_equipped : variation->item_classes_equipped) {
			if (stratagus::get_item_class_slot(item_class_equipped) == stratagus::item_slot::weapon) {
				requires_weapon = true;
				if (is_item_class_equipped(item_class_equipped)) {
					found_weapon = true;
				}
			} else if (stratagus::get_item_class_slot(item_class_equipped) == stratagus::item_slot::shield) {
				requires_shield = true;
				if (is_item_class_equipped(item_class_equipped)) {
					found_shield = true;
				}
			}
		}
		for (size_t j = 0; j < variation->ItemsEquipped.size(); ++j) {
			if (stratagus::get_item_class_slot(variation->ItemsEquipped[j]->get_item_class()) == stratagus::item_slot::weapon) {
				requires_weapon = true;
				if (this->IsItemTypeEquipped(variation->ItemsEquipped[j])) {
					found_weapon = true;
				}
			} else if (stratagus::get_item_class_slot(variation->ItemsEquipped[j]->get_item_class()) == stratagus::item_slot::shield) {
				requires_shield = true;
				if (this->IsItemTypeEquipped(variation->ItemsEquipped[j])) {
					found_shield = true;
				}
			}
		}
		if ((requires_weapon && !found_weapon) || (requires_shield && !found_shield)) {
			continue;
		}
		if (!ignore_old_variation && !priority_variation.empty() && (variation->get_identifier().find(priority_variation) != std::string::npos || priority_variation.find(variation->get_identifier()) != std::string::npos)) { // if the priority variation's ident is included in that of a new viable variation (or vice-versa), give priority to the new variation over others
			if (!found_similar) {
				found_similar = true;
				type_variations.clear();
			}
		} else {
			if (found_similar) {
				continue;
			}
		}
		for (int j = 0; j < variation->Weight; ++j) {
			type_variations.push_back(variation.get());
		}
	}
	if (type_variations.size() > 0) {
		this->SetVariation(type_variations[SyncRand(type_variations.size())], new_type, image_layer);
	}
}

void CUnit::SetVariation(stratagus::unit_type_variation *new_variation, const stratagus::unit_type *new_type, int image_layer)
{
	if (image_layer == -1) {
		if (
			(this->GetVariation() && this->GetVariation()->Animations)
			|| (new_variation && new_variation->Animations)
		) { //if the old (if any) or the new variation has specific animations, set the unit's frame to its type's still frame
			this->Frame = this->Type->StillFrame;
		}
		this->Variation = new_variation ? new_variation->get_index() : 0;
	} else {
		this->LayerVariation[image_layer] = new_variation ? new_variation->get_index() : -1;
	}
}

const stratagus::unit_type_variation *CUnit::GetVariation() const
{
	if (this->Variation < static_cast<int>(this->Type->get_variations().size())) {
		return this->Type->get_variations()[this->Variation].get();
	}
	
	return nullptr;
}

const stratagus::unit_type_variation *CUnit::GetLayerVariation(const unsigned int image_layer) const
{
	if (this->LayerVariation[image_layer] >= 0 && this->LayerVariation[image_layer] < (int) this->Type->LayerVariations[image_layer].size()) {
		return this->Type->LayerVariations[image_layer][this->LayerVariation[image_layer]].get();
	}
	
	return nullptr;
}

void CUnit::UpdateButtonIcons()
{
	this->ChooseButtonIcon(ButtonCmd::Attack);
	this->ChooseButtonIcon(ButtonCmd::Stop);
	this->ChooseButtonIcon(ButtonCmd::Move);
	this->ChooseButtonIcon(ButtonCmd::StandGround);
	this->ChooseButtonIcon(ButtonCmd::Patrol);
	if (this->Type->BoolFlag[HARVESTER_INDEX].value) {
		this->ChooseButtonIcon(ButtonCmd::Return);
	}
}

void CUnit::ChooseButtonIcon(const ButtonCmd button_action)
{
	if (button_action == ButtonCmd::Attack) {
		if (this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)].size() > 0 && this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)][0]->GetIcon().Icon != nullptr) {
			this->ButtonIcons[button_action] = this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)][0]->GetIcon().Icon;
			return;
		}
		
		if (this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)].size() > 0 && this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->Type->get_item_class() != stratagus::item_class::bow && this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->GetIcon().Icon != nullptr) {
			this->ButtonIcons[button_action] = this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->GetIcon().Icon;
			return;
		}
	} else if (button_action == ButtonCmd::Stop) {
		if (this->EquippedItems[static_cast<int>(stratagus::item_slot::shield)].size() > 0 && this->EquippedItems[static_cast<int>(stratagus::item_slot::shield)][0]->Type->get_item_class() == stratagus::item_class::shield && this->EquippedItems[static_cast<int>(stratagus::item_slot::shield)][0]->GetIcon().Icon != nullptr) {
			this->ButtonIcons[button_action] = this->EquippedItems[static_cast<int>(stratagus::item_slot::shield)][0]->GetIcon().Icon;
			return;
		}
	} else if (button_action == ButtonCmd::Move) {
		if (this->EquippedItems[static_cast<int>(stratagus::item_slot::boots)].size() > 0 && this->EquippedItems[static_cast<int>(stratagus::item_slot::boots)][0]->GetIcon().Icon != nullptr) {
			this->ButtonIcons[button_action] = this->EquippedItems[static_cast<int>(stratagus::item_slot::boots)][0]->GetIcon().Icon;
			return;
		}
	} else if (button_action == ButtonCmd::StandGround) {
		if (this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)].size() > 0 && this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)][0]->Type->ButtonIcons.find(button_action) != this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)][0]->Type->ButtonIcons.end()) {
			this->ButtonIcons[button_action] = this->EquippedItems[static_cast<int>(stratagus::item_slot::arrows)][0]->Type->ButtonIcons.find(button_action)->second.Icon;
			return;
		}

		if (this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)].size() > 0 && this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->Type->ButtonIcons.find(button_action) != this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->Type->ButtonIcons.end()) {
			this->ButtonIcons[button_action] = this->EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->Type->ButtonIcons.find(button_action)->second.Icon;
			return;
		}
	}
	
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && variation->ButtonIcons.find(button_action) != variation->ButtonIcons.end()) {
		this->ButtonIcons[button_action] = variation->ButtonIcons.find(button_action)->second.Icon;
		return;
	}
	for (int i = 0; i < MaxImageLayers; ++i) {
		const stratagus::unit_type_variation *layer_variation = this->GetLayerVariation(i);
		if (layer_variation && layer_variation->ButtonIcons.find(button_action) != layer_variation->ButtonIcons.end()) {
			this->ButtonIcons[button_action] = layer_variation->ButtonIcons.find(button_action)->second.Icon;
			return;
		}
	}

	for (int i = (stratagus::upgrade_modifier::UpgradeModifiers.size() - 1); i >= 0; --i) {
		const stratagus::upgrade_modifier *modifier = stratagus::upgrade_modifier::UpgradeModifiers[i];
		const CUpgrade *upgrade = CUpgrade::get_all()[modifier->UpgradeId];
		if (this->Player->Allow.Upgrades[upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
			if (
				(
					(button_action == ButtonCmd::Attack && ((upgrade->is_weapon() && upgrade->Item->get_item_class() != stratagus::item_class::bow) || upgrade->is_arrows()))
					|| (button_action == ButtonCmd::Stop && upgrade->is_shield())
					|| (button_action == ButtonCmd::Move && upgrade->is_boots())
				)
				&& upgrade->Item->Icon.Icon != nullptr
			) {
				this->ButtonIcons[button_action] = upgrade->Item->Icon.Icon;
				return;
			} else if (button_action == ButtonCmd::StandGround && (upgrade->is_weapon() || upgrade->is_arrows()) && upgrade->Item->ButtonIcons.find(button_action) != upgrade->Item->ButtonIcons.end()) {
				this->ButtonIcons[button_action] = upgrade->Item->ButtonIcons.find(button_action)->second.Icon;
				return;
			}
		}
	}
	
	if (button_action == ButtonCmd::Attack) {
		if (this->Type->DefaultEquipment.find(stratagus::item_slot::arrows) != this->Type->DefaultEquipment.end() && this->Type->DefaultEquipment.find(stratagus::item_slot::arrows)->second->Icon.Icon != nullptr) {
			this->ButtonIcons[button_action] = this->Type->DefaultEquipment.find(stratagus::item_slot::arrows)->second->Icon.Icon;
			return;
		}
		
		if (this->Type->DefaultEquipment.find(stratagus::item_slot::weapon) != this->Type->DefaultEquipment.end() && this->Type->DefaultEquipment.find(stratagus::item_slot::weapon)->second->Icon.Icon != nullptr) {
			this->ButtonIcons[button_action] = this->Type->DefaultEquipment.find(stratagus::item_slot::weapon)->second->Icon.Icon;
			return;
		}
	} else if (button_action == ButtonCmd::Stop) {
		if (this->Type->DefaultEquipment.find(stratagus::item_slot::shield) != this->Type->DefaultEquipment.end() && this->Type->DefaultEquipment.find(stratagus::item_slot::shield)->second->get_item_class() == stratagus::item_class::shield && this->Type->DefaultEquipment.find(stratagus::item_slot::shield)->second->Icon.Icon != nullptr) {
			this->ButtonIcons[button_action] = this->Type->DefaultEquipment.find(stratagus::item_slot::shield)->second->Icon.Icon;
			return;
		}
	} else if (button_action == ButtonCmd::Move) {
		if (this->Type->DefaultEquipment.find(stratagus::item_slot::boots) != this->Type->DefaultEquipment.end() && this->Type->DefaultEquipment.find(stratagus::item_slot::boots)->second->Icon.Icon != nullptr) {
			this->ButtonIcons[button_action] = this->Type->DefaultEquipment.find(stratagus::item_slot::boots)->second->Icon.Icon;
			return;
		}
	} else if (button_action == ButtonCmd::StandGround) {
		if (this->Type->DefaultEquipment.find(stratagus::item_slot::arrows) != this->Type->DefaultEquipment.end() && this->Type->DefaultEquipment.find(stratagus::item_slot::arrows)->second->ButtonIcons.find(button_action) != this->Type->DefaultEquipment.find(stratagus::item_slot::arrows)->second->ButtonIcons.end()) {
			this->ButtonIcons[button_action] = this->Type->DefaultEquipment.find(stratagus::item_slot::arrows)->second->ButtonIcons.find(button_action)->second.Icon;
			return;
		}
		
		if (this->Type->DefaultEquipment.find(stratagus::item_slot::weapon) != this->Type->DefaultEquipment.end() && this->Type->DefaultEquipment.find(stratagus::item_slot::weapon)->second->ButtonIcons.find(button_action) != this->Type->DefaultEquipment.find(stratagus::item_slot::weapon)->second->ButtonIcons.end()) {
			this->ButtonIcons[button_action] = this->Type->DefaultEquipment.find(stratagus::item_slot::weapon)->second->ButtonIcons.find(button_action)->second.Icon;
			return;
		}
	}
	
	if (this->Type->ButtonIcons.find(button_action) != this->Type->ButtonIcons.end()) {
		this->ButtonIcons[button_action] = this->Type->ButtonIcons.find(button_action)->second.Icon;
		return;
	}
	
	if (this->Type->get_civilization() != nullptr) {
		const stratagus::civilization *civilization = this->Type->get_civilization();
		int faction = this->Type->Faction;
		
		if (faction == -1 && this->Player->Race == civilization->ID) {
			faction = this->Player->Faction;
		}
		
		if (faction != -1 && stratagus::faction::get_all()[faction]->ButtonIcons.find(button_action) != stratagus::faction::get_all()[faction]->ButtonIcons.end()) {
			this->ButtonIcons[button_action] = stratagus::faction::get_all()[faction]->ButtonIcons[button_action].Icon;
			return;
		} else if (PlayerRaces.ButtonIcons[civilization->ID].find(button_action) != PlayerRaces.ButtonIcons[civilization->ID].end()) {
			this->ButtonIcons[button_action] = PlayerRaces.ButtonIcons[civilization->ID][button_action].Icon;
			return;
		}
	}
	
	if (this->ButtonIcons.find(button_action) != this->ButtonIcons.end()) { //if no proper button icon found, make sure any old button icon set for this button action isn't used either
		this->ButtonIcons.erase(button_action);
	}
}

void CUnit::EquipItem(CUnit &item, bool affect_character)
{
	const stratagus::item_class item_class = item.Type->get_item_class();
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item_class);
	
	if (item_slot == stratagus::item_slot::none) {
		fprintf(stderr, "Trying to equip item of type \"%s\", which has no item slot.\n", item.GetTypeName().c_str());
		return;
	}
	
	if (this->get_item_slot_quantity(item_slot) > 0 && EquippedItems[static_cast<int>(item_slot)].size() == this->get_item_slot_quantity(item_slot)) {
		DeequipItem(*EquippedItems[static_cast<int>(item_slot)][EquippedItems[static_cast<int>(item_slot)].size() - 1]);
	}
	
	if (item_slot == stratagus::item_slot::weapon && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// remove the upgrade modifiers from weapon technologies or from abilities which require the base weapon class but aren't compatible with this weapon's class; and apply upgrade modifiers from abilities which require this weapon's class
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (
				(modifier_upgrade->is_weapon() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type))
				|| (modifier_upgrade->is_ability() && this->GetIndividualUpgrade(modifier_upgrade) && modifier_upgrade->WeaponClasses.size() > 0 && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), this->Type->WeaponClasses[0]) != modifier_upgrade->WeaponClasses.end() && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), item_class) == modifier_upgrade->WeaponClasses.end())
			) {
				if (this->GetIndividualUpgrade(modifier_upgrade)) {
					for (int i = 0; i < this->GetIndividualUpgrade(modifier_upgrade); ++i) {
						RemoveIndividualUpgradeModifier(*this, modifier);
					}
				} else {
					RemoveIndividualUpgradeModifier(*this, modifier);
				}
			} else if (
				modifier_upgrade->is_ability() && this->GetIndividualUpgrade(modifier_upgrade) && modifier_upgrade->WeaponClasses.size() > 0 && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), this->Type->WeaponClasses[0]) == modifier_upgrade->WeaponClasses.end() && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), item_class) != modifier_upgrade->WeaponClasses.end()
			) {
				if (this->GetIndividualUpgrade(modifier_upgrade)) {
					for (int i = 0; i < this->GetIndividualUpgrade(modifier_upgrade); ++i) {
						ApplyIndividualUpgradeModifier(*this, modifier);
					}
				} else {
					ApplyIndividualUpgradeModifier(*this, modifier);
				}
			}
		}
	} else if (item_slot == stratagus::item_slot::shield && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// remove the upgrade modifiers from shield technologies
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (modifier_upgrade->is_shield() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
				RemoveIndividualUpgradeModifier(*this, modifier);
			}
		}
	} else if (item_slot == stratagus::item_slot::boots && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// remove the upgrade modifiers from boots technologies
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (modifier_upgrade->is_boots() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
				RemoveIndividualUpgradeModifier(*this, modifier);
			}
		}
	} else if (item_slot == stratagus::item_slot::arrows && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// remove the upgrade modifiers from arrows technologies
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (modifier_upgrade->is_arrows() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
				RemoveIndividualUpgradeModifier(*this, modifier);
			}
		}
	}
	
	if (item.Unique && item.Unique->Set && this->EquippingItemCompletesSet(&item)) {
		for (const auto &modifier : item.Unique->Set->get_modifiers()) {
			ApplyIndividualUpgradeModifier(*this, modifier.get());
		}
	}

	if (!IsNetworkGame() && Character && this->Player == CPlayer::GetThisPlayer() && affect_character) {
		if (Character->GetItem(item) != nullptr) {
			if (!Character->IsItemEquipped(Character->GetItem(item))) {
				Character->EquippedItems[static_cast<int>(item_slot)].push_back(Character->GetItem(item));
				SaveHero(Character);
			} else {
				fprintf(stderr, "Item is not equipped by character \"%s\"'s unit, but is equipped by the character itself.\n", Character->Ident.c_str());
			}
		} else {
			fprintf(stderr, "Item is present in the inventory of the character \"%s\"'s unit, but not in the character's inventory itself.\n", Character->Ident.c_str());
		}
	}
	EquippedItems[static_cast<int>(item_slot)].push_back(&item);
	
	//change variation, if the current one has become forbidden
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (
		variation
		&& (
			variation->item_classes_not_equipped.contains(item.Type->get_item_class())
			|| std::find(variation->ItemsNotEquipped.begin(), variation->ItemsNotEquipped.end(), item.Type) != variation->ItemsNotEquipped.end()
		)
	) {
		ChooseVariation(); //choose a new variation now
	}
	for (int i = 0; i < MaxImageLayers; ++i) {
		const stratagus::unit_type_variation *layer_variation = this->GetLayerVariation(i);
		if (
			layer_variation
			&& (
				layer_variation->item_classes_not_equipped.contains(item.Type->get_item_class())
				|| std::find(layer_variation->ItemsNotEquipped.begin(), layer_variation->ItemsNotEquipped.end(), item.Type) != layer_variation->ItemsNotEquipped.end()
			)
		) {
			ChooseVariation(nullptr, false, i);
		}
	}
	
	if (item_slot == stratagus::item_slot::weapon || item_slot == stratagus::item_slot::arrows) {
		this->ChooseButtonIcon(ButtonCmd::Attack);
		this->ChooseButtonIcon(ButtonCmd::StandGround);
	} else if (item_slot == stratagus::item_slot::shield) {
		this->ChooseButtonIcon(ButtonCmd::Stop);
	} else if (item_slot == stratagus::item_slot::boots) {
		this->ChooseButtonIcon(ButtonCmd::Move);
	}
	this->ChooseButtonIcon(ButtonCmd::Patrol);
	
	//add item bonuses
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) {
		if (
			i == BASICDAMAGE_INDEX || i == PIERCINGDAMAGE_INDEX || i == THORNSDAMAGE_INDEX
			|| i == FIREDAMAGE_INDEX || i == COLDDAMAGE_INDEX || i == ARCANEDAMAGE_INDEX || i == LIGHTNINGDAMAGE_INDEX
			|| i == AIRDAMAGE_INDEX || i == EARTHDAMAGE_INDEX || i == WATERDAMAGE_INDEX || i == ACIDDAMAGE_INDEX
			|| i == ARMOR_INDEX || i == FIRERESISTANCE_INDEX || i == COLDRESISTANCE_INDEX || i == ARCANERESISTANCE_INDEX || i == LIGHTNINGRESISTANCE_INDEX
			|| i == AIRRESISTANCE_INDEX || i == EARTHRESISTANCE_INDEX || i == WATERRESISTANCE_INDEX || i == ACIDRESISTANCE_INDEX
			|| i == HACKRESISTANCE_INDEX || i == PIERCERESISTANCE_INDEX || i == BLUNTRESISTANCE_INDEX
			|| i == ACCURACY_INDEX || i == EVASION_INDEX || i == SPEED_INDEX || i == CHARGEBONUS_INDEX || i == BACKSTAB_INDEX
			|| i == ATTACKRANGE_INDEX
		) {
			Variable[i].Value += item.Variable[i].Value;
			Variable[i].Max += item.Variable[i].Max;
		} else if (i == HITPOINTBONUS_INDEX) {
			Variable[HP_INDEX].Value += item.Variable[i].Value;
			Variable[HP_INDEX].Max += item.Variable[i].Max;
			Variable[HP_INDEX].Increase += item.Variable[i].Increase;
		} else if (i == SIGHTRANGE_INDEX || i == DAYSIGHTRANGEBONUS_INDEX || i == NIGHTSIGHTRANGEBONUS_INDEX) {
			if (!SaveGameLoading) {
				MapUnmarkUnitSight(*this);
			}
			Variable[i].Value += item.Variable[i].Value;
			Variable[i].Max += item.Variable[i].Max;
			if (!SaveGameLoading) {
				if (i == SIGHTRANGE_INDEX) {
					CurrentSightRange = Variable[i].Value;
				}
				UpdateUnitSightRange(*this);
				MapMarkUnitSight(*this);
			}
		}
	}
}

void CUnit::DeequipItem(CUnit &item, bool affect_character)
{
	//remove item bonuses
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) {
		if (
			i == BASICDAMAGE_INDEX || i == PIERCINGDAMAGE_INDEX || i == THORNSDAMAGE_INDEX
			|| i == FIREDAMAGE_INDEX || i == COLDDAMAGE_INDEX || i == ARCANEDAMAGE_INDEX || i == LIGHTNINGDAMAGE_INDEX
			|| i == AIRDAMAGE_INDEX || i == EARTHDAMAGE_INDEX || i == WATERDAMAGE_INDEX || i == ACIDDAMAGE_INDEX
			|| i == ARMOR_INDEX || i == FIRERESISTANCE_INDEX || i == COLDRESISTANCE_INDEX || i == ARCANERESISTANCE_INDEX || i == LIGHTNINGRESISTANCE_INDEX
			|| i == AIRRESISTANCE_INDEX || i == EARTHRESISTANCE_INDEX || i == WATERRESISTANCE_INDEX || i == ACIDRESISTANCE_INDEX
			|| i == HACKRESISTANCE_INDEX || i == PIERCERESISTANCE_INDEX || i == BLUNTRESISTANCE_INDEX
			|| i == ACCURACY_INDEX || i == EVASION_INDEX || i == SPEED_INDEX || i == CHARGEBONUS_INDEX || i == BACKSTAB_INDEX
			|| i == ATTACKRANGE_INDEX
		) {
			Variable[i].Value -= item.Variable[i].Value;
			Variable[i].Max -= item.Variable[i].Max;
		} else if (i == HITPOINTBONUS_INDEX) {
			Variable[HP_INDEX].Value -= item.Variable[i].Value;
			Variable[HP_INDEX].Max -= item.Variable[i].Max;
			Variable[HP_INDEX].Increase -= item.Variable[i].Increase;
		} else if (i == SIGHTRANGE_INDEX || i == DAYSIGHTRANGEBONUS_INDEX || i == NIGHTSIGHTRANGEBONUS_INDEX) {
			MapUnmarkUnitSight(*this);
			Variable[i].Value -= item.Variable[i].Value;
			Variable[i].Max -= item.Variable[i].Max;
			if (i == SIGHTRANGE_INDEX) {
				CurrentSightRange = Variable[i].Value;
			}
			UpdateUnitSightRange(*this);
			MapMarkUnitSight(*this);
		}
	}
	
	if (item.Unique && item.Unique->Set && this->DeequippingItemBreaksSet(&item)) {
		for (const auto &modifier : item.Unique->Set->get_modifiers()) {
			RemoveIndividualUpgradeModifier(*this, modifier.get());
		}
	}

	const stratagus::item_class item_class = item.Type->get_item_class();
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item_class);
	
	if (item_slot == stratagus::item_slot::none) {
		fprintf(stderr, "Trying to de-equip item of type \"%s\", which has no item slot.\n", item.GetTypeName().c_str());
		return;
	}
	
	if (!IsNetworkGame() && Character && this->Player == CPlayer::GetThisPlayer() && affect_character) {
		if (Character->GetItem(item) != nullptr) {
			if (Character->IsItemEquipped(Character->GetItem(item))) {
				stratagus::vector::remove(this->Character->EquippedItems[static_cast<int>(item_slot)], this->Character->GetItem(item));
				SaveHero(Character);
			} else {
				fprintf(stderr, "Item is equipped by character \"%s\"'s unit, but not by the character itself.\n", Character->Ident.c_str());
			}
		} else {
			fprintf(stderr, "Item is present in the inventory of the character \"%s\"'s unit, but not in the character's inventory itself.\n", Character->Ident.c_str());
		}
	}
	stratagus::vector::remove(this->EquippedItems[static_cast<int>(item_slot)], &item);
	
	if (item_slot == stratagus::item_slot::weapon && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// restore the upgrade modifiers from weapon technologies, and apply ability effects that are weapon class-specific accordingly
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (
				(modifier_upgrade->is_weapon() && Player->Allow.Upgrades[modifier->UpgradeId] == 'R' && modifier->applies_to(this->Type))
				|| (modifier_upgrade->is_ability() && this->GetIndividualUpgrade(modifier_upgrade) && modifier_upgrade->WeaponClasses.size() > 0 && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), this->Type->WeaponClasses[0]) != modifier_upgrade->WeaponClasses.end() && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), item_class) == modifier_upgrade->WeaponClasses.end())
			) {
				if (this->GetIndividualUpgrade(modifier_upgrade)) {
					for (int i = 0; i < this->GetIndividualUpgrade(modifier_upgrade); ++i) {
						ApplyIndividualUpgradeModifier(*this, modifier);
					}
				} else {
					ApplyIndividualUpgradeModifier(*this, modifier);
				}
			} else if (
				modifier_upgrade->is_ability() && this->GetIndividualUpgrade(modifier_upgrade) && modifier_upgrade->WeaponClasses.size() > 0 && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), this->Type->WeaponClasses[0]) == modifier_upgrade->WeaponClasses.end() && std::find(modifier_upgrade->WeaponClasses.begin(), modifier_upgrade->WeaponClasses.end(), item_class) != modifier_upgrade->WeaponClasses.end()
			) {
				if (this->GetIndividualUpgrade(modifier_upgrade)) {
					for (int i = 0; i < this->GetIndividualUpgrade(modifier_upgrade); ++i) {
						RemoveIndividualUpgradeModifier(*this, modifier);
					}
				} else {
					RemoveIndividualUpgradeModifier(*this, modifier);
				}
			}
		}
	} else if (item_slot == stratagus::item_slot::shield && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// restore the upgrade modifiers from shield technologies
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (modifier_upgrade->is_shield() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
				ApplyIndividualUpgradeModifier(*this, modifier);
			}
		}
	} else if (item_slot == stratagus::item_slot::boots && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// restore the upgrade modifiers from boots technologies
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (modifier_upgrade->is_boots() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
				ApplyIndividualUpgradeModifier(*this, modifier);
			}
		}
	} else if (item_slot == stratagus::item_slot::arrows && EquippedItems[static_cast<int>(item_slot)].size() == 0) {
		// restore the upgrade modifiers from arrows technologies
		for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
			const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
			if (modifier_upgrade->is_arrows() && Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) {
				ApplyIndividualUpgradeModifier(*this, modifier);
			}
		}
	}
	
	//change variation, if the current one has become forbidden
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (
		variation
		&& (
			variation->item_classes_equipped.contains(item.Type->get_item_class())
			|| std::find(variation->ItemsEquipped.begin(), variation->ItemsEquipped.end(), item.Type) != variation->ItemsEquipped.end()
		)
	) {
		ChooseVariation(); //choose a new variation now
	}
	for (int i = 0; i < MaxImageLayers; ++i) {
		const stratagus::unit_type_variation *layer_variation = this->GetLayerVariation(i);

		if (
			layer_variation
			&& (
				layer_variation->item_classes_equipped.contains(item.Type->get_item_class())
				|| std::find(layer_variation->ItemsEquipped.begin(), layer_variation->ItemsEquipped.end(), item.Type) != layer_variation->ItemsEquipped.end()
			)
		) {
			ChooseVariation(nullptr, false, i);
		}
	}
	
	if (item_slot == stratagus::item_slot::weapon || item_slot == stratagus::item_slot::arrows) {
		this->ChooseButtonIcon(ButtonCmd::Attack);
		this->ChooseButtonIcon(ButtonCmd::StandGround);
	} else if (item_slot == stratagus::item_slot::shield) {
		this->ChooseButtonIcon(ButtonCmd::Stop);
	} else if (item_slot == stratagus::item_slot::boots) {
		this->ChooseButtonIcon(ButtonCmd::Move);
	}
	this->ChooseButtonIcon(ButtonCmd::Patrol);
}

void CUnit::ReadWork(CUpgrade *work, bool affect_character)
{
	IndividualUpgradeAcquire(*this, work);
	
	if (!IsNetworkGame() && Character && this->Player == CPlayer::GetThisPlayer() && affect_character) {
		if (std::find(Character->ReadWorks.begin(), Character->ReadWorks.end(), work) == Character->ReadWorks.end()) {
			Character->ReadWorks.push_back(work);
			SaveHero(Character);
		}
	}
}

void CUnit::ConsumeElixir(CUpgrade *elixir, bool affect_character)
{
	IndividualUpgradeAcquire(*this, elixir);
	
	if (!IsNetworkGame() && Character && this->Player == CPlayer::GetThisPlayer() && affect_character) {
		if (std::find(Character->ConsumedElixirs.begin(), Character->ConsumedElixirs.end(), elixir) == Character->ConsumedElixirs.end()) {
			Character->ConsumedElixirs.push_back(elixir);
			SaveHero(Character);
		}
	}
}

void CUnit::ApplyAura(int aura_index)
{
	if (aura_index == LEADERSHIPAURA_INDEX) {
		if (!this->IsInCombat()) {
			return;
		}
	}
	
	this->ApplyAuraEffect(aura_index);
			
	//apply aura to all appropriate nearby units
	int aura_range = AuraRange - (this->Type->get_tile_width() - 1);
	std::vector<CUnit *> table;
	SelectAroundUnit(*this, aura_range, table, MakeOrPredicate(HasSamePlayerAs(*this->Player), IsAlliedWith(*this->Player)), true);
	for (size_t i = 0; i != table.size(); ++i) {
		table[i]->ApplyAuraEffect(aura_index);
	}
	
	table.clear();
	SelectAroundUnit(*this, aura_range, table, MakeOrPredicate(MakeOrPredicate(HasSamePlayerAs(*this->Player), IsAlliedWith(*this->Player)), HasSamePlayerAs(*CPlayer::Players[PlayerNumNeutral])), true);
	for (size_t i = 0; i != table.size(); ++i) {
		if (table[i]->UnitInside) {
			CUnit *uins = table[i]->UnitInside;
			for (int j = 0; j < table[i]->InsideCount; ++j, uins = uins->NextContained) {
				if (uins->Player == this->Player || uins->IsAllied(*this->Player)) {
					uins->ApplyAuraEffect(aura_index);
				}
			}
		}
	}
}

void CUnit::ApplyAuraEffect(int aura_index)
{
	int effect_index = -1;
	if (aura_index == LEADERSHIPAURA_INDEX) {
		if (this->Type->BoolFlag[BUILDING_INDEX].value) {
			return;
		}
		effect_index = LEADERSHIP_INDEX;
	} else if (aura_index == REGENERATIONAURA_INDEX) {
		if (!this->Type->BoolFlag[ORGANIC_INDEX].value || this->Variable[HP_INDEX].Value >= this->GetModifiedVariable(HP_INDEX, VariableMax)) {
			return;
		}
		effect_index = REGENERATION_INDEX;
	} else if (aura_index == HYDRATINGAURA_INDEX) {
		if (!this->Type->BoolFlag[ORGANIC_INDEX].value) {
			return;
		}
		effect_index = HYDRATING_INDEX;
		this->Variable[DEHYDRATION_INDEX].Max = 0;
		this->Variable[DEHYDRATION_INDEX].Value = 0;
	}
	
	if (effect_index == -1) {
		return;
	}
	
	this->Variable[effect_index].Enable = 1;
	this->Variable[effect_index].Max = std::max(CYCLES_PER_SECOND + 1, this->Variable[effect_index].Max);
	this->Variable[effect_index].Value = std::max(CYCLES_PER_SECOND + 1, this->Variable[effect_index].Value);
}

void CUnit::SetPrefix(CUpgrade *prefix)
{
	if (Prefix != nullptr) {
		for (const auto &modifier : Prefix->get_modifiers()) {
			RemoveIndividualUpgradeModifier(*this, modifier.get());
		}
		this->Variable[MAGICLEVEL_INDEX].Value -= Prefix->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max -= Prefix->MagicLevel;
	}
	if (!IsNetworkGame() && Container && Container->Character && Container->Player == CPlayer::GetThisPlayer() && Container->Character->GetItem(*this) != nullptr && Container->Character->GetItem(*this)->Prefix != prefix) { //update the persistent item, if applicable and if it hasn't been updated yet
		Container->Character->GetItem(*this)->Prefix = prefix;
		SaveHero(Container->Character);
	}
	Prefix = prefix;
	if (Prefix != nullptr) {
		for (const auto &modifier : Prefix->get_modifiers()) {
			ApplyIndividualUpgradeModifier(*this, modifier.get());
		}
		this->Variable[MAGICLEVEL_INDEX].Value += Prefix->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max += Prefix->MagicLevel;
	}
	
	this->UpdateItemName();
}

void CUnit::SetSuffix(CUpgrade *suffix)
{
	if (Suffix != nullptr) {
		for (const auto &modifier : Suffix->get_modifiers()) {
			RemoveIndividualUpgradeModifier(*this, modifier.get());
		}
		this->Variable[MAGICLEVEL_INDEX].Value -= Suffix->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max -= Suffix->MagicLevel;
	}
	if (!IsNetworkGame() && Container && Container->Character && Container->Player == CPlayer::GetThisPlayer() && Container->Character->GetItem(*this) != nullptr && Container->Character->GetItem(*this)->Suffix != suffix) { //update the persistent item, if applicable and if it hasn't been updated yet
		Container->Character->GetItem(*this)->Suffix = suffix;
		SaveHero(Container->Character);
	}
	Suffix = suffix;
	if (Suffix != nullptr) {
		for (const auto &modifier : Suffix->get_modifiers()) {
			ApplyIndividualUpgradeModifier(*this, modifier.get());
		}
		this->Variable[MAGICLEVEL_INDEX].Value += Suffix->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max += Suffix->MagicLevel;
	}
	
	this->UpdateItemName();
}

void CUnit::SetSpell(CSpell *spell)
{
	if (!IsNetworkGame() && Container && Container->Character && Container->Player == CPlayer::GetThisPlayer() && Container->Character->GetItem(*this) != nullptr && Container->Character->GetItem(*this)->Spell != spell) { //update the persistent item, if applicable and if it hasn't been updated yet
		Container->Character->GetItem(*this)->Spell = spell;
		SaveHero(Container->Character);
	}
	Spell = spell;
	
	this->UpdateItemName();
}

void CUnit::SetWork(CUpgrade *work)
{
	if (this->Work != nullptr) {
		this->Variable[MAGICLEVEL_INDEX].Value -= this->Work->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max -= this->Work->MagicLevel;
	}
	
	if (!IsNetworkGame() && Container && Container->Character && Container->Player == CPlayer::GetThisPlayer() && Container->Character->GetItem(*this) != nullptr && Container->Character->GetItem(*this)->Work != work) { //update the persistent item, if applicable and if it hasn't been updated yet
		Container->Character->GetItem(*this)->Work = work;
		SaveHero(Container->Character);
	}
	
	Work = work;
	
	if (this->Work != nullptr) {
		this->Variable[MAGICLEVEL_INDEX].Value += this->Work->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max += this->Work->MagicLevel;
	}
	
	this->UpdateItemName();
}

void CUnit::SetElixir(CUpgrade *elixir)
{
	if (this->Elixir != nullptr) {
		this->Variable[MAGICLEVEL_INDEX].Value -= this->Elixir->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max -= this->Elixir->MagicLevel;
	}
	
	if (!IsNetworkGame() && Container && Container->Character && Container->Player == CPlayer::GetThisPlayer() && Container->Character->GetItem(*this) != nullptr && Container->Character->GetItem(*this)->Elixir != elixir) { //update the persistent item, if applicable and if it hasn't been updated yet
		Container->Character->GetItem(*this)->Elixir = elixir;
		SaveHero(Container->Character);
	}
	
	Elixir = elixir;
	
	if (this->Elixir != nullptr) {
		this->Variable[MAGICLEVEL_INDEX].Value += this->Elixir->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max += this->Elixir->MagicLevel;
	}
	
	this->UpdateItemName();
}

void CUnit::SetUnique(CUniqueItem *unique)
{
	if (this->Unique && this->Unique->Set) {
		this->Variable[MAGICLEVEL_INDEX].Value -= this->Unique->Set->MagicLevel;
		this->Variable[MAGICLEVEL_INDEX].Max -= this->Unique->Set->MagicLevel;
	}
		
	if (unique != nullptr) {
		SetPrefix(unique->Prefix);
		SetSuffix(unique->Suffix);
		SetSpell(unique->Spell);
		SetWork(unique->Work);
		SetElixir(unique->Elixir);
		if (unique->ResourcesHeld != 0) {
			this->SetResourcesHeld(unique->ResourcesHeld);
			this->Variable[GIVERESOURCE_INDEX].Value = unique->ResourcesHeld;
			this->Variable[GIVERESOURCE_INDEX].Max = unique->ResourcesHeld;
			this->Variable[GIVERESOURCE_INDEX].Enable = 1;
		}
		if (unique->Set) {
			this->Variable[MAGICLEVEL_INDEX].Value += unique->Set->MagicLevel;
			this->Variable[MAGICLEVEL_INDEX].Max += unique->Set->MagicLevel;
		}
		Name = unique->Name;
		Unique = unique;
	} else {
		Name.clear();
		Unique = nullptr;
		SetPrefix(nullptr);
		SetSuffix(nullptr);
		SetSpell(nullptr);
		SetWork(nullptr);
		SetElixir(nullptr);
	}
}

void CUnit::Identify()
{
	if (!IsNetworkGame() && Container && Container->Character && Container->Player == CPlayer::GetThisPlayer() && Container->Character->GetItem(*this) != nullptr && Container->Character->GetItem(*this)->Identified != true) { //update the persistent item, if applicable and if it hasn't been updated yet
		Container->Character->GetItem(*this)->Identified = true;
		SaveHero(Container->Character);
	}
	
	this->Identified = true;
	
	if (this->Container != nullptr && this->Container->Player == CPlayer::GetThisPlayer()) {
		this->Container->Player->Notify(NotifyGreen, this->Container->tilePos, this->Container->MapLayer->ID, _("%s has identified the %s!"), this->Container->GetMessageName().c_str(), this->GetMessageName().c_str());
	}
}

void CUnit::CheckIdentification()
{
	if (!HasInventory()) {
		return;
	}
	
	CUnit *uins = this->UnitInside;
	
	for (int i = 0; i < this->InsideCount; ++i, uins = uins->NextContained) {
		if (!uins->Type->BoolFlag[ITEM_INDEX].value) {
			continue;
		}
		
		if (!uins->Identified && this->Variable[KNOWLEDGEMAGIC_INDEX].Value >= uins->Variable[MAGICLEVEL_INDEX].Value) {
			uins->Identify();
		}
	}
}

void CUnit::CheckKnowledgeChange(int variable, int change) // this happens after the variable has already been changed
{
	if (!change) {
		return;
	}
	
	if (variable == KNOWLEDGEMAGIC_INDEX) {
		int mana_change = (this->Variable[variable].Value / 5) - ((this->Variable[variable].Value - change) / 5); // +1 max mana for every 5 levels in Knowledge (Magic)
		this->Variable[MANA_INDEX].Max += mana_change;
		if (mana_change < 0) {
			this->Variable[MANA_INDEX].Value += mana_change;
		}
		
		this->CheckIdentification();
	} else if (variable == KNOWLEDGEWARFARE_INDEX) {
		int hp_change = (this->Variable[variable].Value / 5) - ((this->Variable[variable].Value - change) / 5); // +1 max HP for every 5 levels in Knowledge (Warfare)
		this->Variable[HP_INDEX].Max += hp_change;
		this->Variable[HP_INDEX].Value += hp_change;
	} else if (variable == KNOWLEDGEMINING_INDEX) {
		int stat_change = (this->Variable[variable].Value / 25) - ((this->Variable[variable].Value - change) / 25); // +1 mining gathering bonus for every 25 levels in Knowledge (Mining)
		this->Variable[COPPERGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[COPPERGATHERINGBONUS_INDEX].Value += stat_change;
		this->Variable[SILVERGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[SILVERGATHERINGBONUS_INDEX].Value += stat_change;
		this->Variable[GOLDGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[GOLDGATHERINGBONUS_INDEX].Value += stat_change;
		this->Variable[IRONGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[IRONGATHERINGBONUS_INDEX].Value += stat_change;
		this->Variable[MITHRILGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[MITHRILGATHERINGBONUS_INDEX].Value += stat_change;
		this->Variable[COALGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[COALGATHERINGBONUS_INDEX].Value += stat_change;
		this->Variable[GEMSGATHERINGBONUS_INDEX].Max += stat_change;
		this->Variable[GEMSGATHERINGBONUS_INDEX].Value += stat_change;
	}
}

void CUnit::UpdateItemName()
{
	if (this->Unique) {
		Name = _(this->Unique->Name.c_str());
		return;
	}
	
	Name.clear();
	if (Prefix == nullptr && Spell == nullptr && Work == nullptr && Suffix == nullptr) { //elixirs use the name of their unit type
		return;
	}
	
	if (Prefix != nullptr) {
		Name += _(Prefix->get_name().c_str());
		Name += " ";
	}
	if (Work != nullptr) {
		Name += _(Work->get_name().c_str());
	} else {
		Name += GetTypeName();
	}
	if (Suffix != nullptr) {
		Name += " ";
		Name += _(Suffix->get_name().c_str());
	} else if (Spell != nullptr) {
		Name += " ";
		Name += _("of");
		Name += " ";
		Name += _(Spell->Name.c_str());
	}
}

void CUnit::GenerateDrop()
{
	bool base_based_mission = false;
	for (int p = 0; p < PlayerMax; ++p) {
		if (CPlayer::Players[p]->NumTownHalls > 0 || CPlayer::Players[p]->LostTownHallTimer) {
			base_based_mission = true;
		}
	}
	
	if (this->Type->BoolFlag[ORGANIC_INDEX].value && !this->Character && !this->Type->BoolFlag[FAUNA_INDEX].value && base_based_mission) { //if the unit is organic and isn't a character (and isn't fauna) and this is a base-based mission, don't generate a drop
		return;
	}
	
	Vec2i drop_pos = this->tilePos;
	drop_pos.x += SyncRand(this->Type->get_tile_width());
	drop_pos.y += SyncRand(this->Type->get_tile_height());
	CUnit *droppedUnit = nullptr;
	stratagus::unit_type *chosen_drop = nullptr;
	std::vector<stratagus::unit_type *> potential_drops;
	for (size_t i = 0; i < this->Type->Drops.size(); ++i) {
		if (CheckDependencies(this->Type->Drops[i], this)) {
			potential_drops.push_back(this->Type->Drops[i]);
		}
	}
	if (this->Player->AiEnabled) {
		for (size_t i = 0; i < this->Type->AiDrops.size(); ++i) {
			if (CheckDependencies(this->Type->AiDrops[i], this)) {
				potential_drops.push_back(this->Type->AiDrops[i]);
			}
		}
		for (std::map<std::string, std::vector<stratagus::unit_type *>>::const_iterator iterator = this->Type->ModAiDrops.begin(); iterator != this->Type->ModAiDrops.end(); ++iterator) {
			for (size_t i = 0; i < iterator->second.size(); ++i) {
				if (CheckDependencies(iterator->second[i], this)) {
					potential_drops.push_back(iterator->second[i]);
				}
			}
		}
	}
	if (potential_drops.size() > 0) {
		chosen_drop = potential_drops[SyncRand(potential_drops.size())];
	}
		
	if (chosen_drop != nullptr) {
		CBuildRestrictionOnTop *ontop_b = OnTopDetails(*this->Type, nullptr);
		if (((chosen_drop->BoolFlag[ITEM_INDEX].value || chosen_drop->BoolFlag[POWERUP_INDEX].value) && (this->MapLayer->Field(drop_pos)->Flags & MapFieldItem)) || (ontop_b && ontop_b->ReplaceOnDie)) { //if the dropped unit is an item (and there's already another item there), or if this building is an ontop one (meaning another will appear under it after it is destroyed), search for another spot
			Vec2i resPos;
			FindNearestDrop(*chosen_drop, drop_pos, resPos, LookingW, this->MapLayer->ID);
			droppedUnit = MakeUnitAndPlace(resPos, *chosen_drop, CPlayer::Players[PlayerNumNeutral], this->MapLayer->ID);
		} else {
			droppedUnit = MakeUnitAndPlace(drop_pos, *chosen_drop, CPlayer::Players[PlayerNumNeutral], this->MapLayer->ID);
		}
			
		if (droppedUnit != nullptr) {
			if (droppedUnit->Type->BoolFlag[FAUNA_INDEX].value) {
				droppedUnit->Name = droppedUnit->Type->GeneratePersonalName(nullptr, static_cast<stratagus::gender>(droppedUnit->Variable[GENDER_INDEX].Value));
			}
			
			droppedUnit->GenerateSpecialProperties(this, this->Player);
			
			if (droppedUnit->Type->BoolFlag[ITEM_INDEX].value && !droppedUnit->Unique) { //save the initial cycle items were placed in the ground to destroy them if they have been there for too long
				int ttl_cycles = (5 * 60 * CYCLES_PER_SECOND);
				if (droppedUnit->Prefix != nullptr || droppedUnit->Suffix != nullptr || droppedUnit->Spell != nullptr || droppedUnit->Work != nullptr || droppedUnit->Elixir != nullptr) {
					ttl_cycles *= 4;
				}
				droppedUnit->TTL = GameCycle + ttl_cycles;
			}
		}
	}
}

void CUnit::GenerateSpecialProperties(CUnit *dropper, CPlayer *dropper_player, bool allow_unique, bool sold_item, bool always_magic)
{
	int magic_affix_chance = 10; //10% chance of the unit having a magic prefix or suffix
	int unique_chance = 5; //0.5% chance of the unit being unique
	if (dropper != nullptr) {
		if (dropper->Character) { //if the dropper is a character, multiply the chances of the item being magic or unique by the character's level
			magic_affix_chance *= dropper->Character->Level;
			unique_chance *= dropper->Character->Level;
		} else if (dropper->Type->BoolFlag[BUILDING_INDEX].value) { //if the dropper is a building, multiply the chances of the drop being magic or unique by a factor according to whether the building itself is magic/unique
			int chance_multiplier = 2;
			if (dropper->Unique) {
				chance_multiplier += 8;
			} else {
				if (dropper->Prefix != nullptr) {
					chance_multiplier += 1;
				}
				if (dropper->Suffix != nullptr) {
					chance_multiplier += 1;
				}
			}
			magic_affix_chance *= chance_multiplier;
			unique_chance *= chance_multiplier;
		}
	}
	
	if (sold_item) {
		magic_affix_chance /= 4;
		unique_chance /= 4;
	}

	if (allow_unique) {
		const bool is_unique = SyncRand(1000) >= (1000 - unique_chance);
		if (is_unique) {
			this->GenerateUnique(dropper, dropper_player);
		}
	}

	if (this->Unique == nullptr) {
		if (this->Type->get_item_class() == stratagus::item_class::scroll || this->Type->get_item_class() == stratagus::item_class::book || this->Type->get_item_class() == stratagus::item_class::ring || this->Type->get_item_class() == stratagus::item_class::amulet || this->Type->get_item_class() == stratagus::item_class::horn || always_magic) { //scrolls, books, jewelry and horns must always have a property
			magic_affix_chance = 100;
		}

		const bool is_magic = SyncRand(100) >= (100 - magic_affix_chance);
		if (is_magic) {
			std::vector<int> magic_types{0, 1, 2, 3};

			while (!magic_types.empty()) {
				const int magic_type = magic_types[SyncRand(magic_types.size())];
				stratagus::vector::remove(magic_types, magic_type);

				switch (magic_type) {
					case 0:
						this->GenerateWork(dropper, dropper_player);
						break;
					case 1:
						this->GeneratePrefix(dropper, dropper_player);
						break;
					case 2:
						this->GenerateSuffix(dropper, dropper_player);
						break;
					case 3:
						this->GenerateSpell(dropper, dropper_player);
						break;
				}

				if (this->Prefix != nullptr || this->Suffix != nullptr || this->Work != nullptr || this->Elixir != nullptr || this->Spell != nullptr) {
					break;
				}
			}
		}
	}
	
	if (this->Type->BoolFlag[ITEM_INDEX].value && (this->Prefix != nullptr || this->Suffix != nullptr)) {
		this->Identified = false;
	}
}
			
void CUnit::GeneratePrefix(CUnit *dropper, CPlayer *dropper_player)
{
	std::vector<CUpgrade *> potential_prefixes;

	for (CUpgrade *affix : this->Type->Affixes) {
		if ((this->Type->get_item_class() == stratagus::item_class::none && affix->MagicPrefix) || (this->Type->get_item_class() != stratagus::item_class::none && affix->ItemPrefix[static_cast<int>(Type->get_item_class())])) {
			potential_prefixes.push_back(affix);
		}
	}

	if (dropper_player != nullptr) {
		for (CUpgrade *upgrade : CUpgrade::get_all()) {
			if (this->Type->get_item_class() == stratagus::item_class::none || !upgrade->ItemPrefix[static_cast<int>(Type->get_item_class())]) {
				continue;
			}

			if (dropper != nullptr) {
				if (!CheckDependencies(upgrade, dropper)) {
					continue;
				}
			} else {
				if (!CheckDependencies(upgrade, dropper_player)) {
					continue;
				}
			}

			potential_prefixes.push_back(upgrade);
		}
	}
	
	if (potential_prefixes.size() > 0) {
		this->SetPrefix(potential_prefixes[SyncRand(potential_prefixes.size())]);
	}
}

void CUnit::GenerateSuffix(CUnit *dropper, CPlayer *dropper_player)
{
	std::vector<CUpgrade *> potential_suffixes;

	for (CUpgrade *affix : this->Type->Affixes) {
		if ((this->Type->get_item_class() == stratagus::item_class::none && affix->MagicSuffix) || (this->Type->get_item_class() != stratagus::item_class::none && affix->ItemSuffix[static_cast<int>(Type->get_item_class())])) {
			if (Prefix == nullptr || !affix->IncompatibleAffixes[Prefix->ID]) { //don't allow a suffix incompatible with the prefix to appear
				potential_suffixes.push_back(affix);
			}
		}
	}

	if (dropper_player != nullptr) {
		for (CUpgrade *upgrade : CUpgrade::get_all()) {
			if (this->Type->get_item_class() == stratagus::item_class::none || !upgrade->ItemSuffix[static_cast<int>(Type->get_item_class())]) {
				continue;
			}

			if (dropper != nullptr) {
				if (!CheckDependencies(upgrade, dropper)) {
					continue;
				}
			} else {
				if (!CheckDependencies(upgrade, dropper_player)) {
					continue;
				}
			}

			if (this->Prefix != nullptr && upgrade->IncompatibleAffixes[this->Prefix->ID]) { //don't allow a suffix incompatible with the prefix to appear
				continue;
			}

			potential_suffixes.push_back(upgrade);
		}
	}
	
	if (potential_suffixes.size() > 0) {
		this->SetSuffix(potential_suffixes[SyncRand(potential_suffixes.size())]);
	}
}

void CUnit::GenerateSpell(CUnit *dropper, CPlayer *dropper_player)
{
	std::vector<CSpell *> potential_spells;
	if (dropper != nullptr) {
		for (CSpell *spell : dropper->Type->DropSpells) {
			if (this->Type->get_item_class() != stratagus::item_class::none && spell->ItemSpell[static_cast<int>(Type->get_item_class())]) {
				potential_spells.push_back(spell);
			}
		}
	}
	
	if (potential_spells.size() > 0) {
		SetSpell(potential_spells[SyncRand(potential_spells.size())]);
	}
}

void CUnit::GenerateWork(CUnit *dropper, CPlayer *dropper_player)
{
	std::vector<CUpgrade *> potential_works;

	for (CUpgrade *affix : this->Type->Affixes) {
		if (this->Type->get_item_class() != stratagus::item_class::none && affix->Work == this->Type->get_item_class() && !affix->UniqueOnly) {
			potential_works.push_back(affix);
		}
	}

	if (dropper_player != nullptr) {
		for (CUpgrade *upgrade : CUpgrade::get_all()) {
			if (this->Type->get_item_class() == stratagus::item_class::none || upgrade->Work != this->Type->get_item_class() || upgrade->UniqueOnly) {
				continue;
			}

			if (dropper != nullptr) {
				if (!CheckDependencies(upgrade, dropper)) {
					continue;
				}
			} else {
				if (!CheckDependencies(upgrade, dropper_player)) {
					continue;
				}
			}

			potential_works.push_back(upgrade);
		}
	}
	
	if (potential_works.size() > 0) {
		this->SetWork(potential_works[SyncRand(potential_works.size())]);
	}
}

void CUnit::GenerateUnique(CUnit *dropper, CPlayer *dropper_player)
{
	std::vector<CUniqueItem *> potential_uniques;

	for (CUniqueItem *unique : UniqueItems) {
		if (this->Type != unique->Type) {
			continue;
		}

		if (unique->Prefix != nullptr) {
			//the dropper unit must be capable of generating this unique item's prefix to drop the item, or else the unit type must be capable of generating it on its own
			if (std::find(this->Type->Affixes.begin(), this->Type->Affixes.end(), unique->Prefix) == this->Type->Affixes.end()) {
				if (dropper_player == nullptr) {
					continue;
				}

				if (dropper != nullptr) {
					if (!CheckDependencies(unique->Prefix, dropper)) {
						continue;
					}
				} else {
					if (!CheckDependencies(unique->Prefix, dropper_player)) {
						continue;
					}
				}
			}
		}

		if (unique->Suffix != nullptr) {
			//the dropper unit must be capable of generating this unique item's suffix to drop the item, or else the unit type must be capable of generating it on its own
			if (std::find(this->Type->Affixes.begin(), this->Type->Affixes.end(), unique->Suffix) == this->Type->Affixes.end()) {
				if (dropper_player == nullptr) {
					continue;
				}

				if (dropper != nullptr) {
					if (!CheckDependencies(unique->Suffix, dropper)) {
						continue;
					}
				} else {
					if (!CheckDependencies(unique->Suffix, dropper_player)) {
						continue;
					}
				}
			}
		}

		if (unique->Set != nullptr) {
			//the dropper unit must be capable of generating this unique item's set to drop the item
			if (dropper_player == nullptr) {
				continue;
			}

			if (dropper != nullptr) {
				if (!CheckDependencies(unique->Set, dropper)) {
					continue;
				}
			} else {
				if (!CheckDependencies(unique->Set, dropper_player)) {
					continue;
				}
			}
		}

		if (unique->Spell != nullptr) {
			//the dropper unit must be capable of generating this unique item's spell to drop the item
			if (dropper == nullptr) {
				continue;
			}

			if (std::find(dropper->Type->DropSpells.begin(), dropper->Type->DropSpells.end(), unique->Spell) == dropper->Type->DropSpells.end()) {
				continue;
			}
		}

		if (unique->Work != nullptr) {
			//the dropper unit must be capable of generating this unique item's work to drop the item, or else the unit type must be capable of generating it on its own
			if (std::find(this->Type->Affixes.begin(), this->Type->Affixes.end(), unique->Work) == this->Type->Affixes.end()) {
				if (dropper_player == nullptr) {
					continue;
				}

				if (dropper != nullptr) {
					if (!CheckDependencies(unique->Work, dropper)) {
						continue;
					}
				} else {
					if (!CheckDependencies(unique->Work, dropper_player)) {
						continue;
					}
				}
			}
		}

		if (unique->Elixir != nullptr) {
			//the dropper unit must be capable of generating this unique item's elixir to drop the item, or else the unit type must be capable of generating it on its own
			if (std::find(this->Type->Affixes.begin(), this->Type->Affixes.end(), unique->Elixir) == this->Type->Affixes.end()) {
				if (dropper_player == nullptr) {
					continue;
				}

				if (dropper != nullptr) {
					if (!CheckDependencies(unique->Elixir, dropper)) {
						continue;
					}
				} else {
					if (!CheckDependencies(unique->Elixir, dropper_player)) {
						continue;
					}
				}
			}
		}

		if (!unique->CanDrop()) {
			continue;
		}

		potential_uniques.push_back(unique);
	}
	
	if (potential_uniques.size() > 0) {
		CUniqueItem *chosen_unique = potential_uniques[SyncRand(potential_uniques.size())];
		this->SetUnique(chosen_unique);
	}
}

void CUnit::UpdateSoldUnits()
{
	static constexpr int recruitable_hero_max = 4;

	if (!this->Type->BoolFlag[RECRUITHEROES_INDEX].value && this->Type->SoldUnits.empty() && this->SoldUnits.empty()) {
		return;
	}
	
	if (this->UnderConstruction == 1 || !CMap::Map.Info.IsPointOnMap(this->tilePos, this->MapLayer) || Editor.Running != EditorNotRunning) {
		return;
	}
	
	for (size_t i = 0; i < this->SoldUnits.size(); ++i) {
		DestroyAllInside(*this->SoldUnits[i]);
		LetUnitDie(*this->SoldUnits[i]);
	}
	this->SoldUnits.clear();
	
	std::vector<stratagus::unit_type *> potential_items;
	std::vector<stratagus::character *> potential_heroes;
	if (this->Type->BoolFlag[RECRUITHEROES_INDEX].value && !IsNetworkGame()) { // allow heroes to be recruited at town halls
		const stratagus::civilization *civilization = this->Type->get_civilization();
		if (civilization != nullptr && civilization->ID != this->Player->Race && this->Player->Race != -1 && this->Player->Faction != -1 && this->Type == stratagus::faction::get_all()[this->Player->Faction]->get_class_unit_type(this->Type->get_unit_class())) {
			civilization = stratagus::civilization::get_all()[this->Player->Race];
		}
		const stratagus::faction *faction = this->Player->Faction != -1 ? stratagus::faction::get_all()[this->Player->Faction] : nullptr;
		
		if (CurrentQuest == nullptr) {
			//give priority to heroes with the building's settlement as their home settlement
			if (this->settlement != nullptr) {
				potential_heroes = this->Player->get_recruitable_heroes_from_list(this->settlement->get_characters());
			}

			//then give priority to heroes belonging to the building's player's faction
			if (faction != nullptr && static_cast<int>(potential_heroes.size()) < recruitable_hero_max) {
				std::vector<stratagus::character *> potential_faction_heroes = this->Player->get_recruitable_heroes_from_list(faction->get_characters());

				while (!potential_faction_heroes.empty() && static_cast<int>(potential_heroes.size()) < recruitable_hero_max) {
					stratagus::character *hero = potential_faction_heroes[SyncRand(potential_faction_heroes.size())];
					stratagus::vector::remove(potential_faction_heroes, hero);

					if (stratagus::vector::contains(potential_heroes, hero)) {
						continue;
					}

					potential_heroes.push_back(hero);
				}
			}

			//if there are still recruitable hero slots available, then try to place characters belonging to the civilization in them
			if (civilization != nullptr && static_cast<int>(potential_heroes.size()) < recruitable_hero_max) {
				std::vector<stratagus::character *> potential_civilization_heroes = this->Player->get_recruitable_heroes_from_list(civilization->get_characters());

				while (!potential_civilization_heroes.empty() && static_cast<int>(potential_heroes.size()) < recruitable_hero_max) {
					stratagus::character *hero = potential_civilization_heroes[SyncRand(potential_civilization_heroes.size())];
					stratagus::vector::remove(potential_civilization_heroes, hero);

					if (stratagus::vector::contains(potential_heroes, hero)) {
						continue;
					}

					potential_heroes.push_back(hero);
				}
			}
		}

		if (this->Player == CPlayer::GetThisPlayer()) {
			for (std::map<std::string, stratagus::character *>::iterator iterator = CustomHeroes.begin(); iterator != CustomHeroes.end(); ++iterator) {
				if (
					(iterator->second->get_civilization() && iterator->second->get_civilization() == civilization || iterator->second->get_unit_type() == civilization->get_class_unit_type(iterator->second->get_unit_type()->get_unit_class()))
					&& CheckDependencies(iterator->second->get_unit_type(), this, true) && iterator->second->CanAppear()
				) {
					potential_heroes.push_back(iterator->second);
				}
			}
		}
	} else {
		for (size_t i = 0; i < this->Type->SoldUnits.size(); ++i) {
			if (CheckDependencies(this->Type->SoldUnits[i], this)) {
				potential_items.push_back(this->Type->SoldUnits[i]);
			}
		}
	}
	
	if (potential_items.empty() && potential_heroes.empty()) {
		return;
	}
	
	int sold_unit_max = recruitable_hero_max;
	if (!potential_items.empty()) {
		sold_unit_max = 15;
	}
	
	for (int i = 0; i < sold_unit_max; ++i) {
		CUnit *new_unit = nullptr;
		if (!potential_heroes.empty()) {
			stratagus::character *chosen_hero = potential_heroes[SyncRand(potential_heroes.size())];
			new_unit = MakeUnitAndPlace(this->tilePos, *chosen_hero->get_unit_type(), CPlayer::Players[PlayerNumNeutral], this->MapLayer->ID);
			new_unit->SetCharacter(chosen_hero->Ident, chosen_hero->Custom);
			potential_heroes.erase(std::remove(potential_heroes.begin(), potential_heroes.end(), chosen_hero), potential_heroes.end());
		} else {
			stratagus::unit_type *chosen_unit_type = potential_items[SyncRand(potential_items.size())];
			new_unit = MakeUnitAndPlace(this->tilePos, *chosen_unit_type, CPlayer::Players[PlayerNumNeutral], this->MapLayer->ID);
			new_unit->GenerateSpecialProperties(this, this->Player, true, true);
			new_unit->Identified = true;
			if (new_unit->Unique && this->Player == CPlayer::GetThisPlayer()) { //send a notification if a unique item is being sold, we don't want the player to have to worry about missing it :)
				this->Player->Notify(NotifyGreen, this->tilePos, this->MapLayer->ID, "%s", _("Unique item available for sale"));
			}
		}
		new_unit->Remove(this);
		this->SoldUnits.push_back(new_unit);
		if (potential_heroes.empty() && potential_items.empty()) {
			break;
		}
	}
	if (IsOnlySelected(*this)) {
		UI.ButtonPanel.Update();
	}
}

void CUnit::SellUnit(CUnit *sold_unit, int player)
{
	this->SoldUnits.erase(std::remove(this->SoldUnits.begin(), this->SoldUnits.end(), sold_unit), this->SoldUnits.end());
	DropOutOnSide(*sold_unit, sold_unit->Direction, this);
	if (!sold_unit->Type->BoolFlag[ITEM_INDEX].value) {
		sold_unit->ChangeOwner(*CPlayer::Players[player]);
	}
	CPlayer::Players[player]->change_resource(stratagus::resource::get_all()[CopperCost], -sold_unit->GetPrice(), true);
	if (CPlayer::Players[player]->AiEnabled && !sold_unit->Type->BoolFlag[ITEM_INDEX].value && !sold_unit->Type->BoolFlag[HARVESTER_INDEX].value) { //add the hero to an AI force, if the hero isn't a harvester
		CPlayer::Players[player]->Ai->Force.RemoveDeadUnit();
		CPlayer::Players[player]->Ai->Force.Assign(*sold_unit, -1, true);
	}
	if (sold_unit->Character) {
		CPlayer::Players[player]->HeroCooldownTimer = HeroCooldownCycles;
		sold_unit->Variable[MANA_INDEX].Value = 0; //start off with 0 mana
	}
	if (IsOnlySelected(*this)) {
		UI.ButtonPanel.Update();
	}
}

/**
**  Produce a resource
**
**  @param resource  Resource to be produced.
*/
void CUnit::ProduceResource(const int resource)
{
	if (resource == this->GivesResource) {
		return;
	}
	
	int old_resource = this->GivesResource;
	
	if (resource != 0) {
		this->GivesResource = resource;
		this->ResourcesHeld = 10000;
	} else {
		this->GivesResource = 0;
		this->ResourcesHeld = 0;
	}
	
	if (old_resource != 0) {
		if (this->Resource.Workers) {
			for (CUnit *uins = this->Resource.Workers; uins; uins = uins->NextWorker) {
				if (uins->Container == this) {
					uins->CurrentOrder()->Finished = true;
					DropOutOnSide(*uins, LookingW, this);
				}
			}
		}
		this->Resource.Active = 0;
	}
}

/**
**  Sells 100 of a resource for copper
**
**  @param resource  Resource to be sold.
*/
void CUnit::SellResource(const int resource, const int player)
{
	if ((CPlayer::Players[player]->Resources[resource] + CPlayer::Players[player]->StoredResources[resource]) < 100) {
		return;
	}

	CPlayer::Players[player]->change_resource(stratagus::resource::get_all()[resource], -100, true);
	CPlayer::Players[player]->change_resource(stratagus::resource::get_all()[CopperCost], this->Player->GetEffectiveResourceSellPrice(resource), true);
	
	this->Player->DecreaseResourcePrice(resource);
}

/**
**  Buy a resource for copper
**
**  @param resource  Resource to be bought.
*/
void CUnit::BuyResource(const int resource, const int player)
{
	if ((CPlayer::Players[player]->Resources[CopperCost] + CPlayer::Players[player]->StoredResources[CopperCost]) < this->Player->GetEffectiveResourceBuyPrice(resource)) {
		return;
	}

	CPlayer::Players[player]->change_resource(stratagus::resource::get_all()[resource], 100, true);
	CPlayer::Players[player]->change_resource(stratagus::resource::get_all()[CopperCost], -this->Player->GetEffectiveResourceBuyPrice(resource), true);
	
	this->Player->IncreaseResourcePrice(resource);
}

void CUnit::Scout()
{
	int scout_range = std::max(16, this->CurrentSightRange * 2);
			
	Vec2i target_pos = this->tilePos;

	target_pos.x += SyncRand(scout_range * 2 + 1) - scout_range;
	target_pos.y += SyncRand(scout_range * 2 + 1) - scout_range;

	// restrict to map
	CMap::Map.Clamp(target_pos, this->MapLayer->ID);

	// move if possible
	if (target_pos != this->tilePos) {
		// if the tile the scout is moving to happens to have a layer connector, use it
		bool found_connector = false;
		CUnitCache &unitcache = CMap::Map.Field(target_pos, this->MapLayer->ID)->UnitCache;
		for (CUnitCache::iterator it = unitcache.begin(); it != unitcache.end(); ++it) {
			CUnit *connector = *it;

			if (connector->ConnectingDestination != nullptr && this->CanUseItem(connector)) {
				CommandUse(*this, *connector, FlushCommands);
				found_connector = true;
				break;
			}
		}
		if (found_connector) {
			return;
		}
				
		UnmarkUnitFieldFlags(*this);
		if (UnitCanBeAt(*this, target_pos, this->MapLayer->ID)) {
			MarkUnitFieldFlags(*this);
			CommandMove(*this, target_pos, FlushCommands, this->MapLayer->ID);
			return;
		}
		MarkUnitFieldFlags(*this);
	}
}
//Wyrmgus end

UnitAction CUnit::CurrentAction() const
{
	return this->CurrentOrder()->Action;
}

void CUnit::ClearAction()
{
	Orders[0]->Finished = true;

	if (Selected) {
		SelectedUnitChanged();
	}
}


bool CUnit::IsIdle() const
{
	//Wyrmgus start
//	return Orders.size() == 1 && CurrentAction() == UnitAction::Still;
	return Orders.size() == 1 && CurrentAction() == UnitAction::Still && this->Variable[STUN_INDEX].Value == 0;
	//Wyrmgus end
}

bool CUnit::IsAlive() const
{
	return !Destroyed && CurrentAction() != UnitAction::Die;
}

int CUnit::GetDrawLevel() const
{
	return ((Type->get_corpse_type() != nullptr && CurrentAction() == UnitAction::Die) ?
		Type->get_corpse_type()->get_draw_level() :
	((CurrentAction() == UnitAction::Die) ? Type->get_draw_level() - 10 : Type->get_draw_level()));
}

/**
**  Initialize the unit slot with default values.
**
**  @param type    Unit-type
*/
void CUnit::Init(const stratagus::unit_type &type)
{
	//  Set refs to 1. This is the "I am alive ref", lost in ReleaseUnit.
	Refs = 1;

	//  Build all unit table
	UnitManager.Add(this);

	//  Initialise unit structure (must be zero filled!)
	Type = &type;

	Seen.Frame = UnitNotSeen; // Unit isn't yet seen

	Frame = type.StillFrame;

	if (UnitTypeVar.GetNumberVariable()) {
		Assert(Variable.empty());
		this->Variable = type.MapDefaultStat.Variables;
	} else {
		this->Variable.clear();
	}

	IndividualUpgrades.clear();

	// Set a heading for the unit if it Handles Directions
	// Don't set a building heading, as only 1 construction direction
	//   is allowed.
	if (type.NumDirections > 1 && type.BoolFlag[NORANDOMPLACING_INDEX].value == false && type.Sprite && !type.BoolFlag[BUILDING_INDEX].value) {
		this->Direction = SyncRand(256); // random heading
		UnitUpdateHeading(*this);
	}

	// Create AutoCastSpell and SpellCoolDownTimers arrays for casters
	//Wyrmgus start
//	if (type.CanCastSpell) {
	//to avoid crashes with spell items for units who cannot ordinarily cast spells
	//Wyrmgus end
		AutoCastSpell = new char[CSpell::Spells.size()];
		SpellCoolDownTimers = new int[CSpell::Spells.size()];
		memset(SpellCoolDownTimers, 0, CSpell::Spells.size() * sizeof(int));
		if (Type->AutoCastActive) {
			memcpy(AutoCastSpell, Type->AutoCastActive, CSpell::Spells.size());
		} else {
			memset(AutoCastSpell, 0, CSpell::Spells.size());
		}
	//Wyrmgus start
//	}
	//Wyrmgus end
	Active = 1;
	Removed = 1;
	
	// Has StartingResources, Use those
	//Wyrmgus start
//	this->ResourcesHeld = type.StartingResources;
	if (type.GivesResource) {
		this->GivesResource = type.GivesResource;
		if (type.StartingResources.size() > 0) {
			this->ResourcesHeld = type.StartingResources[SyncRand(type.StartingResources.size())];
		} else {
			this->ResourcesHeld = 0;
		}
	}
	//Wyrmgus end

	Assert(Orders.empty());

	Orders.push_back(COrder::NewActionStill());

	Assert(NewOrder == nullptr);
	NewOrder = nullptr;
	Assert(SavedOrder == nullptr);
	SavedOrder = nullptr;
	Assert(CriticalOrder == nullptr);
	CriticalOrder = nullptr;
}

/**
**  Restore the saved order
**
**  @return      True if the saved order was restored
*/
bool CUnit::RestoreOrder()
{
	COrder *savedOrder = this->SavedOrder;

	if (savedOrder == nullptr) {
		return false;
	}

	if (savedOrder->IsValid() == false) {
		delete savedOrder;
		this->SavedOrder = nullptr;
		return false;
	}

	// Cannot delete this->Orders[0] since it is generally that order
	// which call this method.
	this->Orders[0]->Finished = true;

	//copy
	this->Orders.insert(this->Orders.begin() + 1, savedOrder);

	this->SavedOrder = nullptr;
	return true;
}

/**
**  Check if we can store this order
**
**  @return      True if the order could be saved
*/
bool CUnit::CanStoreOrder(COrder *order)
{
	Assert(order);

	if ((order && order->Finished == true) || order->IsValid() == false) {
		return false;
	}
	if (this->SavedOrder != nullptr) {
		return false;
	}
	return true;
}

/**
**  Assigns a unit to a player, adjusting buildings, food and totals
**
**  @param player  player which have the unit.
*/
void CUnit::AssignToPlayer(CPlayer &player)
{
	const stratagus::unit_type &type = *Type;

	// Build player unit table
	//Wyrmgus start
//	if (!type.BoolFlag[VANISHES_INDEX].value && CurrentAction() != UnitAction::Die) {
	if (!type.BoolFlag[VANISHES_INDEX].value && CurrentAction() != UnitAction::Die && !this->Destroyed) {
	//Wyrmgus end
		player.AddUnit(*this);
		if (!SaveGameLoading) {
			// If unit is dying, it's already been lost by all players
			// don't count again
			if (type.BoolFlag[BUILDING_INDEX].value) {
				// FIXME: support more races
				//Wyrmgus start
//				if (!type.BoolFlag[WALL_INDEX].value && &type != UnitTypeOrcWall && &type != UnitTypeHumanWall) {
				//Wyrmgus end
					player.TotalBuildings++;
				//Wyrmgus start
//				}
				//Wyrmgus end
			} else {
				player.TotalUnits++;
				
				for (const auto &objective : player.get_quest_objectives()) {
					const stratagus::quest_objective *quest_objective = objective->get_quest_objective();

					if (quest_objective->get_objective_type() != stratagus::objective_type::build_units) {
						continue;
					}

					if (!stratagus::vector::contains(quest_objective->UnitTypes, &type) && !stratagus::vector::contains(quest_objective->get_unit_classes(), type.get_unit_class())) {
						continue;
					}

					//only buildings can belong to settlements, so we don't need to check to see whether the settlement is correct here

					objective->Counter = std::min(objective->Counter + 1, quest_objective->get_quantity());
				}
			}
		}
		
		player.IncreaseCountsForUnit(this);

		player.Demand += type.Stats[player.Index].Variables[DEMAND_INDEX].Value; // food needed
	}

	// Don't Add the building if it's dying, used to load a save game
	//Wyrmgus start
//	if (type.BoolFlag[BUILDING_INDEX].value && CurrentAction() != UnitAction::Die) {
	if (type.BoolFlag[BUILDING_INDEX].value && CurrentAction() != UnitAction::Die && !this->Destroyed && !type.BoolFlag[VANISHES_INDEX].value) {
	//Wyrmgus end
		//Wyrmgus start
//		if (!type.BoolFlag[WALL_INDEX].value && &type != UnitTypeOrcWall && &type != UnitTypeHumanWall) {
		//Wyrmgus end
			player.NumBuildings++;
			//Wyrmgus start
			if (CurrentAction() == UnitAction::Built) {
				player.NumBuildingsUnderConstruction++;
				player.ChangeUnitTypeUnderConstructionCount(&type, 1);
			}
			//Wyrmgus end
		//Wyrmgus start
//		}
		//Wyrmgus end
	}
	Player = &player;
	Stats = &type.Stats[Player->Index];
	if (!SaveGameLoading) {
		if (UnitTypeVar.GetNumberVariable()) {
			Assert(!Stats->Variables.empty());
			this->Variable = Stats->Variables;
		}
	}
	
	//Wyrmgus start
	if (!SaveGameLoading) {
		//assign a gender to the unit
		if (static_cast<stratagus::gender>(this->Variable[GENDER_INDEX].Value) == stratagus::gender::none && this->Type->BoolFlag[ORGANIC_INDEX].value) { // Gender: 0 = Not Set, 1 = Male, 2 = Female, 3 = Asexual
			this->Variable[GENDER_INDEX].Value = SyncRand(2) + 1;
			this->Variable[GENDER_INDEX].Max = static_cast<int>(stratagus::gender::count);
			this->Variable[GENDER_INDEX].Enable = 1;
		}
		
		//generate a personal name for the unit, if applicable
		if (this->Character == nullptr) {
			this->UpdatePersonalName();
		}
		
		this->UpdateSoldUnits();

		if (this->settlement != nullptr && this->settlement->get_site_unit() == this && !this->UnderConstruction) {
			//update settlement ownership
			if (player.Index != PlayerNumNeutral) {
				this->settlement->set_owner(&player);
			} else {
				this->settlement->set_owner(nullptr);
			}
		}
	}
	//Wyrmgus end
}

const stratagus::player_color *CUnit::get_player_color() const
{
	if (this->RescuedFrom != nullptr) {
		return this->RescuedFrom->get_player_color();
	} else if (this->Player != nullptr) {
		return this->Player->get_player_color();
	}

	return nullptr;
}

/**
**  Create a new unit.
**
**  @param type      Pointer to unit-type.
**  @param player    Pointer to owning player.
**
**  @return          Pointer to created unit.
*/
CUnit *MakeUnit(const stratagus::unit_type &type, CPlayer *player)
{
	CUnit *unit = UnitManager.AllocUnit();
	if (unit == nullptr) {
		return nullptr;
	}
	unit->Init(type);
	// Only Assign if a Player was specified
	if (player) {
		unit->AssignToPlayer(*player);

		//Wyrmgus start
		unit->ChooseVariation(nullptr, true);
		for (int i = 0; i < MaxImageLayers; ++i) {
			unit->ChooseVariation(nullptr, true, i);
		}
		unit->UpdateButtonIcons();
		unit->UpdateXPRequired();
		//Wyrmgus end
	}

	//Wyrmgus start
	// grant the unit the civilization/faction upgrades of its respective civilization/faction, so that it is able to pursue its upgrade line in experience upgrades even if it changes hands
	if (unit->Type->get_civilization() != nullptr) {
		CUpgrade *civilization_upgrade = unit->Type->get_civilization()->get_upgrade();
		if (civilization_upgrade != nullptr) {
			unit->SetIndividualUpgrade(civilization_upgrade, 1);
		}
	}
	if (unit->Type->Faction != -1 && !stratagus::faction::get_all()[unit->Type->Faction]->FactionUpgrade.empty()) {
		CUpgrade *faction_upgrade = CUpgrade::try_get(stratagus::faction::get_all()[unit->Type->Faction]->FactionUpgrade);
		if (faction_upgrade) {
			unit->SetIndividualUpgrade(faction_upgrade, 1);
		}
	}

	// generate a trait for the unit, if any are available (only if the editor isn't running)
	if (Editor.Running == EditorNotRunning && unit->Type->Traits.size() > 0) {
		TraitAcquire(*unit, unit->Type->Traits[SyncRand(unit->Type->Traits.size())]);
	}
	
	for (size_t i = 0; i < unit->Type->StartingAbilities.size(); ++i) {
		if (CheckDependencies(unit->Type->StartingAbilities[i], unit)) {
			IndividualUpgradeAcquire(*unit, unit->Type->StartingAbilities[i]);
		}
	}
	
	if (unit->Type->Elixir) { //set the unit type's elixir, if any
		unit->SetElixir(unit->Type->Elixir);
	}
	
	unit->Variable[MANA_INDEX].Value = 0; //start off with 0 mana
	//Wyrmgus end
	
	if (unit->Type->OnInit) {
		unit->Type->OnInit->pushPreamble();
		unit->Type->OnInit->pushInteger(UnitNumber(*unit));
		unit->Type->OnInit->run();
	}
	
	return unit;
}

/**
**  (Un)Mark on vision table the Sight of the unit
**  (and units inside for transporter (recursively))
**
**  @param unit    Unit to (un)mark.
**  @param pos     coord of first container of unit.
**  @param width   Width of the first container of unit.
**  @param height  Height of the first container of unit.
**  @param f       Function to (un)mark for normal vision.
**  @param f2        Function to (un)mark for cloaking vision.
*/
static void MapMarkUnitSightRec(const CUnit &unit, const Vec2i &pos, int width, int height,
								//Wyrmgus start
//								MapMarkerFunc *f, MapMarkerFunc *f2)
								MapMarkerFunc *f, MapMarkerFunc *f2, MapMarkerFunc *f3)
								//Wyrmgus end
{
	Assert(f);
	//Wyrmgus start
	/*
	MapSight(*unit.Player, pos, width, height,
			 unit.GetFirstContainer()->CurrentSightRange, f);

	if (unit.Type && unit.Type->BoolFlag[DETECTCLOAK_INDEX].value && f2) {
		MapSight(*unit.Player, pos, width, height,
				 unit.GetFirstContainer()->CurrentSightRange, f2);
	}
	*/

	MapSight(*unit.Player, pos, width, height,
			 unit.Container && unit.Container->CurrentSightRange >= unit.CurrentSightRange ? unit.Container->CurrentSightRange : unit.CurrentSightRange, f, unit.MapLayer->ID);

	if (unit.Type && unit.Type->BoolFlag[DETECTCLOAK_INDEX].value && f2) {
		MapSight(*unit.Player, pos, width, height,
				 unit.Container && unit.Container->CurrentSightRange >= unit.CurrentSightRange ? unit.Container->CurrentSightRange : unit.CurrentSightRange, f2, unit.MapLayer->ID);
	}
	
	if (unit.Variable[ETHEREALVISION_INDEX].Value && f3) {
		MapSight(*unit.Player, pos, width, height,
				 unit.Container && unit.Container->CurrentSightRange >= unit.CurrentSightRange ? unit.Container->CurrentSightRange : unit.CurrentSightRange, f3, unit.MapLayer->ID);
	}
	//Wyrmgus end

	CUnit *unit_inside = unit.UnitInside;
	for (int i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
		//Wyrmgus start
//		MapMarkUnitSightRec(*unit_inside, pos, width, height, f, f2);
		MapMarkUnitSightRec(*unit_inside, pos, width, height, f, f2, f3);
		//Wyrmgus end
	}
}

/**
**	@brief	Return the topmost container for the unit
**
**	@param	unit	The unit for which to get the topmost container
**
**	@return	The unit's topmost container if present, or the unit itself otherwise; this function should never return null
*/
CUnit *CUnit::GetFirstContainer() const
{
	const CUnit *container = this;

	while (container->Container) {
		container = container->Container;
	}
	
	return const_cast<CUnit *>(container);
}

/**
**  Mark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit  unit to unmark its vision.
**  @see MapUnmarkUnitSight.
*/
void MapMarkUnitSight(CUnit &unit)
{
	CUnit *container = unit.GetFirstContainer(); //first container of the unit.
	Assert(container->Type);

	MapMarkUnitSightRec(unit, container->tilePos, container->Type->get_tile_width(), container->Type->get_tile_height(),
						//Wyrmgus start
//						MapMarkTileSight, MapMarkTileDetectCloak);
						MapMarkTileSight, MapMarkTileDetectCloak, MapMarkTileDetectEthereal);
						//Wyrmgus end

	// Never mark radar, except if the top unit, and unit is usable
	if (&unit == container && !unit.IsUnusable()) {
		if (unit.Stats->Variables[RADAR_INDEX].Value) {
			MapMarkRadar(*unit.Player, unit.tilePos, unit.Type->get_tile_width(),
						 unit.Type->get_tile_height(), unit.Stats->Variables[RADAR_INDEX].Value, unit.MapLayer->ID);
		}
		if (unit.Stats->Variables[RADARJAMMER_INDEX].Value) {
			MapMarkRadarJammer(*unit.Player, unit.tilePos, unit.Type->get_tile_width(),
							   unit.Type->get_tile_height(), unit.Stats->Variables[RADARJAMMER_INDEX].Value, unit.MapLayer->ID);
		}
	}
}

/**
**  Unmark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit    unit to unmark its vision.
**  @see MapMarkUnitSight.
*/
void MapUnmarkUnitSight(CUnit &unit)
{
	Assert(unit.Type);

	CUnit *container = unit.GetFirstContainer();
	Assert(container->Type);
	MapMarkUnitSightRec(unit,
						container->tilePos, container->Type->get_tile_width(), container->Type->get_tile_height(),
						//Wyrmgus start
//						MapUnmarkTileSight, MapUnmarkTileDetectCloak);
						MapUnmarkTileSight, MapUnmarkTileDetectCloak, MapUnmarkTileDetectEthereal);
						//Wyrmgus end

	// Never mark radar, except if the top unit?
	if (&unit == container && !unit.IsUnusable()) {
		if (unit.Stats->Variables[RADAR_INDEX].Value) {
			MapUnmarkRadar(*unit.Player, unit.tilePos, unit.Type->get_tile_width(),
						   unit.Type->get_tile_height(), unit.Stats->Variables[RADAR_INDEX].Value, unit.MapLayer->ID);
		}
		if (unit.Stats->Variables[RADARJAMMER_INDEX].Value) {
			MapUnmarkRadarJammer(*unit.Player, unit.tilePos, unit.Type->get_tile_width(),
								 unit.Type->get_tile_height(), unit.Stats->Variables[RADARJAMMER_INDEX].Value, unit.MapLayer->ID);
		}
		
	}
}

/**
**  Update the Unit Current sight range to good value and transported units inside.
**
**  @param unit  unit to update SightRange
**
**  @internal before using it, MapUnmarkUnitSight(unit)
**  and after MapMarkUnitSight(unit)
**  are often necessary.
**
**  FIXME @todo manage differently unit inside with option.
**  (no vision, min, host value, own value, bonus value, ...)
*/
void UpdateUnitSightRange(CUnit &unit)
{
//Wyrmgus start
/*
#if 0 // which is the better ? caller check ?
	if (SaveGameLoading) {
		return ;
	}
#else
	Assert(!SaveGameLoading);
#endif
*/
//Wyrmgus end
	// FIXME : these values must be configurable.
	//Wyrmgus start
	int unit_sight_range = unit.Variable[SIGHTRANGE_INDEX].Max;
	const stratagus::time_of_day *time_of_day = unit.get_center_tile_time_of_day();
	if (time_of_day != nullptr) {
		if (time_of_day->is_day()) {
			unit_sight_range += unit.Variable[DAYSIGHTRANGEBONUS_INDEX].Value;
		} else if (time_of_day->is_night()) {
			unit_sight_range += unit.Variable[NIGHTSIGHTRANGEBONUS_INDEX].Value;
		}
	}
	unit_sight_range = std::max<int>(1, unit_sight_range);
	//Wyrmgus end
	if (unit.UnderConstruction) { // Units under construction have no sight range.
		unit.CurrentSightRange = 1;
	} else if (!unit.Container) { // proper value.
		//Wyrmgus start
//		unit.CurrentSightRange = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;
		unit.CurrentSightRange = unit_sight_range;
		//Wyrmgus end
	} else { // value of it container.
		//Wyrmgus start
//		unit.CurrentSightRange = unit.Container->CurrentSightRange;
		//if a unit is inside a container, then use the sight of the unit or the container, whichever is greater
		if (unit_sight_range <= unit.Container->CurrentSightRange) {
			unit.CurrentSightRange = unit.Container->CurrentSightRange;
		} else {
			unit.CurrentSightRange = unit_sight_range;
		}
		//Wyrmgus end
	}

	CUnit *unit_inside = unit.UnitInside;
	for (int i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UpdateUnitSightRange(*unit_inside);
	}
}

/**
**  Mark the field with the FieldFlags.
**
**  @param unit  unit to mark.
*/
void MarkUnitFieldFlags(const CUnit &unit)
{
	const unsigned int flags = unit.Type->FieldFlags;
	int h = unit.Type->get_tile_height();          // Tile height of the unit.
	const int width = unit.Type->get_tile_width(); // Tile width of the unit.
	unsigned int index = unit.Offset;

	//Wyrmgus start
//	if (unit.Type->BoolFlag[VANISHES_INDEX].value) {
	if (unit.Type->BoolFlag[VANISHES_INDEX].value || unit.CurrentAction() == UnitAction::Die) {
	//Wyrmgus end
		return ;
	}
	do {
		CMapField *mf = unit.MapLayer->Field(index);
		int w = width;
		do {
			mf->Flags |= flags;
			++mf;
		} while (--w);
		index += unit.MapLayer->get_width();
	} while (--h);
}

class _UnmarkUnitFieldFlags
{
public:
	_UnmarkUnitFieldFlags(const CUnit &unit, CMapField *mf) : main(&unit), mf(mf)
	{}

	void operator()(CUnit *const unit) const
	{
		if (main != unit && unit->CurrentAction() != UnitAction::Die) {
			mf->Flags |= unit->Type->FieldFlags;
		}
	}
private:
	const CUnit *const main;
	CMapField *mf;
};


/**
**  Mark the field with the FieldFlags.
**
**  @param unit  unit to mark.
*/
void UnmarkUnitFieldFlags(const CUnit &unit)
{
	const unsigned int flags = ~unit.Type->FieldFlags;
	const int width = unit.Type->get_tile_width();
	int h = unit.Type->get_tile_height();
	unsigned int index = unit.Offset;

	if (unit.Type->BoolFlag[VANISHES_INDEX].value) {
		return ;
	}
	do {
		CMapField *mf = unit.MapLayer->Field(index);

		int w = width;
		do {
			mf->Flags &= flags;//clean flags
			_UnmarkUnitFieldFlags funct(unit, mf);

			mf->UnitCache.for_each(funct);
			++mf;
		} while (--w);
		index += unit.MapLayer->get_width();
	} while (--h);
}

/**
**  Add unit to a container. It only updates linked list stuff.
**
**  @param host  Pointer to container.
*/
void CUnit::AddInContainer(CUnit &host)
{
	Assert(Container == nullptr);
	Container = &host;
	if (host.InsideCount == 0) {
		NextContained = PrevContained = this;
	} else {
		NextContained = host.UnitInside;
		PrevContained = host.UnitInside->PrevContained;
		host.UnitInside->PrevContained->NextContained = this;
		host.UnitInside->PrevContained = this;
	}
	host.UnitInside = this;
	host.InsideCount++;
	//Wyrmgus start
	if (!SaveGameLoading) { //if host has no range by itself, but the unit has range, and the unit can attack from a transporter, change the host's range to the unit's; but don't do this while loading, as it causes a crash (since one unit needs to be loaded before the other, and when this function is processed both won't already have their variables set)
		host.UpdateContainerAttackRange();
	}
	//Wyrmgus end
}

/**
**  Remove unit from a container. It only updates linked list stuff.
**
**  @param unit  Pointer to unit.
*/
static void RemoveUnitFromContainer(CUnit &unit)
{
	CUnit *host = unit.Container; // transporter which contain unit.
	Assert(unit.Container);
	Assert(unit.Container->InsideCount > 0);
	
	host->InsideCount--;
	unit.NextContained->PrevContained = unit.PrevContained;
	unit.PrevContained->NextContained = unit.NextContained;
	if (host->InsideCount == 0) {
		host->UnitInside = nullptr;
	} else {
		if (host->UnitInside == &unit) {
			host->UnitInside = unit.NextContained;
		}
	}
	unit.Container = nullptr;
	//Wyrmgus start
	//reset host attack range
	host->UpdateContainerAttackRange();
	//Wyrmgus end
}

//Wyrmgus start
void CUnit::UpdateContainerAttackRange()
{
	//reset attack range, if this unit is a transporter (or garrisonable building) from which units can attack
	if (this->Type->CanTransport() && this->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value && !this->Type->CanAttack) {
		this->Variable[ATTACKRANGE_INDEX].Enable = 0;
		this->Variable[ATTACKRANGE_INDEX].Max = 0;
		this->Variable[ATTACKRANGE_INDEX].Value = 0;
		if (this->BoardCount > 0) {
			CUnit *boarded_unit = this->UnitInside;
			for (int i = 0; i < this->InsideCount; ++i, boarded_unit = boarded_unit->NextContained) {
				if (boarded_unit->GetModifiedVariable(ATTACKRANGE_INDEX) > this->Variable[ATTACKRANGE_INDEX].Value && boarded_unit->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value) { //if container has no range by itself, but the unit has range, and the unit can attack from a transporter, change the container's range to the unit's
					this->Variable[ATTACKRANGE_INDEX].Enable = 1;
					this->Variable[ATTACKRANGE_INDEX].Max = boarded_unit->GetModifiedVariable(ATTACKRANGE_INDEX);
					this->Variable[ATTACKRANGE_INDEX].Value = boarded_unit->GetModifiedVariable(ATTACKRANGE_INDEX);
				}
			}
		}
	}
}

void CUnit::UpdateXPRequired()
{
	if (!this->Type->BoolFlag[ORGANIC_INDEX].value) {
		return;
	}
	
	this->Variable[XPREQUIRED_INDEX].Value = this->Type->Stats[this->Player->Index].Variables[POINTS_INDEX].Value * 4 * this->Type->Stats[this->Player->Index].Variables[LEVEL_INDEX].Value;
	int extra_levels = this->Variable[LEVEL_INDEX].Value - this->Type->Stats[this->Player->Index].Variables[LEVEL_INDEX].Value;
	for (int i = 1; i <= extra_levels; ++i) {
		this->Variable[XPREQUIRED_INDEX].Value += 50 * 4 * i;
	}
	this->Variable[XPREQUIRED_INDEX].Max = this->Variable[XPREQUIRED_INDEX].Value;
	this->Variable[XPREQUIRED_INDEX].Enable = 1;
	this->Variable[XP_INDEX].Enable = 1;
}

void CUnit::UpdatePersonalName(bool update_settlement_name)
{
	static constexpr bool surname_generation_enabled = false;

	if (this->Character != nullptr) {
		return;
	} else if (this->Type->BoolFlag[ITEM_INDEX].value || this->Unique || this->Prefix || this->Suffix) {
		this->UpdateItemName();
		return;
	}
	
	const stratagus::civilization *civilization = this->Type->get_civilization();
	stratagus::faction *faction = nullptr;
	if (this->Player->Faction != -1) {
		faction = stratagus::faction::get_all()[this->Player->Faction];
		
		if (civilization != nullptr && civilization != faction->get_civilization() && civilization->get_species() == faction->get_civilization()->get_species() && this->Type == faction->get_class_unit_type(this->Type->get_unit_class())) {
			civilization = faction->get_civilization();
		}
	}
	
	CLanguage *language = PlayerRaces.get_civilization_language(civilization ? civilization->ID : -1);
	
	if (this->Name.empty()) { //this is the first time the unit receives a name
		if (!this->Type->BoolFlag[FAUNA_INDEX].value && this->Trait != nullptr && this->Trait->Epithets.size() > 0 && SyncRand(4) == 0) { // 25% chance to give the unit an epithet based on their trait
			this->ExtraName = this->Trait->Epithets[SyncRand(this->Trait->Epithets.size())];
		}
	}
	
	if (!this->Type->IsPersonalNameValid(this->Name, faction, static_cast<stratagus::gender>(this->Variable[GENDER_INDEX].Value))) {
		// first see if can translate the current personal name
		std::string new_personal_name = PlayerRaces.TranslateName(this->Name, language);
		if (!new_personal_name.empty()) {
			this->Name = new_personal_name;
		} else {
			this->Name = this->Type->GeneratePersonalName(faction, static_cast<stratagus::gender>(this->Variable[GENDER_INDEX].Value));
		}
	}

	if constexpr (surname_generation_enabled) {
		if (civilization != nullptr) {
			const std::vector<std::string> &surnames = civilization->get_surnames();

			if (!surnames.empty() && (this->get_surname().empty() || !stratagus::vector::contains(surnames, this->get_surname()))) {
				this->surname = surnames[SyncRand(surnames.size())];
			}
		}
	}
	
	if (update_settlement_name && (this->Type->BoolFlag[TOWNHALL_INDEX].value || (this->Type->BoolFlag[BUILDING_INDEX].value && !this->settlement))) {
		this->UpdateSettlement();
	}
}

void CUnit::UpdateExtraName()
{
	if (this->Character != nullptr || !this->Type->BoolFlag[ORGANIC_INDEX].value || this->Type->BoolFlag[FAUNA_INDEX].value) {
		return;
	}
	
	if (this->Trait == nullptr) {
		return;
	}
	
	this->ExtraName.clear();
	
	if (this->Trait->Epithets.size() > 0 && SyncRand(4) == 0) { // 25% chance to give the unit an epithet based on their trait
		this->ExtraName = this->Trait->Epithets[SyncRand(this->Trait->Epithets.size())];
	}
}

void CUnit::UpdateSettlement()
{
	if (this->Removed || Editor.Running != EditorNotRunning) {
		return;
	}
	
	if (!this->Type->BoolFlag[BUILDING_INDEX].value || this->Type->TerrainType) {
		return;
	}
	
	if (this->Type->BoolFlag[TOWNHALL_INDEX].value || this->Type == settlement_site_unit_type) {
		if (!this->settlement) {
			const stratagus::civilization *civilization = this->Type->get_civilization();
			if (civilization != nullptr && this->Player->Faction != -1 && (this->Player->Race == civilization->ID || this->Type == stratagus::faction::get_all()[this->Player->Faction]->get_class_unit_type(this->Type->get_unit_class()))) {
				civilization = stratagus::civilization::get_all()[this->Player->Race];
			}
			
			int faction_id = this->Type->Faction;
			if (this->Player->Faction != -1 && this->Player->Race == (civilization ? civilization->ID : -1) && this->Type == stratagus::faction::get_all()[this->Player->Faction]->get_class_unit_type(this->Type->get_unit_class())) {
				faction_id = this->Player->Faction;
			}
			const stratagus::faction *faction = nullptr;
			if (faction_id != -1) {
				faction = stratagus::faction::get_all()[faction_id];
			}

			std::vector<stratagus::site *> potential_settlements;
			if (civilization) {
				for (stratagus::site *site : civilization->sites) {
					if (!site->get_site_unit()) {
						potential_settlements.push_back(site);
					}
				}
			}
			
			if (potential_settlements.empty() && faction) {
				for (stratagus::site *site : faction->sites) {
					if (!site->get_site_unit()) {
						potential_settlements.push_back(site);
					}
				}
			}
			
			if (potential_settlements.empty()) {
				for (stratagus::site *site : stratagus::site::get_all()) {
					if (!site->get_site_unit()) {
						potential_settlements.push_back(site);
					}
				}
			}
			
			if (potential_settlements.size() > 0) {
				this->settlement = potential_settlements[SyncRand(potential_settlements.size())];
				this->settlement->set_site_unit(this);
				CMap::Map.site_units.push_back(this);
			}
		}
		if (this->settlement != nullptr) {
			this->UpdateBuildingSettlementAssignment();
		}
	} else {
		if (this->Player->Index == PlayerNumNeutral) {
			return;
		}
		
		this->settlement = this->Player->GetNearestSettlement(this->tilePos, this->MapLayer->ID, this->Type->get_tile_size());
	}
}

void CUnit::UpdateBuildingSettlementAssignment(stratagus::site *old_settlement)
{
	if (Editor.Running != EditorNotRunning) {
		return;
	}
	
	if (this->Player->Index == PlayerNumNeutral) {
		return;
	}
		
	for (int p = 0; p < PlayerMax; ++p) {
		if (!CPlayer::Players[p]->HasNeutralFactionType() && this->Player->Index != CPlayer::Players[p]->Index) {
			continue;
		}
		for (int i = 0; i < CPlayer::Players[p]->GetUnitCount(); ++i) {
			CUnit *settlement_unit = &CPlayer::Players[p]->GetUnit(i);
			if (!settlement_unit || !settlement_unit->IsAliveOnMap() || !settlement_unit->Type->BoolFlag[BUILDING_INDEX].value || settlement_unit->Type->BoolFlag[TOWNHALL_INDEX].value || settlement_unit->Type == settlement_site_unit_type || this->MapLayer != settlement_unit->MapLayer) {
				continue;
			}
			if (old_settlement && settlement_unit->settlement != old_settlement) {
				continue;
			}
			settlement_unit->UpdateSettlement();
		}
	}
}

void CUnit::XPChanged()
{
	if (!this->Type->BoolFlag[ORGANIC_INDEX].value || this->Type->BoolFlag[BUILDING_INDEX].value) {
		return;
	}
	
	if (this->Variable[XPREQUIRED_INDEX].Value == 0) {
		return;
	}
	
	while (this->Variable[XP_INDEX].Value >= this->Variable[XPREQUIRED_INDEX].Value) {
		this->Variable[XP_INDEX].Max -= this->Variable[XPREQUIRED_INDEX].Max;
		this->Variable[XP_INDEX].Value -= this->Variable[XPREQUIRED_INDEX].Value;
		if (this->Player == CPlayer::GetThisPlayer()) {
			this->Player->Notify(NotifyGreen, this->tilePos, this->MapLayer->ID, _("%s has leveled up!"), GetMessageName().c_str());
		}
		this->IncreaseLevel(1);
	}
	
	if (!IsNetworkGame() && this->Character != nullptr && this->Player == CPlayer::GetThisPlayer()) {
		this->Character->ExperiencePercent = (this->Variable[XP_INDEX].Value * 100) / this->Variable[XPREQUIRED_INDEX].Value;
		SaveHero(this->Character);
	}
}
//Wyrmgus end

/**
**  Affect Tile coord of a unit (with units inside) to tile (x, y).
**
**  @param unit  unit to move.
**  @param pos   map tile position.
**
**  @internal before use it, Map.Remove(unit), MapUnmarkUnitSight(unit)
**  and after Map.Insert(unit), MapMarkUnitSight(unit)
**  are often necessary. Check Flag also for Pathfinder.
*/
static void UnitInXY(CUnit &unit, const Vec2i &pos, const int z)
{
	const stratagus::time_of_day *old_time_of_day = unit.get_center_tile_time_of_day();
	
	CUnit *unit_inside = unit.UnitInside;

	unit.tilePos = pos;
	unit.Offset = CMap::Map.getIndex(pos, z);
	unit.MapLayer = CMap::Map.MapLayers[z];

	const stratagus::time_of_day *new_time_of_day = unit.get_center_tile_time_of_day();

	//Wyrmgus start
	if (!SaveGameLoading && old_time_of_day != new_time_of_day) {
		UpdateUnitSightRange(unit);
	}
	//Wyrmgus end

	for (int i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UnitInXY(*unit_inside, pos, z);
	}
}

/**
**  Move a unit (with units inside) to tile (pos).
**  (Do stuff with vision, cachelist and pathfinding).
**
**  @param pos  map tile position.
**
*/
//Wyrmgus start
//void CUnit::MoveToXY(const Vec2i &pos)
void CUnit::MoveToXY(const Vec2i &pos, int z)
//Wyrmgus end
{
	MapUnmarkUnitSight(*this);
	CMap::Map.Remove(*this);
	UnmarkUnitFieldFlags(*this);

	//Wyrmgus start
//	Assert(UnitCanBeAt(*this, pos));
	Assert(UnitCanBeAt(*this, pos, z));
	//Wyrmgus end
	// Move the unit.
	//Wyrmgus start
//	UnitInXY(*this, pos);
	UnitInXY(*this, pos, z);
	//Wyrmgus end

	CMap::Map.Insert(*this);
	MarkUnitFieldFlags(*this);
	//  Recalculate the seen count.
	UnitCountSeen(*this);
	MapMarkUnitSight(*this);
	
	//Wyrmgus start
	// if there is a trap in the new tile, trigger it
	if ((this->Type->UnitType != UnitTypeType::Fly && this->Type->UnitType != UnitTypeType::FlyLow && this->Type->UnitType != UnitTypeType::Space) || !this->Type->BoolFlag[ORGANIC_INDEX].value) {
		const CUnitCache &cache = CMap::Map.Field(pos, z)->UnitCache;
		for (size_t i = 0; i != cache.size(); ++i) {
			if (!cache[i]) {
				fprintf(stderr, "Error in CUnit::MoveToXY (pos %d, %d): a unit in the tile's unit cache is null.\n", pos.x, pos.y);
			}
			CUnit &unit = *cache[i];
			if (unit.IsAliveOnMap() && unit.Type->BoolFlag[TRAP_INDEX].value) {
				FireMissile(unit, this, this->tilePos, this->MapLayer->ID);
				LetUnitDie(unit);
			}
		}
	}
	//Wyrmgus end
}

/**
**  Place unit on map.
**
**  @param pos  map tile position.
*/
void CUnit::Place(const Vec2i &pos, int z)
{
	Assert(Removed);
	
	const CMapLayer *old_map_layer = this->MapLayer;

	if (Container) {
		MapUnmarkUnitSight(*this);
		RemoveUnitFromContainer(*this);
	}
	if (!SaveGameLoading) {
		UpdateUnitSightRange(*this);
	}
	Removed = 0;
	UnitInXY(*this, pos, z);
	// Pathfinding info.
	MarkUnitFieldFlags(*this);
	// Tha cache list.
	CMap::Map.Insert(*this);
	//  Calculate the seen count.
	UnitCountSeen(*this);
	// Vision
	MapMarkUnitSight(*this);

	// Correct directions for wall units
	if (this->Type->BoolFlag[WALL_INDEX].value && this->CurrentAction() != UnitAction::Built) {
		CorrectWallDirections(*this);
		UnitUpdateHeading(*this);
		CorrectWallNeighBours(*this);
	}

	//Wyrmgus start
	if (this->IsAlive()) {
		if (this->Type->BoolFlag[BUILDING_INDEX].value) {
			this->UpdateSettlement(); // update the settlement name of a building when placing it
		}
		
		//remove pathways, destroyed walls and decoration units under buildings
		if (this->Type->BoolFlag[BUILDING_INDEX].value && !this->Type->TerrainType) {
			for (int x = this->tilePos.x; x < this->tilePos.x + this->Type->get_tile_width(); ++x) {
				for (int y = this->tilePos.y; y < this->tilePos.y + this->Type->get_tile_height(); ++y) {
					if (!CMap::Map.Info.IsPointOnMap(x, y, this->MapLayer)) {
						continue;
					}
					Vec2i building_tile_pos(x, y);
					CMapField &mf = *this->MapLayer->Field(building_tile_pos);
					if ((mf.Flags & MapFieldRoad) || (mf.Flags & MapFieldRailroad) || (mf.Flags & MapFieldWall)) {
						CMap::Map.RemoveTileOverlayTerrain(building_tile_pos, this->MapLayer->ID);
					}
					//remove decorations if a building has been built here
					std::vector<CUnit *> table;
					Select(building_tile_pos, building_tile_pos, table, this->MapLayer->ID);
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
			}
		}
		
		const stratagus::unit_type_variation *variation = this->GetVariation();
		if (variation) {
			// if a unit that is on the tile has a terrain-dependent or season-dependent variation that is not compatible with the new tile, or if this is the first position the unit is being placed in, repick the unit's variation
			if (!old_map_layer || !this->CheckTerrainForVariation(variation) || !this->CheckSeasonForVariation(variation)) {
				this->ChooseVariation();
			}
		}
	}
	//Wyrmgus end
}

/**
**  Create new unit and place on map.
**
**  @param pos     map tile position.
**  @param type    Pointer to unit-type.
**  @param player  Pointer to owning player.
**
**  @return        Pointer to created unit.
*/
CUnit *MakeUnitAndPlace(const Vec2i &pos, const stratagus::unit_type &type, CPlayer *player, int z)
{
	CUnit *unit = MakeUnit(type, player);

	if (unit != nullptr) {
		unit->Place(pos, z);
	}
	return unit;
}

//Wyrmgus start
/**
**  Create a new unit and place it on the map, updating its player accordingly.
**
**  @param pos     map tile position.
**  @param type    Pointer to unit-type.
**  @param player  Pointer to owning player.
**
**  @return        Pointer to created unit.
*/
CUnit *CreateUnit(const Vec2i &pos, const stratagus::unit_type &type, CPlayer *player, int z, bool no_bordering_building, const stratagus::site *settlement)
{
	CUnit *unit = MakeUnit(type, player);

	if (unit != nullptr) {
		unit->MapLayer = CMap::Map.MapLayers[z];

		Vec2i res_pos;
		const int heading = SyncRand(256);
		FindNearestDrop(type, pos, res_pos, heading, z, no_bordering_building, false, settlement);
		
		if (type.BoolFlag[BUILDING_INDEX].value) {
			CBuildRestrictionOnTop *b = OnTopDetails(type, nullptr);
			if (b && b->ReplaceOnBuild) {
				CUnitCache &unitCache = CMap::Map.Field(res_pos, z)->UnitCache;
				CUnitCache::iterator it = std::find_if(unitCache.begin(), unitCache.end(), HasSameTypeAs(*b->Parent));

				if (it != unitCache.end()) {
					CUnit &replacedUnit = **it;
					unit->ReplaceOnTop(replacedUnit);
				}
			}
		}
		
		unit->Place(res_pos, z);
		UpdateForNewUnit(*unit, 0);
	}
	return unit;
}

CUnit *CreateResourceUnit(const Vec2i &pos, const stratagus::unit_type &type, int z, bool allow_unique)
{
	CUnit *unit = CreateUnit(pos, type, CPlayer::Players[PlayerNumNeutral], z, true);
	unit->GenerateSpecialProperties(nullptr, nullptr, allow_unique);
			
	// create metal rocks near metal resources
	stratagus::unit_type *metal_rock_type = nullptr;
	if (type.Ident == "unit-gold-deposit") {
		metal_rock_type = stratagus::unit_type::get("unit-gold-rock");
	} else if (type.Ident == "unit-silver-deposit") {
		metal_rock_type = stratagus::unit_type::get("unit-silver-rock");
	} else if (type.Ident == "unit-copper-deposit") {
		metal_rock_type = stratagus::unit_type::get("unit-copper-rock");
	} else if (type.Ident == "unit-diamond-deposit") {
		metal_rock_type = stratagus::unit_type::get("unit-diamond-rock");
	} else if (type.Ident == "unit-emerald-deposit") {
		metal_rock_type = stratagus::unit_type::get("unit-emerald-rock");
	}
	if (metal_rock_type) {
		Vec2i metal_rock_offset((type.get_tile_size() - QSize(1, 1)) / 2);
		for (int i = 0; i < 9; ++i) {
			CUnit *metal_rock_unit = CreateUnit(unit->tilePos + metal_rock_offset, *metal_rock_type, CPlayer::Players[PlayerNumNeutral], z);
		}
	}
			
	return unit;
}
//Wyrmgus end

/**
**  Find the nearest position at which unit can be placed.
**
**  @param type     Type of the dropped unit.
**  @param goalPos  Goal map tile position.
**  @param resPos   Holds the nearest point.
**  @param heading  preferense side to drop out of.
*/
void FindNearestDrop(const stratagus::unit_type &type, const Vec2i &goalPos, Vec2i &resPos, int heading, int z, bool no_bordering_building, bool ignore_construction_requirements, const stratagus::site *settlement)
{
	int addx = 0;
	int addy = 0;
	Vec2i pos = goalPos;

	if (heading < LookingNE || heading > LookingNW) {
		goto starts;
	} else if (heading < LookingSE) {
		goto startw;
	} else if (heading < LookingSW) {
		goto startn;
	} else {
		goto starte;
	}

	// FIXME: don't search outside of the map
	for (;;) {
startw:
		for (int i = addy; i--; ++pos.y) {
			//Wyrmgus start
//			if (UnitTypeCanBeAt(type, pos)) {
			if (
				(UnitTypeCanBeAt(type, pos, z) || (type.BoolFlag[BUILDING_INDEX].value && OnTopDetails(type, nullptr) && !ignore_construction_requirements))
				&& (!type.BoolFlag[BUILDING_INDEX].value || ignore_construction_requirements || CanBuildHere(nullptr, type, pos, z, no_bordering_building) != nullptr)
				&& (settlement == nullptr || CMap::Map.is_rect_in_settlement(QRect(pos, type.get_tile_size()), z, settlement))
			) {
			//Wyrmgus end
				goto found;
			}
		}
		++addx;
starts:
		for (int i = addx; i--; ++pos.x) {
			//Wyrmgus start
//			if (UnitTypeCanBeAt(type, pos)) {
			if (
				(UnitTypeCanBeAt(type, pos, z) || (type.BoolFlag[BUILDING_INDEX].value && OnTopDetails(type, nullptr) && !ignore_construction_requirements))
				&& (!type.BoolFlag[BUILDING_INDEX].value || ignore_construction_requirements || CanBuildHere(nullptr, type, pos, z, no_bordering_building) != nullptr)
				&& (settlement == nullptr || CMap::Map.is_rect_in_settlement(QRect(pos, type.get_tile_size()), z, settlement))
			) {
			//Wyrmgus end
				goto found;
			}
		}
		++addy;
starte:
		for (int i = addy; i--; --pos.y) {
			//Wyrmgus start
//			if (UnitTypeCanBeAt(type, pos)) {
			if (
				(UnitTypeCanBeAt(type, pos, z) || (type.BoolFlag[BUILDING_INDEX].value && OnTopDetails(type, nullptr) && !ignore_construction_requirements))
				&& (!type.BoolFlag[BUILDING_INDEX].value || ignore_construction_requirements || CanBuildHere(nullptr, type, pos, z, no_bordering_building) != nullptr)
				&& (settlement == nullptr || CMap::Map.is_rect_in_settlement(QRect(pos, type.get_tile_size()), z, settlement))
			) {
			//Wyrmgus end
				goto found;
			}
		}
		++addx;
startn:
		for (int i = addx; i--; --pos.x) {
			//Wyrmgus start
//			if (UnitTypeCanBeAt(type, pos)) {
			if (
				(UnitTypeCanBeAt(type, pos, z) || (type.BoolFlag[BUILDING_INDEX].value && OnTopDetails(type, nullptr) && !ignore_construction_requirements))
				&& (!type.BoolFlag[BUILDING_INDEX].value || ignore_construction_requirements || CanBuildHere(nullptr, type, pos, z, no_bordering_building) != nullptr)
				&& (settlement == nullptr || CMap::Map.is_rect_in_settlement(QRect(pos, type.get_tile_size()), z, settlement))
			) {
			//Wyrmgus end
				goto found;
			}
		}
		++addy;
	}

found:
	resPos = pos;
}

/**
**  Remove unit from map.
**
**  Update selection.
**  Update panels.
**  Update map.
**
**  @param host  Pointer to housing unit.
*/
void CUnit::Remove(CUnit *host)
{
	if (Removed) { // could happen!
		// If unit is removed (inside) and building is destroyed.
		DebugPrint("unit '%s(%d)' already removed\n" _C_ Type->Ident.c_str() _C_ UnitNumber(*this));
		return;
	}
	CMap::Map.Remove(*this);
	MapUnmarkUnitSight(*this);
	UnmarkUnitFieldFlags(*this);
	if (host) {
		AddInContainer(*host);
		UpdateUnitSightRange(*this);
		UnitInXY(*this, host->tilePos, host->MapLayer->ID);
		MapMarkUnitSight(*this);
	}

	Removed = 1;

	// Correct surrounding walls directions
	if (this->Type->BoolFlag[WALL_INDEX].value) {
		CorrectWallNeighBours(*this);
	}

	//  Remove unit from the current selection
	if (this->Selected) {
		if (::Selected.size() == 1) { //  Remove building cursor
			CancelBuildingMode();
		}
		UnSelectUnit(*this);
		//Wyrmgus start
//		SelectionChanged();
		if (GameRunning) { // to avoid a crash when SelectionChanged() calls UI.ButtonPanel.Update()
			SelectionChanged();
		}
		//Wyrmgus end
	}
	// Remove unit from team selections
	if (!Selected && TeamSelected) {
		UnSelectUnit(*this);
	}

	// Unit is seen as under cursor
	if (UnitUnderCursor == this) {
		UnitUnderCursor = nullptr;
	}
}

/**
**  Update information for lost units.
**
**  @param unit  Pointer to unit.
**
**  @note Also called by ChangeUnitOwner
*/
void UnitLost(CUnit &unit)
{
	CPlayer &player = *unit.Player;

	Assert(&player);  // Next code didn't support no player!

	//  Call back to AI, for killed or lost units.
	if (Editor.Running == EditorNotRunning) {
		if (player.AiEnabled) {
			AiUnitKilled(unit);
		} else {
			//  Remove unit from its groups
			if (unit.GroupId) {
				RemoveUnitFromGroups(unit);
			}
		}
	}

	//  Remove the unit from the player's units table.

	const stratagus::unit_type &type = *unit.Type;
	if (!type.BoolFlag[VANISHES_INDEX].value) {
		player.RemoveUnit(unit);

		if (type.BoolFlag[BUILDING_INDEX].value) {
			// FIXME: support more races
			//Wyrmgus start
//			if (!type.BoolFlag[WALL_INDEX].value && &type != UnitTypeOrcWall && &type != UnitTypeHumanWall) {
			//Wyrmgus end
				player.NumBuildings--;
				//Wyrmgus start
				if (unit.CurrentAction() == UnitAction::Built) {
					player.NumBuildingsUnderConstruction--;
					player.ChangeUnitTypeUnderConstructionCount(&type, -1);
				}
				//Wyrmgus end
			//Wyrmgus start
//			}
			//Wyrmgus end
		}
		if (unit.CurrentAction() != UnitAction::Built) {
			if (player.AiEnabled && player.Ai) {
				if (std::find(player.Ai->Scouts.begin(), player.Ai->Scouts.end(), &unit) != player.Ai->Scouts.end()) {
					if (player.Ai->Scouting) { //if an AI player's scout has been lost, unmark it as "scouting" so that the force can see if it now has a viable target
						player.Ai->Scouting = false;
					}
				}
				for (std::map<int, std::vector<CUnit *>>::iterator iterator = player.Ai->Transporters.begin(); iterator != player.Ai->Transporters.end(); ++iterator) {
					if (std::find(iterator->second.begin(), iterator->second.end(), &unit) != iterator->second.end()) {
						iterator->second.erase(std::remove(iterator->second.begin(), iterator->second.end(), &unit), iterator->second.end());
					}
				}
			}
			
			player.DecreaseCountsForUnit(&unit);
			
			if (unit.Variable[LEVELUP_INDEX].Value > 0) {
				player.UpdateLevelUpUnits(); //recalculate level-up units, since this unit no longer should be in that vector
			}
			//Wyrmgus end
		}
	}

	//  Handle unit demand. (Currently only food supported.)
	player.Demand -= type.Stats[player.Index].Variables[DEMAND_INDEX].Value;

	//  Update information.
	if (unit.CurrentAction() != UnitAction::Built) {
		player.Supply -= unit.Variable[SUPPLY_INDEX].Value;
		// Decrease resource limit
		for (int i = 0; i < MaxCosts; ++i) {
			if (player.MaxResources[i] != -1 && type.Stats[player.Index].Storing[i]) {
				const int newMaxValue = player.MaxResources[i] - type.Stats[player.Index].Storing[i];

				player.MaxResources[i] = std::max(0, newMaxValue);
				player.set_resource(stratagus::resource::get_all()[i], player.StoredResources[i], STORE_BUILDING);
			}
		}
		//  Handle income improvements, look if a player loses a building
		//  which have given him a better income, find the next best
		//  income.
		for (int i = 1; i < MaxCosts; ++i) {
			if (player.Incomes[i] && type.Stats[player.Index].ImproveIncomes[i] == player.Incomes[i]) {
				int m = stratagus::resource::get_all()[i]->DefaultIncome;

				for (int j = 0; j < player.GetUnitCount(); ++j) {
					m = std::max(m, player.GetUnit(j).Type->Stats[player.Index].ImproveIncomes[i]);
				}
				player.Incomes[i] = m;
			}
		}
		
		if (type.Stats[player.Index].Variables[TRADECOST_INDEX].Enable) {
			int m = DefaultTradeCost;

			for (int j = 0; j < player.GetUnitCount(); ++j) {
				if (player.GetUnit(j).Type->Stats[player.Index].Variables[TRADECOST_INDEX].Enable) {
					m = std::min(m, player.GetUnit(j).Type->Stats[player.Index].Variables[TRADECOST_INDEX].Value);
				}
			}
			player.TradeCost = m;
		}
		
		//Wyrmgus start
		if (type.BoolFlag[TOWNHALL_INDEX].value) {
			bool lost_town_hall = true;
			for (int j = 0; j < player.GetUnitCount(); ++j) {
				if (player.GetUnit(j).Type->BoolFlag[TOWNHALL_INDEX].value) {
					lost_town_hall = false;
				}
			}
			if (lost_town_hall && CPlayer::GetThisPlayer()->HasContactWith(player)) {
				player.LostTownHallTimer = GameCycle + (30 * CYCLES_PER_SECOND); //30 seconds until being revealed
				for (int j = 0; j < NumPlayers; ++j) {
					if (player.Index != j && CPlayer::Players[j]->Type != PlayerNobody) {
						CPlayer::Players[j]->Notify(_("%s has lost their last town hall, and will be revealed in thirty seconds!"), player.Name.c_str());
					} else {
						CPlayer::Players[j]->Notify("%s", _("You have lost your last town hall, and will be revealed in thirty seconds!"));
					}
				}
			}
		}
		//Wyrmgus end
	}

	//  Handle order cancels.
	unit.CurrentOrder()->Cancel(unit);

	DebugPrint("%d: Lost %s(%d)\n" _C_ player.Index _C_ type.Ident.c_str() _C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	//Wyrmgus start
//	CBuildRestrictionOnTop *b = OnTopDetails(unit, nullptr);
	CBuildRestrictionOnTop *b = OnTopDetails(*unit.Type, nullptr);
	//Wyrmgus end
	if (b != nullptr) {
		//Wyrmgus start
//		if (b->ReplaceOnDie && (type.GivesResource && unit.ResourcesHeld != 0)) {
		if (b->ReplaceOnDie && (!type.GivesResource || unit.ResourcesHeld != 0)) {
		//Wyrmgus end
			CUnit *temp = MakeUnitAndPlace(unit.tilePos, *b->Parent, CPlayer::Players[PlayerNumNeutral], unit.MapLayer->ID);
			if (temp == nullptr) {
				DebugPrint("Unable to allocate Unit");
			} else {
				//Wyrmgus start
//				temp->ResourcesHeld = unit.ResourcesHeld;
//				temp->Variable[GIVERESOURCE_INDEX].Value = unit.Variable[GIVERESOURCE_INDEX].Value;
//				temp->Variable[GIVERESOURCE_INDEX].Max = unit.Variable[GIVERESOURCE_INDEX].Max;
//				temp->Variable[GIVERESOURCE_INDEX].Enable = unit.Variable[GIVERESOURCE_INDEX].Enable;
				//Wyrmgus end
				//Wyrmgus start
				if (unit.Unique != nullptr) {
					temp->SetUnique(unit.Unique);
				} else {
					if (unit.Prefix != nullptr) {
						temp->SetPrefix(unit.Prefix);
					}
					if (unit.Suffix != nullptr) {
						temp->SetSuffix(unit.Suffix);
					}
					if (unit.Spell != nullptr) {
						temp->SetSpell(unit.Spell);
					}
				}
				if (unit.settlement != nullptr) {
					if (unit.Type->BoolFlag[TOWNHALL_INDEX].value) {
						temp->settlement = unit.settlement;
						temp->settlement->set_site_unit(temp);
						CMap::Map.site_units.erase(std::remove(CMap::Map.site_units.begin(), CMap::Map.site_units.end(), &unit), CMap::Map.site_units.end());
						CMap::Map.site_units.push_back(temp);
					}
				}
				if (type.GivesResource && unit.ResourcesHeld != 0) {
					temp->SetResourcesHeld(unit.ResourcesHeld);
					temp->Variable[GIVERESOURCE_INDEX].Value = unit.Variable[GIVERESOURCE_INDEX].Value;
					temp->Variable[GIVERESOURCE_INDEX].Max = unit.Variable[GIVERESOURCE_INDEX].Max;
					temp->Variable[GIVERESOURCE_INDEX].Enable = unit.Variable[GIVERESOURCE_INDEX].Enable;
				}
				//Wyrmgus end
			}
		//Wyrmgus start
		} else if (unit.settlement && unit.settlement->get_site_unit() == &unit) {
			unit.settlement->set_site_unit(nullptr);
		//Wyrmgus end
		}
	}
}

/**
**  Removes all orders from a unit.
**
**  @param unit  The unit that will have all its orders cleared
*/
void UnitClearOrders(CUnit &unit)
{
	for (size_t i = 0; i != unit.Orders.size(); ++i) {
		delete unit.Orders[i];
	}
	unit.Orders.clear();
	unit.Orders.push_back(COrder::NewActionStill());
}

/**
**  Update for new unit. Food and income ...
**
**  @param unit     New unit pointer.
**  @param upgrade  True unit was upgraded.
*/
void UpdateForNewUnit(const CUnit &unit, int upgrade)
{
	const stratagus::unit_type &type = *unit.Type;
	CPlayer &player = *unit.Player;

	// Handle unit supply and max resources.
	// Note an upgraded unit can't give more supply.
	if (!upgrade) {
		player.Supply += unit.Variable[SUPPLY_INDEX].Value;
		for (int i = 0; i < MaxCosts; ++i) {
			if (player.MaxResources[i] != -1 && type.Stats[player.Index].Storing[i]) {
				player.MaxResources[i] += type.Stats[player.Index].Storing[i];
			}
		}
	}

	// Update resources
	for (int u = 1; u < MaxCosts; ++u) {
		player.Incomes[u] = std::max(player.Incomes[u], type.Stats[player.Index].ImproveIncomes[u]);
	}
	
	if (type.Stats[player.Index].Variables[TRADECOST_INDEX].Enable) {
		player.TradeCost = std::min(player.TradeCost, type.Stats[player.Index].Variables[TRADECOST_INDEX].Value);
	}
	
	//Wyrmgus start
	if (player.LostTownHallTimer != 0 && type.BoolFlag[TOWNHALL_INDEX].value && CPlayer::GetThisPlayer()->HasContactWith(player)) {
		player.LostTownHallTimer = 0;
		player.set_revealed(false);
		for (int j = 0; j < NumPlayers; ++j) {
			if (player.Index != j && CPlayer::Players[j]->Type != PlayerNobody) {
				CPlayer::Players[j]->Notify(_("%s has rebuilt a town hall, and will no longer be revealed!"), player.Name.c_str());
			} else {
				CPlayer::Players[j]->Notify("%s", _("You have rebuilt a town hall, and will no longer be revealed!"));
			}
		}
	}
	//Wyrmgus end
}

/**
**  Find nearest point of unit.
**
**  @param unit  Pointer to unit.
**  @param pos   tile map position.
**  @param dpos  Out: nearest point tile map position to (tx,ty).
*/
void NearestOfUnit(const CUnit &unit, const Vec2i &pos, Vec2i *dpos)
{
	const int x = unit.tilePos.x;
	const int y = unit.tilePos.y;

	*dpos = pos;
	clamp<short int>(&dpos->x, x, x + unit.Type->get_tile_width() - 1);
	clamp<short int>(&dpos->y, y, y + unit.Type->get_tile_height() - 1);
}

/**
**  Copy the unit look in Seen variables. This should be called when
**  buildings go under fog of war for ThisPlayer.
**
**  @param unit  The unit to work on
*/
static void UnitFillSeenValues(CUnit &unit)
{
	// Seen values are undefined for visible units.
	unit.Seen.tilePos = unit.tilePos;
	unit.Seen.pixel_offset = unit.get_pixel_offset();
	unit.Seen.Frame = unit.Frame;
	unit.Seen.Type = unit.Type;
	unit.Seen.UnderConstruction = unit.UnderConstruction;

	unit.CurrentOrder()->FillSeenValues(unit);
}

// Wall unit positions
enum {
	W_NORTH = 0x10,
	W_WEST = 0x20,
	W_SOUTH = 0x40,
	W_EAST = 0x80
};

/**
**  Correct direction for placed wall.
**
**  @param unit    The wall unit.
*/
void CorrectWallDirections(CUnit &unit)
{
	Assert(unit.Type->BoolFlag[WALL_INDEX].value);
	Assert(unit.Type->NumDirections == 16);
	Assert(!unit.Type->Flip);

	if (!CMap::Map.Info.IsPointOnMap(unit.tilePos, unit.MapLayer)) {
		return;
	}
	const struct {
		Vec2i offset;
		const int dirFlag;
	} configs[] = {{Vec2i(0, -1), W_NORTH}, {Vec2i(1, 0), W_EAST},
		{Vec2i(0, 1), W_SOUTH}, {Vec2i(-1, 0), W_WEST}
	};
	int flags = 0;

	for (int i = 0; i != sizeof(configs) / sizeof(*configs); ++i) {
		const Vec2i pos = unit.tilePos + configs[i].offset;
		const int dirFlag = configs[i].dirFlag;

		if (CMap::Map.Info.IsPointOnMap(pos, unit.MapLayer) == false) {
			flags |= dirFlag;
		} else {
			const CUnitCache &unitCache = CMap::Map.Field(pos, unit.MapLayer->ID)->UnitCache;
			const CUnit *neighbor = unitCache.find(HasSamePlayerAndTypeAs(unit));

			if (neighbor != nullptr) {
				flags |= dirFlag;
			}
		}
	}
	unit.Direction = flags;
}

/**
** Correct the surrounding walls.
**
** @param unit The wall unit.
*/
void CorrectWallNeighBours(CUnit &unit)
{
	Assert(unit.Type->BoolFlag[WALL_INDEX].value);

	const Vec2i offset[] = {Vec2i(1, 0), Vec2i(-1, 0), Vec2i(0, 1), Vec2i(0, -1)};

	for (unsigned int i = 0; i < sizeof(offset) / sizeof(*offset); ++i) {
		const Vec2i pos = unit.tilePos + offset[i];

		if (CMap::Map.Info.IsPointOnMap(pos, unit.MapLayer) == false) {
			continue;
		}
		CUnitCache &unitCache = unit.MapLayer->Field(pos)->UnitCache;
		CUnit *neighbor = unitCache.find(HasSamePlayerAndTypeAs(unit));

		if (neighbor != nullptr) {
			CorrectWallDirections(*neighbor);
			UnitUpdateHeading(*neighbor);
		}
	}
}

/**
**  This function should get called when a unit goes under fog of war.
**
**  @param unit    The unit that goes under fog.
**  @param player  The player the unit goes out of fog for.
*/
void UnitGoesUnderFog(CUnit &unit, const CPlayer &player)
{
	if (unit.Type->BoolFlag[VISIBLEUNDERFOG_INDEX].value) {
		if (player.Type == PlayerPerson && !unit.Destroyed) {
			unit.RefsIncrease();
		}
		//
		// Icky yucky icky Seen.Destroyed trickery.
		// We track for each player if he's seen the unit as destroyed.
		// Remember, a unit is marked Destroyed when it's gone as in
		// completely gone, the corpses vanishes. In that case the unit
		// only survives since some players did NOT see the unit destroyed.
		// Keeping trackof that is hard, mostly due to complex shared vision
		// configurations.
		// A unit does NOT get a reference when it goes under fog if it's
		// Destroyed. Furthermore, it shouldn't lose a reference if it was
		// Seen destroyed. That only happened with complex shared vision, and
		// it's sort of the whole point of this tracking.
		//
		if (unit.Destroyed) {
			unit.Seen.Destroyed |= (1 << player.Index);
		}
		if (&player == CPlayer::GetThisPlayer()) {
			UnitFillSeenValues(unit);
		}
	}
}

/**
**  This function should get called when a unit goes out of fog of war.
**
**  @param unit    The unit that goes out of fog.
**  @param player  The player the unit goes out of fog for.
**
**  @note For units that are visible under fog (mostly buildings)
**  we use reference counts, from the players that know about
**  the building. When an building goes under fog it gets a refs
**  increase, and when it shows up it gets a decrease. It must
**  not get an decrease the first time it's seen, so we have to
**  keep track of what player saw what units, with SeenByPlayer.
*/
void UnitGoesOutOfFog(CUnit &unit, const CPlayer &player)
{
	if (!unit.Type->BoolFlag[VISIBLEUNDERFOG_INDEX].value) {
		return;
	}
	if (unit.Seen.ByPlayer & (1 << (player.Index))) {
		if ((player.Type == PlayerPerson) && (!(unit.Seen.Destroyed & (1 << player.Index)))) {
			unit.RefsDecrease();
		}
	} else {
		unit.Seen.ByPlayer |= (1 << (player.Index));
	}
}

/**
**  Recalculates a units visiblity count. This happens really often,
**  Like every time a unit moves. It's really fast though, since we
**  have per-tile counts.
**
**  @param unit  pointer to the unit to check if seen
*/
void UnitCountSeen(CUnit &unit)
{
	Assert(unit.Type);

	// FIXME: optimize, only work on certain players?
	// This is for instance good for updating shared vision...

	//  Store old values in oldv[p]. This store if the player could see the
	//  unit before this calc.
	int oldv[PlayerMax];
	for (int p = 0; p < PlayerMax; ++p) {
		if (CPlayer::Players[p]->Type != PlayerNobody) {
			oldv[p] = unit.IsVisible(*CPlayer::Players[p]);
		}
	}

	//  Calculate new VisCount values.
	const int height = unit.Type->get_tile_height();
	const int width = unit.Type->get_tile_width();

	for (int p = 0; p < PlayerMax; ++p) {
		if (CPlayer::Players[p]->Type != PlayerNobody) {
			int newv = 0;
			int y = height;
			unsigned int index = unit.Offset;
			do {
				CMapField *mf = unit.MapLayer->Field(index);
				int x = width;
				do {
					if (unit.Type->BoolFlag[PERMANENTCLOAK_INDEX].value && unit.Player != CPlayer::Players[p]) {
						if (mf->playerInfo.VisCloak[p]) {
							newv++;
						}
					//Wyrmgus start
					} else if (unit.Type->BoolFlag[ETHEREAL_INDEX].value && unit.Player != CPlayer::Players[p]) {
						if (mf->playerInfo.VisEthereal[p]) {
							newv++;
						}
					//Wyrmgus end
					} else {
						if (mf->playerInfo.IsVisible(*CPlayer::Players[p])) {
							newv++;
						}
					}
					++mf;
				} while (--x);
				index += unit.MapLayer->get_width();
			} while (--y);
			unit.VisCount[p] = newv;
		}
	}

	//
	// Now here comes the tricky part. We have to go in and out of fog
	// for players. Hopefully this works with shared vision just great.
	//
	for (int p = 0; p < PlayerMax; ++p) {
		if (CPlayer::Players[p]->Type != PlayerNobody) {
			int newv = unit.IsVisible(*CPlayer::Players[p]);
			if (!oldv[p] && newv) {
				// Might have revealed a destroyed unit which caused it to
				// be released
				if (!unit.Type) {
					break;
				}
				UnitGoesOutOfFog(unit, *CPlayer::Players[p]);
			}
			if (oldv[p] && !newv) {
				UnitGoesUnderFog(unit, *CPlayer::Players[p]);
			}
		}
	}
}

/**
**  Returns true, if the unit is visible. It check the Viscount of
**  the player and everyone who shares vision with him.
**
**  @note This understands shared vision, and should be used all around.
**
**  @param player  The player to check.
*/
bool CUnit::IsVisible(const CPlayer &player) const
{
	if (VisCount[player.Index]) {
		return true;
	}

	for (const int p : player.get_shared_vision()) {
		if (player.has_mutual_shared_vision_with(*CPlayer::Players[p])) {
			if (VisCount[p]) {
				return true;
			}
		}
	}

	for (const CPlayer *other_player : CPlayer::get_revealed_players()) {
		if (VisCount[other_player->Index]) {
			return true;
		}
	}

	return false;
}

/**
**  Returns true, if unit is shown on minimap.
**
**  @warning This is for ::ThisPlayer only.
**  @todo radar support
**
**  @return      True if visible, false otherwise.
*/
bool CUnit::IsVisibleOnMinimap() const
{
	//Wyrmgus start
	if (this->MapLayer != UI.CurrentMapLayer) {
		return false;
	}
	//Wyrmgus end

	// Invisible units.
	if (IsInvisibile(*CPlayer::GetThisPlayer())) {
		return false;
	}

	if (IsVisible(*CPlayer::GetThisPlayer()) || ReplayRevealMap || IsVisibleOnRadar(*CPlayer::GetThisPlayer())) {
		return IsAliveOnMap();
	}

	return this->Type->BoolFlag[VISIBLEUNDERFOG_INDEX].value && Seen.State != 3
		&& (Seen.ByPlayer & (1 << CPlayer::GetThisPlayer()->Index))
		&& !(Seen.Destroyed & (1 << CPlayer::GetThisPlayer()->Index))
		&& !Destroyed
		&& CMap::Map.Info.IsPointOnMap(this->tilePos, this->MapLayer)
		&& this->MapLayer->Field(this->tilePos)->playerInfo.IsTeamExplored(*CPlayer::GetThisPlayer());
}

/**
**  Returns true, if unit is visible in viewport.
**
**  @warning This is only true for ::ThisPlayer
**
**  @param vp  Viewport pointer.
**
**  @return    True if visible, false otherwise.
*/
bool CUnit::IsVisibleInViewport(const CViewport &vp) const
{
	// Check if the graphic is inside the viewport.

	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	int frame_width = this->Type->get_frame_width();
	int frame_height = this->Type->get_frame_height();
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && variation->FrameWidth && variation->FrameHeight) {
		frame_width = variation->FrameWidth;
		frame_height = variation->FrameHeight;
	}
	frame_width *= scale_factor;
	frame_height *= scale_factor;

	int x = tilePos.x * stratagus::defines::get()->get_scaled_tile_width() + this->get_scaled_pixel_offset().x() - (frame_width - Type->get_tile_width() * stratagus::defines::get()->get_scaled_tile_width()) / 2 + Type->OffsetX * scale_factor;
	int y = tilePos.y * stratagus::defines::get()->get_scaled_tile_height() + this->get_scaled_pixel_offset().y() - (frame_height - Type->get_tile_height() * stratagus::defines::get()->get_scaled_tile_height()) / 2 + Type->OffsetY * scale_factor;
	const PixelSize vpSize = vp.GetPixelSize();
	const PixelPos vpTopLeftMapPos = CMap::Map.tile_pos_to_scaled_map_pixel_pos_top_left(vp.MapPos) + vp.Offset;
	const PixelPos vpBottomRightMapPos = vpTopLeftMapPos + vpSize;

	//Wyrmgus start
//	if (x + Type->Width < vpTopLeftMapPos.x || x > vpBottomRightMapPos.x
//		|| y + Type->Height < vpTopLeftMapPos.y || y > vpBottomRightMapPos.y) {
	if (x + frame_width < vpTopLeftMapPos.x || x > vpBottomRightMapPos.x
		|| y + frame_height < vpTopLeftMapPos.y || y > vpBottomRightMapPos.y) {
	//Wyrmgus end
		return false;
	}

	if (!CPlayer::GetThisPlayer()) {
		//FIXME: ARI: Added here for early game setup state by
		// MakeAndPlaceUnit() from LoadMap(). ThisPlayer not yet set,
		// so don't show anything until first real map-draw.
		DebugPrint("FIXME: ThisPlayer not set yet?!\n");
		return false;
	}

	// Those are never ever visible.
	if (IsInvisibile(*CPlayer::GetThisPlayer())) {
		return false;
	}

	if (IsVisible(*CPlayer::GetThisPlayer()) || ReplayRevealMap) {
		return !Destroyed;
	} else {
		// Unit has to be 'discovered'
		// Destroyed units ARE visible under fog of war, if we haven't seen them like that.
		if (!Destroyed || !(Seen.Destroyed & (1 << CPlayer::GetThisPlayer()->Index))) {
			return (Type->BoolFlag[VISIBLEUNDERFOG_INDEX].value && (Seen.ByPlayer & (1 << CPlayer::GetThisPlayer()->Index)));
		} else {
			return false;
		}
	}
}

/**
**  Change the unit's owner
**
**  @param newplayer  New owning player.
*/
//Wyrmgus start
//void CUnit::ChangeOwner(CPlayer &newplayer)
void CUnit::ChangeOwner(CPlayer &newplayer, bool show_change)
//Wyrmgus end
{
	CPlayer *oldplayer = Player;

	// This shouldn't happen
	if (oldplayer == &newplayer) {
		DebugPrint("Change the unit owner to the same player???\n");
		return;
	}

	// Can't change owner for dead units
	if (this->IsAlive() == false) {
		return;
	}

	// Rescue all units in buildings/transporters.
	//Wyrmgus start
//	CUnit *uins = UnitInside;
//	for (int i = InsideCount; i; --i, uins = uins->NextContained) {
//		uins->ChangeOwner(newplayer);
//	}

	//only rescue units inside if the player is actually a rescuable player (to avoid, for example, unintended worker owner changes when a depot changes hands)
	if (oldplayer->Type == PlayerRescueActive || oldplayer->Type == PlayerRescuePassive) {
		CUnit *uins = UnitInside;
		for (int i = InsideCount; i; --i, uins = uins->NextContained) {
			uins->ChangeOwner(newplayer);
		}
	}
	//Wyrmgus end

	//  Must change food/gold and other.
	UnitLost(*this);

	//  Now the new side!

	if (Type->BoolFlag[BUILDING_INDEX].value) {
		//Wyrmgus start
//		if (!Type->BoolFlag[WALL_INDEX].value) {
		//Wyrmgus end
			newplayer.TotalBuildings++;
		//Wyrmgus start
//		}
		//Wyrmgus end
	} else {
		newplayer.TotalUnits++;
	}

	MapUnmarkUnitSight(*this);
	newplayer.AddUnit(*this);
	Stats = &Type->Stats[newplayer.Index];

	//  Must change food/gold and other.
	//Wyrmgus start
//	if (Type->GivesResource) {
	if (this->GivesResource) {
	//Wyrmgus end
		DebugPrint("Resource transfer not supported\n");
	}
	newplayer.Demand += Type->Stats[newplayer.Index].Variables[DEMAND_INDEX].Value;
	newplayer.Supply += this->Variable[SUPPLY_INDEX].Value;
	// Increase resource limit
	for (int i = 0; i < MaxCosts; ++i) {
		if (newplayer.MaxResources[i] != -1 && Type->Stats[newplayer.Index].Storing[i]) {
			newplayer.MaxResources[i] += Type->Stats[newplayer.Index].Storing[i];
		}
	}
	//Wyrmgus start
//	if (Type->BoolFlag[BUILDING_INDEX].value && !Type->BoolFlag[WALL_INDEX].value) {
	if (Type->BoolFlag[BUILDING_INDEX].value) {
	//Wyrmgus end
		newplayer.NumBuildings++;
	}
	//Wyrmgus start
	if (CurrentAction() == UnitAction::Built) {
		newplayer.NumBuildingsUnderConstruction++;
		newplayer.ChangeUnitTypeUnderConstructionCount(this->Type, 1);
	}
	//Wyrmgus end

	//apply upgrades of the new player, if the old one doesn't have that upgrade
	for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
		const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
		if (oldplayer->Allow.Upgrades[modifier_upgrade->ID] != 'R' && newplayer.Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)) { //if the old player doesn't have the modifier's upgrade (but the new one does), and the upgrade is applicable to the unit
			//Wyrmgus start
//			ApplyIndividualUpgradeModifier(*this, modifier);
			if ( // don't apply equipment-related upgrades if the unit has an item of that equipment type equipped
				(!modifier_upgrade->is_weapon() || EquippedItems[static_cast<int>(stratagus::item_slot::weapon)].size() == 0)
				&& (!modifier_upgrade->is_shield() || EquippedItems[static_cast<int>(stratagus::item_slot::shield)].size() == 0)
				&& (!modifier_upgrade->is_boots() || EquippedItems[static_cast<int>(stratagus::item_slot::boots)].size() == 0)
				&& (!modifier_upgrade->is_arrows() || EquippedItems[static_cast<int>(stratagus::item_slot::arrows)].size() == 0)
				&& !(newplayer.Race != -1 && modifier_upgrade == stratagus::civilization::get_all()[newplayer.Race]->get_upgrade())
				&& !(newplayer.Race != -1 && newplayer.Faction != -1 && modifier_upgrade->Ident == stratagus::faction::get_all()[newplayer.Faction]->FactionUpgrade)
			) {
				ApplyIndividualUpgradeModifier(*this, modifier);
			}
			//Wyrmgus end
		}
	}

	newplayer.IncreaseCountsForUnit(this);
	UpdateForNewUnit(*this, 1);
	
	UpdateUnitSightRange(*this);
	MapMarkUnitSight(*this);
	
	//not very elegant way to make sure the tile ownership is calculated correctly
	MapUnmarkUnitSight(*this);
	MapMarkUnitSight(*this);
	
	//Wyrmgus start
	if (newplayer.Index == CPlayer::GetThisPlayer()->Index && show_change) {
		this->Blink = 5;
		PlayGameSound(GameSounds.Rescue[newplayer.Race].Sound, MaxSampleVolume);
	}
	//Wyrmgus end
}

static bool IsMineAssignedBy(const CUnit &mine, const CUnit &worker)
{
	for (CUnit *it = mine.Resource.Workers; it; it = it->NextWorker) {
		if (it == &worker) {
			return true;
		}
	}
	return false;
}


void CUnit::AssignWorkerToMine(CUnit &mine)
{
	if (IsMineAssignedBy(mine, *this) == true) {
		return;
	}
	Assert(this->NextWorker == nullptr);

	CUnit *head = mine.Resource.Workers;
#if 0
	DebugPrint("%d: Worker [%d] is adding into %s [%d] on %d pos\n"
			   _C_ this->Player->Index _C_ this->Slot
			   _C_ mine.Type->Name.c_str()
			   _C_ mine.Slot
			   _C_ mine.Data.Resource.Assigned);
#endif
	this->RefsIncrease();
	this->NextWorker = head;
	mine.Resource.Workers = this;
	mine.Resource.Assigned++;
}

void CUnit::DeAssignWorkerFromMine(CUnit &mine)
{
	if (IsMineAssignedBy(mine, *this) == false) {
		return ;
	}
	CUnit *prev = nullptr, *worker = mine.Resource.Workers;
#if 0
	DebugPrint("%d: Worker [%d] is removing from %s [%d] left %d units assigned\n"
			   _C_ this->Player->Index _C_ this->Slot
			   _C_ mine.Type->Name.c_str()
			   _C_ mine.Slot
			   _C_ mine.CurrentOrder()->Data.Resource.Assigned);
#endif
	for (int i = 0; nullptr != worker; worker = worker->NextWorker, ++i) {
		if (worker == this) {
			CUnit *next = worker->NextWorker;
			worker->NextWorker = nullptr;
			if (prev) {
				prev->NextWorker = next;
			}
			if (worker == mine.Resource.Workers) {
				mine.Resource.Workers = next;
			}
			worker->RefsDecrease();
			break;
		}
		prev = worker;
		Assert(i <= mine.Resource.Assigned);
	}
	mine.Resource.Assigned--;
	Assert(mine.Resource.Assigned >= 0);
}


/**
**  Change the owner of all units of a player.
**
**  @param oldplayer    Old owning player.
**  @param newplayer    New owning player.
*/
static void ChangePlayerOwner(CPlayer &oldplayer, CPlayer &newplayer)
{
	if (&oldplayer == &newplayer) {
		return ;
	}

	for (int i = 0; i != oldplayer.GetUnitCount(); ++i) {
		CUnit &unit = oldplayer.GetUnit(i);

		unit.Blink = 5;
		unit.RescuedFrom = &oldplayer;
	}
	// ChangeOwner remove unit from the player: so change the array.
	while (oldplayer.GetUnitCount() != 0) {
		CUnit &unit = oldplayer.GetUnit(0);

		unit.ChangeOwner(newplayer);
	}
}

/**
**  Rescue units.
**
**  Look through all rescueable players, if they could be rescued.
*/
void RescueUnits()
{
	if (NoRescueCheck) {  // all possible units are rescued
		return;
	}
	NoRescueCheck = true;

	//  Look if player could be rescued.
	for (CPlayer *p : CPlayer::Players) {
		if (p->Type != PlayerRescuePassive && p->Type != PlayerRescueActive) {
			continue;
		}
		if (p->GetUnitCount() != 0) {
			NoRescueCheck = false;
			// NOTE: table is changed.
			std::vector<CUnit *> table;
			table.insert(table.begin(), p->UnitBegin(), p->UnitEnd());

			const size_t l = table.size();
			for (size_t j = 0; j != l; ++j) {
				CUnit &unit = *table[j];
				// Do not rescue removed units. Units inside something are
				// rescued by ChangeUnitOwner
				if (unit.Removed) {
					continue;
				}
				std::vector<CUnit *> around;

				SelectAroundUnit(unit, 1, around);
				//  Look if ally near the unit.
				for (size_t i = 0; i != around.size(); ++i) {
					//Wyrmgus start
//					if (around[i]->Type->CanAttack && unit.IsAllied(*around[i]) && around[i]->Player->Type != PlayerRescuePassive && around[i]->Player->Type != PlayerRescueActive) {
					if (around[i]->CanAttack() && unit.IsAllied(*around[i]) && around[i]->Player->Type != PlayerRescuePassive && around[i]->Player->Type != PlayerRescueActive) {
					//Wyrmgus end
						//  City center converts complete race
						//  NOTE: I use a trick here, centers could
						//        store gold. FIXME!!!
						//Wyrmgus start
//						if (unit.Type->CanStore[GoldCost]) {
						if (unit.Type->BoolFlag[TOWNHALL_INDEX].value) {
						//Wyrmgus end
							ChangePlayerOwner(*p, *around[i]->Player);
							break;
						}
						unit.RescuedFrom = unit.Player;
						//Wyrmgus start
//						unit.ChangeOwner(*around[i]->Player);
						unit.ChangeOwner(*around[i]->Player, true);
//						unit.Blink = 5;
//						PlayGameSound(GameSounds.Rescue[unit.Player->Race].Sound, MaxSampleVolume);
						//Wyrmgus end
						break;
					}
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------
--  Unit headings
----------------------------------------------------------------------------*/

/**
**  Fast arc tangent function.
**
**  @param val  atan argument
**
**  @return     atan(val)
*/
static int myatan(int val)
{
	static int init;
	static unsigned char atan_table[2608];

	if (val >= 2608) {
		return 63;
	}
	if (!init) {
		for (; init < 2608; ++init) {
			atan_table[init] =
				(unsigned char)(atan((double)init / 64) * (64 * 4 / 6.2831853));
		}
	}

	return atan_table[val];
}

/**
**  Convert direction to heading.
**
**  @param delta  Delta.
**
**  @return         Angle (0..255)
*/
int DirectionToHeading(const Vec2i &delta)
{
	//  Check which quadrant.
	if (delta.x > 0) {
		if (delta.y < 0) { // Quadrant 1?
			return myatan((delta.x * 64) / -delta.y);
		}
		// Quadrant 2?
		return myatan((delta.y * 64) / delta.x) + 64;
	}
	if (delta.y > 0) { // Quadrant 3?
		return myatan((delta.x * -64) / delta.y) + 64 * 2;
	}
	if (delta.x) { // Quadrant 4.
		return myatan((delta.y * -64) / -delta.x) + 64 * 3;
	}
	return 0;
}

/**
**  Convert direction to heading.
**
**  @param delta  Delta.
**
**  @return         Angle (0..255)
*/
int DirectionToHeading(const PixelDiff &delta)
{
	// code is identic for Vec2i and PixelDiff
	Vec2i delta2(delta.x, delta.y);
	return DirectionToHeading(delta2);
}

/**
**  Update sprite frame for new heading.
*/
void UnitUpdateHeading(CUnit &unit)
{
	//Wyrmgus start
	//fix direction if it does not correspond to one of the defined directions
	int num_dir = std::max<int>(8, unit.Type->NumDirections);
	if (unit.Direction % (256 / num_dir) != 0) {
		unit.Direction = unit.Direction - (unit.Direction % (256 / num_dir));
	}
	//Wyrmgus end
	
	int dir;
	int nextdir;
	bool neg;

	if (unit.Frame < 0) {
		unit.Frame = -unit.Frame - 1;
		neg = true;
	} else {
		neg = false;
	}
	unit.Frame /= unit.Type->NumDirections / 2 + 1;
	unit.Frame *= unit.Type->NumDirections / 2 + 1;
	// Remove heading, keep animation frame

	nextdir = 256 / unit.Type->NumDirections;
	dir = ((unit.Direction + nextdir / 2) & 0xFF) / nextdir;
	if (dir <= LookingS / nextdir) { // north->east->south
		unit.Frame += dir;
	} else {
		unit.Frame += 256 / nextdir - dir;
		unit.Frame = -unit.Frame - 1;
	}
	if (neg && !unit.Frame && unit.Type->BoolFlag[BUILDING_INDEX].value) {
		unit.Frame = -1;
	}
}

/**
**  Change unit heading/frame from delta direction x, y.
**
**  @param unit  Unit for new direction looking.
**  @param delta  map tile delta direction.
*/
void UnitHeadingFromDeltaXY(CUnit &unit, const Vec2i &delta)
{
	//Wyrmgus start
//	unit.Direction = DirectionToHeading(delta);
	int num_dir = std::max<int>(8, unit.Type->NumDirections);
	int heading = DirectionToHeading(delta) + ((256 / num_dir) / 2);
	if (heading % (256 / num_dir) != 0) {
		heading = heading - (heading % (256 / num_dir));
	}
	unit.Direction = heading;
	//Wyrmgus end
	UnitUpdateHeading(unit);
}

/*----------------------------------------------------------------------------
  -- Drop out units
  ----------------------------------------------------------------------------*/

/**
**  Place a unit on the map to the side of a unit.
**
**  @param unit       Unit to drop out.
**  @param heading    Direction in which the unit should appear.
**  @param container  Unit "containing" unit to drop (may be different of unit.Container).
*/
void DropOutOnSide(CUnit &unit, int heading, const CUnit *container)
{
	Vec2i pos;
	int addx = 0;
	int addy = 0;
	//Wyrmgus start
	int z;
	//Wyrmgus end

	if (container) {
		pos = container->tilePos;
		pos -= Vec2i(unit.Type->get_tile_size() - QSize(1, 1));
		addx = container->Type->get_tile_width() + unit.Type->get_tile_width() - 1;
		addy = container->Type->get_tile_height() + unit.Type->get_tile_height() - 1;
		z = container->MapLayer->ID;

		if (heading < LookingNE || heading > LookingNW) {
			pos.x += addx - 1;
			--pos.y;
			goto startn;
		} else if (heading < LookingSE) {
			pos.x += addx;
			pos.y += addy - 1;
			goto starte;
		} else if (heading < LookingSW) {
			pos.y += addy;
			goto starts;
		} else {
			--pos.x;
			goto startw;
		}
	} else {
		pos = unit.tilePos;
		z = unit.MapLayer->ID;

		if (heading < LookingNE || heading > LookingNW) {
			goto starts;
		} else if (heading < LookingSE) {
			goto startw;
		} else if (heading < LookingSW) {
			goto startn;
		} else {
			goto starte;
		}
	}
	// FIXME: don't search outside of the map
	for (;;) {
startw:
		for (int i = addy; i--; ++pos.y) {
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				goto found;
			}
		}
		++addx;
starts:
		for (int i = addx; i--; ++pos.x) {
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				goto found;
			}
		}
		++addy;
starte:
		for (int i = addy; i--; --pos.y) {
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				goto found;
			}
		}
		++addx;
startn:
		for (int i = addx; i--; --pos.x) {
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				goto found;
			}
		}
		++addy;
	}

found:
	//Wyrmgus start
//	unit.Place(pos);
	unit.Place(pos, z);
	//Wyrmgus end
}

/**
**  Place a unit on the map nearest to goalPos.
**
**  @param unit  Unit to drop out.
**  @param goalPos Goal map tile position.
**  @param addx  Tile width of unit it's dropping out of.
**  @param addy  Tile height of unit it's dropping out of.
*/
void DropOutNearest(CUnit &unit, const Vec2i &goalPos, const CUnit *container)
{
	Vec2i pos;
	Vec2i bestPos;
	int bestd = 99999;
	int addx = 0;
	int addy = 0;
	//Wyrmgus start
	int z;
	//Wyrmgus end

	if (container) {
		Assert(unit.Removed);
		pos = container->tilePos;
		pos -= Vec2i(unit.Type->get_tile_size() - QSize(1, 1));
		addx = container->Type->get_tile_width() + unit.Type->get_tile_width() - 1;
		addy = container->Type->get_tile_height() + unit.Type->get_tile_height() - 1;
		--pos.x;
		z = container->MapLayer->ID;
	} else {
		pos = unit.tilePos;
		z = unit.MapLayer->ID;
	}
	// FIXME: if we reach the map borders we can go fast up, left, ...

	for (;;) {
		for (int i = addy; i--; ++pos.y) { // go down
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addx;
		for (int i = addx; i--; ++pos.x) { // go right
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addy;
		for (int i = addy; i--; --pos.y) { // go up
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addx;
		for (int i = addx; i--; --pos.x) { // go left
			//Wyrmgus start
//			if (UnitCanBeAt(unit, pos)) {
			if (UnitCanBeAt(unit, pos, z)) {
			//Wyrmgus end
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		if (bestd != 99999) {
			//Wyrmgus start
//			unit.Place(bestPos);
			unit.Place(bestPos, z);
			//Wyrmgus end
			return;
		}
		++addy;
	}
}

/**
**  Drop out all units inside unit.
**
**  @param source  All units inside source are dropped out.
*/
void DropOutAll(const CUnit &source)
{
	CUnit *unit = source.UnitInside;

	for (int i = source.InsideCount; i; --i, unit = unit->NextContained) {
		DropOutOnSide(*unit, LookingW, &source);
	}
	
	//Wyrmgus start
	if (unit->Type->BoolFlag[ITEM_INDEX].value && !unit->Unique) { //save the initial cycle items were placed in the ground to destroy them if they have been there for too long
		int ttl_cycles = (5 * 60 * CYCLES_PER_SECOND);
		if (unit->Prefix != nullptr || unit->Suffix != nullptr || unit->Spell != nullptr || unit->Work != nullptr || unit->Elixir != nullptr) {
			ttl_cycles *= 4;
		}
		unit->TTL = GameCycle + ttl_cycles;
	}
	//Wyrmgus end
}

/*----------------------------------------------------------------------------
  -- Select units
  ----------------------------------------------------------------------------*/

/**
**  Unit on map screen.
**
**  Select units on screen. (x, y are in pixels relative to map 0,0).
**  Not GAMEPLAY safe, uses ReplayRevealMap
**
**  More units on same position.
**    Cycle through units.
**    First take highest unit.
**
**  @param x      X pixel position.
**  @param y      Y pixel position.
**
**  @return       An unit on x, y position.
*/
CUnit *UnitOnScreen(int x, int y)
{
	CUnit *candidate = nullptr;
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		if (unit.MapLayer != UI.CurrentMapLayer) {
			continue;
		}
		if (!ReplayRevealMap && !unit.IsVisibleAsGoal(*CPlayer::GetThisPlayer())) {
			continue;
		}
		const stratagus::unit_type &type = *unit.Type;
		if (!type.Sprite) {
			continue;
		}

		//
		// Check if mouse is over the unit.
		//
		PixelPos unitSpritePos = unit.get_scaled_map_pixel_pos_center();
		const int scale_factor = stratagus::defines::get()->get_scale_factor();
		//Wyrmgus start
//		unitSpritePos.x = unitSpritePos.x - type.BoxWidth / 2 -
//						  (type.Width - type.Sprite->Width) / 2 + type.BoxOffsetX;
//		unitSpritePos.y = unitSpritePos.y - type.BoxHeight / 2 -
//						  (type.Height - type.Sprite->Height) / 2 + type.BoxOffsetY;
		const stratagus::unit_type_variation *variation = unit.GetVariation();
		if (variation && variation->FrameWidth && variation->FrameHeight && !variation->get_image_file().empty()) {
			unitSpritePos.x = unitSpritePos.x - type.get_box_width() * scale_factor / 2 -
							  (variation->FrameWidth * scale_factor - variation->Sprite->Width) / 2 + type.BoxOffsetX * scale_factor;
			unitSpritePos.y = unitSpritePos.y - type.get_box_height() * scale_factor / 2 -
							  (variation->FrameHeight * scale_factor - variation->Sprite->Height) / 2 + type.BoxOffsetY * scale_factor;
		} else {
			unitSpritePos.x = unitSpritePos.x - type.get_box_width() * scale_factor / 2 -
							  (type.get_frame_width() * scale_factor - type.Sprite->Width) / 2 + type.BoxOffsetX * scale_factor;
			unitSpritePos.y = unitSpritePos.y - type.get_box_height() * scale_factor / 2 -
							  (type.get_frame_height() * scale_factor - type.Sprite->Height) / 2 + type.BoxOffsetY * scale_factor;
		}
		//Wyrmgus end
		if (x >= unitSpritePos.x && x < (unitSpritePos.x + type.get_box_width() * scale_factor)
			&& y >= unitSpritePos.y  && y < (unitSpritePos.y + type.get_box_height() * scale_factor)) {
			// Check if there are other units on this place
			candidate = &unit;
			//Wyrmgus start
			std::vector<CUnit *> table;
			Select(candidate->tilePos, candidate->tilePos, table, candidate->MapLayer->ID, HasNotSamePlayerAs(*CPlayer::Players[PlayerNumNeutral]));
//			if (IsOnlySelected(*candidate) || candidate->Type->BoolFlag[ISNOTSELECTABLE_INDEX].value) {
			if (IsOnlySelected(*candidate) || candidate->Type->BoolFlag[ISNOTSELECTABLE_INDEX].value || (candidate->Player->Type == PlayerNeutral && table.size()) || !candidate->IsAlive()) { // don't select a neutral unit if there's a player-owned unit there as well; don't selected a dead unit
			//Wyrmgus end
				continue;
			} else {
				break;
			}
		} else {
			continue;
		}
	}
	return candidate;
}

PixelPos CUnit::get_map_pixel_pos_top_left() const
{
	const PixelPos pos(tilePos.x * stratagus::defines::get()->get_tile_width() + this->get_pixel_offset().x(), tilePos.y * stratagus::defines::get()->get_tile_height() + this->get_pixel_offset().y());
	return pos;
}

PixelPos CUnit::get_scaled_map_pixel_pos_top_left() const
{
	return this->get_map_pixel_pos_top_left() * stratagus::defines::get()->get_scale_factor();
}

PixelPos CUnit::get_map_pixel_pos_center() const
{
	return this->get_map_pixel_pos_top_left() + this->get_half_tile_pixel_size();
}

PixelPos CUnit::get_scaled_map_pixel_pos_center() const
{
	return this->get_scaled_map_pixel_pos_top_left() + this->get_scaled_half_tile_pixel_size();
}

//Wyrmgus start
Vec2i CUnit::GetTileSize() const
{
	return this->Type->get_tile_size();
}

Vec2i CUnit::GetHalfTileSize() const
{
	return this->GetTileSize() / 2;
}

PixelSize CUnit::get_tile_pixel_size() const
{
	return PixelSize(this->GetTileSize()) * stratagus::defines::get()->get_tile_size();
}

PixelSize CUnit::get_scaled_tile_pixel_size() const
{
	return this->get_tile_pixel_size() * stratagus::defines::get()->get_scale_factor();
}

PixelSize CUnit::get_half_tile_pixel_size() const
{
	return this->get_tile_pixel_size() / 2;
}

PixelSize CUnit::get_scaled_half_tile_pixel_size() const
{
	return this->get_half_tile_pixel_size() * stratagus::defines::get()->get_scale_factor();
}

QPoint CUnit::get_center_tile_pos() const
{
	const CUnit *first_container = this->GetFirstContainer();
	return first_container->tilePos + first_container->Type->get_tile_center_pos_offset();
}

const CMapField *CUnit::get_center_tile() const
{
	return this->MapLayer->Field(this->get_center_tile_pos());
}

QPoint CUnit::get_scaled_pixel_offset() const
{
	return this->get_pixel_offset() * stratagus::defines::get()->get_scale_factor();
}

void CUnit::SetIndividualUpgrade(const CUpgrade *upgrade, int quantity)
{
	if (!upgrade) {
		return;
	}
	
	if (quantity <= 0) {
		if (this->IndividualUpgrades.find(upgrade->ID) != this->IndividualUpgrades.end()) {
			this->IndividualUpgrades.erase(upgrade->ID);
		}
	} else {
		this->IndividualUpgrades[upgrade->ID] = quantity;
	}
}

int CUnit::GetIndividualUpgrade(const CUpgrade *upgrade) const
{
	if (upgrade && this->IndividualUpgrades.find(upgrade->ID) != this->IndividualUpgrades.end()) {
		return this->IndividualUpgrades.find(upgrade->ID)->second;
	} else {
		return 0;
	}
}

int CUnit::GetAvailableLevelUpUpgrades(bool only_units) const
{
	int value = 0;
	int upgrade_value = 0;
	
	if (((int) AiHelpers.ExperienceUpgrades.size()) > Type->Slot) {
		for (size_t i = 0; i != AiHelpers.ExperienceUpgrades[Type->Slot].size(); ++i) {
			if (this->Character == nullptr || std::find(this->Character->ForbiddenUpgrades.begin(), this->Character->ForbiddenUpgrades.end(), AiHelpers.ExperienceUpgrades[Type->Slot][i]) == this->Character->ForbiddenUpgrades.end()) {
				int local_upgrade_value = 1;
				
				if (!only_units) {
					local_upgrade_value += AiHelpers.ExperienceUpgrades[Type->Slot][i]->GetAvailableLevelUpUpgrades();
				}
				
				if (local_upgrade_value > upgrade_value) {
					upgrade_value = local_upgrade_value;
				}
			}
		}
	}
	
	value += upgrade_value;
	
	if (!only_units && ((int) AiHelpers.LearnableAbilities.size()) > Type->Slot) {
		for (size_t i = 0; i != AiHelpers.LearnableAbilities[Type->Slot].size(); ++i) {
			value += AiHelpers.LearnableAbilities[Type->Slot][i]->MaxLimit - this->GetIndividualUpgrade(AiHelpers.LearnableAbilities[Type->Slot][i]);
		}
	}
	
	return value;
}

int CUnit::GetModifiedVariable(int index, int variable_type) const
{
	int value = 0;
	if (variable_type == VariableValue) {
		value = this->get_variable_value(index);
	} else if (variable_type == VariableMax) {
		value = this->get_variable_max(index);
	} else if (variable_type == VariableIncrease) {
		value = this->get_variable_increase(index);
	}
	
	if (index == ATTACKRANGE_INDEX) {
		if (this->Container && this->Container->Variable[GARRISONEDRANGEBONUS_INDEX].Enable) {
			value += this->Container->Variable[GARRISONEDRANGEBONUS_INDEX].Value; //treat the container's attack range as a bonus to the unit's attack range
		}
		value = std::min<int>(this->CurrentSightRange, value); // if the unit's current sight range is smaller than its attack range, use it instead
	} else if (index == SPEED_INDEX) {
		if (this->MapLayer && this->Type->UnitType != UnitTypeType::Fly && this->Type->UnitType != UnitTypeType::FlyLow && this->Type->UnitType != UnitTypeType::Space) {
			value += DefaultTileMovementCost - this->MapLayer->Field(this->Offset)->getCost();
		}
	}
	
	return value;
}

int CUnit::GetReactionRange() const
{
	int reaction_range = this->CurrentSightRange;

	if (this->Player->Type != PlayerPerson) {
		reaction_range += 2;
	}
	
	return reaction_range;
}

int CUnit::get_item_slot_quantity(const stratagus::item_slot item_slot) const
{
	if (!this->HasInventory()) {
		return 0;
	}
	
	if ( //if the item are arrows and the weapon of this unit's type is not a bow, return false
		item_slot == stratagus::item_slot::arrows
		&& Type->WeaponClasses[0] != stratagus::item_class::bow
	) {
		return 0;
	}
	
	if (item_slot == stratagus::item_slot::ring) {
		return 2;
	}
	
	return 1;
}

stratagus::item_class CUnit::GetCurrentWeaponClass() const
{
	if (HasInventory() && EquippedItems[static_cast<int>(stratagus::item_slot::weapon)].size() > 0) {
		return EquippedItems[static_cast<int>(stratagus::item_slot::weapon)][0]->Type->get_item_class();
	}
	
	return Type->WeaponClasses[0];
}

int CUnit::GetItemVariableChange(const CUnit *item, int variable_index, bool increase) const
{
	if (item->Type->get_item_class() == stratagus::item_class::none) {
		return 0;
	}
	
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item->Type->get_item_class());
	if (item->Work == nullptr && item->Elixir == nullptr && (item_slot == stratagus::item_slot::none || this->get_item_slot_quantity(item_slot) == 0 || !this->can_equip_item_class(item->Type->get_item_class()))) {
		return 0;
	}
	
	int value = 0;
	if (item->Work != nullptr) {
		if (this->GetIndividualUpgrade(item->Work) == 0) {
			for (const auto &modifier : item->Work->get_modifiers()) {
				if (!increase) {
					value += modifier->Modifier.Variables[variable_index].Value;
				} else {
					value += modifier->Modifier.Variables[variable_index].Increase;
				}
			}
		}
	} else if (item->Elixir != nullptr) {
		if (this->GetIndividualUpgrade(item->Elixir) == 0) {
			for (const auto &modifier : item->Elixir->get_modifiers()) {
				if (!increase) {
					value += modifier->Modifier.Variables[variable_index].Value;
				} else {
					value += modifier->Modifier.Variables[variable_index].Increase;
				}
			}
		}
	} else {
		if (!increase) {
			value = item->Variable[variable_index].Value;
		} else {
			value = item->Variable[variable_index].Increase;
		}
		
		if (!item->Identified) { //if the item is unidentified, don't show the effects of its affixes
			for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
				if (
					(item->Prefix != nullptr && modifier->UpgradeId == item->Prefix->ID)
					|| (item->Suffix != nullptr && modifier->UpgradeId == item->Suffix->ID)
				) {
					if (!increase) {
						value -= modifier->Modifier.Variables[variable_index].Value;
					} else {
						value -= modifier->Modifier.Variables[variable_index].Increase;
					}
				}
			}
		}
		
		if (item->Unique && item->Unique->Set) {
			if (this->EquippingItemCompletesSet(item)) {
				for (const auto &modifier : item->Unique->Set->get_modifiers()) {
					if (!increase) {
						value += modifier->Modifier.Variables[variable_index].Value;
					} else {
						value += modifier->Modifier.Variables[variable_index].Increase;
					}
				}
			}
		}
		
		if (EquippedItems[static_cast<int>(item_slot)].size() == this->get_item_slot_quantity(item_slot)) {
			int item_slot_used = EquippedItems[static_cast<int>(item_slot)].size() - 1;
			for (size_t i = 0; i < EquippedItems[static_cast<int>(item_slot)].size(); ++i) {
				if (EquippedItems[static_cast<int>(item_slot)][i] == item) {
					item_slot_used = i;
				}
			}

			const CUnit *equipped_item = this->EquippedItems[static_cast<int>(item_slot)][item_slot_used];
			if (!increase) {
				value -= equipped_item->Variable[variable_index].Value;
			} else {
				value -= equipped_item->Variable[variable_index].Increase;
			}
			if (equipped_item != item && equipped_item->Unique && equipped_item->Unique->Set) {
				if (this->DeequippingItemBreaksSet(equipped_item)) {
					for (const auto &modifier : equipped_item->Unique->Set->get_modifiers()) {
						if (!increase) {
							value -= modifier->Modifier.Variables[variable_index].Value;
						} else {
							value -= modifier->Modifier.Variables[variable_index].Increase;
						}
					}
				}
			}
		} else if (EquippedItems[static_cast<int>(item_slot)].size() == 0 && (item_slot == stratagus::item_slot::weapon || item_slot == stratagus::item_slot::shield || item_slot == stratagus::item_slot::boots || item_slot == stratagus::item_slot::arrows)) {
			for (const stratagus::upgrade_modifier *modifier : stratagus::upgrade_modifier::UpgradeModifiers) {
				const CUpgrade *modifier_upgrade = CUpgrade::get_all()[modifier->UpgradeId];
				if (
					(
						(
							(modifier_upgrade->is_weapon() && item_slot == stratagus::item_slot::weapon)
							|| (modifier_upgrade->is_shield() && item_slot == stratagus::item_slot::shield)
							|| (modifier_upgrade->is_boots() && item_slot == stratagus::item_slot::boots)
							|| (modifier_upgrade->is_arrows() && item_slot == stratagus::item_slot::arrows)
						)
						&& Player->Allow.Upgrades[modifier_upgrade->ID] == 'R' && modifier->applies_to(this->Type)
					)
					|| (item_slot == stratagus::item_slot::weapon && modifier_upgrade->is_ability() && this->GetIndividualUpgrade(modifier_upgrade) && modifier_upgrade->WeaponClasses.size() > 0 && modifier_upgrade->WeaponClasses.contains(this->GetCurrentWeaponClass()) && !modifier_upgrade->WeaponClasses.contains(item->Type->get_item_class()))
				) {
					if (this->GetIndividualUpgrade(modifier_upgrade)) {
						for (int i = 0; i < this->GetIndividualUpgrade(modifier_upgrade); ++i) {
							if (!increase) {
								value -= modifier->Modifier.Variables[variable_index].Value;
							} else {
								value -= modifier->Modifier.Variables[variable_index].Increase;
							}
						}
					} else {
						if (!increase) {
							value -= modifier->Modifier.Variables[variable_index].Value;
						} else {
							value -= modifier->Modifier.Variables[variable_index].Increase;
						}
					}
				} else if (
					modifier_upgrade->is_ability() && this->GetIndividualUpgrade(modifier_upgrade) && modifier_upgrade->WeaponClasses.size() > 0 && !modifier_upgrade->WeaponClasses.contains(this->GetCurrentWeaponClass()) && modifier_upgrade->WeaponClasses.contains(item->Type->get_item_class())
				) {
					if (this->GetIndividualUpgrade(modifier_upgrade)) {
						for (int i = 0; i < this->GetIndividualUpgrade(modifier_upgrade); ++i) {
							if (!increase) {
								value += modifier->Modifier.Variables[variable_index].Value;
							} else {
								value += modifier->Modifier.Variables[variable_index].Increase;
							}
						}
					} else {
						if (!increase) {
							value += modifier->Modifier.Variables[variable_index].Value;
						} else {
							value += modifier->Modifier.Variables[variable_index].Increase;
						}
					}
				}
			}
		}
	}
	
	return value;
}

int CUnit::GetDisplayPlayer() const
{
	if (this->Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && this->Player != CPlayer::GetThisPlayer()) {
		return PlayerNumNeutral;
	} else {
		return this->RescuedFrom ? this->RescuedFrom->Index : this->Player->Index;
	}
}

int CUnit::GetPrice() const
{
	int cost = this->Type->Stats[this->Player->Index].GetPrice();
	
	if (this->Prefix != nullptr) {
		cost += this->Prefix->MagicLevel * 1000;
	}
	if (this->Suffix != nullptr) {
		cost += this->Suffix->MagicLevel * 1000;
	}
	if (this->Spell != nullptr) {
		cost += 1000;
	}
	if (this->Work != nullptr) {
		if (this->Type->get_item_class() == stratagus::item_class::book) {
			cost += 5000;
		} else {
			cost += 1000;
		}
	}
	if (this->Elixir != nullptr) {
		cost += this->Elixir->MagicLevel * 1000;
	}
	if (this->Character) {
		cost += (this->Variable[LEVEL_INDEX].Value - this->Type->Stats[this->Player->Index].Variables[LEVEL_INDEX].Value) * 250;
	}
	
	return cost;
}

int CUnit::GetUnitStock(const stratagus::unit_type *unit_type) const
{
	if (unit_type == nullptr) {
		return 0;
	}

	auto find_iterator = this->UnitStock.find(unit_type->Slot);
	if (find_iterator != this->UnitStock.end()) {
		return find_iterator->second;
	} else {
		return 0;
	}
}

void CUnit::SetUnitStock(const stratagus::unit_type *unit_type, const int quantity)
{
	if (!unit_type) {
		return;
	}
	
	if (quantity <= 0) {
		if (this->UnitStock.contains(unit_type->Slot)) {
			this->UnitStock.erase(unit_type->Slot);
		}
	} else {
		this->UnitStock[unit_type->Slot] = quantity;
	}
}

void CUnit::ChangeUnitStock(const stratagus::unit_type *unit_type, const int quantity)
{
	this->SetUnitStock(unit_type, this->GetUnitStock(unit_type) + quantity);
}

int CUnit::GetUnitStockReplenishmentTimer(const stratagus::unit_type *unit_type) const
{
	if (this->UnitStockReplenishmentTimers.find(unit_type) != this->UnitStockReplenishmentTimers.end()) {
		return this->UnitStockReplenishmentTimers.find(unit_type)->second;
	} else {
		return 0;
	}
}

void CUnit::SetUnitStockReplenishmentTimer(const stratagus::unit_type *unit_type, int quantity)
{
	if (!unit_type) {
		return;
	}
	
	if (quantity <= 0) {
		if (this->UnitStockReplenishmentTimers.find(unit_type) != this->UnitStockReplenishmentTimers.end()) {
			this->UnitStockReplenishmentTimers.erase(unit_type);
		}
	} else {
		this->UnitStockReplenishmentTimers[unit_type] = quantity;
	}
}

void CUnit::ChangeUnitStockReplenishmentTimer(const stratagus::unit_type *unit_type, int quantity)
{
	this->SetUnitStockReplenishmentTimer(unit_type, this->GetUnitStockReplenishmentTimer(unit_type) + quantity);
}

int CUnit::GetResourceStep(const int resource) const
{
	if (!this->Type->ResInfo[resource]) {
		throw std::runtime_error("Tried to get the resource step for resource \"" + std::to_string(resource) + "\" for a unit of type \"" + this->Type->get_identifier() + "\", which doesn't support gathering that resource.");
	}

	int resource_step = this->Type->ResInfo[resource]->ResourceStep;
	
	resource_step += this->Variable[GATHERINGBONUS_INDEX].Value;
	
	if (resource == CopperCost) {
		resource_step += this->Variable[COPPERGATHERINGBONUS_INDEX].Value;
	} else if (resource == SilverCost) {
		resource_step += this->Variable[SILVERGATHERINGBONUS_INDEX].Value;
	} else if (resource == GoldCost) {
		resource_step += this->Variable[GOLDGATHERINGBONUS_INDEX].Value;
	} else if (resource == IronCost) {
		resource_step += this->Variable[IRONGATHERINGBONUS_INDEX].Value;
	} else if (resource == MithrilCost) {
		resource_step += this->Variable[MITHRILGATHERINGBONUS_INDEX].Value;
	} else if (resource == WoodCost) {
		resource_step += this->Variable[LUMBERGATHERINGBONUS_INDEX].Value;
	} else if (resource == StoneCost || resource == LimestoneCost) {
		resource_step += this->Variable[STONEGATHERINGBONUS_INDEX].Value;
	} else if (resource == CoalCost) {
		resource_step += this->Variable[COALGATHERINGBONUS_INDEX].Value;
	} else if (resource == JewelryCost) {
		resource_step += this->Variable[JEWELRYGATHERINGBONUS_INDEX].Value;
	} else if (resource == FurnitureCost) {
		resource_step += this->Variable[FURNITUREGATHERINGBONUS_INDEX].Value;
	} else if (resource == LeatherCost) {
		resource_step += this->Variable[LEATHERGATHERINGBONUS_INDEX].Value;
	} else if (resource == DiamondsCost || resource == EmeraldsCost) {
		resource_step += this->Variable[GEMSGATHERINGBONUS_INDEX].Value;
	}
	
	return resource_step;
}

int CUnit::GetTotalInsideCount(const CPlayer *player, const bool ignore_items, const bool ignore_saved_cargo, const stratagus::unit_type *type) const
{
	if (!this->UnitInside) {
		return 0;
	}
	
	if (this->Type->BoolFlag[SAVECARGO_INDEX].value && ignore_saved_cargo) {
		return 0;
	}
	
	int inside_count = 0;
	
	CUnit *inside_unit = this->UnitInside;
	for (int j = 0; j < this->InsideCount; ++j, inside_unit = inside_unit->NextContained) {
		if ( //only count units of the faction, ignore items
			(!player || inside_unit->Player == player)
			&& (!ignore_items || !inside_unit->Type->BoolFlag[ITEM_INDEX].value)
			&& (!type || inside_unit->Type == type)
		) {
			inside_count++;
		}
		inside_count += inside_unit->GetTotalInsideCount(player, ignore_items, ignore_saved_cargo);
	}

	return inside_count;
}

bool CUnit::CanAttack(bool count_inside) const
{
	if (this->Type->CanTransport() && this->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value && !this->Type->BoolFlag[CANATTACK_INDEX].value) { //transporters without an attack can only attack through a unit within them
		if (count_inside && this->BoardCount > 0) {
			CUnit *boarded_unit = this->UnitInside;
			for (int i = 0; i < this->InsideCount; ++i, boarded_unit = boarded_unit->NextContained) {
				if (boarded_unit->GetModifiedVariable(ATTACKRANGE_INDEX) > 1 && boarded_unit->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value) {
					return true;
				}
			}
		}
		return false;
	}
	
	if (this->Container && (!this->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value || !this->Container->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value)) {
		return false;
	}
	
	return this->Type->BoolFlag[CANATTACK_INDEX].value;
}

bool CUnit::IsInCombat() const
{
	// Select all units around the unit
	std::vector<CUnit *> table;
	SelectAroundUnit(*this, this->GetReactionRange(), table, IsEnemyWith(*this->Player));

	for (size_t i = 0; i < table.size(); ++i) {
		const CUnit &target = *table[i];

		if (target.IsVisibleAsGoal(*this->Player) && (CanTarget(*this->Type, *target.Type) || CanTarget(*target.Type, *this->Type))) {
			return true;
		}
	}
		
	return false;
}

bool CUnit::CanHarvest(const CUnit *dest, bool only_harvestable) const
{
	if (!dest) {
		return false;
	}
	
	if (!dest->GivesResource) {
		return false;
	}
	
	if (!this->Type->ResInfo[dest->GivesResource]) {
		return false;
	}
	
	if (!dest->Type->BoolFlag[CANHARVEST_INDEX].value && only_harvestable) {
		return false;
	}
	
	if (!this->Type->BoolFlag[HARVESTER_INDEX].value) {
		return false;
	}
	
	if (dest->GivesResource == TradeCost) {
		if (dest->Player == this->Player) { //can only trade with markets owned by other players
			return false;
		}
		
		if (this->Type->UnitType != UnitTypeType::Naval && dest->Type->BoolFlag[SHOREBUILDING_INDEX].value) { //only ships can trade with docks
			return false;
		}
		if (this->Type->UnitType == UnitTypeType::Naval && !dest->Type->BoolFlag[SHOREBUILDING_INDEX].value && dest->Type->UnitType != UnitTypeType::Naval) { //ships cannot trade with land markets
			return false;
		}
	} else {
		if (dest->Player != this->Player && !(dest->Player->IsAllied(*this->Player) && this->Player->IsAllied(*dest->Player)) && dest->Player->Index != PlayerNumNeutral) {
			return false;
		}
	}
	
	if (this->BoardCount) { //cannot harvest if carrying units
		return false;
	}
	
	return true;
}

bool CUnit::CanReturnGoodsTo(const CUnit *dest, int resource) const
{
	if (!dest) {
		return false;
	}
	
	if (!resource) {
		resource = this->CurrentResource;
	}
	
	if (!resource) {
		return false;
	}
	
	if (!dest->Type->CanStore[this->CurrentResource]) {
		return false;
	}
	
	if (resource == TradeCost) {
		if (dest->Player != this->Player) { //can only return trade to markets owned by the same player
			return false;
		}
		
		if (this->Type->UnitType != UnitTypeType::Naval && dest->Type->BoolFlag[SHOREBUILDING_INDEX].value) { //only ships can return trade to docks
			return false;
		}
		if (this->Type->UnitType == UnitTypeType::Naval && !dest->Type->BoolFlag[SHOREBUILDING_INDEX].value && dest->Type->UnitType != UnitTypeType::Naval) { //ships cannot return trade to land markets
			return false;
		}
	} else {
		if (dest->Player != this->Player && !(dest->Player->IsAllied(*this->Player) && this->Player->IsAllied(*dest->Player))) {
			return false;
		}
	}
	
	return true;
}

/**
**	@brief	Get whether a unit can cast a given spell
**
**	@return	True if the unit can cast the given spell, or false otherwise
*/
bool CUnit::CanCastSpell(const CSpell *spell, const bool ignore_mana_and_cooldown) const
{
	if (spell->IsAvailableForUnit(*this)) {
		if (!ignore_mana_and_cooldown) {
			if (this->Variable[MANA_INDEX].Value < spell->ManaCost) {
				return false;
			}
			
			if (this->SpellCoolDownTimers[spell->Slot]) {
				return false;
			}
		}
		
		return true;
	} else {
		return false;
	}
}

/**
**	@brief	Get whether a unit can cast any spell
**
**	@return	True if the unit can cast any spell, or false otherwise
*/
bool CUnit::CanCastAnySpell() const
{
	for (size_t i = 0; i < this->Type->Spells.size(); ++i) {
		if (this->CanCastSpell(this->Type->Spells[i], true)) {
			return true;
		}
	}
	
	return false;
}

/**
**	@brief	Get whether the unit can autocast a given spell
**
**	@param	spell	The spell
**
**	@return	True if the unit can autocast the spell, false otherwise
*/
bool CUnit::CanAutoCastSpell(const CSpell *spell) const
{
	if (!this->AutoCastSpell || !spell || !this->AutoCastSpell[spell->Slot] || !spell->AutoCast) {
		return false;
	}
	
	if (!CanCastSpell(spell, false)) {
		return false;
	}
	
	return true;
}

bool CUnit::IsItemEquipped(const CUnit *item) const
{
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item->Type->get_item_class());
	
	if (item_slot == stratagus::item_slot::none) {
		return false;
	}
	
	if (stratagus::vector::contains(this->EquippedItems[static_cast<int>(item_slot)], item)) {
		return true;
	}
	
	return false;
}

bool CUnit::is_item_class_equipped(const stratagus::item_class item_class) const
{
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item_class);
	
	if (item_slot == stratagus::item_slot::none) {
		return false;
	}
	
	for (size_t i = 0; i < EquippedItems[static_cast<int>(item_slot)].size(); ++i) {
		if (EquippedItems[static_cast<int>(item_slot)][i]->Type->get_item_class() == item_class) {
			return true;
		}
	}
	
	return false;
}

bool CUnit::IsItemTypeEquipped(const stratagus::unit_type *item_type) const
{
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item_type->get_item_class());
	
	if (item_slot == stratagus::item_slot::none) {
		return false;
	}
	
	for (size_t i = 0; i < EquippedItems[static_cast<int>(item_slot)].size(); ++i) {
		if (EquippedItems[static_cast<int>(item_slot)][i]->Type == item_type) {
			return true;
		}
	}
	
	return false;
}

bool CUnit::IsUniqueItemEquipped(const CUniqueItem *unique) const
{
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(unique->Type->get_item_class());
		
	if (item_slot == stratagus::item_slot::none) {
		return false;
	}
		
	int item_equipped_quantity = 0;
	for (size_t i = 0; i < this->EquippedItems[static_cast<int>(item_slot)].size(); ++i) {
		if (EquippedItems[static_cast<int>(item_slot)][i]->Unique == unique) {
			return true;
		}
	}
	
	return false;
}

bool CUnit::CanEquipItem(CUnit *item) const
{
	if (item->Container != this) {
		return false;
	}
	
	if (!item->Identified) {
		return false;
	}
	
	if (!this->can_equip_item_class(item->Type->get_item_class())) {
		return false;
	}
	
	return true;
}

bool CUnit::can_equip_item_class(const stratagus::item_class item_class) const
{
	if (item_class == stratagus::item_class::none) {
		return false;
	}
	
	if (stratagus::get_item_class_slot(item_class) == stratagus::item_slot::none) { //can't equip items that don't correspond to an equippable slot
		return false;
	}
	
	if (stratagus::get_item_class_slot(item_class) == stratagus::item_slot::weapon && !stratagus::vector::contains(this->Type->WeaponClasses, item_class)) { //if the item is a weapon and its item class isn't a weapon class used by this unit's type, return false
		return false;
	}
	
	if ( //if the item uses the shield (off-hand) slot, but that slot is unavailable for the weapon (because it is two-handed), return false
		stratagus::get_item_class_slot(item_class) == stratagus::item_slot::shield
		&& this->Type->WeaponClasses.size() > 0
		&& (
			this->Type->WeaponClasses[0] == stratagus::item_class::bow
			// add other two-handed weapons here as necessary
		)
	) {
		return false;
	}
	
	if ( //if the item is a shield and the weapon of this unit's type is incompatible with shields, return false
		item_class == stratagus::item_class::shield
		&& (
			Type->WeaponClasses.size() == 0
			|| stratagus::is_shield_incompatible_weapon_item_class(this->Type->WeaponClasses[0])
			|| Type->BoolFlag[HARVESTER_INDEX].value //workers can't use shields
		)
	) {
		return false;
	}
	
	if (this->get_item_slot_quantity(stratagus::get_item_class_slot(item_class)) == 0) {
		return false;
	}
	
	return true;
}

bool CUnit::CanUseItem(CUnit *item) const
{
	if (item->ConnectingDestination != nullptr) {
		if (item->Type->BoolFlag[ETHEREAL_INDEX].value && !this->Variable[ETHEREALVISION_INDEX].Value) {
			return false;
		}
		
		if (this->Type->BoolFlag[RAIL_INDEX].value && !item->ConnectingDestination->HasAdjacentRailForUnitType(this->Type)) {
			return false;
		}
		
		if (this->Player == item->Player || this->Player->IsAllied(*item->Player) || item->Player->Type == PlayerNeutral) {
			return true;
		}
	}
	
	if (!item->Type->BoolFlag[ITEM_INDEX].value && !item->Type->BoolFlag[POWERUP_INDEX].value) {
		return false;
	}
	
	if (item->Type->BoolFlag[ITEM_INDEX].value && !stratagus::is_consumable_item_class(item->Type->get_item_class())) {
		return false;
	}
	
	if (item->Spell != nullptr) {
		if (!this->HasInventory() || !::CanCastSpell(*this, *item->Spell, this, this->tilePos, this->MapLayer)) {
			return false;
		}
	}
	
	if (item->Work != nullptr) {
		if (!this->HasInventory() || this->GetIndividualUpgrade(item->Work)) {
			return false;
		}
	}
	
	if (item->Elixir != nullptr) {
		if (!this->HasInventory() || this->GetIndividualUpgrade(item->Elixir)) {
			return false;
		}
	}
	
	if (item->Elixir == nullptr && item->Variable[HITPOINTHEALING_INDEX].Value > 0 && this->Variable[HP_INDEX].Value >= this->GetModifiedVariable(HP_INDEX, VariableMax)) {
		return false;
	}
	
	return true;
}

bool CUnit::IsItemSetComplete(const CUnit *item) const
{
	for (size_t i = 0; i < item->Unique->Set->UniqueItems.size(); ++i) {
		if (!this->IsUniqueItemEquipped(item->Unique->Set->UniqueItems[i])) {
			return false;
		}
	}

	return true;
}

bool CUnit::EquippingItemCompletesSet(const CUnit *item) const
{
	for (size_t i = 0; i < item->Unique->Set->UniqueItems.size(); ++i) {
		const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item->Unique->Set->UniqueItems[i]->Type->get_item_class());
		
		if (item_slot == stratagus::item_slot::none) {
			return false;
		}
		
		bool has_item_equipped = false;
		for (size_t j = 0; j < this->EquippedItems[static_cast<int>(item_slot)].size(); ++j) {
			if (EquippedItems[static_cast<int>(item_slot)][j]->Unique == item->Unique->Set->UniqueItems[i]) {
				has_item_equipped = true;
				break;
			}
		}
		
		if (has_item_equipped && item->Unique->Set->UniqueItems[i] == item->Unique) { //if the unique item is already equipped, it won't complete the set (either the set is already complete, or needs something else)
			return false;
		} else if (!has_item_equipped && item->Unique->Set->UniqueItems[i] != item->Unique) {
			return false;
		}
		
	}

	return true;
}

bool CUnit::DeequippingItemBreaksSet(const CUnit *item) const
{
	if (!IsItemSetComplete(item)) {
		return false;
	}
	
	const stratagus::item_slot item_slot = stratagus::get_item_class_slot(item->Type->get_item_class());
		
	if (item_slot == stratagus::item_slot::none) {
		return false;
	}
		
	int item_equipped_quantity = 0;
	for (size_t i = 0; i < this->EquippedItems[static_cast<int>(item_slot)].size(); ++i) {
		if (EquippedItems[static_cast<int>(item_slot)][i]->Unique == item->Unique) {
			item_equipped_quantity += 1;
		}
	}
	
	if (item_equipped_quantity > 1) {
		return false;
	} else {
		return true;
	}
}

bool CUnit::HasInventory() const
{
	if (this->Type->BoolFlag[INVENTORY_INDEX].value) {
		return true;
	}
	
	if (!this->Type->BoolFlag[FAUNA_INDEX].value) {
		if (this->Character != nullptr) {
			return true;
		}
		
		if (this->Variable[LEVEL_INDEX].Value >= 3 && this->Type->BoolFlag[ORGANIC_INDEX].value) {
			return true;
		}
	}
	
	return false;
}

bool CUnit::CanLearnAbility(const CUpgrade *ability, bool pre) const
{
	if (!strncmp(ability->Ident.c_str(), "upgrade-deity-", 14)) { //if is a deity choice "ability", only allow for custom heroes (but display the icon for already-acquired deities for all heroes)
		if (!this->Character) {
			return false;
		}
		if (!this->Character->Custom && this->GetIndividualUpgrade(ability) == 0) {
			return false;
		}
		if (!pre && this->UpgradeRemovesExistingUpgrade(ability)) {
			return false;
		}
	}
	
	if (!pre && this->GetIndividualUpgrade(ability) >= ability->MaxLimit) { // already learned
		return false;
	}
	
	if (!pre && this->Variable[LEVELUP_INDEX].Value < 1 && ability->is_ability()) {
		return false;
	}
	
	if (!CheckDependencies(ability, this, false, pre)) {
		return false;
	}
	
	return true;
}

bool CUnit::CanHireMercenary(stratagus::unit_type *type, int civilization_id) const
{
	if (civilization_id == -1) {
		civilization_id = type->get_civilization() ? type->get_civilization()->ID : -1;
	}
	for (int p = 0; p < PlayerMax; ++p) {
		if (CPlayer::Players[p]->Type != PlayerNobody && CPlayer::Players[p]->Type != PlayerNeutral && civilization_id == CPlayer::Players[p]->Race && CheckDependencies(type, CPlayer::Players[p], true) && CPlayer::Players[p]->StartMapLayer == this->MapLayer->ID) {
			return true;
		}
	}
	
	return false;
}

bool CUnit::CanEat(const CUnit &unit) const
{
	if (this->Type->BoolFlag[CARNIVORE_INDEX].value && unit.Type->BoolFlag[FLESH_INDEX].value) {
		return true;
	}
	
	if (this->Type->BoolFlag[INSECTIVORE_INDEX].value && unit.Type->BoolFlag[INSECT_INDEX].value) {
		return true;
	}
	
	if (this->Type->BoolFlag[HERBIVORE_INDEX].value && unit.Type->BoolFlag[VEGETABLE_INDEX].value) {
		return true;
	}
	
	if (
		this->Type->BoolFlag[DETRITIVORE_INDEX].value
		&& (
			unit.Type->BoolFlag[DETRITUS_INDEX].value
			|| (unit.CurrentAction() == UnitAction::Die && (unit.Type->BoolFlag[FLESH_INDEX].value || unit.Type->BoolFlag[INSECT_INDEX].value))
		)
	) {
		return true;
	}
		
	return false;
}

bool CUnit::LevelCheck(const int level) const
{
	if (this->Variable[LEVEL_INDEX].Value == 0) {
		return false;
	}
	
	return SyncRand((this->Variable[LEVEL_INDEX].Value * 2) + 1) >= level;
}

bool CUnit::IsAbilityEmpowered(const CUpgrade *ability) const
{
	const stratagus::plane *plane = this->MapLayer->plane;

	if (plane != nullptr) {
		if (!plane->EmpoweredDeityDomains.empty()) {
			for (const CDeityDomain *deity_domain : ability->DeityDomains) {
				if (std::find(plane->EmpoweredDeityDomains.begin(), plane->EmpoweredDeityDomains.end(), deity_domain) != plane->EmpoweredDeityDomains.end()) {
					return true;
				}
			}
		}
		
		if (!plane->EmpoweredSchoolsOfMagic.empty()) {
			for (const CSchoolOfMagic *school_of_magic : ability->SchoolsOfMagic) {
				if (std::find(plane->EmpoweredSchoolsOfMagic.begin(), plane->EmpoweredSchoolsOfMagic.end(), school_of_magic) != plane->EmpoweredSchoolsOfMagic.end()) {
					return true;
				}
			}
		}
	}
	
	return false;
}

bool CUnit::IsSpellEmpowered(const CSpell *spell) const
{
	if (spell->DependencyId != -1) {
		return this->IsAbilityEmpowered(CUpgrade::get_all()[spell->DependencyId]);
	} else {
		return false;
	}
}

/**
**  Check if the upgrade removes an existing individual upgrade of the unit.
**
**  @param upgrade    Upgrade.
*/
bool CUnit::UpgradeRemovesExistingUpgrade(const CUpgrade *upgrade) const
{
	for (const auto &modifier : upgrade->get_modifiers()) {
		for (size_t j = 0; j < modifier->RemoveUpgrades.size(); ++j) {
			if (this->GetIndividualUpgrade(modifier->RemoveUpgrades[j]) > 0) {
				return true;
			}
		}
	}
	
	return false;
}

bool CUnit::HasAdjacentRailForUnitType(const stratagus::unit_type *type) const
{
	bool has_adjacent_rail = false;
	Vec2i top_left_pos(this->tilePos - Vec2i(1, 1));
	Vec2i bottom_right_pos(this->tilePos + this->Type->get_tile_size());
			
	for (int x = top_left_pos.x; x <= bottom_right_pos.x; ++x) {
		Vec2i tile_pos(x, top_left_pos.y);
		if (CMap::Map.Info.IsPointOnMap(tile_pos, this->MapLayer) && UnitTypeCanBeAt(*type, tile_pos, this->MapLayer->ID)) {
			has_adjacent_rail = true;
			break;
		}
				
		tile_pos.y = bottom_right_pos.y;
		if (CMap::Map.Info.IsPointOnMap(tile_pos, this->MapLayer) && UnitTypeCanBeAt(*type, tile_pos, this->MapLayer->ID)) {
			has_adjacent_rail = true;
			break;
		}
	}
			
	if (!has_adjacent_rail) {
		for (int y = top_left_pos.y; y <= bottom_right_pos.y; ++y) {
			Vec2i tile_pos(top_left_pos.x, y);
			if (CMap::Map.Info.IsPointOnMap(tile_pos, this->MapLayer) && UnitTypeCanBeAt(*type, tile_pos, this->MapLayer->ID)) {
				has_adjacent_rail = true;
				break;
			}
					
			tile_pos.x = bottom_right_pos.x;
			if (CMap::Map.Info.IsPointOnMap(tile_pos, this->MapLayer) && UnitTypeCanBeAt(*type, tile_pos, this->MapLayer->ID)) {
				has_adjacent_rail = true;
				break;
			}
		}
	}
			
	return has_adjacent_rail;
}

stratagus::animation_set *CUnit::GetAnimations() const
{
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && variation->Animations) {
		return variation->Animations;
	} else {
		return this->Type->get_animation_set();
	}
}

CConstruction *CUnit::GetConstruction() const
{
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && variation->Construction) {
		return variation->Construction;
	} else {
		return this->Type->Construction;
	}
}

IconConfig CUnit::GetIcon() const
{
	if (this->Character != nullptr && this->Character->Level >= 3 && this->Character->HeroicIcon.Icon) {
		return this->Character->HeroicIcon;
	} else if (this->Character != nullptr && this->Character->Icon.Icon) {
		return this->Character->Icon;
	} else if (this->Unique != nullptr && this->Unique->Icon.Icon) {
		return this->Unique->Icon;
	}
	
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && variation->Icon.Icon) {
		return variation->Icon;
	} else {
		return this->Type->Icon;
	}
}

stratagus::icon *CUnit::GetButtonIcon(const ButtonCmd button_action) const
{
	if (this->ButtonIcons.find(button_action) != this->ButtonIcons.end()) {
		return this->ButtonIcons.find(button_action)->second;
	} else if (this->Player == CPlayer::GetThisPlayer() && CPlayer::GetThisPlayer()->Faction != -1 && stratagus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->ButtonIcons.find(button_action) != stratagus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->ButtonIcons.end()) {
		return stratagus::faction::get_all()[CPlayer::GetThisPlayer()->Faction]->ButtonIcons[button_action].Icon;
	} else if (this->Player == CPlayer::GetThisPlayer() && PlayerRaces.ButtonIcons[CPlayer::GetThisPlayer()->Race].find(button_action) != PlayerRaces.ButtonIcons[CPlayer::GetThisPlayer()->Race].end()) {
		return PlayerRaces.ButtonIcons[CPlayer::GetThisPlayer()->Race][button_action].Icon;
	}
	
	return nullptr;
}

MissileConfig CUnit::GetMissile() const
{
	if (this->Variable[FIREDAMAGE_INDEX].Value > 0 && this->Type->FireMissile.Missile) {
		return this->Type->FireMissile;
	} else {
		return this->Type->Missile;
	}
}

CPlayerColorGraphic *CUnit::GetLayerSprite(int image_layer) const
{
	const stratagus::unit_type_variation *layer_variation = this->GetLayerVariation(image_layer);
	if (layer_variation && layer_variation->Sprite) {
		return layer_variation->Sprite;
	}
	
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && variation->LayerSprites[image_layer]) {
		return variation->LayerSprites[image_layer];
	} else if (this->Type->LayerSprites[image_layer])  {
		return this->Type->LayerSprites[image_layer];
	} else {
		return nullptr;
	}
}

std::string CUnit::GetName() const
{
	if (GameRunning && this->Character && this->Character->Deity) {
		if (CPlayer::GetThisPlayer()->Race >= 0) {
			std::string cultural_name = this->Character->Deity->GetCulturalName(stratagus::civilization::get_all()[CPlayer::GetThisPlayer()->Race]);
			
			if (!cultural_name.empty()) {
				return cultural_name;
			}
		}
		
		return this->Character->Deity->get_name();
	}
	
	std::string name = this->Name;
	
	if (name.empty()) {
		return name;
	}
	
	if (!this->ExtraName.empty()) {
		name += " " + this->ExtraName;
	}

	if (!this->get_surname().empty()) {
		name += " " + this->get_surname();
	}
	
	return name;
}

std::string CUnit::GetTypeName() const
{
	if (this->Character && this->Character->Deity) {
		return _("Deity");
	}
	
	const stratagus::unit_type_variation *variation = this->GetVariation();
	if (variation && !variation->TypeName.empty()) {
		return _(variation->TypeName.c_str());
	} else {
		return _(this->Type->get_name().c_str());
	}
}

std::string CUnit::GetMessageName() const
{
	std::string name = GetName();
	if (name.empty()) {
		return GetTypeName();
	}
	
	if (!this->Identified) {
		return GetTypeName() + " (" + _("Unidentified") + ")";
	}
	
	if (!this->Unique && this->Work == nullptr && (this->Prefix != nullptr || this->Suffix != nullptr || this->Spell != nullptr)) {
		return name;
	}
	
	return name + " (" + GetTypeName() + ")";
}
//Wyrmgus end

const stratagus::time_of_day *CUnit::get_center_tile_time_of_day() const
{
	if (this->MapLayer == nullptr) {
		return nullptr;
	}

	//get the time of day for the unit's tile
	return this->MapLayer->get_tile_time_of_day(this->get_center_tile_pos());
}

/**
**  Let an unit die.
**
**  @param unit    Unit to be destroyed.
*/
void LetUnitDie(CUnit &unit, bool suicide)
{
	unit.Variable[HP_INDEX].Value = std::min<int>(0, unit.Variable[HP_INDEX].Value);
	unit.Moving = 0;
	unit.TTL = 0;
	unit.Anim.Unbreakable = 0;

	const stratagus::unit_type *type = unit.Type;

	while (unit.Resource.Workers) {
		unit.Resource.Workers->DeAssignWorkerFromMine(unit);
	}

	// removed units,  just remove.
	if (unit.Removed) {
		DebugPrint("Killing a removed unit?\n");
		if (unit.UnitInside) {
			DestroyAllInside(unit);
		}
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
		return;
	}

	PlayUnitSound(unit, stratagus::unit_sound_type::dying);

	//
	// Catapults,... explodes.
	//
	if (type->ExplodeWhenKilled) {
		const PixelPos pixelPos = unit.get_map_pixel_pos_center();

		MakeMissile(*type->Explosion.Missile, pixelPos, pixelPos, unit.MapLayer->ID);
	}
	if (type->DeathExplosion) {
		const PixelPos pixelPos = unit.get_map_pixel_pos_center();

		type->DeathExplosion->pushPreamble();
		//Wyrmgus start
		type->DeathExplosion->pushInteger(UnitNumber(unit));
		//Wyrmgus end
		type->DeathExplosion->pushInteger(pixelPos.x);
		type->DeathExplosion->pushInteger(pixelPos.y);
		type->DeathExplosion->run();
	}
	if (suicide) {
		const PixelPos pixelPos = unit.get_map_pixel_pos_center();
		
		if (unit.GetMissile().Missile) {
			MakeMissile(*unit.GetMissile().Missile, pixelPos, pixelPos, unit.MapLayer->ID);
		}
	}
	// Handle Teleporter Destination Removal
	if (type->BoolFlag[TELEPORTER_INDEX].value && unit.Goal) {
		unit.Goal->Remove(nullptr);
		UnitLost(*unit.Goal);
		UnitClearOrders(*unit.Goal);
		unit.Goal->Release();
		unit.Goal = nullptr;
	}
	
	//Wyrmgus start
	for (size_t i = 0; i < unit.SoldUnits.size(); ++i) {
		DestroyAllInside(*unit.SoldUnits[i]);
		LetUnitDie(*unit.SoldUnits[i]);
	}
	unit.SoldUnits.clear();
	//Wyrmgus end

	// Transporters lose or save their units and buildings their workers
	//Wyrmgus start
//	if (unit.UnitInside && unit.Type->BoolFlag[SAVECARGO_INDEX].value) {
	if (
		unit.UnitInside
		&& (
			unit.Type->BoolFlag[SAVECARGO_INDEX].value
			|| (unit.HasInventory() && unit.Character == nullptr)
		)
	) {
	//Wyrmgus end
		DropOutAll(unit);
	} else if (unit.UnitInside) {
		DestroyAllInside(unit);
	}
	
	//Wyrmgus start
	//if is a raft or bridge, destroy all land units on it
	if (unit.Type->BoolFlag[BRIDGE_INDEX].value) {
		std::vector<CUnit *> table;
		Select(unit.tilePos, unit.tilePos, table, unit.MapLayer->ID);
		for (size_t i = 0; i != table.size(); ++i) {
			if (table[i]->IsAliveOnMap() && !table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->Type->UnitType == UnitTypeType::Land) {
				table[i]->Variable[HP_INDEX].Value = std::min<int>(0, unit.Variable[HP_INDEX].Value);
				table[i]->Moving = 0;
				table[i]->TTL = 0;
				table[i]->Anim.Unbreakable = 0;
				PlayUnitSound(*table[i], stratagus::unit_sound_type::dying);
				table[i]->Remove(nullptr);
				UnitLost(*table[i]);
				UnitClearOrders(*table[i]);
				table[i]->Release();
			}
		}
	}
	//Wyrmgus end

	//Wyrmgus start
	//drop items upon death
	if (!suicide && unit.CurrentAction() != UnitAction::Built && (unit.Character || unit.Type->BoolFlag[BUILDING_INDEX].value || SyncRand(100) >= 66)) { //66% chance nothing will be dropped, unless the unit is a character or building, in which it case it will always drop an item
		unit.GenerateDrop();
	}
	//Wyrmgus end
	
	//Wyrmgus start
	std::vector<CUnit *> seeing_table; //units seeing this unit
	if (type->BoolFlag[AIRUNPASSABLE_INDEX].value) {
		SelectAroundUnit(unit, 16, seeing_table); //a range of 16 should be safe enough; there should be no unit or building in the game with a sight range that high, let alone higher
		for (size_t i = 0; i != seeing_table.size(); ++i) {
			MapUnmarkUnitSight(*seeing_table[i]);
		}
	}
	//Wyrmgus end

	unit.Remove(nullptr);
	UnitLost(unit);
	UnitClearOrders(unit);


	// Unit has death animation.

	// Not good: UnitUpdateHeading(unit);
	delete unit.Orders[0];
	unit.Orders[0] = COrder::NewActionDie();
	if (type->get_corpse_type() != nullptr) {
#ifdef DYNAMIC_LOAD
		if (!type->Sprite) {
			LoadUnitTypeSprite(type);
		}
#endif
		unit.pixel_offset.setX((type->get_corpse_type()->get_frame_width() - type->get_corpse_type()->Sprite->get_original_frame_size().width()) / 2);
		unit.pixel_offset.setY((type->get_corpse_type()->get_frame_height() - type->get_corpse_type()->Sprite->get_original_frame_size().height()) / 2);

		unit.CurrentSightRange = type->get_corpse_type()->Stats[unit.Player->Index].Variables[SIGHTRANGE_INDEX].Max;
	} else {
		unit.CurrentSightRange = 0;
	}

	// If we have a corpse, or a death animation, we are put back on the map
	// This enables us to be tracked.  Possibly for spells (eg raise dead)
	if (type->get_corpse_type() != nullptr || (unit.GetAnimations() && unit.GetAnimations()->Death)) {
		unit.Removed = 0;
		CMap::Map.Insert(unit);

		// FIXME: rb: Maybe we need this here because corpse of cloaked units
		//	may crash Sign code

		// Recalculate the seen count.
		//UnitCountSeen(unit);
	}
	
	MapMarkUnitSight(unit);
	
	//Wyrmgus start
	if (unit.settlement) {
		unit.UpdateBuildingSettlementAssignment(unit.settlement);
	}
	//Wyrmgus end
	
	//Wyrmgus start
	if (type->BoolFlag[AIRUNPASSABLE_INDEX].value) {
		for (size_t i = 0; i != seeing_table.size(); ++i) {
			MapMarkUnitSight(*seeing_table[i]);
		}
	}
	//Wyrmgus end
}

/**
**  Destroy all units inside unit.
**
**  @param source  container.
*/
void DestroyAllInside(CUnit &source)
{
	CUnit *unit = source.UnitInside;

	// No Corpses, we are inside something, and we can't be seen
	for (int i = source.InsideCount; i; --i, unit = unit->NextContained) {
		// Transporter inside a transporter?
		if (unit->UnitInside) {
			DestroyAllInside(*unit);
		}
		UnitLost(*unit);
		UnitClearOrders(*unit);
		unit->Release();
	}
}

/*----------------------------------------------------------------------------
  -- Unit AI
  ----------------------------------------------------------------------------*/

int ThreatCalculate(const CUnit &unit, const CUnit &dest)
{
	const stratagus::unit_type &type = *unit.Type;
	const stratagus::unit_type &dtype = *dest.Type;
	int cost = 0;

	// Buildings, non-aggressive and invincible units have the lowest priority
	if (dest.IsAgressive() == false || dest.Variable[UNHOLYARMOR_INDEX].Value > 0
		|| dest.Type->BoolFlag[INDESTRUCTIBLE_INDEX].value) {
		if (dest.Type->CanMove() == false) {
			return INT_MAX;
		} else {
			return INT_MAX / 2;
		}
	}

	// Priority 0-255
	cost -= dest.Variable[PRIORITY_INDEX].Value * PRIORITY_FACTOR;
	// Remaining HP (Health) 0-65535
	//Wyrmgus start
//	cost += dest.Variable[HP_INDEX].Value * 100 / dest.Variable[HP_INDEX].Max * HEALTH_FACTOR;
	cost += dest.Variable[HP_INDEX].Value * 100 / dest.GetModifiedVariable(HP_INDEX, VariableMax) * HEALTH_FACTOR;
	//Wyrmgus end

	const int d = unit.MapDistanceTo(dest);

	if (d <= unit.GetModifiedVariable(ATTACKRANGE_INDEX) && d >= type.MinAttackRange) {
		cost += d * INRANGE_FACTOR;
		cost -= INRANGE_BONUS;
	} else {
		cost += d * DISTANCE_FACTOR;
	}

	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (type.BoolFlag[i].AiPriorityTarget != CONDITION_TRUE) {
			if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_ONLY) & (dtype.BoolFlag[i].value)) {
				cost -= AIPRIORITY_BONUS;
			}
			if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_FALSE) & (dtype.BoolFlag[i].value)) {
				cost += AIPRIORITY_BONUS;
			}
		}
	}

	// Unit can attack back.
	if (CanTarget(dtype, type)) {
		cost -= CANATTACK_BONUS;
	}
	return cost;
}

static void HitUnit_LastAttack(const CUnit *attacker, CUnit &target)
{
	const unsigned long last_attack_cycle = target.Attacked;

	target.Attacked = GameCycle ? GameCycle : 1;
	if (target.Type->BoolFlag[WALL_INDEX].value || (last_attack_cycle && GameCycle <= (last_attack_cycle + 2 * CYCLES_PER_SECOND))) {
		return;
	}
	// NOTE: perhaps this should also be moved into the notify?
	if (target.Player == CPlayer::GetThisPlayer()) {
		// FIXME: Problem with load+save.

		//
		// One help cry each 2 second is enough
		// If on same area ignore it for 2 minutes.
		//
		if (HelpMeLastCycle < GameCycle) {
			if (!HelpMeLastCycle
				|| HelpMeLastCycle + CYCLES_PER_SECOND * 120 < GameCycle
				|| target.tilePos.x < HelpMeLastX - 14
				|| target.tilePos.x > HelpMeLastX + 14
				|| target.tilePos.y < HelpMeLastY - 14
				|| target.tilePos.y > HelpMeLastY + 14) {
				HelpMeLastCycle = GameCycle + CYCLES_PER_SECOND * 2;
				HelpMeLastX = target.tilePos.x;
				HelpMeLastY = target.tilePos.y;
				PlayUnitSound(target, stratagus::unit_sound_type::help);
				target.Player->Notify(NotifyRed, target.tilePos, target.MapLayer->ID, _("%s attacked"), target.GetMessageName().c_str());
			}
		}
	}

	//only trigger this every two minutes for the unit
	if (attacker && (last_attack_cycle == 0 || GameCycle > (last_attack_cycle + 2 * (CYCLES_PER_SECOND * 60)))) {
		if (
			target.Player->AiEnabled
			&& !attacker->Type->BoolFlag[INDESTRUCTIBLE_INDEX].value // don't attack indestructible units back
		) {
			AiHelpMe(attacker->GetFirstContainer(), target);
		}
	}
}

static void HitUnit_Raid(CUnit *attacker, CUnit &target, int damage)
{
	if (!attacker) {
		return;
	}
	
	if (attacker->Player == target.Player || attacker->Player->Index == PlayerNumNeutral || target.Player->Index == PlayerNumNeutral) {
		return;
	}
	
	int var_index;
	if (target.Type->BoolFlag[BUILDING_INDEX].value) {
		var_index = RAIDING_INDEX;
	} else {
		var_index = MUGGING_INDEX;
	}
	
	if (!attacker->Variable[var_index].Value) {
		return;
	}
	
	if (!attacker->Variable[SHIELDPIERCING_INDEX].Value) {
		int shieldDamage = target.Variable[SHIELDPERMEABILITY_INDEX].Value < 100
							? std::min(target.Variable[SHIELD_INDEX].Value, damage * (100 - target.Variable[SHIELDPERMEABILITY_INDEX].Value) / 100)
							: 0;
		
		damage -= shieldDamage;
	}
	
	damage = std::min(damage, target.Variable[HP_INDEX].Value);
	
	if (damage <= 0) {
		return;
	}
	
	for (int i = 0; i < MaxCosts; ++i) {
		if (target.Type->Stats[target.Player->Index].Costs[i] > 0) {
			int resource_change = target.Type->Stats[target.Player->Index].Costs[i] * damage * attacker->Variable[var_index].Value / target.GetModifiedVariable(HP_INDEX, VariableMax) / 100;
			resource_change = std::min(resource_change, target.Player->get_resource(stratagus::resource::get_all()[i], STORE_BOTH));
			attacker->Player->change_resource(stratagus::resource::get_all()[i], resource_change);
			attacker->Player->TotalResources[i] += resource_change;
			target.Player->change_resource(stratagus::resource::get_all()[i], -resource_change);
		}
	}
}

static bool HitUnit_IsUnitWillDie(const CUnit *attacker, const CUnit &target, int damage)
{
	int shieldDamage = target.Variable[SHIELDPERMEABILITY_INDEX].Value < 100
					   ? std::min(target.Variable[SHIELD_INDEX].Value, damage * (100 - target.Variable[SHIELDPERMEABILITY_INDEX].Value) / 100)
					   : 0;
	return (target.Variable[HP_INDEX].Value <= damage && attacker && attacker->Variable[SHIELDPIERCING_INDEX].Value)
		   || (target.Variable[HP_INDEX].Value <= damage - shieldDamage)
		   || (target.Variable[HP_INDEX].Value == 0);
}

static void HitUnit_IncreaseScoreForKill(CUnit &attacker, CUnit &target)
{
	attacker.Player->Score += target.Variable[POINTS_INDEX].Value;
	if (target.Type->BoolFlag[BUILDING_INDEX].value) {
		attacker.Player->TotalRazings++;
	} else {
		attacker.Player->TotalKills++;
	}
	
	//Wyrmgus start
	attacker.Player->UnitTypeKills[target.Type->Slot]++;
	
	//distribute experience between nearby units belonging to the same player
	if (!target.Type->BoolFlag[BUILDING_INDEX].value) {
		attacker.ChangeExperience(UseHPForXp ? target.Variable[HP_INDEX].Value : target.Variable[POINTS_INDEX].Value, ExperienceRange);
	}
	//Wyrmgus end
	
	attacker.Variable[KILL_INDEX].Value++;
	attacker.Variable[KILL_INDEX].Max++;
	attacker.Variable[KILL_INDEX].Enable = 1;
	
	//Wyrmgus start
	for (const auto &objective : attacker.Player->get_quest_objectives()) {
		const stratagus::quest_objective *quest_objective = objective->get_quest_objective();
		if (
			(
				quest_objective->get_objective_type() == stratagus::objective_type::destroy_units
				&& (stratagus::vector::contains(quest_objective->UnitTypes, target.Type) || stratagus::vector::contains(quest_objective->get_unit_classes(), target.Type->get_unit_class()))
				&& (quest_objective->get_settlement() == nullptr || quest_objective->get_settlement() == target.settlement)
			)
			|| (quest_objective->get_objective_type() == stratagus::objective_type::destroy_hero && target.Character && quest_objective->get_character() == target.Character)
			|| (quest_objective->get_objective_type() == stratagus::objective_type::destroy_unique && target.Unique && quest_objective->Unique == target.Unique)
		) {
			if (quest_objective->get_faction() == nullptr || quest_objective->get_faction()->ID == target.Player->Faction) {
				objective->Counter = std::min(objective->Counter + 1, quest_objective->get_quantity());
			}
		} else if (quest_objective->get_objective_type() == stratagus::objective_type::destroy_faction) {
			const CPlayer *faction_player = GetFactionPlayer(quest_objective->get_faction());
			
			if (faction_player) {
				int dying_faction_units = faction_player == target.Player ? 1 : 0;
				dying_faction_units += target.GetTotalInsideCount(faction_player, true, true);
				
				if (dying_faction_units > 0 && faction_player->GetUnitCount() <= dying_faction_units) {
					objective->Counter = 1;
				}
			}
		}
	}
	
	//also increase score for units inside the target that will be destroyed when the target dies
	if (
		target.UnitInside
		&& !target.Type->BoolFlag[SAVECARGO_INDEX].value
	) {
		CUnit *boarded_unit = target.UnitInside;
		for (int i = 0; i < target.InsideCount; ++i, boarded_unit = boarded_unit->NextContained) {
			if (!boarded_unit->Type->BoolFlag[ITEM_INDEX].value) { //ignore items
				HitUnit_IncreaseScoreForKill(attacker, *boarded_unit);
			}
		}
	}
	//Wyrmgus end
}

static void HitUnit_ApplyDamage(CUnit *attacker, CUnit &target, int damage)
{
	if (attacker && attacker->Variable[SHIELDPIERCING_INDEX].Value) {
		target.Variable[HP_INDEX].Value -= damage;
	} else {
		int shieldDamage = target.Variable[SHIELDPERMEABILITY_INDEX].Value < 100
						   ? std::min(target.Variable[SHIELD_INDEX].Value, damage * (100 - target.Variable[SHIELDPERMEABILITY_INDEX].Value) / 100)
						   : 0;
		if (shieldDamage) {
			target.Variable[SHIELD_INDEX].Value -= shieldDamage;
			clamp(&target.Variable[SHIELD_INDEX].Value, 0, target.Variable[SHIELD_INDEX].Max);
		}
		target.Variable[HP_INDEX].Value -= damage - shieldDamage;
	}
	
	//Wyrmgus start
	//distribute experience between nearby units belonging to the same player

//	if (UseHPForXp && attacker && target.IsEnemy(*attacker)) {
	if (UseHPForXp && attacker && (target.IsEnemy(*attacker) || target.Player->Type == PlayerNeutral) && !target.Type->BoolFlag[BUILDING_INDEX].value) {
		attacker->ChangeExperience(damage, ExperienceRange);
	}
	//Wyrmgus end
	
	//Wyrmgus start
	//use a healing item if any are available
	if (target.HasInventory()) {
		target.HealingItemAutoUse();
	}
	//Wyrmgus end
}

static void HitUnit_BuildingCapture(CUnit *attacker, CUnit &target, int damage)
{
	// FIXME: this is dumb. I made repairers capture. crap.
	// david: capture enemy buildings
	// Only worker types can capture.
	// Still possible to destroy building if not careful (too many attackers)
	if (EnableBuildingCapture && attacker
		&& target.Type->BoolFlag[BUILDING_INDEX].value && target.Variable[HP_INDEX].Value <= damage * 3
		&& attacker->IsEnemy(target)
		&& attacker->Type->RepairRange) {
		target.ChangeOwner(*attacker->Player);
		CommandStopUnit(*attacker); // Attacker shouldn't continue attack!
	}
}

static void HitUnit_ShowDamageMissile(const CUnit &target, int damage)
{
	const PixelPos targetPixelCenter = target.get_map_pixel_pos_center();

	if ((target.IsVisibleOnMap(*CPlayer::GetThisPlayer()) || ReplayRevealMap) && !DamageMissile.empty()) {
		const stratagus::missile_type *mtype = stratagus::missile_type::get(DamageMissile);
		const PixelDiff offset(3, -mtype->get_range());

		MakeLocalMissile(*mtype, targetPixelCenter, targetPixelCenter + offset, target.MapLayer->ID)->Damage = -damage;
	}
}

static void HitUnit_ShowImpactMissile(const CUnit &target)
{
	const PixelPos targetPixelCenter = target.get_map_pixel_pos_center();
	const stratagus::unit_type &type = *target.Type;

	if (target.Variable[SHIELD_INDEX].Value > 0
		&& !type.Impact[ANIMATIONS_DEATHTYPES + 1].Name.empty()) { // shield impact
		MakeMissile(*type.Impact[ANIMATIONS_DEATHTYPES + 1].Missile, targetPixelCenter, targetPixelCenter, target.MapLayer->ID);
	} else if (target.DamagedType && !type.Impact[target.DamagedType].Name.empty()) { // specific to damage type impact
		MakeMissile(*type.Impact[target.DamagedType].Missile, targetPixelCenter, targetPixelCenter, target.MapLayer->ID);
	} else if (!type.Impact[ANIMATIONS_DEATHTYPES].Name.empty()) { // generic impact
		MakeMissile(*type.Impact[ANIMATIONS_DEATHTYPES].Missile, targetPixelCenter, targetPixelCenter, target.MapLayer->ID);
	}
}

static void HitUnit_ChangeVariable(CUnit &target, const Missile &missile)
{
	const int var = missile.Type->ChangeVariable;

	target.Variable[var].Enable = 1;
	target.Variable[var].Value += missile.Type->ChangeAmount;
	if (target.Variable[var].Value > target.Variable[var].Max) {
		if (missile.Type->ChangeMax) {
			target.Variable[var].Max = target.Variable[var].Value;
		//Wyrmgus start
//		} else {
		} else if (target.Variable[var].Value > target.GetModifiedVariable(var, VariableMax)) {
		//Wyrmgus end
			//Wyrmgus start
//			target.Variable[var].Value = target.Variable[var].Max;
			target.Variable[var].Value = target.GetModifiedVariable(var, VariableMax);
			//Wyrmgus end
		}
	}
	
	//Wyrmgus start
	if (var == ATTACKRANGE_INDEX && target.Container) {
		target.Container->UpdateContainerAttackRange();
	} else if (var == LEVEL_INDEX || var == POINTS_INDEX) {
		target.UpdateXPRequired();
	} else if (var == XP_INDEX) {
		target.XPChanged();
	} else if (var == STUN_INDEX && target.Variable[var].Value > 0) { //if unit has become stunned, stop it
		CommandStopUnit(target);
	} else if (var == KNOWLEDGEMAGIC_INDEX) {
		target.CheckIdentification();
	}
	//Wyrmgus end
}


static void HitUnit_Burning(CUnit &target)
{
	//Wyrmgus start
//	const int f = (100 * target.Variable[HP_INDEX].Value) / target.Variable[HP_INDEX].Max;
	const int f = (100 * target.Variable[HP_INDEX].Value) / target.GetModifiedVariable(HP_INDEX, VariableMax);
	//Wyrmgus end
	stratagus::missile_type *fire = MissileBurningBuilding(f);

	if (fire) {
		const PixelPos targetPixelCenter = target.get_map_pixel_pos_center();
		const PixelDiff offset(0, -stratagus::defines::get()->get_tile_height());
		Missile *missile = MakeMissile(*fire, targetPixelCenter + offset, targetPixelCenter + offset, target.MapLayer->ID);

		missile->SourceUnit = &target;
		target.Burning = 1;
	}
}

//Wyrmgus start
void HitUnit_NormalHitSpecialDamageEffects(CUnit &attacker, CUnit &target)
{
	if (attacker.Variable[FIREDAMAGE_INDEX].Value > 0 && attacker.Variable[FIREDAMAGE_INDEX].Value >= attacker.Variable[COLDDAMAGE_INDEX].Value) { // apply only either the fire damage or cold damage effects, but not both at the same time; apply the one with greater value, but if both are equal fire damage takes precedence
		HitUnit_SpecialDamageEffect(target, FIREDAMAGE_INDEX);
	} else if (attacker.Variable[COLDDAMAGE_INDEX].Value > 0) {
		HitUnit_SpecialDamageEffect(target, COLDDAMAGE_INDEX);
	}
	
	if (attacker.Variable[LIGHTNINGDAMAGE_INDEX].Value > 0) {
		HitUnit_SpecialDamageEffect(target, LIGHTNINGDAMAGE_INDEX);
	}
}

void HitUnit_SpecialDamageEffect(CUnit &target, int dmg_var)
{
	if (dmg_var == COLDDAMAGE_INDEX && target.Variable[COLDRESISTANCE_INDEX].Value < 100 && target.Type->BoolFlag[ORGANIC_INDEX].value) { //if resistance to cold is 100%, the effect has no chance of being applied
		int rand_max = 100 * 100 / (100 - target.Variable[COLDRESISTANCE_INDEX].Value);
		if (SyncRand(rand_max) == 0) {
			target.Variable[SLOW_INDEX].Enable = 1;
			target.Variable[SLOW_INDEX].Value = std::max(200, target.Variable[SLOW_INDEX].Value);
			target.Variable[SLOW_INDEX].Max = 1000;
		}
	} else if (dmg_var == LIGHTNINGDAMAGE_INDEX && target.Variable[LIGHTNINGRESISTANCE_INDEX].Value < 100 && target.Type->BoolFlag[ORGANIC_INDEX].value) {
		int rand_max = 100 * 100 / (100 - target.Variable[LIGHTNINGRESISTANCE_INDEX].Value);
		if (SyncRand(rand_max) == 0) {
			target.Variable[STUN_INDEX].Enable = 1;
			target.Variable[STUN_INDEX].Value = std::max(50, target.Variable[STUN_INDEX].Value);
			target.Variable[STUN_INDEX].Max = 1000;
		}
	}
}
//Wyrmgus end

//Wyrmgus start
//static void HitUnit_RunAway(CUnit &target, const CUnit &attacker)
void HitUnit_RunAway(CUnit &target, const CUnit &attacker)
//Wyrmgus end
{
	Vec2i pos = target.tilePos - attacker.tilePos;
	int d = isqrt(pos.x * pos.x + pos.y * pos.y);

	if (!d) {
		d = 1;
	}
	pos.x = target.tilePos.x + (pos.x * 5) / d + SyncRand(4);
	pos.y = target.tilePos.y + (pos.y * 5) / d + SyncRand(4);
	CMap::Map.Clamp(pos, target.MapLayer->ID);
	CommandStopUnit(target);
	CommandMove(target, pos, 0, target.MapLayer->ID);
}

static void HitUnit_AttackBack(CUnit &attacker, CUnit &target)
{
	const int threshold = 30;
	COrder *savedOrder = nullptr;

	if (target.Player == CPlayer::GetThisPlayer() && target.Player->Type != PlayerNeutral) { // allow neutral units to strike back
		if (target.CurrentAction() == UnitAction::Attack) {
			COrder_Attack &order = dynamic_cast<COrder_Attack &>(*target.CurrentOrder());
			if (order.IsWeakTargetSelected() == false) {
				return;
			}
		//Wyrmgus start
//		} else {
		} else if (target.CurrentAction() != UnitAction::Still) {
		//Wyrmgus end
			return;
		}
	}
	if (target.CanStoreOrder(target.CurrentOrder())) {
		savedOrder = target.CurrentOrder()->Clone();
	}
	CUnit *oldgoal = target.CurrentOrder()->GetGoal();
	CUnit *goal, *best = oldgoal;

	if (RevealAttacker && CanTarget(*target.Type, *attacker.Type)) {
		// Reveal Unit that is attacking
		goal = &attacker;
	} else {
		if (target.CurrentAction() == UnitAction::StandGround) {
			goal = AttackUnitsInRange(target);
		} else {
			// Check for any other units in range
			goal = AttackUnitsInReactRange(target);
		}
	}

	// Calculate the best target we could attack
	if (!best || (goal && (ThreatCalculate(target, *goal) < ThreatCalculate(target, *best)))) {
		best = goal;
	}
	if (CanTarget(*target.Type, *attacker.Type)
		&& (!best || (goal != &attacker
					  && (ThreatCalculate(target, attacker) < ThreatCalculate(target, *best))))) {
		best = &attacker;
	}
	//Wyrmgus start
//	if (best && best != oldgoal && best->Player != target.Player && best->IsAllied(target) == false) {
	if (best && best != oldgoal && (best->Player != target.Player || target.Player->Type == PlayerNeutral) && best->IsAllied(target) == false) {
	//Wyrmgus end
		CommandAttack(target, best->tilePos, best, FlushCommands, best->MapLayer->ID);
		// Set threshold value only for aggressive units
		if (best->IsAgressive()) {
			target.Threshold = threshold;
		}
		if (savedOrder != nullptr) {
			target.SavedOrder = savedOrder;
		}
	}
}

/**
**  Unit is hit by missile or other damage.
**
**  @param attacker    Unit that attacks.
**  @param target      Unit that is hit.
**  @param damage      How many damage to take.
**  @param missile     Which missile took the damage.
*/
//Wyrmgus start
//void HitUnit(CUnit *attacker, CUnit &target, int damage, const Missile *missile)
void HitUnit(CUnit *attacker, CUnit &target, int damage, const Missile *missile, bool show_damage)
//Wyrmgus end
{
	const stratagus::unit_type *type = target.Type;
	if (!damage) {
		// Can now happen by splash damage
		// Multiple places send x/y as damage, which may be zero
		return;
	}

	if (target.Variable[UNHOLYARMOR_INDEX].Value > 0 || target.Type->BoolFlag[INDESTRUCTIBLE_INDEX].value) {
		// vladi: units with active UnholyArmour are invulnerable
		return;
	}
	if (target.Removed) {
		DebugPrint("Removed target hit\n");
		return;
	}

	Assert(damage != 0 && target.CurrentAction() != UnitAction::Die && !target.Type->BoolFlag[VANISHES_INDEX].value);

	//Wyrmgus start
	if (
		(attacker != nullptr && attacker->Player == CPlayer::GetThisPlayer())
		&& target.Player != CPlayer::GetThisPlayer()
	) {
		// If player is hitting or being hit add tension to our music
		AddMusicTension(1);
	}
	//Wyrmgus end

	if (GodMode) {
		if (attacker && attacker->Player == CPlayer::GetThisPlayer()) {
			damage = target.Variable[HP_INDEX].Value;
		}
		if (target.Player == CPlayer::GetThisPlayer()) {
			damage = 0;
		}
	}
	//Wyrmgus start
//	HitUnit_LastAttack(attacker, target);
	//Wyrmgus end
	if (attacker) {
		//Wyrmgus start
		HitUnit_LastAttack(attacker, target); //only trigger the help me notification and AI code if there is actually an attacker
		//Wyrmgus end
		target.DamagedType = ExtraDeathIndex(attacker->Type->DamageType.c_str());
	}

	// OnHit callback
	if (type->OnHit) {
		const int tarSlot = UnitNumber(target);
		const int atSlot = attacker && attacker->IsAlive() ? UnitNumber(*attacker) : -1;

		type->OnHit->pushPreamble();
		type->OnHit->pushInteger(tarSlot);
		type->OnHit->pushInteger(atSlot);
		type->OnHit->pushInteger(damage);
		type->OnHit->run();
	}

	// Increase variables and call OnImpact
	if (missile && missile->Type) {
		if (missile->Type->ChangeVariable != -1) {
			HitUnit_ChangeVariable(target, *missile);
		}
		if (missile->Type->OnImpact) {
			const int attackerSlot = attacker ? UnitNumber(*attacker) : -1;
			const int targetSlot = UnitNumber(target);
			missile->Type->OnImpact->pushPreamble();
			missile->Type->OnImpact->pushInteger(attackerSlot);
			missile->Type->OnImpact->pushInteger(targetSlot);
			missile->Type->OnImpact->pushInteger(damage);
			missile->Type->OnImpact->run();
		}
	}
	
	HitUnit_Raid(attacker, target, damage);

	if (HitUnit_IsUnitWillDie(attacker, target, damage)) { // unit is killed or destroyed
		if (attacker) {
			//  Setting ai threshold counter to 0 so it can target other units
			attacker->Threshold = 0;
		}
		
		CUnit *destroyer = attacker;
		if (!destroyer) {
			int best_distance = 0;
			std::vector<CUnit *> table;
			SelectAroundUnit(target, ExperienceRange, table, IsEnemyWith(*target.Player));
			for (size_t i = 0; i < table.size(); i++) {
				CUnit *potential_destroyer = table[i];
				int distance = target.MapDistanceTo(*potential_destroyer);
				if (!destroyer || distance < best_distance) {
					destroyer = potential_destroyer;
					best_distance = distance;
				}
			}
		}
		if (destroyer) {
			if (target.IsEnemy(*destroyer) || target.Player->Type == PlayerNeutral) {
				HitUnit_IncreaseScoreForKill(*destroyer, target);
			}
		}
		LetUnitDie(target);
		return;
	}

	HitUnit_ApplyDamage(attacker, target, damage);
	HitUnit_BuildingCapture(attacker, target, damage);
	//Wyrmgus start
//	HitUnit_ShowDamageMissile(target, damage);
	if (show_damage) {
		HitUnit_ShowDamageMissile(target, damage);
	}
	//Wyrmgus end

	HitUnit_ShowImpactMissile(target);

	//Wyrmgus start
//	if (type->BoolFlag[BUILDING_INDEX].value && !target.Burning) {
	if (type->BoolFlag[BUILDING_INDEX].value && !target.Burning && !target.UnderConstruction && target.Type->get_tile_width() != 1 && target.Type->get_tile_height() != 1) { //the building shouldn't burn if it's still under construction, or if it's too small
	//Wyrmgus end
		HitUnit_Burning(target);
	}

	/* Target Reaction on Hit */
	if (target.Player->AiEnabled) {
		if (target.CurrentOrder()->OnAiHitUnit(target, attacker, damage)) {
			return;
		}
	}

	if (!attacker) {
		return;
	}

	// Can't attack run away.
	//Wyrmgus start
//	if (!target.IsAgressive() && target.CanMove() && target.CurrentAction() == UnitAction::Still && !target.BoardCount) {
	if (
		(!target.IsAgressive() || attacker->Type->BoolFlag[INDESTRUCTIBLE_INDEX].value)
		&& target.CanMove()
		&& (target.CurrentAction() == UnitAction::Still || target.Variable[TERROR_INDEX].Value > 0)
		&& !target.BoardCount
		&& !target.Type->BoolFlag[BRIDGE_INDEX].value
	) {
	//Wyrmgus end
		HitUnit_RunAway(target, *attacker);
	}

	const int threshold = 30;

	if (target.Threshold && target.CurrentOrder()->HasGoal() && target.CurrentOrder()->GetGoal() == attacker) {
		target.Threshold = threshold;
		return;
	}

	//Wyrmgus start
//	if (target.Threshold == 0 && target.IsAgressive() && target.CanMove() && !target.ReCast) {
	if (
		target.Threshold == 0
		&& (target.IsAgressive() || (target.CanAttack() && target.Type->BoolFlag[COWARD_INDEX].value && (attacker->Type->BoolFlag[COWARD_INDEX].value || attacker->Variable[HP_INDEX].Value <= 3))) // attacks back if isn't coward, or if attacker is also coward, or if attacker has 3 HP or less 
		&& target.CanMove()
		&& !target.ReCast
		&& !attacker->Type->BoolFlag[INDESTRUCTIBLE_INDEX].value // don't attack indestructible units back
	) {
	//Wyrmgus end
		// Attack units in range (which or the attacker?)
		// Don't bother unit if it casting repeatable spell
		HitUnit_AttackBack(*attacker->GetFirstContainer(), target); //if the unit is in a container, attack it instead of the unit (which is removed and thus unreachable)
	}

	// What should we do with workers on :
	// case UnitAction::Repair:
	// Drop orders and run away or return after escape?
}

/*----------------------------------------------------------------------------
--  Conflicts
----------------------------------------------------------------------------*/

/**
 **	@brief	Returns the map distance between this unit and another one
 **
 **	@param	dst	The unit the distance to which is to be obtained
 **
 **	@return	The distance to the other unit, in tiles
 */
int CUnit::MapDistanceTo(const CUnit &dst) const
{
	if (this->MapLayer != dst.MapLayer) {
		return 16384;
	}
	
	return MapDistanceBetweenTypes(*this->GetFirstContainer()->Type, this->tilePos, this->MapLayer->ID, *dst.Type, dst.tilePos, dst.MapLayer->ID);
}

/**
 **  Returns the map distance to unit.
 **
 **  @param pos   map tile position.
 **
 **  @return      The distance between in tiles.
 */
int CUnit::MapDistanceTo(const Vec2i &pos, int z) const
{
	//Wyrmgus start
	if (z != this->MapLayer->ID) {
		return 16384;
	}
	//Wyrmgus end
	
	int dx;
	int dy;

	if (pos.x <= tilePos.x) {
		dx = tilePos.x - pos.x;
	//Wyrmgus start
	} else if (this->Container) { //if unit is within another, use the tile size of the transporter to calculate the distance
		dx = std::max<int>(0, pos.x - tilePos.x - this->Container->Type->get_tile_width() + 1);
	//Wyrmgus end
	} else {
		dx = std::max<int>(0, pos.x - tilePos.x - Type->get_tile_width() + 1);
	}
	if (pos.y <= tilePos.y) {
		dy = tilePos.y - pos.y;
	//Wyrmgus start
	} else if (this->Container) {
		dy = std::max<int>(0, pos.y - tilePos.y - this->Container->Type->get_tile_height() + 1);
	//Wyrmgus end
	} else {
		dy = std::max<int>(0, pos.y - tilePos.y - Type->get_tile_height() + 1);
	}
	return isqrt(dy * dy + dx * dx);
}

/**
**	@brief	Returns the map distance between two points with unit type
**
**	@param	src		Source unit type
**	@param	pos1	Map tile position of the source (upper-left)
**	@param	dst		Destination unit type to take into account
**	@param	pos2	Map tile position of the destination
**
**	@return	The distance between the types
*/
int MapDistanceBetweenTypes(const stratagus::unit_type &src, const Vec2i &pos1, int src_z, const stratagus::unit_type &dst, const Vec2i &pos2, int dst_z)
{
	return MapDistance(src.get_tile_size(), pos1, src_z, dst.get_tile_size(), pos2, dst_z);
}

int MapDistance(const Vec2i &src_size, const Vec2i &pos1, int src_z, const Vec2i &dst_size, const Vec2i &pos2, int dst_z)
{
	if (src_z != dst_z) {
		return 16384;
	}
	
	int dx;
	int dy;

	if (pos1.x + src_size.x <= pos2.x) {
		dx = std::max<int>(0, pos2.x - pos1.x - src_size.x + 1);
	} else {
		dx = std::max<int>(0, pos1.x - pos2.x - dst_size.x + 1);
	}
	if (pos1.y + src_size.y <= pos2.y) {
		dy = pos2.y - pos1.y - src_size.y + 1;
	} else {
		dy = std::max<int>(0, pos1.y - pos2.y - dst_size.y + 1);
	}
	return isqrt(dy * dy + dx * dx);
}

/**
**  Compute the distance from the view point to a given point.
**
**  @param pos  map tile position.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistance(const Vec2i &pos)
{
	const CViewport &vp = *UI.SelectedViewport;
	const Vec2i vpSize(vp.MapWidth, vp.MapHeight);
	const Vec2i middle = vp.MapPos + vpSize / 2;

	return Distance(middle, pos);
}

/**
**  Compute the distance from the view point to a given unit.
**
**  @param dest  Distance to this unit.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistanceToUnit(const CUnit &dest)
{
	const CViewport &vp = *UI.SelectedViewport;
	const Vec2i vpSize(vp.MapWidth, vp.MapHeight);
	const Vec2i midPos = vp.MapPos + vpSize / 2;

	//Wyrmgus start
//	return dest.MapDistanceTo(midPos);
	return dest.MapDistanceTo(midPos, UI.CurrentMapLayer->ID);
	//Wyrmgus end
}

/**
**  Can the source unit attack the destination unit.
**
**  @param source  Unit type pointer of the attacker.
**  @param dest    Unit type pointer of the target.
**
**  @return        0 if attacker can't target the unit, else a positive number.
*/
int CanTarget(const stratagus::unit_type &source, const stratagus::unit_type &dest)
{
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (source.BoolFlag[i].CanTargetFlag != CONDITION_TRUE) {
			if ((source.BoolFlag[i].CanTargetFlag == CONDITION_ONLY) ^
				(dest.BoolFlag[i].value)) {
				return 0;
			}
		}
	}
	if (dest.UnitType == UnitTypeType::Land) {
		if (dest.BoolFlag[SHOREBUILDING_INDEX].value) {
			return source.CanTarget & (CanTargetLand | CanTargetSea);
		}
		return source.CanTarget & CanTargetLand;
	}
	if (dest.UnitType == UnitTypeType::Fly || dest.UnitType == UnitTypeType::Space) {
		return source.CanTarget & CanTargetAir;
	}
	//Wyrmgus start
	if (dest.UnitType == UnitTypeType::FlyLow) {
		return (source.CanTarget & CanTargetLand) || (source.CanTarget & CanTargetAir) || (source.CanTarget & CanTargetSea);
	}
	//Wyrmgus end
	if (dest.UnitType == UnitTypeType::Naval) {
		return source.CanTarget & CanTargetSea;
	}
	return 0;
}

/**
**  Can the transporter transport the other unit.
**
**  @param transporter  Unit which is the transporter.
**  @param unit         Unit which wants to go in the transporter.
**
**  @return             1 if transporter can transport unit, 0 else.
*/
int CanTransport(const CUnit &transporter, const CUnit &unit)
{
	if (!transporter.Type->CanTransport()) {
		return 0;
	}
	if (transporter.CurrentAction() == UnitAction::Built) { // Under construction
		return 0;
	}
	if (&transporter == &unit) { // Cannot transporter itself.
		return 0;
	}
	//Wyrmgus start
	/*
	if (transporter.BoardCount >= transporter.Type->MaxOnBoard) { // full
		return 0;
	}
	*/
	
	if (transporter.ResourcesHeld > 0 && transporter.CurrentResource) { //cannot transport units if already has cargo
		return 0;
	}
	//Wyrmgus end

	if (transporter.BoardCount + unit.Type->BoardSize > transporter.Type->MaxOnBoard) { // too big unit
		return 0;
	}

	// Can transport only allied unit.
	// FIXME : should be parametrable.
	//Wyrmgus start
//	if (!transporter.IsTeamed(unit)) {
	if (!transporter.IsTeamed(unit) && !transporter.IsAllied(unit) && transporter.Player->Type != PlayerNeutral && unit.Player->Type != PlayerNeutral) {
	//Wyrmgus end
		return 0;
	}
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (transporter.Type->BoolFlag[i].CanTransport != CONDITION_TRUE) {
			if ((transporter.Type->BoolFlag[i].CanTransport == CONDITION_ONLY) ^ unit.Type->BoolFlag[i].value) {
				return 0;
			}
		}
	}
	return 1;
}

//Wyrmgus start
/**
**  Can the unit pick up the other unit.
**
**  @param picker		Unit which is the picker.
**  @param unit         Unit which wants to be picked.
**
**  @return             true if picker can pick up unit, false else.
*/
bool CanPickUp(const CUnit &picker, const CUnit &unit)
{
	if (!picker.Type->BoolFlag[ORGANIC_INDEX].value) { //only organic units can pick up power-ups and items
		return false;
	}
	if (!unit.Type->BoolFlag[ITEM_INDEX].value && !unit.Type->BoolFlag[POWERUP_INDEX].value) { //only item and powerup units can be picked up
		return false;
	}
	if (!unit.Type->BoolFlag[POWERUP_INDEX].value && !picker.HasInventory() && !stratagus::is_consumable_item_class(unit.Type->get_item_class())) { //only consumable items can be picked up as if they were power-ups for units with no inventory
		return false;
	}
	if (picker.CurrentAction() == UnitAction::Built) { // Under construction
		return false;
	}
	if (&picker == &unit) { // Cannot pick up itself.
		return false;
	}
	if (picker.HasInventory() && unit.Type->BoolFlag[ITEM_INDEX].value && picker.InsideCount >= ((int) UI.InventoryButtons.size())) { // full
		if (picker.Player == CPlayer::GetThisPlayer()) {
			std::string picker_name = picker.Name + "'s (" + picker.GetTypeName() + ")";
			picker.Player->Notify(NotifyRed, picker.tilePos, picker.MapLayer->ID, _("%s inventory is full."), picker_name.c_str());
		}
		return false;
	}

	return true;
}
//Wyrmgus end

/**
**  Check if the player is an enemy
**
**  @param player  Player to check
*/
bool CUnit::IsEnemy(const CPlayer &player) const
{
	//Wyrmgus start
	if (this->Player->Index != player.Index && player.Type != PlayerNeutral && !this->Player->HasBuildingAccess(player) && this->Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && this->IsAgressive()) {
		return true;
	}
	//Wyrmgus end
	
	return this->Player->IsEnemy(player);
}

/**
**  Check if the unit is an enemy
**
**  @param unit  Unit to check
*/
bool CUnit::IsEnemy(const CUnit &unit) const
{
	if (
		this->Player->Type == PlayerNeutral
		&& this->Type->BoolFlag[NEUTRAL_HOSTILE_INDEX].value
		&& unit.Player->Type != PlayerNeutral
		) {
		return true;
	}

	if (
		this->Player->Type == PlayerNeutral
		&& this->Type->BoolFlag[FAUNA_INDEX].value
		&& this->Type->BoolFlag[ORGANIC_INDEX].value
		&& unit.Type->BoolFlag[ORGANIC_INDEX].value
		&& this->Type->Slot != unit.Type->Slot
	) {
		if (
			this->Type->BoolFlag[PREDATOR_INDEX].value
			&& !unit.Type->BoolFlag[PREDATOR_INDEX].value
			&& this->CanEat(unit)
		) {
			return true;
		} else if (
			this->Type->BoolFlag[PEOPLEAVERSION_INDEX].value
			&& !unit.Type->BoolFlag[FAUNA_INDEX].value
			&& unit.Player->Type != PlayerNeutral
			&& this->MapDistanceTo(unit) <= 1
		) {
			return true;
		}
	}
		
	if (
		unit.Player->Type == PlayerNeutral
		&& (unit.Type->BoolFlag[NEUTRAL_HOSTILE_INDEX].value || unit.Type->BoolFlag[PREDATOR_INDEX].value)
		&& this->Player->Type != PlayerNeutral
	) {
		return true;
	}
	
	if (
		this->Player != unit.Player
		&& this->Player->Type != PlayerNeutral
		&& unit.CurrentAction() == UnitAction::Attack
		&& unit.CurrentOrder()->HasGoal()
		&& unit.CurrentOrder()->GetGoal()->Player == this->Player
		&& !unit.CurrentOrder()->GetGoal()->Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value
	) {
		return true;
	}
	
	if (
		this->Player != unit.Player && this->Player->Type != PlayerNeutral && unit.Player->Type != PlayerNeutral && !this->Player->HasBuildingAccess(*unit.Player) && !this->Player->HasNeutralFactionType()
		&& ((this->Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && this->IsAgressive()) || (unit.Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && unit.IsAgressive()))
	) {
		return true;
	}

	return IsEnemy(*unit.Player);
	//Wyrmgus end
}

/**
**  Check if the player is an ally
**
**  @param player  Player to check
*/
bool CUnit::IsAllied(const CPlayer &player) const
{
	return this->Player->IsAllied(player);
}

/**
**  Check if the unit is an ally
**
**  @param x  Unit to check
*/
bool CUnit::IsAllied(const CUnit &unit) const
{
	return IsAllied(*unit.Player);
}

/**
**  Check if unit shares vision with the player
**
**  @param x  Player to check
*/
bool CUnit::has_shared_vision_with(const CPlayer &player) const
{
	return this->Player->has_shared_vision_with(player);
}

/**
**  Check if the unit shares vision with the unit
**
**  @param x  Unit to check
*/
bool CUnit::has_shared_vision_with(const CUnit &unit) const
{
	return this->has_shared_vision_with(*unit.Player);
}

/**
**  Check if both players share vision
**
**  @param x  Player to check
*/
bool CUnit::has_mutual_shared_vision_with(const CPlayer &player) const
{
	return this->Player->has_mutual_shared_vision_with(player);
}

/**
**  Check if both units share vision
**
**  @param x  Unit to check
*/
bool CUnit::has_mutual_shared_vision_with(const CUnit &unit) const
{
	return this->has_mutual_shared_vision_with(*unit.Player);
}

/**
**  Check if the player is on the same team
**
**  @param x  Player to check
*/
bool CUnit::IsTeamed(const CPlayer &player) const
{
	return (this->Player->Team == player.Team);
}

/**
**  Check if the unit is on the same team
**
**  @param x  Unit to check
*/
bool CUnit::IsTeamed(const CUnit &unit) const
{
	return this->IsTeamed(*unit.Player);
}

/**
**  Check if the unit is unusable (for attacking...)
**  @todo look if correct used (UnitAction::Built is no problem if attacked)?
*/
bool CUnit::IsUnusable(bool ignore_built_state) const
{
	return (!IsAliveOnMap() || (!ignore_built_state && CurrentAction() == UnitAction::Built));
}

/**
**  Check if the unit attacking its goal will result in a ranged attack
*/
//Wyrmgus start
//bool CUnit::IsAttackRanged(CUnit *goal, const Vec2i &goalPos)
bool CUnit::IsAttackRanged(CUnit *goal, const Vec2i &goalPos, int z)
//Wyrmgus end
{
	if (this->Variable[ATTACKRANGE_INDEX].Value <= 1) { //always return false if the units attack range is 1 or lower
		return false;
	}
	
	if (this->Container) { //if the unit is inside a container, the attack will always be ranged
		return true;
	}
	
	const bool is_flying = this->Type->UnitType == UnitTypeType::Fly || this->Type->UnitType == UnitTypeType::Space;
	const bool is_goal_flying = goal->Type->UnitType == UnitTypeType::Fly || goal->Type->UnitType == UnitTypeType::Space;
	if (
		goal
		&& goal->IsAliveOnMap()
		&& (this->MapDistanceTo(*goal) > 1 || (is_flying != is_goal_flying))
	) {
		return true;
	}
	
	if (!goal && CMap::Map.Info.IsPointOnMap(goalPos, z) && this->MapDistanceTo(goalPos, z) > 1) {
		return true;
	}
	
	return false;
}

/*----------------------------------------------------------------------------
--  Initialize/Cleanup
----------------------------------------------------------------------------*/

/**
**  Initialize unit module.
*/
void InitUnits()
{
	if (!SaveGameLoading) {
		UnitManager.Init();
	}
}

/**
**  Clean up unit module.
*/
void CleanUnits()
{
	//  Free memory for all units in unit table.
	std::vector<CUnit *> units(UnitManager.begin(), UnitManager.end());

	for (std::vector<CUnit *>::iterator it = units.begin(); it != units.end(); ++it) {
		//Wyrmgus start
		if (*it == nullptr) {
			fprintf(stderr, "Error in CleanUnits: unit is null.\n");
			continue;
		}
		//Wyrmgus end
		CUnit &unit = **it;

		//Wyrmgus start
		/*
		if (&unit == nullptr) {
			continue;
		}
		*/
		//Wyrmgus end
		//Wyrmgus start
		if (unit.Type == nullptr) {
			fprintf(stderr, "Unit \"%d\"'s type is null.\n", UnitNumber(unit));
		}
		//Wyrmgus end
		if (!unit.Destroyed) {
			if (!unit.Removed) {
				unit.Remove(nullptr);
			}
			UnitClearOrders(unit);
		}
		unit.Release(true);
	}

	UnitManager.Init();

	HelpMeLastCycle = 0;
}
