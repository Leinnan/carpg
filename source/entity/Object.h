#pragma once

//-----------------------------------------------------------------------------
#include "BaseObject.h"
#include "Mesh.h"

//-----------------------------------------------------------------------------
// Object in game
struct Object
{
	BaseObject* base;
	Vec3 pos, rot;
	float scale;
	Mesh* mesh;
	bool require_split;

	static const int MIN_SIZE = 29;

	Object() : require_split(false)
	{
	}

	float GetRadius() const
	{
		return mesh->head.radius * scale;
	}
	bool RequireNoCulling() const
	{
		if(base)
			return IsSet(base->flags, OBJ_NO_CULLING);
		return false;
	}
	bool IsBillboard() const
	{
		return base && IsSet(base->flags, OBJ_BILLBOARD);
	}
	void Save(FileWriter& f);
	void Load(FileReader& f);
	void Write(BitStreamWriter& f) const;
	bool Read(BitStreamReader& f);
};
