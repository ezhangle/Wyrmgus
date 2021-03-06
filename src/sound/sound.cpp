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
//      (c) Copyright 1998-2020 by Lutz Sammer, Fabrice Rossi,
//		Jimmy Salmon and Andrettin
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

#include "sound/sound.h"

#include "action/action_resource.h"
#include "civilization.h"
#include "config.h"
#include "database/defines.h"
#include "faction.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/tileset.h"
#include "mod.h"
#include "missile.h"
#include "sound/sound_server.h"
#include "sound/unit_sound_type.h"
#include "ui/ui.h"
#include "unit/unit.h"
#include "util/container_util.h"
#include "util/vector_util.h"
#include "video.h"
#include "widgets.h"

/**
**  Various sounds used in game.
*/
GameSound GameSounds;

/**
**  Selection handling
*/
struct SelectionHandling {
	Origin Source;         /// origin of the sound
	stratagus::sound *sound;         /// last sound played by this unit
	unsigned char HowMany; /// number of sound played in this group
};

/// FIXME: docu
SelectionHandling SelectionHandler;

static int ViewPointOffset;      /// Distance to Volume Mapping
int DistanceSilent;              /// silent distance

/**
**  "Randomly" choose a sample from a sound group.
*/
static stratagus::sample *SimpleChooseSample(const stratagus::sound &sound)
{
	if (sound.Number == ONE_SOUND) {
		return sound.get_samples().front().get();
	} else {
		//FIXME: check for errors
		//FIXME: valid only in shared memory context (FrameCounter)
		return sound.get_samples()[FrameCounter % sound.Number].get();
	}
}

/**
**  Choose the sample to play
*/
static stratagus::sample *ChooseSample(stratagus::sound *sound, bool selection, Origin &source)
{
	stratagus::sample *result = nullptr;

	if (!sound || !SoundEnabled()) {
		return nullptr;
	}

	if (sound->Number == TWO_GROUPS) {
		// handle a special sound (selection)
		if (SelectionHandler.sound != nullptr && (SelectionHandler.Source.Base == source.Base && SelectionHandler.Source.Id == source.Id)) {
			if (SelectionHandler.sound == sound->get_first_sound()) {
				result = SimpleChooseSample(*SelectionHandler.sound);
				SelectionHandler.HowMany++;
				if (SelectionHandler.HowMany >= 3) {
					SelectionHandler.HowMany = 0;
					SelectionHandler.sound = sound->get_second_sound();
				}
			} else {
				//FIXME: checks for error
				// check whether the second group is really a group
				if (SelectionHandler.sound->Number > 1) {
					//Wyrmgus start
//					result = SelectionHandler.Sound->Sound.OneGroup[SelectionHandler.HowMany];
					result = SimpleChooseSample(*SelectionHandler.sound);
					//Wyrmgus end
					SelectionHandler.HowMany++;
					if (SelectionHandler.HowMany >= SelectionHandler.sound->Number) {
						SelectionHandler.HowMany = 0;
						SelectionHandler.sound = sound->get_first_sound();
					}
				} else {
					result = SelectionHandler.sound->get_samples().front().get();
					SelectionHandler.HowMany = 0;
					SelectionHandler.sound = sound->get_first_sound();
				}
			}
		} else {
			SelectionHandler.Source = source;
			SelectionHandler.sound = sound->get_first_sound();
			result = SimpleChooseSample(*SelectionHandler.sound);
			SelectionHandler.HowMany = 1;
		}
	} else {
		// normal sound/sound group handling
		result = SimpleChooseSample(*sound);
		if (SelectionHandler.Source.Base == source.Base && SelectionHandler.Source.Id == source.Id) {
			SelectionHandler.HowMany = 0;
			SelectionHandler.sound = nullptr;
		}
		if (selection) {
			SelectionHandler.Source = source;
		}
	}

	return result;
}

/**
**  Maps a unit_sound_type to a sound*.
**
**  @param unit    Sound initiator
**  @param voice   Type of sound wanted
**
**  @return        Sound identifier
*/
static stratagus::sound *ChooseUnitVoiceSound(const CUnit &unit, const stratagus::unit_sound_type unit_sound_type)
{
	const CMapField &mf = *unit.MapLayer->Field(unit.tilePos);

	const stratagus::civilization *civilization = unit.Type->get_civilization();
	if (civilization != nullptr && unit.Player->Race != -1 && unit.Player->Race != civilization->ID && unit.Player->Faction != -1 && unit.Type == stratagus::faction::get_all()[unit.Player->Faction]->get_class_unit_type(unit.Type->get_unit_class())) {
		civilization = stratagus::civilization::get_all()[unit.Player->Race];
	}

	switch (unit_sound_type) {
		case stratagus::unit_sound_type::acknowledging:
			if (unit.Type->MapSound.Acknowledgement.Sound) {
				return unit.Type->MapSound.Acknowledgement.Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				return civilization->UnitSounds.Acknowledgement.Sound;
			} else {
				return nullptr;
			}
		case stratagus::unit_sound_type::attack:
			if (unit.Type->MapSound.Attack.Sound) {
				return unit.Type->MapSound.Attack.Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				if (civilization->UnitSounds.Attack.Sound) {
					return civilization->UnitSounds.Attack.Sound;
				}
			}

			return ChooseUnitVoiceSound(unit, stratagus::unit_sound_type::acknowledging);
		case stratagus::unit_sound_type::idle:
			return unit.Type->MapSound.Idle.Sound;
		case stratagus::unit_sound_type::hit:
			return unit.Type->MapSound.Hit.Sound;
		case stratagus::unit_sound_type::miss:
			if (unit.Type->MapSound.Miss.Sound) {
				return unit.Type->MapSound.Miss.Sound;
			} else {
				return unit.Type->MapSound.Hit.Sound;
			}
		case stratagus::unit_sound_type::fire_missile:
			return unit.Type->MapSound.FireMissile.Sound;
		case stratagus::unit_sound_type::step:
			if (unit.Type->MapSound.StepMud.Sound && ((mf.getFlag() & MapFieldMud) || (mf.getFlag() & MapFieldSnow))) {
				return unit.Type->MapSound.StepMud.Sound;
			} else if (unit.Type->MapSound.StepDirt.Sound && ((mf.getFlag() & MapFieldDirt) || (mf.getFlag() & MapFieldIce))) {
				return unit.Type->MapSound.StepDirt.Sound;
			} else if (unit.Type->MapSound.StepGravel.Sound && mf.getFlag() & MapFieldGravel) {
				return unit.Type->MapSound.StepGravel.Sound;
			} else if (unit.Type->MapSound.StepGrass.Sound && ((mf.getFlag() & MapFieldGrass) || (mf.getFlag() & MapFieldStumps))) {
				return unit.Type->MapSound.StepGrass.Sound;
			} else if (unit.Type->MapSound.StepStone.Sound && mf.getFlag() & MapFieldStoneFloor) {
				return unit.Type->MapSound.StepStone.Sound;
			} else {
				return unit.Type->MapSound.Step.Sound;
			}
		case stratagus::unit_sound_type::used:
			return unit.Type->MapSound.Used.Sound;
		case stratagus::unit_sound_type::build:
			if (unit.Type->MapSound.Build.Sound) {
				return unit.Type->MapSound.Build.Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				if (civilization->UnitSounds.Build.Sound) {
					return civilization->UnitSounds.Build.Sound;
				}
			}
			
			return ChooseUnitVoiceSound(unit, stratagus::unit_sound_type::acknowledging);
		case stratagus::unit_sound_type::ready:
			if (unit.Type->MapSound.Ready.Sound) {
				return unit.Type->MapSound.Ready.Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				return civilization->UnitSounds.Ready.Sound;
			} else {
				return nullptr;
			}
		case stratagus::unit_sound_type::selected:
			if (unit.Type->MapSound.Selected.Sound) {
				return unit.Type->MapSound.Selected.Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				return civilization->UnitSounds.Selected.Sound;
			} else {
				return nullptr;
			}
		case stratagus::unit_sound_type::help:
			if (unit.Type->MapSound.Help.Sound) {
				return unit.Type->MapSound.Help.Sound;
			} else if (civilization != nullptr) {
				if (unit.Type->BoolFlag[BUILDING_INDEX].value && civilization->UnitSounds.HelpTown.Sound) {
					return civilization->UnitSounds.HelpTown.Sound;
				} else {
					return civilization->UnitSounds.Help.Sound;
				}
			} else {
				return nullptr;
			}
		case stratagus::unit_sound_type::dying:
			if (unit.Type->MapSound.Dead[unit.DamagedType].Sound) {
				return unit.Type->MapSound.Dead[unit.DamagedType].Sound;
			} else if (unit.Type->MapSound.Dead[ANIMATIONS_DEATHTYPES].Sound) {
				return unit.Type->MapSound.Dead[ANIMATIONS_DEATHTYPES].Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				if (civilization->UnitSounds.Dead[unit.DamagedType].Sound) {
					return civilization->UnitSounds.Dead[unit.DamagedType].Sound;
				} else {
					return civilization->UnitSounds.Dead[ANIMATIONS_DEATHTYPES].Sound;
				}
			} else {
				return nullptr;
			}
		case stratagus::unit_sound_type::work_completed:
			return GameSounds.WorkComplete[CPlayer::GetThisPlayer()->Race].Sound;
		case stratagus::unit_sound_type::construction:
			return GameSounds.BuildingConstruction[CPlayer::GetThisPlayer()->Race].Sound;
		case stratagus::unit_sound_type::docking:
			return GameSounds.Docking.Sound;
		case stratagus::unit_sound_type::repairing:
			if (unit.Type->MapSound.Repair.Sound) {
				return unit.Type->MapSound.Repair.Sound;
			} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
				if (civilization->UnitSounds.Repair.Sound) {
					return civilization->UnitSounds.Repair.Sound;
				}
			}
			
			return ChooseUnitVoiceSound(unit, stratagus::unit_sound_type::acknowledging);
		case stratagus::unit_sound_type::harvesting:
			for (size_t i = 0; i != unit.Orders.size(); ++i) {
				if (unit.Orders[i]->Action == UnitAction::Resource) {
					COrder_Resource &order = dynamic_cast<COrder_Resource &>(*unit.Orders[i]);
					if (unit.Type->MapSound.Harvest[order.GetCurrentResource()].Sound) {
						return unit.Type->MapSound.Harvest[order.GetCurrentResource()].Sound;
					} else if (unit.Type->BoolFlag[ORGANIC_INDEX].value && civilization != nullptr) {
						if (civilization->UnitSounds.Harvest[order.GetCurrentResource()].Sound) {
							return civilization->UnitSounds.Harvest[order.GetCurrentResource()].Sound;
						}
					}

					return ChooseUnitVoiceSound(unit, stratagus::unit_sound_type::acknowledging);
				}
			}
			break;
	}

	return nullptr;
}

/**
**  Compute a suitable volume for something taking place at a given
**  distance from the current view point.
**
**  @param d      distance
**  @param range  range
**
**  @return       volume for given distance (0..??)
*/
unsigned char VolumeForDistance(unsigned short d, unsigned char range)
{
	// FIXME: THIS IS SLOW!!!!!!!
	if (d <= ViewPointOffset || range == INFINITE_SOUND_RANGE) {
		return MaxVolume;
	} else {
		if (range) {
			d -= ViewPointOffset;
			int d_tmp = d * MAX_SOUND_RANGE;
			int range_tmp = DistanceSilent * range;
			if (d_tmp > range_tmp) {
				return 0;
			} else {
				return (unsigned char)((range_tmp - d_tmp) * MAX_SOUND_RANGE / range_tmp);
			}
		} else {
			return 0;
		}
	}
}

/**
**  Calculate the volume associated with a request, either by clipping the
**  range parameter of this request, or by mapping this range to a volume.
*/
unsigned char CalculateVolume(bool isVolume, int power, unsigned char range)
{
	if (isVolume) {
		return std::min(MaxVolume, power);
	} else {
		// map distance to volume
		return VolumeForDistance(power, range);
	}
}

/**
**  Calculate the stereo value for a unit
*/
static char CalculateStereo(const CUnit &unit)
{
	int stereo = ((unit.tilePos.x * stratagus::defines::get()->get_scaled_tile_width() + unit.Type->get_tile_width() * stratagus::defines::get()->get_scaled_tile_width() / 2 +
				   unit.get_scaled_pixel_offset().x() - UI.SelectedViewport->MapPos.x * stratagus::defines::get()->get_scaled_tile_width()) * 256 /
				  ((UI.SelectedViewport->MapWidth - 1) * stratagus::defines::get()->get_scaled_tile_width())) - 128;
	clamp(&stereo, -128, 127);
	return stereo;
}

/**
**  Ask to the sound server to play a sound attached to a unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param voice  Type of sound wanted (Ready,Die,Yes,...)
*/
void PlayUnitSound(const CUnit &unit, const stratagus::unit_sound_type unit_sound_type)
{
	if (!UI.CurrentMapLayer || unit.MapLayer != UI.CurrentMapLayer) {
		return;
	}
	
	if (unit.Variable[STUN_INDEX].Value > 0 && stratagus::is_voice_unit_sound_type(unit_sound_type)) { //don't speak if stunned
		return;
	}
	
	stratagus::sound *sound = ChooseUnitVoiceSound(unit, unit_sound_type);
	if (!sound) {
		return;
	}

	bool selection = (unit_sound_type == stratagus::unit_sound_type::selected || unit_sound_type == stratagus::unit_sound_type::construction);
	Origin source = {&unit, unsigned(UnitNumber(unit))};
	
	//don't speak if already speaking
	if (stratagus::is_voice_unit_sound_type(unit_sound_type) && UnitSoundIsPlaying(&source)) {
		return;
	}

	const int volume = CalculateVolume(false, ViewPointDistanceToUnit(unit), stratagus::get_unit_sound_type_range(unit_sound_type)) * sound->VolumePercent / 100;

	if (volume == 0) {
		return;
	}

	int channel = PlaySample(ChooseSample(sound, selection, source), &source);
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, volume);
	SetChannelStereo(channel, CalculateStereo(unit));
	SetChannelVoiceGroup(channel, unit_sound_type);
}

/**
**  Ask to the sound server to play a sound attached to a unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param sound  Sound to be generated
*/
void PlayUnitSound(const CUnit &unit, stratagus::sound *sound)
{
	//Wyrmgus start
	if (!&unit) {
		fprintf(stderr, "Error in PlayUnitSound: unit is null.\n");
		return;
	}
	
	if (!sound) {
		return;
	}

	if (unit.MapLayer != UI.CurrentMapLayer) {
		return;
	}

	Origin source = {&unit, unsigned(UnitNumber(unit))};

	const int volume = CalculateVolume(false, ViewPointDistanceToUnit(unit), sound->range) * sound->VolumePercent / 100;

	if (volume == 0) {
		return;
	}

	int channel = PlaySample(ChooseSample(sound, false, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, volume);
	SetChannelStereo(channel, CalculateStereo(unit));
}

/**
**  Ask the sound server to play a sound for a missile.
**
**  @param missile  Sound initiator, missile exploding
**  @param sound    Sound to be generated
*/
void PlayMissileSound(const Missile &missile, stratagus::sound *sound)
{
	if (!sound) {
		return;
	}
	int stereo = ((missile.position.x + (missile.Type->G ? missile.Type->G->Width / 2 : 0) +
				   UI.SelectedViewport->MapPos.x * stratagus::defines::get()->get_tile_width()) * 256 /
				  ((UI.SelectedViewport->MapWidth - 1) * stratagus::defines::get()->get_tile_width())) - 128;
	clamp(&stereo, -128, 127);

	Origin source = {nullptr, 0};
	const int volume = CalculateVolume(false, ViewPointDistanceToMissile(missile), sound->range) * sound->VolumePercent / 100;

	if (volume == 0) {
		return;
	}

	int channel = PlaySample(ChooseSample(sound, false, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, volume);
	SetChannelStereo(channel, stereo);
}

/**
**  Play a game sound
**
**  @param sound   Sound to play
**  @param volume  Volume level to play the sound
*/
void PlayGameSound(stratagus::sound *sound, unsigned char volume, bool always)
{
	if (!sound) {
		return;
	}
	Origin source = {nullptr, 0};

	stratagus::sample *sample = ChooseSample(sound, false, source);

	if (!always && SampleIsPlaying(sample)) {
		return;
	}

	volume = CalculateVolume(true, volume, sound->range) * sound->VolumePercent / 100;
	if (volume == 0) {
		return;
	}

	int channel = PlaySample(sample);
	if (channel == -1) {
		return;
	}

	SetChannelVolume(channel, volume);
}

static std::map<int, LuaActionListener *> ChannelMap;

/**
**  Callback for PlaySoundFile
*/
static void PlaySoundFileCallback(int channel)
{
	LuaActionListener *listener = ChannelMap[channel];
	if (listener != nullptr) {
		listener->action("");
		ChannelMap[channel] = nullptr;
	}
	delete GetChannelSample(channel);
}

/**
**  Ask the sound server to change the range of a sound.
**
**  @param sound  the id of the sound to modify.
**  @param range  the new range for this sound.
*/
void SetSoundRange(stratagus::sound *sound, unsigned char range)
{
	if (sound != nullptr) {
		sound->range = range;
	}
}

//Wyrmgus start
/**
**  Ask the sound server to change the volume percent of a sound.
**
**  @param sound  the id of the sound to modify.
**  @param volume_percent  the new volume percent for this sound.
*/
void SetSoundVolumePercent(stratagus::sound *sound, int volume_percent)
{
	if (sound != nullptr) {
		sound->VolumePercent = volume_percent;
	}
}
//Wyrmgus end

/**
**  Ask the sound server to register a sound (and currently to load it)
**  and to return an unique identifier for it. The unique identifier is
**  memory pointer of the server.
**
**  @param files   An array of wav files.
**  @param number  Number of files belonging together.
**
**  @return        the sound unique identifier
**
**  @todo FIXME: Must handle the errors better.
*/
stratagus::sound *RegisterSound(const std::string &identifier, const std::vector<std::filesystem::path> &files)
{
	stratagus::sound *id = stratagus::sound::add(identifier, nullptr);
	size_t number = files.size();

	id->files = files;
	id->initialize();

	return id;
}

/**
**  Ask the sound server to put together two sounds to form a special sound.
**
**  @param first   first part of the group
**  @param second  second part of the group
**
**  @return        the special sound unique identifier
*/
stratagus::sound *RegisterTwoGroups(const std::string &identifier, stratagus::sound *first, stratagus::sound *second)
{
	if (first == nullptr || second == nullptr) {
		return nullptr;
	}
	stratagus::sound *id = stratagus::sound::add(identifier, nullptr);
	id->Number = TWO_GROUPS;
	id->set_first_sound(first);
	id->set_second_sound(second);
	id->range = MAX_SOUND_RANGE;
	//Wyrmgus start
	id->VolumePercent = first->VolumePercent + second->VolumePercent / 2;
	//Wyrmgus end

	return id;
}

/**
**  Lookup the sound id's for the game sounds.
*/
void InitSoundClient()
{
	if (!SoundEnabled()) { // No sound enabled
		return;
	}
	// let's map game sounds, look if already setup in ccl.

	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.PlacementError[i].Sound) {
			GameSounds.PlacementError[i].MapSound();
		}
	}

	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.PlacementSuccess[i].Sound) {
			GameSounds.PlacementSuccess[i].MapSound();
		}
	}

	if (!GameSounds.Click.Sound) {
		GameSounds.Click.MapSound();
	}
	if (!GameSounds.Docking.Sound) {
		GameSounds.Docking.MapSound();
	}

	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.BuildingConstruction[i].Sound) {
			GameSounds.BuildingConstruction[i].MapSound();
		}
	}
	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.WorkComplete[i].Sound) {
			GameSounds.WorkComplete[i].MapSound();
		}
	}
	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.ResearchComplete[i].Sound) {
			GameSounds.ResearchComplete[i].MapSound();
		}
	}
	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		for (unsigned int j = 0; j < MaxCosts; ++j) {
			if (!GameSounds.NotEnoughRes[i][j].Sound) {
				GameSounds.NotEnoughRes[i][j].MapSound();
			}
		}
	}
	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.NotEnoughFood[i].Sound) {
			GameSounds.NotEnoughFood[i].MapSound();
		}
	}
	for (size_t i = 0; i < stratagus::civilization::get_all().size(); ++i) {
		if (!GameSounds.Rescue[i].Sound) {
			GameSounds.Rescue[i].MapSound();
		}
	}
	if (!GameSounds.ChatMessage.Sound) {
		GameSounds.ChatMessage.MapSound();
	}

	int MapWidth = (UI.MapArea.EndX - UI.MapArea.X + stratagus::defines::get()->get_scaled_tile_width()) / stratagus::defines::get()->get_scaled_tile_width();
	int MapHeight = (UI.MapArea.EndY - UI.MapArea.Y + stratagus::defines::get()->get_scaled_tile_height()) / stratagus::defines::get()->get_scaled_tile_height();
	DistanceSilent = 3 * std::max<int>(MapWidth, MapHeight);
	ViewPointOffset = std::max<int>(MapWidth / 2, MapHeight / 2);
}

namespace stratagus {

void sound::initialize_all()
{
	sample::initialize_decoding_loop();

	data_type::initialize_all();

	sample::run_decoding_loop();
	sample::clear_decoders();
}

sound::sound(const std::string &identifier) : data_entry(identifier)
{
}

sound::~sound()
{
}

void sound::initialize()
{
	const size_t file_count = this->get_files().size();
	if (file_count > 1) { // sound group
		this->Number = file_count;
	} else if (file_count == 1) { // unique sound
		this->Number = ONE_SOUND;
	} else if (this->get_first_sound() != nullptr && this->get_second_sound() != nullptr) {
		this->Number = TWO_GROUPS;
	} else {
		throw std::runtime_error("Sound \"" + this->get_identifier() + "\" is neither a sound group, nor does it have any files.");
	}

	for (const std::filesystem::path &filepath : this->get_files()) {
		this->samples.push_back(LoadSample(filepath));
	}

	data_entry::initialize();
}

QVariantList sound::get_files_qvariant_list() const
{
	return container::to_qvariant_list(this->get_files());
}

void sound::add_file(const std::filesystem::path &filepath)
{
	this->files.push_back(database::get_sounds_path(this->get_module()) / filepath);
}

void sound::remove_file(const std::filesystem::path &filepath)
{
	vector::remove(this->files, filepath);
}

}