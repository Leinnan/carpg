#pragma once

//-----------------------------------------------------------------------------
enum BaseLocationId
{
	// fort ludzi, 2-3 poziom�w, bez pu�apek czy ukrytych przej��
	HUMAN_FORT,
	// fort krasnolud�w 3-5 poziom�w, pu�apki, tajne przej�cia, na dole skarby
	DWARF_FORT,
	// wie�a mag�w, 4-6 okr�g�ych ma�ych poziom�w
	// 50% szansy na mag�w, 25% na mag�w i golemy
	MAGE_TOWER,
	// kryj�wka bandyt�w, 1 poziom, pu�apki przy wej�ciu
	// 75% szansy na bandyt�w
	BANDITS_HIDEOUT,
	// krypta w kt�rej pochowano jak�� znan� osob�, 2-3 poziom�w, na pocz�tku ostrze�enia, na ostatnim poziomie du�o pu�apek i centralna sala ze zw�okami i skarbami
	// 50% szansy na nieumar�ych, 25% nekro
	HERO_CRYPT,
	// uwi�ziono tu jakiego� z�ego potwora, 2-3 poziom�w, jak wy�ej
	// 75% szansy na nieumar�ych
	MONSTER_CRYPT,
	// stara �wi�tynia, 1-3 poziom�w, mo�e by� oczyszczona lub nie ze z�ych kap�an�w/nekromant�w
	// 25% nieumarli, 25% nekro, 25% �li
	OLD_TEMPLE,
	// ukryta skrytka, 1 poziom, wygl�da jak zwyk�y poziom ale s� ukryte przej�cia i pu�apki, na ko�cu skarb
	// 25% nic, 25% bandyci
	VAULT,
	// baza nekromant�w
	// 50% nekro, 25% z�o
	NECROMANCER_BASE,
	// labirynt
	LABYRINTH,
	// jaskina
	CAVE,
	// ko�cowy poziom questu kopalnia
	ANCIENT_ARMORY,
	// jak HUMAN_FORT ale 100% szansy na drzwi
	TUTORIAL_FORT,
	// jak HUMAN_FORT ale z sal� tronow�
	THRONE_FORT,
	// jak VAULT ale z sal� tronow�
	THRONE_VAULT,
	// druga tekstura krypty
	CRYPT_2_TEXTURE,
	BASE_LOCATION_MAX
};
static_assert(BASE_LOCATION_MAX <= 32, "Too many base locations, used as flags!");

//-----------------------------------------------------------------------------
// Location flags
enum BaseLocationOptions
{
	BLO_ROUND = 1 << 0,
	BLO_LABYRINTH = 1 << 1,
	BLO_MAGIC_LIGHT = 1 << 2,
	BLO_LESS_FOOD = 1 << 3
};

//-----------------------------------------------------------------------------
struct RoomType;

//-----------------------------------------------------------------------------
template<typename T>
struct NameValue
{
	cstring id;
	T* value;

	NameValue(nullptr_t) : id(nullptr), value(nullptr) {}
	NameValue(cstring id) : id(id), value(nullptr) {}
};

//-----------------------------------------------------------------------------
typedef NameValue<RoomType> RoomStr;
typedef NameValue<UnitGroup> GroupStr;

//-----------------------------------------------------------------------------
struct RoomStrChance
{
	cstring id;
	RoomType* room;
	int chance;

	RoomStrChance(cstring id, int chance) : id(id), chance(chance), room(nullptr)
	{
	}
};

//-----------------------------------------------------------------------------
// Traps flags
enum TRAP_FLAGS
{
	TRAPS_NORMAL = 1 << 0,
	TRAPS_MAGIC = 1 << 1,
	TRAPS_NEAR_ENTRANCE = 1 << 2,
	TRAPS_NEAR_END = 1 << 3
};

//-----------------------------------------------------------------------------
struct LocationTexturePack
{
	struct Entry
	{
		cstring id, id_normal, id_specular;
		TexturePtr tex, tex_normal, tex_specular;

		Entry() : id(nullptr), id_normal(nullptr), id_specular(nullptr), tex(nullptr), tex_normal(nullptr), tex_specular(nullptr)
		{
		}
		Entry(cstring id, cstring id_normal, cstring id_specular) : id(id), id_normal(id_normal), id_specular(id_specular), tex(nullptr), tex_normal(nullptr),
			tex_specular(nullptr)
		{
		}
	};

	Entry floor, wall, ceil;

	LocationTexturePack()
	{
	}
	LocationTexturePack(cstring id_floor, cstring id_wall, cstring id_ceil) : floor(id_floor, nullptr, nullptr), wall(id_wall, nullptr, nullptr), ceil(id_ceil, nullptr, nullptr)
	{
	}
	LocationTexturePack(cstring id_floor, cstring id_floor_nrm, cstring id_floor_spec, cstring id_wall, cstring id_wall_nrm, cstring id_wall_spec, cstring id_ceil,
		cstring id_ceil_nrm, cstring id_ceil_spec) : floor(id_floor, id_floor_nrm, id_floor_spec), wall(id_wall, id_wall_nrm, id_wall_spec),
		ceil(id_ceil, id_ceil_nrm, id_ceil_spec)
	{
	}
};

//-----------------------------------------------------------------------------
// Base location types
struct BaseLocation
{
	cstring name;
	Int2 levels;
	int size, size_lvl, join_room, join_corridor, corridor_chance;
	Int2 corridor_size, room_size;
	int options;
	RoomStr stairs, required;
	Color fog_color, ambient_color;
	Vec2 fog_range;
	float draw_range;
	RoomStrChance* rooms;
	uint room_count, room_total;
	int door_chance, door_open, bars_chance;
	GroupStr group[3];
	int group_chance[3];
	int traps, tex2;
	LocationTexturePack tex;

	RoomType* GetRandomRoomType() const;
	UnitGroup* GetRandomGroup() const;
	static void PreloadTextures();
	static uint SetRoomPointers();
};
extern BaseLocation g_base_locations[];
extern const uint n_base_locations;
