#pragma once

//-----------------------------------------------------------------------------
#include "Quest.h"

//-----------------------------------------------------------------------------
// Traveler found artifact but lost it inside dungeon. Need to recover it from ground.
class Quest_LostArtifact final : public Quest_Dungeon
{
public:
	enum Progress
	{
		None,
		Started,
		Finished,
		Timeout
	};

	void Start() override;
	GameDialog* GetDialog(int type2) override;
	void SetProgress(int prog2) override;
	cstring FormatString(const string& str) override;
	bool IsTimedout() const override;
	bool OnTimeout(TimeoutType ttype) override;
	bool IfHaveQuestItem2(cstring id) const override;
	const Item* GetQuestItem() override;
	void Save(GameWriter& f) override;
	LoadResult Load(GameReader& f) override;

private:
	int GetReward() const;

	const Item* item;
	OtherItem quest_item;
	int st;
};
