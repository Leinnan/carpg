#pragma once

//-----------------------------------------------------------------------------
#include "BaseUsable.h"
#include "ItemContainer.h"
#include "GameDialog.h"

//-----------------------------------------------------------------------------
struct Usable : EntityType<Usable>
{
	BaseUsable* base;
	Vec3 pos;
	float rot;
	Entity<Unit> user;
	ItemContainer* container;
	int variant;

	static const float SOUND_DIST;
	static const int MIN_SIZE = 22;

	Usable() : variant(-1), container(nullptr) {}
	~Usable() { delete container; }

	void Save(GameWriter& f);
	void Load(GameReader& f);
	void Write(BitStreamWriter& f) const;
	bool Read(BitStreamReader& f);
	Mesh* GetMesh() const;
	Vec3 GetCenter() const;
};
