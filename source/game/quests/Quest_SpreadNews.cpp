#include "Pch.h"
#include "Base.h"
#include "Quest_SpreadNews.h"
#include "Dialog.h"
#include "DialogDefine.h"
#include "Game.h"
#include "Journal.h"

//-----------------------------------------------------------------------------
DialogEntry spread_news_start[] = {
	TALK(35),
	CHOICE(36),
		IF_RAND(3),
			TALK2(37),
		ELSE,
			RANDOM_TEXT(4),
				TALK(38),
				TALK(39),
				TALK2(40),
				TALK(41),
		END_IF,
		TALK(42),
		TALK2(43),
		TALK(44),
		SET_QUEST_PROGRESS(Quest_SpreadNews::Progress::Started),
		END,
	END_CHOICE,
	CHOICE(45),
		END,
	END_CHOICE,
	ESCAPE_CHOICE,
	SHOW_CHOICES,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry spread_news_tell[] = {
	TALK(46),
	SET_QUEST_PROGRESS(Quest_SpreadNews::Progress::Deliver),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry spread_news_timeout[] = {
	TALK(47),
	SET_QUEST_PROGRESS(Quest_SpreadNews::Progress::Timeout),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry spread_news_end[] = {
	TALK(48),
	TALK(49),
	SET_QUEST_PROGRESS(Quest_SpreadNews::Progress::Finished),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
bool SortEntries(const Quest_SpreadNews::Entry& e1, const Quest_SpreadNews::Entry& e2)
{
	return e1.dist < e2.dist;
}

//=================================================================================================
void Quest_SpreadNews::Start()
{
	type = Type::Mayor;
	quest_id = Q_SPREAD_NEWS;
	start_loc = game->current_location;
	VEC2 pos = game->locations[start_loc]->pos;
	bool sorted = false;
	for(uint i=0, count = game->cities; i<count; ++i)
	{
		if(i == start_loc)
			continue;
		Location& loc = *game->locations[i];
		if(loc.type != L_CITY && loc.type != L_VILLAGE)
			continue;
		float dist = distance(pos, loc.pos);
		bool ok = false;
		if(entries.size() < 5)
			ok = true;
		else
		{
			if(!sorted)
			{
				std::sort(entries.begin(), entries.end(), SortEntries);
				sorted = true;
			}
			if(entries.back().dist > dist)
			{
				ok = true;
				sorted = false;
				entries.pop_back();
			}
		}
		if(ok)
		{
			Entry& e = Add1(entries);
			e.location = i;
			e.given = false;
			e.dist = dist;
		}
	}
}

//=================================================================================================
DialogEntry* Quest_SpreadNews::GetDialog(int type2)
{
	switch(type2)
	{
	case QUEST_DIALOG_START:
		return spread_news_start;
	case QUEST_DIALOG_FAIL:
		return spread_news_timeout;
	case QUEST_DIALOG_NEXT:
		if(prog == Progress::Started)
			return spread_news_tell;
		else
			return spread_news_end;
	default:
		assert(0);
		return NULL;
	}
}

//=================================================================================================
void Quest_SpreadNews::SetProgress(int prog2)
{
	switch(prog2)
	{
	case Progress::Started:
		// told info to spread by player
		{
			prog = Progress::Started;
			start_time = game->worldtime;
			state = Quest::Started;

			quest_index = game->quests.size();
			game->quests.push_back(this);
			RemoveElement<Quest*>(game->unaccepted_quests, this);

			Location& loc = *game->locations[start_loc];
			name = game->txQuest[213];
			msgs.push_back(Format(game->txQuest[3], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str(), game->day+1, game->month+1, game->year));
			msgs.push_back(Format(game->txQuest[17], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str(), FormatString("targets")));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_AddQuest(refid);
		}
		break;
	case Progress::Deliver:
		// player told news to mayor
		{
			uint ile = 0;
			for(vector<Entry>::iterator it = entries.begin(), end = entries.end(); it != end; ++it)
			{
				if(game->current_location == it->location)
				{
					it->given = true;
					++ile;
				}
				else if(it->given)
					++ile;
			}

			Location& loc = *game->locations[game->current_location];
			msgs.push_back(Format(game->txQuest[18], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str()));

			if(ile != entries.size())
			{
				prog = Progress::Deliver;
				msgs.push_back(Format(game->txQuest[19], game->locations[start_loc]->name.c_str()));
			}

			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				if(prog == Progress::Deliver)
					game->Net_UpdateQuestMulti(refid, 2);
				else
					game->Net_UpdateQuest(refid);
			}
		}
		break;
	case Progress::Timeout:
		// player failed to spread news in time
		{
			prog = Progress::Timeout;
			state = Quest::Failed;
			((City*)game->locations[start_loc])->quest_mayor = CityQuestState::Failed;

			msgs.push_back(game->txQuest[20]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::Finished:
		// player spread news to all mayors, end of quest
		{
			prog = Progress::Finished;
			state = Quest::Completed;
			((City*)game->locations[start_loc])->quest_mayor = CityQuestState::None;
			game->AddReward(200);

			msgs.push_back(game->txQuest[21]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	}
}

//=================================================================================================
cstring Quest_SpreadNews::FormatString(const string& str)
{
	if(str == "targets")
	{
		static string s;
		s.clear();
		for(uint i=0, count = entries.size(); i<count; ++i)
		{
			s += game->locations[entries[i].location]->name;
			if(i == count-2)
				s += game->txQuest[264];
			else if(i != count-1)
				s += ", ";
		}
		return s.c_str();
	}
	else if(str == "start_loc")
		return game->locations[start_loc]->name.c_str();
	else
	{
		assert(0);
		return NULL;
	}
}

//=================================================================================================
bool Quest_SpreadNews::IsTimedout()
{
	return game->worldtime - start_time > 60;
}

//=================================================================================================
bool Quest_SpreadNews::IfNeedTalk(cstring topic)
{
	if(strcmp(topic, "tell_news") == 0)
	{
		if(prog == Progress::Started)
		{
			for(vector<Entry>::iterator it = entries.begin(), end = entries.end(); it != end; ++it)
			{
				if(it->location == game->current_location)
					return !it->given;
			}
		}
	}
	else if(strcmp(topic, "tell_news_end") == 0)
	{
		return prog == Progress::Deliver && game->current_location == start_loc;
	}
	return false;
}

//=================================================================================================
void Quest_SpreadNews::Save(HANDLE file)
{
	Quest::Save(file);

	if(IsActive())
	{
		uint count = entries.size();
		WriteFile(file, &count, sizeof(count), &tmp, NULL);
		WriteFile(file, &entries[0], sizeof(Entry)*count, &tmp, NULL);
	}
}

//=================================================================================================
void Quest_SpreadNews::Load(HANDLE file)
{
	Quest::Load(file);

	if(IsActive())
	{
		uint count;
		ReadFile(file, &count, sizeof(count), &tmp, NULL);
		entries.resize(count);
		ReadFile(file, &entries[0], sizeof(Entry)*count, &tmp, NULL);
	}
}