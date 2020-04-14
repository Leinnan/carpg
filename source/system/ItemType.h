#pragma once

//-----------------------------------------------------------------------------
/* Item types - Adding new item type require changes in:
ItemLoader.cpp
UnitLoader.cpp - to allow selling
If equippable
	Item.IsWearable
	ItemSlot.h - Add new slots in ItemSlot.h (ItemTypeToSlot)
	Unit.UpdateInventory, IsBetterItem, CanWear, Save/Load, Read/Write for visible slots
	team->CheckTeamItemShares, BuyTeamItems
	If item have multiple slots:
		Unit.AddItemAndEquipIfNone
		Unit.HaveItemEquipped
		Inventory.cpp
	UnitStats default_priorities
If need special texts in inventory:
	ItemSlot.GetItemString
If require special ordering:
	Item.cpp:ItemCmp
If can be generated:
	ItemHelper.GenerateTreasure
Game2.cpp:GetItemSound
*/
enum ITEM_TYPE
{
	IT_NONE = -1,

	IT_WEAPON,
	IT_BOW,
	IT_SHIELD,
	IT_ARMOR,
	IT_AMULET,
	IT_RING,
	IT_OTHER,
	IT_MAX_WEARABLE = IT_OTHER, // used in Team.cpp priorities
	IT_CONSUMABLE,
	IT_BOOK,
	IT_MAX_GEN = IT_BOOK, // items generated in treasure
	IT_GOLD,

	// special types (not really items)
	IT_LIST,
	IT_STOCK,
	IT_BOOK_SCHEME,
	IT_START_ITEMS,
	IT_BETTER_ITEMS,
	IT_RECIPE,
	IT_ALIAS,

	IT_MAX
};

enum ITEM_PROP
{
	ITEM_NONE = 0,
	ITEM_WEAPON = 1 << 0,
	ITEM_BOW = 1 << 1,
	ITEM_SHIELD = 1 << 2,
	ITEM_ARMOR = 1 << 3,
	ITEM_AMULET = 1 << 4,
	ITEM_RING = 1 << 5,
	ITEM_OTHER = 1 << 6,
	ITEM_CONSUMABLE = 1 << 7,
	ITEM_BOOK = 1 << 8
};

