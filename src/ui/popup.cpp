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
/**@name popup.cpp - The popup globals. */
//
//      (c) Copyright 2012-2020 by cybermind, Joris Dauphin and Andrettin
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

#include "ui/popup.h"

#include "database/defines.h"
#include "faction.h"
#include "font.h"
#include "player.h"
#include "script/trigger.h"
#include "spells.h"
//Wyrmgus start
#include "translate.h"
//Wyrmgus end
#include "ui/button.h"
#include "ui/ui.h"
//Wyrmgus start
#include "unit/unit.h"
//Wyrmgus end
#include "unit/unit_class.h"
//Wyrmgus start
#include "unit/unit_manager.h"
//Wyrmgus end
#include "unit/unit_type.h"
#include "upgrade/dependency.h"
#include "upgrade/upgrade.h"
#include "upgrade/upgrade_class.h"
#include "video.h"

/* virtual */ int CPopupContentTypeButtonInfo::GetWidth(const stratagus::button &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	std::string draw("");
	switch (this->InfoType) {
		case PopupButtonInfo_Hint:
			draw = button.get_hint();
			break;
		case PopupButtonInfo_Description:
			draw = button.Description;
			break;
		case PopupButtonInfo_Dependencies:
			draw = PrintDependencies(*CPlayer::GetThisPlayer(), button);
			break;
	}
	int width = 0;
	std::string sub;
	if (draw.length()) {
		//Wyrmgus start
		/*
		if (this->MaxWidth) {
			return std::min((unsigned int)font.getWidth(draw), this->MaxWidth);
		}
		*/
		//Wyrmgus end
		int i = 1;
		while (!(sub = GetLineFont(i++, draw, 0, &font)).empty()) {
			width = std::max(width, font.getWidth(sub));
		}
		//Wyrmgus start
		if (this->MaxWidth) {
			width = std::min((unsigned int) width, this->MaxWidth);
		}
		//Wyrmgus end
	}
	return width;
}

/* virtual */ int CPopupContentTypeButtonInfo::GetHeight(const stratagus::button &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	std::string draw;
	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	switch (this->InfoType) {
		case PopupButtonInfo_Hint:
			draw = button.get_hint();
			break;
		case PopupButtonInfo_Description:
			draw = button.Description;
			break;
		case PopupButtonInfo_Dependencies:
			draw = PrintDependencies(*CPlayer::GetThisPlayer(), button);
			break;
	}
	int height = 0;
	if (draw.length()) {
		int i = 1;
		while ((GetLineFont(i++, draw, this->MaxWidth, &font)).length()) {
			height += font.Height() + 2 * scale_factor;
		}
	}
	return height;
}

/* virtual */ void CPopupContentTypeButtonInfo::Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const stratagus::button &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	CLabel label(font, this->TextColor, this->HighlightColor);
	std::string draw("");
	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	switch (this->InfoType) {
		case PopupButtonInfo_Hint:
			draw = _(button.get_hint().c_str());
			break;
		case PopupButtonInfo_Description:
			draw = button.Description;
			break;
		case PopupButtonInfo_Dependencies:
			draw = PrintDependencies(*CPlayer::GetThisPlayer(), button);
			break;
	}
	std::string sub(draw);
	if (draw.length()) {
		int i = 0;
		int y_off = y;
		unsigned int width = this->MaxWidth
							 ? std::min(this->MaxWidth, popupWidth - 2 * popup.MarginX)
							 : 0;
		while ((sub = GetLineFont(++i, draw, width, &font)).length()) {
			label.Draw(x, y_off, sub);
			y_off += font.Height() + 2 * scale_factor;
		}
		return;
	}
}

/* virtual */ void CPopupContentTypeButtonInfo::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1));

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "InfoType")) {
			std::string temp(LuaToString(l, -1));
			if (temp == "Hint") {
				this->InfoType = PopupButtonInfo_Hint;
			} else if (temp == "Description") {
				this->InfoType = PopupButtonInfo_Description;
			} else if (temp == "Dependencies") {
				this->InfoType = PopupButtonInfo_Dependencies;
			}
		} else if (!strcmp(key, "MaxWidth")) {
			this->MaxWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Font")) {
			this->Font = CFont::Get(LuaToString(l, -1));
		} else {
			LuaError(l, "'%s' invalid for method 'Name' in DefinePopups" _C_ key);
		}
	}
}

/* virtual */ int CPopupContentTypeText::GetWidth(const stratagus::button &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	//Wyrmgus start
	button.SetTriggerData();
	int resource = button.Value;
	if (button.Action == ButtonCmd::ProduceResource || button.Action == ButtonCmd::SellResource || button.Action == ButtonCmd::BuyResource) {
		TriggerData.Resource = &resource;
	}
	std::string text = EvalString(this->Text);
	button.CleanTriggerData();
	//Wyrmgus end
	
	//Wyrmgus start
	/*
	if (this->MaxWidth) {
		//Wyrmgus start
//		return std::min((unsigned int)font.getWidth(this->Text), this->MaxWidth);
		return std::min((unsigned int)font.getWidth(text), this->MaxWidth);
		//Wyrmgus end
	}
	*/
	//Wyrmgus end
	int width = 0;
	std::string sub;
	int i = 1;
	//Wyrmgus start
//	while (!(sub = GetLineFont(i++, this->Text, 0, &font)).empty()) {
	while (!(sub = GetLineFont(i++, text, 0, &font)).empty()) {
	//Wyrmgus end
		width = std::max(width, font.getWidth(sub));
	}
	//Wyrmgus start
	if (this->MaxWidth) {
		width = std::min((unsigned int) width, this->MaxWidth);
	}
	//Wyrmgus end
	return width;
}

/* virtual */ int CPopupContentTypeText::GetHeight(const stratagus::button &button, int *) const
{
	CFont &font = this->Font ? *this->Font : GetSmallFont();
	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	//Wyrmgus start
	button.SetTriggerData();
	int resource = button.Value;
	if (button.Action == ButtonCmd::ProduceResource || button.Action == ButtonCmd::SellResource || button.Action == ButtonCmd::BuyResource) {
		TriggerData.Resource = &resource;
	}
	std::string text = EvalString(this->Text);
	button.CleanTriggerData();
	//Wyrmgus end
	int height = 0;
	int i = 1;
	//Wyrmgus start
//	while ((GetLineFont(i++, this->Text, this->MaxWidth, &font)).length()) {
	while ((GetLineFont(i++, text, this->MaxWidth, &font)).length()) {
	//Wyrmgus end
		height += font.Height() + 2 * scale_factor;
	}
	return height;
}

/* virtual */ void CPopupContentTypeText::Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const stratagus::button &button, int *) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	//Wyrmgus start
	button.SetTriggerData();
	int resource = button.Value;
	if (button.Action == ButtonCmd::ProduceResource || button.Action == ButtonCmd::SellResource || button.Action == ButtonCmd::BuyResource) {
		TriggerData.Resource = &resource;
	}
	std::string text = EvalString(this->Text);
	button.CleanTriggerData();
	//Wyrmgus end
	CLabel label(font, this->TextColor, this->HighlightColor);
	std::string sub;
	int i = 0;
	int y_off = y;
	unsigned int width = this->MaxWidth
						 ? std::min(this->MaxWidth, popupWidth - 2 * popup.MarginX)
						 : 0;
	//Wyrmgus start
//	while ((sub = GetLineFont(++i, this->Text, width, &font)).length()) {
	while ((sub = GetLineFont(++i, text, width, &font)).length()) {
	//Wyrmgus end
		label.Draw(x, y_off, sub);
		y_off += font.Height() + 2 * scale_factor;
	}
}

/* virtual */ void CPopupContentTypeText::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1));

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Text")) {
			//Wyrmgus start
//			this->Text = LuaToString(l, -1);
			this->Text = CclParseStringDesc(l);
			lua_pushnil(l); // ParseStringDesc eat token
			//Wyrmgus end
		} else if (!strcmp(key, "MaxWidth")) {
			this->MaxWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Font")) {
			this->Font = CFont::Get(LuaToString(l, -1));
		} else {
			LuaError(l, "'%s' invalid for method 'Text' in DefinePopups" _C_ key);
		}
	}
}

/* virtual */ int CPopupContentTypeCosts::GetWidth(const stratagus::button &button, int *Costs) const
{
	int popupWidth = 0;
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	for (unsigned int i = 1; i <= MaxCosts; ++i) {
		if (Costs[i]) {
			if (UI.Resources[i].IconWidth != -1)	{
				popupWidth += (UI.Resources[i].IconWidth + 5 * scale_factor);
			} else {
				const CGraphic *G = UI.Resources[i].G;
				if (G) {
					popupWidth += (G->Width + 5 * scale_factor);
				}
			}
			popupWidth += (font.Width(Costs[i]) + 5 * scale_factor);
		}
	}
	if (Costs[ManaResCost]) {
		const CGraphic *G = UI.Resources[ManaResCost].G;
		const CSpell *spell = CSpell::Spells[button.Value];

		if (spell->ManaCost) {
			popupWidth = 10;
			if (UI.Resources[ManaResCost].IconWidth != -1) {
				popupWidth += (UI.Resources[ManaResCost].IconWidth + 5 * scale_factor);
			} else {
				if (G) {
					popupWidth += (G->Width + 5 * scale_factor);
				}
			}
			popupWidth += font.Width(spell->ManaCost);
			popupWidth = std::max<int>(popupWidth, font.Width(spell->Name) + 10 * scale_factor);
		} else {
			popupWidth = font.Width(button.get_hint()) + 10 * scale_factor;
		}
		popupWidth = std::max<int>(popupWidth, 100 * scale_factor);
	}
	return popupWidth;
}

/* virtual */ int CPopupContentTypeCosts::GetHeight(const stratagus::button &button, int *Costs) const
{
	int popupHeight = 0;
	const CFont &font = this->Font ? *this->Font : GetSmallFont();

	for (unsigned int i = 1; i <= ManaResCost; ++i) {
		if (Costs[i] && UI.Resources[i].G) {
			popupHeight = std::max(UI.Resources[i].G->Height, popupHeight);
		}
	}
	return std::max(popupHeight, font.Height());
}

/* virtual */ void CPopupContentTypeCosts::Draw(int x, int y, const CPopup &, const unsigned int, const stratagus::button &button, int *Costs) const
{
	const CFont &font = this->Font ? *this->Font : GetSmallFont();
	CLabel label(font, this->TextColor, this->HighlightColor);
	const int scale_factor = stratagus::defines::get()->get_scale_factor();

	for (unsigned int i = 1; i <= MaxCosts; ++i) {
		if (Costs[i]) {
			int y_offset = 0;
			//Wyrmgus start
//			const CGraphic *G = UI.Resources[i].G;
			CGraphic *G = UI.Resources[i].G;
			//Wyrmgus end
			if (G) {
				int x_offset = UI.Resources[i].IconWidth;
				G->DrawFrameClip(UI.Resources[i].IconFrame,	x , y);
				x += ((x_offset != -1 ? x_offset : G->Width) + 5 * scale_factor);
				y_offset = G->Height;
				y_offset -= label.Height();
				y_offset /= 2;
			}
			x += label.Draw(x, y + y_offset, Costs[i]);
			x += 5 * scale_factor;
		}
	}
	if (Costs[ManaResCost]) {
		const CSpell &spell = *CSpell::Spells[button.Value];
		//Wyrmgus start
//		const CGraphic *G = UI.Resources[ManaResCost].G;
		CGraphic *G = UI.Resources[ManaResCost].G;
		//Wyrmgus end
		if (spell.ManaCost) {
			int y_offset = 0;
			if (G) {
				int x_offset =  UI.Resources[ManaResCost].IconWidth;
				x += 5 * scale_factor;
				G->DrawFrameClip(UI.Resources[ManaResCost].IconFrame, x, y);
				x += ((x_offset != -1 ? x_offset : G->Width) + 5 * scale_factor);
				y_offset = G->Height;
				y_offset -= font.Height();
				y_offset /= 2;
			}
			label.Draw(x, y + y_offset, spell.ManaCost);
		}
	}
}

/* virtual */ void CPopupContentTypeCosts::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isnil(l, -1));

	if (!lua_isnil(l, -1)) {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Font")) {
				this->Font = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Centered")) {
				this->Centered = LuaToBoolean(l, -1);
			} else {
				LuaError(l, "'%s' invalid for method 'Costs' in DefinePopups" _C_ key);
			}
		}
	}
}

CPopupContentTypeLine::CPopupContentTypeLine() : Color(ColorWhite), Width(0), Height(1)
{
}

/* virtual */ int CPopupContentTypeLine::GetWidth(const stratagus::button &button, int *Costs) const
{
	return this->Width;
}

/* virtual */ int CPopupContentTypeLine::GetHeight(const stratagus::button &button, int *Costs) const
{
	return this->Height;
}

/* virtual */ void CPopupContentTypeLine::Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const stratagus::button &button, int *Costs) const
{
	Video.FillRectangle(this->Color, x - popup.MarginX - this->MarginX + 1,
						y, this->Width && Width < popupWidth ? Width : popupWidth - 2, Height);
}

/* virtual */ void CPopupContentTypeLine::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isnil(l, -1));

	if (!lua_isnil(l, -1)) {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Width")) {
				this->Width = LuaToNumber(l, -1);
			} else if (!strcmp(key, "Height")) {
				this->Height = LuaToNumber(l, -1);
			} else if (!strcmp(key, "Color")) {
				this->Color = LuaToUnsignedNumber(l, -1);
			} else {
				LuaError(l, "'%s' invalid for method 'Costs' in DefinePopups" _C_ key);
			}
		}
	}
}

/* virtual */ int CPopupContentTypeVariable::GetWidth(const stratagus::button &button, int *) const
{
	CFont &font = this->Font ? *this->Font : GetSmallFont();
	//Wyrmgus start
//	TriggerData.Type = UnitTypes[button.Value];
	button.SetTriggerData();
	int resource = button.Value;
	if (button.Action == ButtonCmd::ProduceResource || button.Action == ButtonCmd::SellResource || button.Action == ButtonCmd::BuyResource) {
		TriggerData.Resource = &resource;
	}
	//Wyrmgus end
	std::string text = EvalString(this->Text);
	//Wyrmgus start
//	TriggerData.Type = nullptr;
	button.CleanTriggerData();
	//Wyrmgus end
	return font.getWidth(text);
}

/* virtual */ int CPopupContentTypeVariable::GetHeight(const stratagus::button &, int *) const
{
	CFont &font = this->Font ? *this->Font : GetSmallFont();
	return font.Height();
}

/* virtual */ void CPopupContentTypeVariable::Draw(int x, int y, const CPopup &, const unsigned int, const stratagus::button &button, int *) const
{
	std::string text;										// Optional text to display.
	CFont &font = this->Font ? *this->Font : GetSmallFont(); // Font to use.

	Assert(this->Index == -1 || ((unsigned int) this->Index < UnitTypeVar.GetNumberVariable()));

	CLabel label(font, this->TextColor, this->HighlightColor);

	if (this->Text) {
		button.SetTriggerData();
		int resource = button.Value;
		if (button.Action == ButtonCmd::ProduceResource || button.Action == ButtonCmd::SellResource || button.Action == ButtonCmd::BuyResource) {
			TriggerData.Resource = &resource;
		}
		text = EvalString(this->Text);
		button.CleanTriggerData();
		if (this->Centered) {
			x += (label.DrawCentered(x, y, text) * 2);
		} else {
			x += label.Draw(x, y, text);
		}
	}

	if (this->Index != -1) {
		int value;

		const stratagus::unit_class *unit_class = nullptr;
		const stratagus::unit_type *unit_type = nullptr;
		switch (button.Action) {
			case ButtonCmd::TrainClass:
			case ButtonCmd::BuildClass:
				unit_class = stratagus::unit_class::get_all()[button.Value];
				if (Selected[0]->Player->Faction != -1) {
					unit_type = stratagus::faction::get_all()[Selected[0]->Player->Faction]->get_class_unit_type(unit_class);
				}
				break;
			case ButtonCmd::Unit:
			case ButtonCmd::Buy:
				unit_type = UnitManager.GetSlotUnit(button.Value).Type;
				break;
			default:
				unit_type = stratagus::unit_type::get_all()[button.Value];
				break;
		}

		switch (button.Action) {
			case ButtonCmd::Unit:
			case ButtonCmd::Buy:
				if (
					unit_type->BoolFlag[ITEM_INDEX].value
					&& this->Index != HITPOINTHEALING_INDEX
					&& UnitManager.GetSlotUnit(button.Value).Container
					&& (UnitManager.GetSlotUnit(button.Value).Container->CanEquipItem(&UnitManager.GetSlotUnit(button.Value)) || UnitManager.GetSlotUnit(button.Value).Work != nullptr || UnitManager.GetSlotUnit(button.Value).Elixir != nullptr)
					) {
					value = UnitManager.GetSlotUnit(button.Value).Container->GetItemVariableChange(&UnitManager.GetSlotUnit(button.Value), this->Index);
					if (value >= 0) {
						x += label.Draw(x, y, "+");
					}
				} else if (UnitManager.GetSlotUnit(button.Value).Work != nullptr || UnitManager.GetSlotUnit(button.Value).Elixir != nullptr) {
					value = UnitManager.GetSlotUnit(button.Value).GetItemVariableChange(&UnitManager.GetSlotUnit(button.Value), this->Index);
					if (value >= 0) {
						x += label.Draw(x, y, "+");
					}
				} else {
					value = UnitManager.GetSlotUnit(button.Value).Variable[this->Index].Value;
					if (
						(unit_type->BoolFlag[ITEM_INDEX].value && button.Action == ButtonCmd::Buy)
						|| IsBonusVariable(this->Index)
						) {
						if (value >= 0) {
							x += label.Draw(x, y, "+");
						}
					}
				}
				break;
			default:
				value = unit_type->Stats[CPlayer::GetThisPlayer()->Index].Variables[this->Index].Value;
				if (value >= 0 && IsBonusVariable(this->Index)) {
					x += label.Draw(x, y, "+");
				}
				break;
		}

		x += label.Draw(x, y, value);
		if (IsPercentageVariable(this->Index)) {
			x += label.Draw(x, y, "%");
		}
		//Wyrmgus end
	}
}

/* virtual */ void CPopupContentTypeVariable::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isstring(l, -1));

	if (lua_isstring(l, -1)) {
		this->Text = CclParseStringDesc(l);
		lua_pushnil(l); // ParseStringDesc eat token
	} else {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Text")) {
				this->Text = CclParseStringDesc(l);
				lua_pushnil(l); // ParseStringDesc eat token
			} else if (!strcmp(key, "Font")) {
				this->Font = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Centered")) {
				this->Centered = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "Variable")) {
				const char *const name = LuaToString(l, -1);
				this->Index = UnitTypeVar.VariableNameLookup[name];
				if (this->Index == -1) {
					LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
				}
			} else {
				LuaError(l, "'%s' invalid for method 'Text' in DefinePopups" _C_ key);
			}
		}
	}
}

/**
**  Parse the popup conditions.
**
**  @param l   Lua State.
*/
static PopupConditionPanel *ParsePopupConditions(lua_State *l)
{
	Assert(lua_istable(l, -1));

	PopupConditionPanel *condition = new PopupConditionPanel;
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);

		if (!strcmp(key, "HasHint")) {
			condition->HasHint = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "HasDescription")) {
			condition->HasDescription = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "HasDependencies")) {
			condition->HasDependencies = LuaToBoolean(l, -1);
		//Wyrmgus start
		} else if (!strcmp(key, "Class")) {
			condition->Class = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "Description")) {
			condition->Description = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "Quote")) {
			condition->Quote = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "Encyclopedia")) {
			condition->Encyclopedia = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "SettlementName")) {
			condition->settlement_name = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "CanActiveHarvest")) {
			condition->CanActiveHarvest = LuaToBoolean(l, -1);
		//Wyrmgus end
		} else if (!strcmp(key, "ButtonValue")) {
			condition->ButtonValue = LuaToString(l, -1);
		} else if (!strcmp(key, "ButtonAction")) {
			std::string value = LuaToString(l, -1);
			ButtonCmd button_action_id = GetButtonActionIdByName(value);
			if (button_action_id != ButtonCmd::None) {
				condition->ButtonAction = button_action_id;
			} else {
				LuaError(l, "Unsupported button action: %s" _C_ value.c_str());
			}
			//Wyrmgus end
		//Wyrmgus start
		} else if (!strcmp(key, "Opponent")) {
			condition->Opponent = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Neutral")) {
			condition->Neutral = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "AutoCast")) {
			condition->AutoCast = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Equipped")) {
			condition->Equipped = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Equippable")) {
			condition->Equippable = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Consumable")) {
			condition->Consumable = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Affixed")) {
			condition->Affixed = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Spell")) {
			condition->Spell = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "CanUse")) {
			condition->CanUse = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Work")) {
			condition->Work = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "ReadWork")) {
			condition->ReadWork = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Elixir")) {
			condition->Elixir = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "ConsumedElixir")) {
			condition->ConsumedElixir = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Unique")) {
			condition->Unique = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "UniqueSet")) {
			condition->UniqueSet = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Bound")) {
			condition->Bound = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Identified")) {
			condition->Identified = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "UnitTypeType")) {
			const char *unit_type_type = LuaToString(l, -1);
			if (!strcmp(unit_type_type, "land")) {
				condition->UnitTypeType = UnitTypeType::Land;
			} else if (!strcmp(unit_type_type, "fly")) {
				condition->UnitTypeType = UnitTypeType::Fly;
			} else if (!strcmp(unit_type_type, "fly-low")) {
				condition->UnitTypeType = UnitTypeType::FlyLow;
			} else if (!strcmp(unit_type_type, "naval")) {
				condition->UnitTypeType = UnitTypeType::Naval;
			} else if (!strcmp(unit_type_type, "space")) {
				condition->UnitTypeType = UnitTypeType::Space;
			} else {
				LuaError(l, "Unsupported Type: %s" _C_ unit_type_type);
			}
		} else if (!strcmp(key, "UnitTypeClass")) {
			condition->unit_class = stratagus::unit_class::get(LuaToString(l, -1));
		} else if (!strcmp(key, "ItemClass")) {
			condition->item_class = stratagus::string_to_item_class(LuaToString(l, -1));
		} else if (!strcmp(key, "CanStore")) {
			condition->CanStore = GetResourceIdByName(LuaToString(l, -1));
		} else if (!strcmp(key, "ImproveIncome")) {
			condition->ImproveIncome = GetResourceIdByName(LuaToString(l, -1));
		} else if (!strcmp(key, "Weapon")) {
			condition->Weapon = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Shield")) {
			condition->Shield = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Boots")) {
			condition->Boots = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Arrows")) {
			condition->Arrows = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Regeneration")) {
			condition->Regeneration = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "FactionUpgrade")) {
			condition->FactionUpgrade = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "FactionCoreSettlements")) {
			condition->FactionCoreSettlements = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "ResearchedUpgrade")) {
			condition->ResearchedUpgrade = CUpgrade::get(LuaToString(l, -1));
		} else if (!strcmp(key, "ResearchedUpgradeClass")) {
			condition->researched_upgrade_class = stratagus::upgrade_class::get(LuaToString(l, -1));
		} else if (!strcmp(key, "UpgradeResearched")) {
			condition->UpgradeResearched = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Ability")) {
			condition->Ability = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "ChildResources")) {
			condition->ChildResources = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "ImproveIncomes")) {
			condition->ImproveIncomes = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "LuxuryResource")) {
			condition->LuxuryResource = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "RequirementsString")) {
			condition->RequirementsString = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "ExperienceRequirementsString")) {
			condition->ExperienceRequirementsString = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "BuildingRulesString")) {
			condition->BuildingRulesString = Ccl2Condition(l, LuaToString(l, -1));
		//Wyrmgus end
		} else if (!strcmp(key, "Overlord")) {
			condition->Overlord = Ccl2Condition(l, LuaToString(l, -1));
		} else if (!strcmp(key, "TopOverlord")) {
			condition->TopOverlord = Ccl2Condition(l, LuaToString(l, -1));
		} else {
			int index = UnitTypeVar.BoolFlagNameLookup[key];
			if (index != -1) {
				if (!condition->BoolFlags) {
					size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();
					condition->BoolFlags = new char[new_bool_size];
					memset(condition->BoolFlags, 0, new_bool_size * sizeof(char));
				}
				condition->BoolFlags[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			index = UnitTypeVar.VariableNameLookup[key];
			if (index != -1) {
				if (!condition->Variables) {
					size_t new_variables_size = UnitTypeVar.GetNumberVariable();
					condition->Variables = new char[new_variables_size];
					memset(condition->Variables, 0, new_variables_size * sizeof(char));
				}
				condition->Variables[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			LuaError(l, "'%s' invalid for Condition in DefinePopups" _C_ key);
		}
	}
	return condition;
}

/* static */ CPopupContentType *CPopupContentType::ParsePopupContent(lua_State *l)
{
	Assert(lua_istable(l, -1));

	bool wrap = true;
	int marginX = MARGIN_X * stratagus::defines::get()->get_scale_factor();
	int marginY = MARGIN_Y * stratagus::defines::get()->get_scale_factor();
	int minWidth = 0;
	int minHeight = 0;
	std::string textColor("white");
	std::string highColor("red");
	CPopupContentType *content = nullptr;
	PopupConditionPanel *condition = nullptr;

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);

		if (!strcmp(key, "Wrap")) {
			wrap = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "TextColor")) {
			textColor = LuaToString(l, -1);
		} else if (!strcmp(key, "HighlightColor")) {
			highColor = LuaToString(l, -1);
		} else if (!strcmp(key, "Margin")) {
			CclGetPos(l, &marginX, &marginY);
		} else if (!strcmp(key, "MinWidth")) {
			minWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "MinHeight")) {
			minHeight = LuaToNumber(l, -1);
		} else if (!strcmp(key, "More")) {
			Assert(lua_istable(l, -1));
			key = LuaToString(l, -1, 1); // Method name
			lua_rawgeti(l, -1, 2); // Method data
			if (!strcmp(key, "ButtonInfo")) {
				content = new CPopupContentTypeButtonInfo;
			} else if (!strcmp(key, "Text")) {
				content = new CPopupContentTypeText;
			} else if (!strcmp(key, "Costs")) {
				content = new CPopupContentTypeCosts;
			} else if (!strcmp(key, "Line")) {
				content = new CPopupContentTypeLine;
			} else if (!strcmp(key, "Variable")) {
				content = new CPopupContentTypeVariable;
			} else {
				LuaError(l, "Invalid drawing method '%s' in DefinePopups" _C_ key);
			}
			content->Parse(l);
			lua_pop(l, 1); // Pop Variable Method data
		} else if (!strcmp(key, "Condition")) {
			condition = ParsePopupConditions(l);
		} else {
			LuaError(l, "'%s' invalid for Contents in DefinePopups" _C_ key);
		}
	}
	content->Wrap = wrap;
	content->MarginX = marginX;
	content->MarginY = marginY;
	content->minSize.x = minWidth;
	content->minSize.y = minHeight;
	content->Condition = condition;
	content->TextColor = textColor;
	content->HighlightColor = highColor;
	return content;
}

CPopup::CPopup() :
	Contents(), MarginX(MARGIN_X *stratagus::defines::get()->get_scale_factor()), MarginY(MARGIN_Y *stratagus::defines::get()->get_scale_factor()), MinWidth(0), MinHeight(0),
	DefaultFont(nullptr), BackgroundColor(ColorBlue), BorderColor(ColorWhite)
{}

CPopup::~CPopup()
{
	for (std::vector<CPopupContentType *>::iterator content = Contents.begin();
		 content != Contents.end(); ++content) {
		delete *content;
	}
}
