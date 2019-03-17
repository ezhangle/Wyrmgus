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
/**@name missile.h - The missile header file. */
//
//      (c) Copyright 1998-2019 by Lutz Sammer and Andrettin
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

#ifndef __MISSILE_H__
#define __MISSILE_H__

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class MissileType missile.h
**
**  \#include "missile/missile.h"
**
**  This structure defines the base type information of all missiles. It
**  contains all information that all missiles of the same type shares.
**  The fields are filled from the configuration files.
**
**
**  The missile-type structure members:
**
**  MissileType::Ident
**
**    Unique identifier of the missile-type, used to reference it in
**    config files and during startup.
**    @note Don't use this member in game, use instead the pointer
**    to this structure. See MissileTypeByIdent().
**
**  MissileType::DrawLevel
**
**    The Level/Order to draw the missile in, usually 0-255
**
**  MissileType::Width MissileType::Height
**
**    Size (width and height) of a frame in the image. All sprite
**    frames of the missile-type must have the same size.
**
**  MissileType::SpriteFrames
**
**    Total number of sprite frames in the graphic image.
**    @note If the image is smaller than the number of directions,
**    width/height and/or framecount suggest the engine crashes.
**    @note There is currently a limit of 127 sprite frames, which
**    can be lifted if needed.
**
**  MissileType::NumDirections
**
**    Number of directions missile can face.
**
**  MissileType::Transparency
**
**    Set a missile transparency.
**
**  MissileType::FiredSound
**
**    Sound of the missile, if fired. @note currently not used.
**
**  MissileType::ImpactSound
**
**    Impact sound for this missile.
**
**  MissileType::CanHitOwner
**
**    Can hit the unit that have fired the missile.
**    @note Currently no missile can hurt its owner.
**
**  MissileType::FriendlyFire
**
**    Can't hit the units of the same player, that has the
**    missile fired.
**
**  MissileType::Class
**
**    Class of the missile-type, defines the basic effects of the
**    missile. Look at the different class identifiers for more
**    information (::MissileClassNone, ...).
**
**  MissileType::NumBounces
**
**    This is the number of bounces, and it is only valid with
**    MissileClassBounce. The missile will hit this many times in
**    a row.
**
**  MissileType::StartDelay
**
**    Delay in game cycles after the missile generation, until the
**    missile animation and effects starts. Delay denotes the number
**    of display cycles to skip before drawing the first sprite frame
**    and only happens once at start.
**
**  MissileType::Sleep
**
**    This are the number of game cycles to wait for the next
**    animation or the sleeping between the animation steps.
**    All animations steps use the same delay.  0 is the
**    fastest and 255 the slowest animation.
**    @note Perhaps we should later allow animation scripts for
**    more complex animations.
**
**  MissileType::Speed
**
**    The speed how fast the missile moves. 0 the missile didn't
**    move, 1 is the slowest speed and 32 s the fastest supported
**    speed. This is how many pixels the missiles moves with each
**    animation step.  The real use of this member depends on the
**    MissileType::Class
**    @note This is currently only used by the point-to-point
**    missiles (::MissileClassPointToPoint, ...).  Perhaps we should
**    later allow animation scripts for more complex animations.
**
**  MissileType::Range
**
**    Determines the range in which a projectile will deal its damage.
**    A range of 0 will mean that the damage will be limited to the
**    targeted unit only.  So if you shot a missile at a unit, it
**    would only damage that unit.  A value of 1 only affects the
**    field where the missile hits.  A value of 2  would mean that
**    the damage for that particular missile would be dealt for a range
**    of 1 around the impact spot. All fields that aren't the center
**    get only 1/SpashFactor of the damage. Fields 2 away get
**    1/(SplashFactor*2), and following...
**
**  MissileType::SplashFactor
**
**    Determines The Splash damage divisor, see Range
**
**  MissileType::Impact
**
**    The config of the next (other) missile to generate, when this
**    missile reached its end point or its life time is over.  So it
**    can be used to generate a chain of missiles.
**
**  MissileType::Smoke
**
**    The config of the next (other) missile to generate a trailing smoke.  So it
**    can be used to generate a chain of missiles.
**
**  MissileType::Sprite
**
**    Missile sprite image loaded from MissileType::File
**
**  MissileType::G
**
**    File containing the image (sprite) graphics of the missile.
**    The file can contain multiple sprite frames.  The sprite frames
**    for the different directions are placed in the row.
**    The different animations steps are placed in the column. But
**    the correct order depends on MissileType::Class. Missiles like fire
**    have no directions, missiles like arrows have a direction.
*/

/**
**  @class Missile missile.h
**
**  \#include "missile/missile.h"
**
**  This structure contains all information about a missile in game.
**  A missile could have different effects, based on its missile-type.
**  Missiles could be saved and stored with CCL. See (missile).
**  Currently only a tile, a unit or a missile could be placed on the map.
**
**
**  The missile structure members:
**
**  Missile::position
**
**    Missile current map position in pixels.
**
**  Missile::source
**
**    Missile original map position in pixels.
**
**  Missile::destination
**
**    Missile destination on the map in pixels.  If
**    Missile::X==Missile::DX and Missile::Y==Missile::DY the missile
**    stays at its position.  But the movement also depends on
**    MissileType::Class.
**
**  Missile::Type
**
**    ::MissileType pointer of the missile, contains the shared
**    information of all missiles of the same type.
**
**  Missile::SpriteFrame
**
**    Current sprite frame of the missile.  The range is from 0
**    to MissileType::SpriteFrames-1.  The topmost bit (128) is
**    used as flag to mirror the sprites in X direction.
**    Animation scripts aren't currently supported for missiles,
**    everything is handled by MissileType::Class
**    @note If wanted, we can add animation scripts support to the
**    engine.
**
**  Missile::State
**
**    Current state of the missile.  Used for a simple state machine.
**
**  Missile::AnimWait
**
**    Animation wait. Used internally by missile actions, to run the
**    animation in parallel with the rest.
**
**  Missile::Wait
**
**    Wait this number of game cycles until the next state or
**    animation of this missile is handled. This counts down from
**    MissileType::Sleep to 0.
**
**  Missile::Delay
**
**    Number of game cycles the missile isn't shown on the map.
**    This counts down from MissileType::StartDelay to 0, before this
**    happens the missile isn't shown and has no effects.
**    @note This can also be used by MissileType::Class
**    for temporary removement of the missile.
**
**  Missile::SourceUnit
**
**    The owner of the missile. Normally the one who fired the
**    missile.  Used to check units, to prevent hitting the owner
**    when field MissileType::CanHitOwner==true. Also used for kill
**    and experience points.
**
**  Missile::TargetUnit
**
**    The target of the missile.  Normally the unit which should be
**    hit by the missile.
**
**  Missile::Damage
**
**    Damage done by missile. See also MissileType::Range, which
**    denoted the 100% damage in center.
**
**  Missile::TTL
**
**    Time to live in game cycles of the missile, if it reaches zero
**    the missile is automatic removed from the map. If -1 the
**    missile lives for ever and the lifetime is handled by
**    Missile::Type:MissileType::Class
**
**  Missile::Hidden
**
**    When you set this to 1 the unit becomes hidden for a while.
**
**  Missile::CurrentStep
**
**    Movement step. Used for the different trajectories.
**
**  Missile::TotalStep
**
**    Maximum number of step. When CurrentStep >= TotalStep, the movement is finished.
**
**  Missile::Local
**
**    This is a local missile, which can be different on all
**    computer in play. Used for the user interface (fe the green
**    cross).
**
**  Missile::MissileSlot
**
**    Pointer to the slot of this missile. Used for faster freeing.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_type.h"
#include "missile/missile_config.h"
#include "script.h"
#include "sound/unit_sound.h"
#include "unit/unit_ptr.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CUnit;
class CViewport;
class CFile;
class LuaCallback;

/*----------------------------------------------------------------------------
--  Missile-type
----------------------------------------------------------------------------*/

/**
**  Missile-class this defines how a missile-type reacts.
*/
enum {
	MissileClassNone,                     /// Missile does nothing
	MissileClassPointToPoint,             /// Missile flies from x,y to x1,y1
	MissileClassPointToPointWithHit,      /// Missile flies from x,y to x1,y1 than shows hit animation.
	MissileClassPointToPointCycleOnce,    /// Missile flies from x,y to x1,y1 and animates ONCE from start to finish and back
	MissileClassPointToPointBounce,       /// Missile flies from x,y to x1,y1 than bounces three times.
	MissileClassStay,                     /// Missile appears at x,y, does it's anim and vanishes.
	MissileClassCycleOnce,                /// Missile appears at x,y, then cycle through the frames once.
	MissileClassFire,                     /// Missile doesn't move, than checks the source unit for HP.
	MissileClassHit,                      /// Missile shows the hit points.
	MissileClassParabolic,                /// Missile flies from x,y to x1,y1 using a parabolic path
	MissileClassLandMine,                 /// Missile wait on x,y until a non-air unit comes by, the explodes.
	MissileClassWhirlwind,                /// Missile appears at x,y, is whirlwind
	MissileClassFlameShield,              /// Missile surround x,y
	MissileClassDeathCoil,                /// Missile is death coil.
	MissileClassTracer,                   /// Missile seeks towards to target unit
	MissileClassClipToTarget,             /// Missile remains clipped to target's current goal and plays his animation once
	MissileClassContinious,               /// Missile stays and plays it's animation several times
	MissileClassStraightFly               /// Missile flies from x,y to x1,y1 then continues to fly, until incompatible terrain is detected
};

/// Base structure of missile-types
class MissileType : public CDataType
{
public:
	explicit MissileType(const std::string &ident);
	~MissileType();

	static const char *MissileClassNames[];
	
	virtual void ProcessConfigData(const CConfigData *config_data) override;
	
	/// load the graphics for a missile type
	void LoadMissileSprite();
	void Init();
	void DrawMissileType(int frame, const PixelPos &pos) const;

	void Load(lua_State *l);

	int Width() const { return size.x; }
	int Height() const { return size.y; }

	//private:
	int Transparency = 0;		/// missile transparency
	PixelSize size = PixelSize(0, 0);	/// missile size in pixels
	int DrawLevel = 0;			/// Level to draw missile at
	int SpriteFrames = 0;		/// number of sprite frames in graphic
	int NumDirections = 1;		/// number of directions missile can face
	int ChangeVariable = -1;		/// variable to change
	int ChangeAmount = 0;		/// how many to change
	bool ChangeMax = false;		/// modify the max, if value will exceed it

	SoundConfig FiredSound;	/// fired sound
	SoundConfig ImpactSound;	/// impact sound for this missile-type

	bool CorrectSphashDamage = false;	/// restricts the radius damage depending on land, air, naval
	bool Flip = true;			/// flip image when facing left
	bool CanHitOwner = false;	/// missile can hit the owner
	bool FriendlyFire = true;	/// missile can't hit own units
	bool AlwaysFire = false;	/// missile will always fire (even if target is dead)
	bool Pierce = false;		/// missile will hit every unit on his way
	bool PierceOnce = false;	/// pierce every target only once
	bool PierceIgnoreBeforeGoal = false;	/// only pierce targets after the goal
	bool IgnoreWalls = true;	/// missile ignores Wall units on it's way
	bool KillFirstUnit = false;	/// missile kills first unit blocking it's way
	//Wyrmgus start
	bool AlwaysHits = false;	/// missile never misses
	//Wyrmgus end

	int Class;					/// missile class
	int NumBounces = 0;			/// number of bounces
	int MaxBounceSize = 0;		/// if the unit has a size greater than this, the missile won't bounce further
	int ParabolCoefficient = 2048;	/// parabol coefficient in parabolic missile
	int StartDelay = 0;			/// missile start delay
	int Sleep = 0;				/// missile sleep
	int Speed = 0;				/// missile speed
	int BlizzardSpeed = 0;		/// speed for blizzard shards
	//Wyrmgus start
	int AttackSpeed = 10;		/// attack speed; used by whirlwind missiles
	//Wyrmgus end
	int TTL = -1;				/// missile time-to-live
	int ReduceFactor = 100;		/// Multiplier for reduce or increase damage dealt to the next unit
	int SmokePrecision = 0;		/// How frequently the smoke missile will generate itself
	int MissileStopFlags = 0;	/// On which terrain types missile won't fly
	NumberDesc *Damage = nullptr;	/// missile damage (used for non-direct missiles, e.g. impacts)

	int Range = 0;				/// missile damage range
	int SplashFactor = 100;		/// missile splash divisor
	std::vector <MissileConfig *> Impact;	/// missile produces an impact
	MissileConfig Smoke;		/// trailing missile
	LuaCallback *OnImpact = nullptr;	/// called when

	// --- FILLED UP ---
	CGraphic *G = nullptr;		/// missile graphic
};

/*----------------------------------------------------------------------------
--  Missile
----------------------------------------------------------------------------*/

/// Missile on the map
class Missile
{
protected:
	Missile();

public:
	virtual ~Missile();

	//Wyrmgus start
//	static Missile *Init(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos);
	static Missile *Init(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos, int z);
	//Wyrmgus end

	virtual void Action() = 0;

	void DrawMissile(const CViewport &vp) const;
	void SaveMissile(CFile &file) const;
	void MissileHit(CUnit *unit = nullptr);
	bool NextMissileFrame(char sign, char longAnimation);
	void NextMissileFrameCycle();
	void MissileNewHeadingFromXY(const PixelPos &delta);


	//private:
	PixelPos source = PixelPos(0, 0); 	/// Missile source position
	PixelPos position = PixelPos(0, 0);	/// missile pixel position
	PixelPos destination = PixelPos(0, 0);	/// missile pixel destination
	const MissileType *Type = nullptr;	/// missile-type pointer
	int SpriteFrame = 0;	/// sprite frame counter
	int State = 0;			/// state
	int AnimWait = 0;		/// Animation wait.
	int Wait = 0;			/// delay between frames
	int Delay = 0;			/// delay to show up
	//Wyrmgus start
	int MapLayer;		/// map layer the missile is in
	//Wyrmgus end

	CUnitPtr SourceUnit;	/// unit that fires (could be killed)
	CUnitPtr TargetUnit;	/// target unit, used for spells

	std::vector<CUnit *> PiercedUnits;	/// Units which are already pierced by this missile

	int Damage = 0;				/// direct damage that the missile applies
	int LightningDamage = 0;	/// direct lightning damage that the missile applies

	int TTL = -1;		/// time to live (ticks) used for spells
	int Hidden = 0;		/// If this is 1 then the missile is invisible
	int DestroyMissile = 0;	/// this tells missile-class-straight-fly, that it's time to die

	// Internal use:
	int CurrentStep = 0;	/// Current step (0 <= x < TotalStep).
	int TotalStep = 0;		/// Total step.
	
	//Wyrmgus start
	bool AlwaysHits = false;		/// Missile always hits
	bool AlwaysCritical = false;	/// Whether the missile always results in a critical hit
	//Wyrmgus end

	unsigned Local : 1;		/// missile is a local missile
	unsigned int Slot;		/// unique number for draw level.

	static unsigned int Count;	/// slot number generator.
};

extern bool MissileInitMove(Missile &missile);
extern bool PointToPointMissile(Missile &missile);
extern bool IsPiercedUnit(const Missile &missile, const CUnit &unit);
extern void MissileHandlePierce(Missile &missile, const Vec2i &pos);
extern bool MissileHandleBlocking(Missile &missile, const PixelPos &position);

class MissileNone : public Missile
{
public:
	virtual void Action();
};
class MissilePointToPoint : public Missile
{
public:
	virtual void Action();
};
class MissilePointToPointWithHit : public Missile
{
public:
	virtual void Action();
};
class MissilePointToPointCycleOnce : public Missile
{
public:
	virtual void Action();
};
class MissilePointToPointBounce : public Missile
{
public:
	virtual void Action();
};
class MissileStay : public Missile
{
public:
	virtual void Action();
};
class MissileCycleOnce : public Missile
{
public:
	virtual void Action();
};
class MissileFire : public Missile
{
public:
	virtual void Action();
};
class MissileHit : public Missile
{
public:
	virtual void Action();
};
class MissileParabolic : public Missile
{
public:
	virtual void Action();
};
class MissileLandMine : public Missile
{
public:
	virtual void Action();
};
class MissileWhirlwind : public Missile
{
public:
	virtual void Action();
};
class MissileFlameShield : public Missile
{
public:
	virtual void Action();
};
class MissileDeathCoil : public Missile
{
public:
	virtual void Action();
};

class MissileTracer : public Missile
{
public:
	virtual void Action();
};

class MissileClipToTarget : public Missile
{
public:
	virtual void Action();
};

class MissileContinious : public Missile
{
public:
	virtual void Action();
};

class MissileStraightFly : public Missile
{
public:
	virtual void Action();
};


class BurningBuildingFrame
{
public:
	int Percent = 0;				/// HP percent
	MissileType *Missile = nullptr;	/// Missile to draw
} ;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<BurningBuildingFrame *> BurningBuildingFrames;  /// Burning building frames

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

// In ccl_missile.c

/// register ccl features
extern void MissileCclRegister();

// In missile.c

/// load all missile sprites
extern void LoadMissileSprites();
/// count missile sprites
extern int GetMissileSpritesCount();
/// allocate an empty missile-type slot
extern MissileType *NewMissileTypeSlot(const std::string &ident);
/// Get missile-type by ident
extern MissileType *MissileTypeByIdent(const std::string &ident);
/// create a missile
//Wyrmgus start
//extern Missile *MakeMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos);
extern Missile *MakeMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos, int z);
//Wyrmgus end
/// create a local missile
//Wyrmgus start
//extern Missile *MakeLocalMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos);
extern Missile *MakeLocalMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos, int z);
//Wyrmgus end

/// Calculates damage done to goal by attacker using formula
//Wyrmgus start
//extern int CalculateDamage(const CUnit &attacker, const CUnit &goal, const NumberDesc *formula);
extern int CalculateDamage(const CUnit &attacker, const CUnit &goal, const NumberDesc *formula, const Missile *missile = nullptr);
//Wyrmgus end
/// fire a missile
//Wyrmgus start
//extern void FireMissile(CUnit &unit, CUnit *goal, const Vec2i &goalPos);
extern void FireMissile(CUnit &unit, CUnit *goal, const Vec2i &goalPos, int z);
//Wyrmgus end

extern void FindAndSortMissiles(const CViewport &vp, std::vector<Missile *> &table);

/// handle all missiles
extern void MissileActions();
/// distance from view point to missile
extern int ViewPointDistanceToMissile(const Missile &missile);

/// Get the burning building missile based on hp percent
extern MissileType *MissileBurningBuilding(int percent);

/// Save missiles
extern void SaveMissiles(CFile &file);

/// Initialize missile-types
extern void InitMissileTypes();
/// Clean missile-types
extern void CleanMissileTypes();
/// Initialize missiles
extern void InitMissiles();
/// Clean missiles
extern void CleanMissiles();

extern void FreeBurningBuildingFrames();

#endif