#include "Pch.h"
#include "Base.h"
#include "StatsPanel.h"
#include "Unit.h"
#include "PlayerController.h"
#include "KeyStates.h"
#include "Game.h"
#include "Language.h"

//-----------------------------------------------------------------------------
#define INDEX_ATTRIB 0
#define INDEX_SKILL 1
#define INDEX_DATE 2

//=================================================================================================
StatsPanel::StatsPanel() : tbAttribs(true)
{
	visible = false;

	txStatsPanel = Str("statsPanel");
	txTraitsText = Str("traitsText");
	txStatsText = Str("statsText");
	txYearMonthDay = Str("yearMonthDay");
	txBase = Str("base");
	txRelatedAttributes = Str("relatedAttributes");

	flow.pos = INT2(10,40);
	flow.parent = this;
	flow.moved = 0;

	flow.Add(new StaticText(Str("attributes"), GUI.fBig));

	tAttribs = new StaticText("", GUI.default_font);
	flow.Add(tAttribs);

	flow.Add(new StaticText(Str("skills"), GUI.fBig));

	tSkills = new StaticText("", GUI.default_font);
	flow.Add(tSkills);

	flow.Add(new StaticText(Str("traits"), GUI.fBig));

	tTraits = new StaticText("", GUI.default_font);
	flow.Add(tTraits);

	flow.Add(new StaticText(Str("stats"), GUI.fBig));

	tStats = new StaticText("", GUI.default_font);
	flow.Add(tStats);

	tooltip.Init(TooltipGetText(this, &StatsPanel::GetTooltip));


	tbAttribs.text = "Atrybuty";
	tbAttribs.readonly = true;
	tbStats.text = "Statsy";
	tbSkills.text = "Skille";
	tbFeats.text = "Perki";
}

//=================================================================================================
StatsPanel::~StatsPanel()
{
	flow.DeleteItems();
}

//=================================================================================================
void StatsPanel::Draw(ControlDrawData*)
{
	GamePanel::Draw();
	/*scrollbar.Draw();

	SetText();

	RECT rect = {
		pos.x+8,
		pos.y+8,
		pos.x+size.x-16,
		pos.y+size.y-16
	};
	GUI.DrawText(GUI.fBig, txStatsPanel, DT_TOP|DT_CENTER, BLACK, rect);

	flow.Draw();

	tooltip.Draw();*/

	tbAttribs.Draw();
}

//=================================================================================================
void StatsPanel::Event(GuiEvent e)
{
	GamePanel::Event(e);

	if(e == GuiEvent_Moved)
	{
		flow.global_pos = global_pos + flow.pos;
		scrollbar.global_pos = global_pos + scrollbar.pos;

		tbAttribs.UpdateSize(global_pos+INT2(16,16), INT2(150, 150));
	}
	else if(e == GuiEvent_Resize)
	{
		flow.size = size - INT2(44,48);
		scrollbar.pos = INT2(size.x-28, 48);
		scrollbar.global_pos = global_pos + scrollbar.pos;
		scrollbar.size = INT2(16, size.y-60);
		if(scrollbar.offset+scrollbar.part > scrollbar.total)
			scrollbar.offset = float(scrollbar.total-scrollbar.part);
		if(scrollbar.offset < 0)
			scrollbar.offset = 0;
		flow.moved = int(scrollbar.offset);
	}
	else if(e == GuiEvent_Show)
		SetText();
	else if(e == GuiEvent_LostFocus)
		scrollbar.LostFocus();
}

//=================================================================================================
void StatsPanel::Update(float dt)
{
	GamePanel::Update(dt);
	if(focus && Key.Focus() && IsInside(GUI.cursor_pos))
		scrollbar.ApplyMouseWheel();
	scrollbar.Update(dt);
	flow.moved = int(scrollbar.offset);

	int group = -1, id = -1;

	if(focus)
		GUI.Intersect(flow.hitboxes, GUI.cursor_pos, &group, &id);

	tbAttribs.mouse_focus = focus;
	tbAttribs.Update(dt);

	tooltip.Update(dt, group, id);

	if(focus && Key.Focus() && Key.PressedRelease(VK_ESCAPE))
		Hide();
}

//=================================================================================================
void StatsPanel::SetText()
{
	{
		string& s = tAttribs->text;
		s.clear();
		for(int i=0; i<(int)Attribute::MAX; ++i)
		{
			s += Format("$g+0,%d;%s: ", i, g_attributes[i].name.c_str());
			int mod = pc->unit->GetAttributeState(i);
			if(mod != 0)
			{
				if(mod == 1)
					s += "$cg";
				else if(mod == 2)
					s += "$cr";
				else
					s += "$cy";
			}
			s += Format("%d", pc->unit->GetAttribute(i));
			if(i == (int)Attribute::DEX)
			{
				int dex = (int)pc->unit->CalculateDexterity();
				if(dex != pc->unit->GetAttribute((int)Attribute::DEX))
					s += Format(" (%d)", dex);
			}
			if(mod != 0)
				s += "$c-";
			s += "$g-\n";
		}
	}

	{
		string& s = tSkills->text;
		s.clear();
		for(int i=0; i<(int)Skill::MAX; ++i)
		{
			s += Format("$g+1,%d;%s: ", i, g_skills[i].name.c_str());
			int mod = pc->unit->GetSkillState(i);
			if(mod != 0)
			{
				if(mod == 1)
					s += "$cg";
				else if(mod == 2)
					s += "$cr";
				else
					s += "$cy";
			}
			s += Format("%d", pc->unit->GetSkill(i));
			if(mod != 0)
				s += "$c-";
			s += "$g-\n";
		}
	}

	int hp = int(pc->unit->hp);
	if(hp == 0 && pc->unit->hp > 0)
		hp = 1;
	cstring meleeAttack = (pc->unit->HaveWeapon() ? Format("%d", (int)pc->unit->CalculateAttack(&pc->unit->GetWeapon())) : "-");
	cstring rangedAttack = (pc->unit->HaveBow() ? Format("%d", (int)pc->unit->CalculateAttack(&pc->unit->GetBow())) : "-");
	tTraits->text = Format(txTraitsText, g_classes[(int)pc->clas].name.c_str(), hp, int(pc->unit->hpmax), meleeAttack, rangedAttack,
		(int)pc->unit->CalculateDefense(), float(pc->unit->weight)/10, float(pc->unit->weight_max)/10, pc->unit->gold);

	Game& game = Game::Get();
	tStats->text = Format(txStatsText, game.year, game.month+1, game.day+1, game.gt_hour, game.gt_minute, game.gt_second, pc->kills, pc->knocks, pc->dmg_done, pc->dmg_taken, pc->arena_fights);

	flow.Calculate();

	scrollbar.total = flow.total_size;
	scrollbar.part = flow.size.y;
	if(scrollbar.offset+scrollbar.part > scrollbar.total)
		scrollbar.offset = float(scrollbar.total-scrollbar.part);
	if(scrollbar.offset < 0)
		scrollbar.offset = 0;
	flow.moved = int(scrollbar.offset);
}

//=================================================================================================
void StatsPanel::GetTooltip(TooltipController*, int group, int id)
{
	tooltip.anything = true;
	tooltip.img = NULL;
	if(group == INDEX_DATE)
	{
		Game& game = Game::Get();
		tooltip.text = Format(txYearMonthDay, game.year, game.month + 1, game.day + 1);
		tooltip.big_text.clear();
		tooltip.small_text.clear();
	}
	else if(group == INDEX_ATTRIB)
	{
		AttributeInfo& ai = g_attributes[id];
		tooltip.big_text = Format("%s: %d", ai.name.c_str(), pc->unit->attrib[id]);
		int base, base_start;
		StatState unused;
		pc->unit->GetAttribute((Attribute)id, base, base_start, unused);
		tooltip.text = Format("%s: %d/%d\n%s", txBase, base, base_start, ai.desc.c_str());
		tooltip.small_text.clear();

	}
	else if(group == INDEX_SKILL)
	{
		SkillInfo& si = g_skills[id];
		tooltip.big_text = Format("%s: %d", si.name.c_str(), pc->unit->skill[id]);
		int base, base_start;
		StatState unused;
		pc->unit->GetSkill((Skill)id, base, base_start, unused);
		tooltip.text = Format("%s: %d/%d\n%s", txBase, base, base_start, si.desc.c_str());
		if(si.attrib2 != Attribute::NONE)
			tooltip.small_text = Format("%s: %s, %s", txRelatedAttributes, g_attributes[(int)si.attrib].name.c_str(), g_attributes[(int)si.attrib2].name.c_str());
		else
			tooltip.small_text = Format("%s: %s", txRelatedAttributes, g_attributes[(int)si.attrib].name.c_str());
	}
	else
		tooltip.anything = false;
}

//=================================================================================================
void StatsPanel::Show()
{
	visible = true;
	Event(GuiEvent_Show);
	GainFocus();
}

//=================================================================================================
void StatsPanel::Hide()
{
	LostFocus();
	visible = false;
}
