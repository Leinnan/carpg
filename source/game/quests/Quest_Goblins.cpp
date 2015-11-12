#include "Pch.h"
#include "Base.h"
#include "Quest_Goblins.h"
#include "Dialog.h"
#include "DialogDefine.h"
#include "Game.h"
#include "Journal.h"
#include "SaveState.h"
#include "GameFile.h"

//-----------------------------------------------------------------------------
DialogEntry goblins_nobleman[] = {
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::None),
		TALK(557),
		SPECIAL("tell_name"),
		TALK(558),
		TALK(559),
		TALK(560),
		TALK(561),
		TALK(562),
		CHOICE(563),
			SET_QUEST_PROGRESS(Quest_Goblins::Progress::Started),
			TALK2(564),
			TALK(565),
			TALK(566),
			END,
		END_CHOICE,
		CHOICE(567),
			TALK(568),
			TALK(569),
			SET_QUEST_PROGRESS(Quest_Goblins::Progress::NotAccepted),
			END,
		END_CHOICE,
		ESCAPE_CHOICE,
		SHOW_CHOICES,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::NotAccepted),
		TALK(570),
		TALK(571),
		CHOICE(572),
			SET_QUEST_PROGRESS(Quest_Goblins::Progress::Started),
			TALK2(573),
			TALK(574),
			TALK(575),
			END,
		END_CHOICE,
		CHOICE(576),
			TALK(577),
			TALK(578),
			END,
		END_CHOICE,
		ESCAPE_CHOICE,
		SHOW_CHOICES,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::Started),
		TALK(579),
		END,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::BowStolen),
		TALK(580),
		CHOICE(581),
			SET_QUEST_PROGRESS(Quest_Goblins::Progress::TalkedAboutStolenBow),
			TALK(582),
			TALK(583),
			END,
		END_CHOICE,
		CHOICE(584),
			END,
		END_CHOICE,
		ESCAPE_CHOICE,
		SHOW_CHOICES,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::TalkedAboutStolenBow),
		TALK(585),
		END,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::InfoAboutGoblinBase),
		IF_HAVE_ITEM("q_gobliny_luk"),
			TALK(586),
			CHOICE(587),
				TALK(588),
				TALK(589),
				TALK(590),
				TALK(591),
				SET_QUEST_PROGRESS(Quest_Goblins::Progress::GivenBow),
				END,
			END_CHOICE,
			CHOICE(592),
				END,
			END_CHOICE,
			ESCAPE_CHOICE,
			SHOW_CHOICES,
		ELSE,
			TALK(593),
			END,
		END_IF,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::GivenBow),
		TALK(594),
		END,
	END_IF,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry goblins_encounter[] = {
	SET_QUEST_PROGRESS(Quest_Goblins::Progress::BowStolen),
	TALK(595),
	TALK(596),
	SPECIAL("attack"),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry goblins_messenger[] = {
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::TalkedAboutStolenBow),
		TALK(597),
		TALK(598),
		TALK(599),
		SET_QUEST_PROGRESS(Quest_Goblins::Progress::InfoAboutGoblinBase),
		TALK2(600),
		TALK(601),
		END,
	ELSE,
		TALK(602),
		END,
	END_IF,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry goblins_mage[] = {
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::GivenBow),
		TALK(603),
		TALK(604),
		TALK(605),
		CHOICE(606),
			TALK(607),
			TALK(608),
			SET_QUEST_PROGRESS(Quest_Goblins::Progress::TalkedAboutBow),
			END,
		END_CHOICE,
		CHOICE(609),
			SET_QUEST_PROGRESS(Quest_Goblins::Progress::DidntTalkedAboutBow),
			TALK(610),
			END,
		END_CHOICE,
		SHOW_CHOICES,
	END_IF,
	IF_QUEST_PROGRESS(Quest_Goblins::Progress::DidntTalkedAboutBow),
		TALK(611),
		TALK(612),
		CHOICE(613),
			IF_SPECIAL("have_100"),
				TALK(614),
				TALK(615),
				SET_QUEST_PROGRESS(Quest_Goblins::Progress::PayedAndTalkedAboutBow),
				END,
			ELSE,
				TALK(616),
				END,
			END_IF,
		END_CHOICE,
		CHOICE(617),
			END,
		END_CHOICE,
		ESCAPE_CHOICE,
		SHOW_CHOICES,
	END_IF,
	TALK2(618),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry goblins_innkeeper[] = {
	TALK(619),
	TALK(620),
	SET_QUEST_PROGRESS(Quest_Goblins::Progress::TalkedWithInnkeeper),
	TALK2(621),
	TALK(622),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry goblins_boss[] = {
	TALK(623),
	TALK(624),
	TALK(625),
	SPECIAL("attack"),
	END,
	END_OF_DIALOG
};

//=================================================================================================
void Quest_Goblins::Start()
{
	type = Type::Unique;
	quest_id = Q_GOBLINS;
	enc = -1;
	goblins_state = State::None;
	nobleman = NULL;
	messenger = NULL;
}

//=================================================================================================
DialogEntry* Quest_Goblins::GetDialog(int type2)
{
	assert(type2 == QUEST_DIALOG_NEXT);

	const string& id = game->current_dialog->talker->data->id;

	if(id == "q_gobliny_szlachcic")
		return goblins_nobleman;
	else if(id == "q_gobliny_mag")
		return goblins_mage;
	else if(id == "innkeeper")
		return goblins_innkeeper;
	else if(id == "q_gobliny_szlachcic2")
		return goblins_boss;
	else
	{
		assert(id == "q_gobliny_poslaniec");
		return goblins_messenger;
	}
}

//=================================================================================================
bool CzyMajaStaryLuk()
{
	return Game::Get().HaveQuestItem(FindItem("q_gobliny_luk"));
}

//=================================================================================================
void DodajStraznikow()
{
	Game& game = Game::Get();
	Unit* u = NULL;
	UnitData* ud = FindUnitData("q_gobliny_szlachcic2");

	// szukaj szlachcica
	for(vector<Unit*>::iterator it = game.local_ctx.units->begin(), end = game.local_ctx.units->end(); it != end; ++it)
	{
		if((*it)->data == ud)
		{
			u = *it;
			break;
		}
	}
	assert(u);

	// szukaj tronu
	Useable* use = NULL;

	for(vector<Useable*>::iterator it = game.local_ctx.useables->begin(), end = game.local_ctx.useables->end(); it != end; ++it)
	{
		if((*it)->type == U_TRON)
		{
			use = *it;
			break;
		}
	}
	assert(use);

	// przesu� szlachcica w poblirze tronu
	game.WarpUnit(*u, use->pos);

	// usu� pozosta�e osoby z pomieszczenia
	InsideLocation* inside = (InsideLocation*)game.location;
	InsideLocationLevel& lvl = inside->GetLevelData();
	Room* room = lvl.GetNearestRoom(u->pos);
	assert(room);
	for(vector<Unit*>::iterator it = game.local_ctx.units->begin(), end = game.local_ctx.units->end(); it != end; ++it)
	{
		if((*it)->data != ud && room->IsInside((*it)->pos))
		{
			(*it)->to_remove = true;
			game.to_remove.push_back(*it);
		}
	}

	// dodaj ochron�
	UnitData* ud2 = FindUnitData("q_gobliny_ochroniarz");
	for(int i=0; i<3; ++i)
	{
		Unit* u2 = game.SpawnUnitInsideRoom(*room, *ud2, 10);
		if(u2)
		{
			u2->dont_attack = true;
			u2->guard_target = u;
		}
	}
	
	// ustaw szlachcica
	u->hero->name = game.txQuest[215];
	u->hero->know_name = true;
	u->ApplyHumanData(game.quest_goblins->hd_nobleman);
}

//=================================================================================================
void Quest_Goblins::SetProgress(int prog2)
{
	prog = prog2;
	switch(prog2)
	{
	case Progress::NotAccepted:
		// nie zaakceptowano
		{
			// dodaj plotk�
			if(!game->plotka_questowa[P_GOBLINY])
			{
				game->plotka_questowa[P_GOBLINY] = true;
				--game->ile_plotek_questowych;
				cstring text = Format(game->txQuest[211], game->locations[start_loc]->name.c_str());
				game->rumors.push_back(Format(game->game_gui->journal->txAddNote, game->day+1, game->month+1, game->year, text));
				game->game_gui->journal->NeedUpdate(Journal::Rumors);
				game->AddGameMsg3(GMS_ADDED_RUMOR);
				if(game->IsOnline())
				{
					NetChange& c = Add1(game->net_changes);
					c.type = NetChange::ADD_RUMOR;
					c.id = int(game->rumors.size())-1;
				}
			}
		}
		break;
	case Progress::Started:
		// zaakceptowano
		{
			state = Quest::Started;
			name = game->txQuest[212];
			start_time = game->worldtime;
			// usu� plotk�
			if(!game->plotka_questowa[P_GOBLINY])
			{
				game->plotka_questowa[P_GOBLINY] = true;
				--game->ile_plotek_questowych;
			}
			// dodaj lokalizacje
			target_loc = game->GetNearestLocation2(GetStartLocation().pos, 1<<L_FOREST, true);
			Location& target = GetTargetLocation();
			if(target.state == LS_UNKNOWN)
				target.state = LS_KNOWN;
			target.reset = true;
			target.active_quest = this;
			target.st = 7;
			spawn_item = Quest_Event::Item_OnGround;
			item_to_give[0] = FindItem("q_gobliny_luk");
			// questowe rzeczy
			quest_index = game->quests.size();
			game->quests.push_back(this);
			RemoveElement<Quest*>(game->unaccepted_quests, this);
			msgs.push_back(Format(game->txQuest[217], GetStartLocationName(), game->day+1, game->month+1, game->year));
			msgs.push_back(Format(game->txQuest[218], GetTargetLocationName(), GetTargetLocationDir()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			// encounter
			Encounter* e = game->AddEncounter(enc);
			e->check_func = CzyMajaStaryLuk;
			e->dialog = goblins_encounter;
			e->dont_attack = true;
			e->grupa = SG_GOBLINY;
			e->location_event_handler = NULL;
			e->pos = GetStartLocation().pos;
			e->quest = (Quest_Encounter*)this;
			e->szansa = 10000;
			e->zasieg = 32.f;
			e->text = game->txQuest[219];
			e->timed = false;

			if(game->IsOnline())
				game->Net_AddQuest(refid);
		}
		break;
	case Progress::BowStolen:
		// gobliny ukrad�y �uk
		{
			game->RemoveQuestItem(FindItem("q_gobliny_luk"));
			msgs.push_back(game->txQuest[220]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			game->RemoveEncounter(enc);
			enc = -1;
			GetTargetLocation().active_quest = NULL;
			game->AddNews(game->txQuest[221]);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::TalkedAboutStolenBow:
		// poinformowano o kradzie�y
		{
			state = Quest::Failed;
			msgs.push_back(game->txQuest[222]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			goblins_state = State::Counting;
			days = random(15, 30);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::InfoAboutGoblinBase:
		// pos�aniec dostarczy� info o bazie goblin�w
		{
			state = Quest::Started;
			target_loc = game->GetRandomSpawnLocation(GetStartLocation().pos, SG_GOBLINY);
			Location& target = GetTargetLocation();
			bool now_known = false;
			if(target.state == LS_UNKNOWN)
			{
				target.state = LS_KNOWN;
				now_known = true;
			}
			target.st = 11;
			target.reset = true;
			target.active_quest = this;
			done = false;
			spawn_item = Quest_Event::Item_GiveSpawned;
			unit_to_spawn = FindUnitData(GetSpawnLeader(SG_GOBLINY));
			unit_spawn_level = -3;
			item_to_give[0] = FindItem("q_gobliny_luk");
			at_level = target.GetLastLevel();
			msgs.push_back(Format(game->txQuest[223], target.name.c_str(), GetTargetLocationDir(), GetStartLocationName()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			goblins_state = State::MessengerTalked;

			if(game->IsOnline())
			{
				game->Net_UpdateQuest(refid);
				if(now_known)
					game->Net_ChangeLocationState(target_loc, false);
			}
		}
		break;
	case Progress::GivenBow:
		// oddano �uk
		{
			state = Quest::Completed;
			const Item* item = FindItem("q_gobliny_luk");
			game->RemoveItem(*game->current_dialog->pc->unit, item, 1);
			game->current_dialog->talker->AddItem(item, 1, true);
			game->AddReward(500);
			msgs.push_back(game->txQuest[224]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			goblins_state = State::GivenBow;
			GetTargetLocation().active_quest = NULL;
			target_loc = -1;
			game->AddNews(game->txQuest[225]);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::DidntTalkedAboutBow:
		// nie chcia�e� powiedzie� o �uku
		{
			msgs.push_back(game->txQuest[226]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			goblins_state = State::MageTalkedStart;

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::TalkedAboutBow:
		// powiedzia�e� o �uku
		{
			state = Quest::Started;
			msgs.push_back(game->txQuest[227]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			goblins_state = State::MageTalked;

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::PayedAndTalkedAboutBow:
		// zap�aci�e� i powiedzia�e� o �uku
		{
			game->current_dialog->pc->unit->gold -= 100;

			state = Quest::Started;
			msgs.push_back(game->txQuest[228]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			goblins_state = State::MageTalked;

			if(game->IsOnline())
			{
				game->Net_UpdateQuest(refid);
				if(!game->current_dialog->is_local)
					game->GetPlayerInfo(game->current_dialog->pc).UpdateGold();
			}
		}
		break;
	case Progress::TalkedWithInnkeeper:
		// pogadano z karczmarzem
		{
			goblins_state = State::KnownLocation;
			target_loc = game->CreateLocation(L_DUNGEON, game->world_pos, 128.f, THRONE_FORT, SG_GOBLINY, false);
			Location& target = GetTargetLocation();
			target.st = 13;
			target.state = LS_KNOWN;
			target.active_quest = this;
			done = false;
			unit_to_spawn = FindUnitData("q_gobliny_szlachcic2");
			spawn_unit_room = POKOJ_CEL_TRON;
			callback = DodajStraznikow;
			unit_dont_attack = true;
			unit_auto_talk = true;
			unit_event_handler = this;
			item_to_give[0] = NULL;
			spawn_item = Quest_Event::Item_DontSpawn;
			at_level = target.GetLastLevel();
			msgs.push_back(Format(game->txQuest[229], target.name.c_str(), GetTargetLocationDir(), GetStartLocationName()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				game->Net_UpdateQuest(refid);
				game->Net_ChangeLocationState(target_loc, false);
			}
		}
		break;
	case Progress::KilledBoss:
		// zabito szlachcica
		{
			state = Quest::Completed;
			msgs.push_back(game->txQuest[230]);
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);
			GetTargetLocation().active_quest = NULL;
			game->EndUniqueQuest();
			game->AddNews(game->txQuest[231]);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	}
}

//=================================================================================================
cstring Quest_Goblins::FormatString(const string& str)
{
	if(str == "target_loc")
		return GetTargetLocationName();
	else if(str == "target_dir")
		return GetTargetLocationDir();
	else if(str == "start_loc")
		return GetStartLocationName();
	else
	{
		assert(0);
		return NULL;
	}
}

//=================================================================================================
bool Quest_Goblins::IfNeedTalk(cstring topic) const
{
	return strcmp(topic, "gobliny") == 0;
}

//=================================================================================================
void Quest_Goblins::HandleUnitEvent(UnitEventHandler::TYPE event, Unit* unit)
{
	assert(unit);

	if(event == UnitEventHandler::DIE && prog == Progress::TalkedWithInnkeeper)
	{
		SetProgress(Progress::KilledBoss);
		unit->event_handler = NULL;
	}
}

//=================================================================================================
void Quest_Goblins::Save(HANDLE file)
{
	Quest_Dungeon::Save(file);

	GameWriter f(file);

	f << enc;
	f << goblins_state;
	f << days;
	f << nobleman;
	f << messenger;
	f << hd_nobleman;
}

//=================================================================================================
void Quest_Goblins::Load(HANDLE file)
{
	Quest_Dungeon::Load(file);

	GameReader f(file);

	f >> enc;

	if(LOAD_VERSION >= V_0_4)
	{
		f >> goblins_state;
		f >> days;
		f >> nobleman;
		f >> messenger;
		f >> hd_nobleman;
	}

	if(!done)
	{
		if(prog == Progress::Started)
		{
			spawn_item = Quest_Event::Item_OnGround;
			item_to_give[0] = FindItem("q_gobliny_luk");
		}
		else if(prog == Progress::InfoAboutGoblinBase)
		{
			spawn_item = Quest_Event::Item_GiveSpawned;
			unit_to_spawn = FindUnitData(GetSpawnLeader(SG_GOBLINY));
			unit_spawn_level = -3;
			item_to_give[0] = FindItem("q_gobliny_luk");
		}
		else if(prog == Progress::TalkedWithInnkeeper)
		{
			unit_to_spawn = FindUnitData("q_gobliny_szlachcic2");
			spawn_unit_room = POKOJ_CEL_BRAK;
			callback = DodajStraznikow;
			unit_dont_attack = true;
			unit_auto_talk = true;
		}
	}

	unit_event_handler = this;

	if(enc != -1)
	{
		Encounter* e = game->RecreateEncounter(enc);
		e->check_func = CzyMajaStaryLuk;
		e->dialog = goblins_encounter;
		e->dont_attack = true;
		e->grupa = SG_GOBLINY;
		e->location_event_handler = NULL;
		e->pos = GetStartLocation().pos;
		e->quest = (Quest_Encounter*)this;
		e->szansa = 10000;
		e->zasieg = 32.f;
		e->text = game->txQuest[219];
		e->timed = false;
	}
}

//=================================================================================================
void Quest_Goblins::LoadOld(HANDLE file)
{
	GameReader f(file);
	int refid, city;

	f >> goblins_state;
	f >> refid;
	f >> city;
	f >> days;
	f >> nobleman;
	f >> messenger;
	f >> hd_nobleman;
}
