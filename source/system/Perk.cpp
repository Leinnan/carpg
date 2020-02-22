#include "Pch.h"
#include "Perk.h"
#include "Attribute.h"
#include "Skill.h"
#include "Language.h"
#include "PlayerController.h"
#include "CreatedCharacter.h"
#include "Unit.h"

//-----------------------------------------------------------------------------
vector<Perk*> Perk::perks;
std::map<int, Perk*> Perk::hash_perks;

//=================================================================================================
old::v2::Perk old::Convert(v1::Perk perk, int value, bool& upgrade)
{
	upgrade = false;
	switch(perk)
	{
	case v1::Perk::Weakness:
		upgrade = true;
		switch((AttributeId)value)
		{
		case AttributeId::STR:
			return v2::Perk::BadBack;
		case AttributeId::DEX:
			return v2::Perk::Sluggish;
		case AttributeId::END:
			return v2::Perk::ChronicDisease;
		case AttributeId::INT:
			return v2::Perk::SlowLearner;
		case AttributeId::WIS:
			return v2::Perk::None;
		case AttributeId::CHA:
			return v2::Perk::Asocial;
		}
	case v1::Perk::Strength:
		return v2::Perk::Talent;
	case v1::Perk::Skilled:
		return v2::Perk::Skilled;
	case v1::Perk::SkillFocus:
		return v2::Perk::None;
	case v1::Perk::Talent:
		return v2::Perk::SkillFocus;
	case v1::Perk::AlchemistApprentice:
		return v2::Perk::AlchemistApprentice;
	case v1::Perk::Wealthy:
		return v2::Perk::Wealthy;
	case v1::Perk::VeryWealthy:
		return v2::Perk::VeryWealthy;
	case v1::Perk::FamilyHeirloom:
		return v2::Perk::FamilyHeirloom;
	case v1::Perk::Leader:
		return v2::Perk::Leader;
	default:
		return v2::Perk::None;
	}
}

//=================================================================================================
::Perk* old::Convert(v2::Perk perk)
{
	cstring id;
	switch(perk)
	{
	case v2::Perk::BadBack:
		id = "bad_back";
		break;
	case v2::Perk::ChronicDisease:
		id = "chronic_disease";
		break;
	case v2::Perk::Sluggish:
		id = "sluggish";
		break;
	case v2::Perk::SlowLearner:
		id = "slow_learner";
		break;
	case v2::Perk::Asocial:
		id = "asocial";
		break;
	case v2::Perk::Poor:
		id = "poor";
		break;
	case v2::Perk::Talent:
		id = "talent";
		break;
	case v2::Perk::Skilled:
		id = "skilled";
		break;
	case v2::Perk::SkillFocus:
		id = "skill_focus";
		break;
	case v2::Perk::AlchemistApprentice:
		id = "alchemist_apprentice";
		break;
	case v2::Perk::Wealthy:
		id = "wealthy";
		break;
	case v2::Perk::VeryWealthy:
		id = "very_wealthy";
		break;
	case v2::Perk::FamilyHeirloom:
		id = "heirloom";
		break;
	case v2::Perk::Leader:
		id = "leader";
		break;
	case v2::Perk::StrongBack:
		id = "strong_back";
		break;
	case v2::Perk::Aggressive:
		id = "aggressive";
		break;
	case v2::Perk::Mobility:
		id = "mobility";
		break;
	case v2::Perk::Finesse:
		id = "finesse";
		break;
	case v2::Perk::Tough:
		id = "tough";
		break;
	case v2::Perk::HardSkin:
		id = "hard_skin";
		break;
	case v2::Perk::Adaptation:
		id = "adaptation";
		break;
	case v2::Perk::PerfectHealth:
		id = "perfect_health";
		break;
	case v2::Perk::Energetic:
		id = "energetic";
		break;
	case v2::Perk::StrongAura:
		id = "strong_aura";
		break;
	case v2::Perk::ManaHarmony:
		id = "mana_harmony";
		break;
	case v2::Perk::MagicAdept:
		id = "magic_adept";
		break;
	case v2::Perk::TravelingMerchant:
		id = "traveling_merchant";
		break;
	default:
		return nullptr;
	}
	return Perk::Get(id);
}

//=================================================================================================
Perk* Perk::Get(int hash)
{
	auto it = hash_perks.find(hash);
	if(it != hash_perks.end())
		return it->second;
	return nullptr;
}

//=================================================================================================
void Perk::Validate(uint& err)
{
	for(Perk* perk : perks)
	{
		if(perk->name.empty())
		{
			Warn("Test: Perk %s: empty name.", perk->id.c_str());
			++err;
		}
		if(perk->desc.empty())
		{
			Warn("Test: Perk %s: empty desc.", perk->id.c_str());
			++err;
		}
	}
}

//=================================================================================================
bool PerkContext::HavePerk(Perk* perk)
{
	if(cc)
		return cc->HavePerk(perk);
	else
		return pc->HavePerk(perk, -1);
}

//=================================================================================================
bool PerkContext::Have(AttributeId a, int value)
{
	if(cc)
		return cc->a[(int)a].value >= value;
	else
		return pc->unit->stats->attrib[(int)a] >= value;
}

//=================================================================================================
bool PerkContext::Have(SkillId s, int value)
{
	if(cc)
		return cc->s[(int)s].value >= value;
	else
		return pc->unit->stats->skill[(int)s] >= value;
}

//=================================================================================================
bool PerkContext::CanMod(AttributeId a)
{
	if(check_remove)
		return true;
	if(cc)
		return !cc->a[(int)a].mod;
	else
		return true;
}

//=================================================================================================
void PerkContext::AddEffect(Perk* perk, EffectId effect, float power)
{
	if(pc)
	{
		Effect e;
		e.effect = effect;
		e.source = EffectSource::Perk;
		e.source_id = perk->hash;
		e.value = -1;
		e.power = power;
		e.time = 0.f;
		pc->unit->AddEffect(e, false); // don't send ADD_EFFECT, adding perk will add effects
	}
}

//=================================================================================================
void PerkContext::RemoveEffect(Perk* perk)
{
	if(pc)
		pc->unit->RemoveEffects(EffectId::None, EffectSource::Perk, perk->hash, -1);
}

//=================================================================================================
void PerkContext::Mod(AttributeId a, int value, bool mod)
{
	if(cc)
		cc->a[(int)a].Mod(value, mod);
	else
	{
		if(startup)
			pc->unit->stats->attrib[(int)a] += value;
		else
			pc->unit->Set(a, pc->unit->GetBase(a) + value);
		pc->attrib[(int)a].apt = (pc->unit->stats->attrib[(int)a] - 50) / 5;
	}
}

//=================================================================================================
void PerkContext::Mod(SkillId s, int value, bool mod)
{
	if(cc)
	{
		cc->s[(int)s].Mod(value, mod);
		cc->to_update.push_back(s);
	}
	else
	{
		if(startup)
			pc->unit->stats->skill[(int)s] += value;
		else
			pc->unit->Set(s, pc->unit->GetBase(s) + value);
		pc->skill[(int)s].apt = pc->unit->stats->skill[(int)s] / 5;
	}
}

//=================================================================================================
void PerkContext::AddPerk(Perk* perk)
{
	if(cc)
		cc->taken_perks.push_back(TakenPerk(perk));
	else
		pc->AddPerk(perk, -1);
}

//=================================================================================================
void PerkContext::RemovePerk(Perk* perk)
{
	if(cc)
	{
		for(auto it = cc->taken_perks.begin(), end = cc->taken_perks.end(); it != end; ++it)
		{
			if(it->perk == perk)
			{
				cc->taken_perks.erase(it);
				return;
			}
		}
	}
	else
		pc->RemovePerk(perk, -1);
}

//=================================================================================================
void TakenPerk::GetDetails(string& str) const
{
	str = perk->details;
	uint pos = str.find("{value}", 0);
	if(pos != string::npos)
	{
		cstring text;
		switch(perk->value_type)
		{
		case Perk::Attribute:
			text = Attribute::attributes[value].name.c_str();
			break;
		case Perk::Skill:
			text = Skill::skills[value].name.c_str();
			break;
		default:
			assert(0);
			return;
		}
		str.replace(pos, 7, text);
	}
}

//=================================================================================================
bool TakenPerk::CanTake(PerkContext& ctx)
{
	// can take start perk only at startup
	if(IsSet(perk->flags, Perk::Start) && !ctx.startup)
		return false;

	// can take only one history perk
	if(IsSet(perk->flags, Perk::History) && ctx.cc)
	{
		for(TakenPerk& tp : ctx.cc->taken_perks)
		{
			if(IsSet(tp.perk->flags, Perk::History))
				return false;
		}
	}

	// can't take more then 2 flaws
	if(IsSet(perk->flags, Perk::Flaw) && ctx.cc && ctx.cc->perks_max >= 4)
		return false;

	// check requirements
	for(Perk::Required& req : perk->required)
	{
		switch(req.type)
		{
		case Perk::Required::HAVE_PERK:
			if(!ctx.HavePerk(Perk::Get(req.subtype)))
			{
				if(!(ctx.validate && perk->parent == req.subtype))
					return false;
			}
			break;
		case Perk::Required::HAVE_NO_PERK:
			if(ctx.HavePerk(Perk::Get(req.subtype)))
				return false;
			break;
		case Perk::Required::HAVE_ATTRIBUTE:
			if(!ctx.Have((AttributeId)req.subtype, req.value))
				return false;
			break;
		case Perk::Required::HAVE_SKILL:
			if(!ctx.Have((SkillId)req.subtype, req.value))
				return false;
			break;
		case Perk::Required::CAN_MOD:
			if(!ctx.CanMod((AttributeId)req.subtype))
				return false;
			break;
		}
	}

	// check value type
	if(ctx.validate)
	{
		if(perk->value_type == Perk::Attribute)
		{
			if(value < 0 || value >= (int)AttributeId::MAX)
			{
				Error("Invalid perk 'Talent' value %d.", value);
				return false;
			}
		}
		else if(perk->value_type == Perk::Skill)
		{
			if(value < 0 || value >= (int)SkillId::MAX)
			{
				Error("Invalid perk 'SkillFocus' value %d.", value);
				return false;
			}
		}
	}

	return true;
}

//=================================================================================================
void TakenPerk::Apply(PerkContext& ctx)
{
	if(perk->parent != 0)
	{
		ctx.RemovePerk(Perk::Get(perk->parent));
		if(ctx.validate)
			ctx.cc->perks--; // takes two points
	}

	for(Perk::Effect& e : perk->effects)
	{
		switch(e.type)
		{
		case Perk::Effect::ATTRIBUTE:
			if(!ctx.upgrade)
			{
				AttributeId a = (AttributeId)e.subtype;
				if(a == AttributeId::NONE)
					a = (AttributeId)value;
				ctx.Mod(a, e.value, true);
			}
			break;
		case Perk::Effect::SKILL:
			{
				SkillId s = (SkillId)e.subtype;
				if(s == SkillId::NONE)
					s = (SkillId)value;
				ctx.Mod(s, e.value, true);
			}
			break;
		case Perk::Effect::EFFECT:
			ctx.AddEffect(perk, (EffectId)e.subtype, e.valuef);
			break;
		case Perk::Effect::APTITUDE:
			if(ctx.pc)
			{
				for(int i = 0; i < (int)SkillId::MAX; ++i)
					ctx.pc->skill[i].apt += e.value;
			}
			break;
		case Perk::Effect::SKILL_POINT:
			if(ctx.cc)
			{
				ctx.cc->sp += e.value;
				ctx.cc->sp_max += e.value;
				ctx.cc->update_skills = true;
			}
			break;
		case Perk::Effect::GOLD:
			if(ctx.pc && ctx.startup)
				ctx.pc->unit->gold += e.value;
			break;
		}
	}

	if(ctx.cc)
	{
		if(IsSet(perk->flags, Perk::Flaw))
		{
			ctx.cc->perks_max++;
			ctx.cc->perks++;
		}
		else
			ctx.cc->perks--;
	}
}

//=================================================================================================
void TakenPerk::Remove(PerkContext& ctx)
{
	if(perk->parent != 0)
		ctx.AddPerk(Perk::Get(perk->parent));

	for(Perk::Effect& e : perk->effects)
	{
		switch(e.type)
		{
		case Perk::Effect::ATTRIBUTE:
			{
				AttributeId a = (AttributeId)e.subtype;
				if(a == AttributeId::NONE)
					a = (AttributeId)value;
				ctx.Mod(a, -e.value, false);
			}
			break;
		case Perk::Effect::SKILL:
			{
				SkillId s = (SkillId)e.subtype;
				if(s == SkillId::NONE)
					s = (SkillId)value;
				ctx.Mod(s, -e.value, false);
			}
			break;
		case Perk::Effect::SKILL_POINT:
			if(ctx.cc)
			{
				ctx.cc->sp -= e.value;
				ctx.cc->sp_max -= e.value;
				ctx.cc->update_skills = true;
			}
			break;
		}
	}

	ctx.RemoveEffect(perk);

	if(ctx.cc)
	{
		if(IsSet(perk->flags, Perk::Flaw))
		{
			ctx.cc->perks_max--;
			ctx.cc->perks--;
		}
		else
			ctx.cc->perks++;
	}
}

//=================================================================================================
cstring TakenPerk::FormatName()
{
	switch(perk->value_type)
	{
	case Perk::Attribute:
		return Format("%s (%s)", perk->name.c_str(), Attribute::attributes[value].name.c_str());
	case Perk::Skill:
		return Format("%s (%s)", perk->name.c_str(), Skill::skills[value].name.c_str());
	default:
		assert(0);
		return perk->name.c_str();
	}
}
