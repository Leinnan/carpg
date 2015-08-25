#include "Pch.h"
#include "Base.h"
#include "Quest_DeliverLetter.h"
#include "Dialog.h"
#include "DialogDefine.h"
#include "Game.h"
#include "Journal.h"

//-----------------------------------------------------------------------------
DialogEntry deliver_letter_start[] = {
	TALK2(0),
	CHOICE(1),
		RANDOM_TEXT(2),
			TALK(2),
			TALK(3),
		TALK2(4),
		SET_QUEST_PROGRESS(Quest_DeliverLetter::Progress::Started),
		END,
	END_CHOICE,
	CHOICE(5),
		END,
	END_CHOICE,
	ESCAPE_CHOICE,
	SHOW_CHOICES,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry deliver_letter_timeout[] = {
	TALK2(6),
	SET_QUEST_PROGRESS(Quest_DeliverLetter::Progress::Timeout),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry deliver_letter_give[] = {
	TALK(7),
	TALK(8),
	IF_QUEST_TIMEOUT,
		TALK(9),
		SET_QUEST_PROGRESS(Quest_DeliverLetter::Progress::Timeout),
		END,
	END_IF,
	RANDOM_TEXT(3),
		TALK(10),
		TALK(11),
		TALK(12),
	TALK(13),
	SET_QUEST_PROGRESS(Quest_DeliverLetter::Progress::GotResponse),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry deliver_letter_end[] = {
	TALK(14),
	TALK(15),
	TALK(16),
	SET_QUEST_PROGRESS(Quest_DeliverLetter::Progress::Finished),
	END,
	END_OF_DIALOG
};

//=================================================================================================
void Quest_DeliverLetter::Start()
{
	start_loc = game->current_location;
	end_loc = game->GetRandomCityLocation(start_loc);
	quest_id = Q_DELIVER_LETTER;
	type = Type::Mayor;
}

//=================================================================================================
DialogEntry* Quest_DeliverLetter::GetDialog(int type)
{
	switch(type)
	{
	case QUEST_DIALOG_START:
		return deliver_letter_start;
	case QUEST_DIALOG_FAIL:
		return deliver_letter_timeout;
	case QUEST_DIALOG_NEXT:
		if(prog == Progress::Started)
			return deliver_letter_give;
		else
			return deliver_letter_end;
	default:
		assert(0);
		return NULL;
	}
}

//=================================================================================================
void Quest_DeliverLetter::SetProgress(int prog2)
{
	prog = prog2;
	switch(prog)
	{
	case Progress::Started:
		// give letter to player
		{
			Location& loc = *game->locations[end_loc];
			letter.name = Format(game->txQuest[0], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str());
			letter.type = IT_OTHER;
			letter.weight = 1;
			letter.value = 0;
			letter.flags = ITEM_QUEST|ITEM_DONT_DROP|ITEM_IMPORTANT|ITEM_TEX_ONLY;
			letter.id = "$letter";
			letter.mesh.clear();
			letter.tex = game->tList;
			letter.refid = refid;
			letter.other_type = OtherItems;
			game->current_dialog->pc->unit->AddItem(&letter, 1, true);
			start_time = game->worldtime;
			state = Quest::Started;

			quest_index = game->quests.size();
			game->quests.push_back(this);
			RemoveElement<Quest*>(game->unaccepted_quests, this);

			Location& loc2 = *game->locations[start_loc];
			name = game->txQuest[2];
			msgs.push_back(Format(game->txQuest[3], loc2.type == L_CITY ? game->txForMayor : game->txForSoltys, loc2.name.c_str(), game->day+1, game->month+1, game->year));
			msgs.push_back(Format(game->txQuest[4], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str(), kierunek_nazwa[GetLocationDir(loc2.pos, loc.pos)]));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				game->Net_AddQuest(refid);
				game->Net_RegisterItem(&letter);
				if(!game->current_dialog->is_local)
				{
					game->Net_AddItem(game->current_dialog->pc, &letter, true);
					game->Net_AddedItemMsg(game->current_dialog->pc);
				}
				else
					game->AddGameMsg3(GMS_ADDED_ITEM);
			}
			else
				game->AddGameMsg3(GMS_ADDED_ITEM);
		}
		break;
	case Progress::Timeout:
		// player failed to deliver letter in time
		{
			state = Quest::Failed;
			((City*)game->locations[start_loc])->quest_mayor = CityQuestState::Failed;

			msgs.push_back(game->txQuest[5]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::GotResponse:
		// given letter, got response
		{
			Location& loc = *game->locations[end_loc];
			letter.name = Format(game->txQuest[1], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str());

			msgs.push_back(game->txQuest[6]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				game->Net_RenameItem(&letter);
				game->Net_UpdateQuest(refid);
			}
		}
		break;
	case Progress::Finished:
		// given response, end of quest
		{
			state = Quest::Completed;
			game->AddReward(100);

			((City*)game->locations[start_loc])->quest_mayor = CityQuestState::None;
			game->current_dialog->pc->unit->RemoveQuestItem(refid);

			msgs.push_back(game->txQuest[7]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				if(!game->current_dialog->is_local)
					game->Net_RemoveQuestItem(game->current_dialog->pc, refid);
				game->Net_UpdateQuest(refid);
			}
		}
		break;
	}
}

//=================================================================================================
cstring Quest_DeliverLetter::FormatString(const string& str)
{
	Location& loc = *game->locations[end_loc];
	if(str == "target_burmistrza")
		return (loc.type == L_CITY ? game->txForMayor : game->txForSoltys);
	else if(str == "target_locname")
		return loc.name.c_str();
	else if(str == "target_dir")
		return kierunek_nazwa[GetLocationDir(game->locations[start_loc]->pos, loc.pos)];
	else
	{
		assert(0);
		return NULL;
	}
}

//=================================================================================================
bool Quest_DeliverLetter::IsTimedout()
{
	return game->worldtime - start_time > 30;
}

//=================================================================================================
bool Quest_DeliverLetter::IfHaveQuestItem()
{
	if(prog == Progress::Started)
		return game->current_location == end_loc;
	else
		return game->current_location == start_loc && prog == Progress::GotResponse;
}

//=================================================================================================
const Item* Quest_DeliverLetter::GetQuestItem()
{
	return &letter;
}

//=================================================================================================
void Quest_DeliverLetter::Save(HANDLE file)
{
	Quest::Save(file);

	if(prog < Progress::Finished)
		WriteFile(file, &end_loc, sizeof(end_loc), &tmp, NULL);
}

//=================================================================================================
void Quest_DeliverLetter::Load(HANDLE file)
{
	Quest::Load(file);

	if(prog < Progress::Finished)
	{
		ReadFile(file, &end_loc, sizeof(end_loc), &tmp, NULL);

		if(prog == Progress::GotResponse)
		{
			Location& loc = *game->locations[start_loc];
			letter.name = Format(game->txQuest[1], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str());
		}
		else
		{
			Location& loc = *game->locations[end_loc];
			letter.name = Format(game->txQuest[0], loc.type == L_CITY ? game->txForMayor : game->txForSoltys, loc.name.c_str());
		}

		letter.type = IT_OTHER;
		letter.weight = 1;
		letter.value = 0;
		letter.flags = ITEM_QUEST|ITEM_DONT_DROP|ITEM_IMPORTANT|ITEM_TEX_ONLY;
		letter.id = "$letter";
		letter.mesh.clear();
		letter.tex = game->tList;
		letter.refid = refid;
		letter.other_type = OtherItems;

		if(game->mp_load)
			game->Net_RegisterItem(&letter);
	}
}