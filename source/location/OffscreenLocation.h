#pragma once

//-----------------------------------------------------------------------------
#include "Location.h"
#include "LevelArea.h"

//-----------------------------------------------------------------------------
// Special location that is used to store units that are not inside any normal location
struct OffscreenLocation : public Location, public LevelArea
{
	OffscreenLocation();
	void Apply(vector<std::reference_wrapper<LevelArea>>& areas) override { assert(0); }
	void Save(GameWriter& f) override;
	void Load(GameReader& f) override;
	void Write(BitStreamWriter& f) override { assert(0); }
	bool Read(BitStreamReader& f) { assert(0); return false; }

	Unit* CreateUnit(UnitData& data, int level = -1);
};
