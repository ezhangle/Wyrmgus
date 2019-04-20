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
/**@name language_family.h - The language family header file. */
//
//      (c) Copyright 2019 by Andrettin
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

#ifndef __LANGUAGE_FAMILY_H__
#define __LANGUAGE_FAMILY_H__

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "data_element.h"
#include "data_type.h"

/*----------------------------------------------------------------------------
--  Definition
----------------------------------------------------------------------------*/

class CLanguageFamily : public DataElement, public DataType<CLanguageFamily>
{
	DATA_TYPE(CLanguageFamily, DataElement)

public:
	static constexpr const char *ClassIdentifier = "language_family";
	
private:
	static inline bool InitializeClass()
	{
		REGISTER_PROPERTY(Family);
		
		return true;
	}
	
	static inline bool ClassInitialized = InitializeClass();

public:
	Property<CLanguageFamily *> Family;			/// the family to which this language family belongs
	
protected:
	static void _bind_methods();
};

#endif