/*************************************************************************/
/*  register_types.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "register_types.h"

#include "achievement.h"
#include "campaign.h"
#include "civilization.h"
#include "conversible_color.h"
#include "faction.h"
#include "hair_color.h"
#include "icon.h"
#include "player.h"
#include "player_color.h"
#include "skin_color.h"
#include "unit/unit_type.h"
#include "wyrmgus.h"

void register_wyrmgus_types()
{
	ClassDB::register_class<CAchievement>();
	ClassDB::register_class<CCampaign>();
	ClassDB::register_class<CCivilization>();
	ClassDB::register_virtual_class<CConversibleColor>();
	ClassDB::register_class<CFaction>();
	ClassDB::register_class<CHairColor>();
	ClassDB::register_class<CIcon>();
	ClassDB::register_class<CPlayer>();
	ClassDB::register_class<CPlayerColor>();
	ClassDB::register_class<CSkinColor>();
	ClassDB::register_class<CUnitType>();
	ClassDB::register_class<Wyrmgus>();
}

void unregister_wyrmgus_types()
{
}
