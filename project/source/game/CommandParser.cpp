#include "Pch.h"
#include "GameCore.h"
#include "CommandParser.h"
#include "BitStreamFunc.h"
#include "ConsoleCommands.h"
#include "Level.h"
#include "PlayerInfo.h"
#include "NetChangePlayer.h"

bool CommandParser::ParseStream(BitStreamReader& f, PlayerInfo& info)
{
	LocalString str;
	PrintMsgFunc prev_func = g_print_func;
	g_print_func = [&str](cstring s)
	{
		if(!str.empty())
			str += "\n";
		str += s;
	};

	bool result = ParseStreamInner(f);

	g_print_func = prev_func;

	if(result && !str.empty())
	{
		NetChangePlayer& c = Add1(info.changes);
		c.type = NetChangePlayer::GENERIC_CMD_RESPONSE;
		c.str = str.Pin();
	}

	return result;
}

bool CommandParser::ParseStreamInner(BitStreamReader& f)
{
	CMD cmd = (CMD)f.Read<byte>();
	switch(cmd)
	{
	case CMD_ADD_EFFECT:
		{
			int netid;
			Effect e;

			f >> netid;
			f.ReadCasted<char>(e.effect);
			f.ReadCasted<char>(e.source);
			f.ReadCasted<char>(e.source_id);
			f >> e.power;
			f >> e.time;

			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_ADD_EFFECT: Missing unit %d.", netid);
				return false;
			}
			if(e.effect >= EffectId::Max)
			{
				Error("CommandParser CMD_ADD_EFFECT: Invalid effect %d.", e.effect);
				return false;
			}
			if(e.source >= EffectSource::Max)
			{
				Error("CommandParser CMD_ADD_EFFECT: Invalid effect source %d.", e.source);
				return false;
			}
			if(e.source == EffectSource::Perk)
			{
				if(e.source_id >= (int)Perk::Max)
				{
					Error("CommandParser CMD_ADD_EFFECT: Invalid source id %d for perk source.", e.source_id);
					return false;
				}
			}
			else if(e.source_id != -1)
			{
				Error("CommandParser CMD_ADD_EFFECT: Invalid source id %d for source %d.", e.source_id, e.source);
				return false;
			}
			if(e.time > 0 && e.source != EffectSource::Temporary)
			{
				Error("CommandParser CMD_ADD_EFFECT: Invalid time %g for source %d.", e.time, e.source);
				return false;
			}

			unit->AddEffect(e);
		}
		break;
	case CMD_REMOVE_EFFECT:
		{
			int netid;
			EffectId effect;
			EffectSource source;
			int source_id;

			f >> netid;
			f.ReadCasted<char>(effect);
			f.ReadCasted<char>(source);
			f.ReadCasted<char>(source_id);

			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_REMOVE_EFFECT: Missing unit %d.", netid);
				return false;
			}
			if(effect >= EffectId::Max)
			{
				Error("CommandParser CMD_REMOVE_EFFECT: Invalid effect %d.", effect);
				return false;
			}
			if(source >= EffectSource::Max)
			{
				Error("CommandParser CMD_REMOVE_EFFECT: Invalid effect source %d.", source);
				return false;
			}
			if(source == EffectSource::Perk)
			{
				if(source_id >= (int)Perk::Max)
				{
					Error("CommandParser CMD_REMOVE_EFFECT: Invalid source id %d for perk source.", source_id);
					return false;
				}
			}
			else if(source_id != -1)
			{
				Error("CommandParser CMD_REMOVE_EFFECT: Invalid source id %d for source %d.", source_id, source);
				return false;
			}

			RemoveEffect(unit, effect, source, source_id);
		}
		break;
	case CMD_LIST_EFFECTS:
		{
			int netid;
			f >> netid;
			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_LIST_EFFECTS: Missing unit %d.", netid);
				return false;
			}
			ListEffects(unit);
		}
		break;
	case CMD_ADD_PERK:
		{
			int netid, value;
			Perk perk;

			f >> netid;
			f.ReadCasted<char>(perk);
			f.ReadCasted<char>(value);

			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_ADD_PERK: Missing unit %d.", netid);
				return false;
			}
			if(!unit->player)
			{
				Error("CommandParser CMD_ADD_PERK: Unit %d is not player.", netid);
				return false;
			}
			if(perk >= Perk::Max)
			{
				Error("CommandParser CMD_ADD_PERK: Invalid perk %d.", perk);
				return false;
			}
			PerkInfo& info = PerkInfo::perks[(int)perk];
			if(info.value_type == PerkInfo::None)
			{
				if(value != -1)
				{
					Error("CommandParser CMD_ADD_PERK: Invalid value %d for perk '%s'.", value, info.id);
					return false;
				}
			}
			else if(info.value_type == PerkInfo::Attribute)
			{
				if(value <= (int)AttributeId::MAX)
				{
					Error("CommandParser CMD_ADD_PERK: Invalid value %d for perk '%s'.", value, info.id);
					return false;
				}
			}
			else if(info.value_type == PerkInfo::Skill)
			{
				if(value <= (int)SkillId::MAX)
				{
					Error("CommandParser CMD_ADD_PERK: Invalid value %d for perk '%s'.", value, info.id);
					return false;
				}
			}

			AddPerk(unit->player, perk, value);
		}
		break;
	case CMD_REMOVE_PERK:
		{
			int netid, value;
			Perk perk;

			f >> netid;
			f.ReadCasted<char>(perk);
			f.ReadCasted<char>(value);

			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_REMOVE_PERK: Missing unit %d.", netid);
				return false;
			}
			if(!unit->player)
			{
				Error("CommandParser CMD_REMOVE_PERK: Unit %d is not player.", netid);
				return false;
			}
			if(perk >= Perk::Max)
			{
				Error("CommandParser CMD_REMOVE_PERK: Invalid perk %d.", perk);
				return false;
			}
			PerkInfo& info = PerkInfo::perks[(int)perk];
			if(info.value_type == PerkInfo::None)
			{
				if(value != -1)
				{
					Error("CommandParser CMD_REMOVE_PERK: Invalid value %d for perk '%s'.", value, info.id);
					return false;
				}
			}
			else if(info.value_type == PerkInfo::Attribute)
			{
				if(value <= (int)AttributeId::MAX)
				{
					Error("CommandParser CMD_REMOVE_PERK: Invalid value %d for perk '%s'.", value, info.id);
					return false;
				}
			}
			else if(info.value_type == PerkInfo::Skill)
			{
				if(value <= (int)SkillId::MAX)
				{
					Error("CommandParser CMD_REMOVE_PERK: Invalid value %d for perk '%s'.", value, info.id);
					return false;
				}
			}

			RemovePerk(unit->player, perk, value);
		}
		break;
	case CMD_LIST_PERKS:
		{
			int netid;
			f >> netid;
			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_LIST_PERKS: Missing unit %d.", netid);
				return false;
			}
			if(!unit->player)
			{
				Error("CommandParser CMD_LIST_PERKS: Unit %d is not player.", netid);
				return false;
			}
			ListPerks(unit->player);
		}
		break;
	case CMD_LIST_STATS:
		{
			int netid;
			f >> netid;
			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_LIST_STAT: Missing unit %d.", netid);
				return false;
			}
			ListStats(unit);
		}
		break;
	case CMD_ADD_LEARNING_POINTS:
		{
			int netid, count;

			f >> netid;
			f >> count;

			Unit* unit = L.FindUnit(netid);
			if(!unit)
			{
				Error("CommandParser CMD_ADD_LEARNING_POINTS: Missing unit %d.", netid);
				return false;
			}
			if(!unit->player)
			{
				Error("CommandParser CMD_ADD_LEARNING_POINTS: Unit %d is not player.", netid);
				return false;
			}
			if(count < 1)
			{
				Error("CommandParser CMD_ADD_LEARNING_POINTS: Invalid count %d.", count);
				return false;
			}
			
			unit->player->AddLearningPoint(count);
		}
		break;
	default:
		Error("Unknown generic command %u.", cmd);
		return false;
	}
	return true;
}

void CommandParser::RemoveEffect(Unit* u, EffectId effect, EffectSource source, int source_id)
{
	uint removed = u->RemoveEffects(effect, source, source_id);
	Msg("%u effects removed.", removed);
}

void CommandParser::ListEffects(Unit* u)
{
	if(u->effects.empty())
	{
		Msg("Unit have no effects.");
		return;
	}

	LocalString s;
	s = Format("Unit effects (%u):", u->effects.size());
	for(Effect& e : u->effects)
	{
		s += Format("\n%s, power %g, source ", EffectInfo::effects[(int)e.effect].id, e.power);
		switch(e.source)
		{
		case EffectSource::Temporary:
			s += Format("temporary, time %g", e.time);
			break;
		case EffectSource::Perk:
			s += Format("perk (%s)", PerkInfo::perks[e.source_id].id);
			break;
		case EffectSource::Permanent:
			s += "permanent";
			break;
		}
	}
	Msg(s.c_str());
}

void CommandParser::AddPerk(PlayerController* pc, Perk perk, int value)
{
	if(!pc->AddPerk(perk, value))
		Msg("Unit already have this perk.");
}

void CommandParser::RemovePerk(PlayerController* pc, Perk perk, int value)
{
	if(!pc->RemovePerk(perk, value))
		Msg("Unit don't have this perk.");
}

void CommandParser::ListPerks(PlayerController* pc)
{
	if(pc->perks.empty())
	{
		Msg("Unit have no perks.");
		return;
	}

	LocalString s;
	s = Format("Unit perks (%u):", pc->perks.size());
	for(TakenPerk& tp : pc->perks)
	{
		PerkInfo& info = PerkInfo::perks[(int)tp.perk];
		s += Format("\n%s", info.id);
		if(info.value_type == PerkInfo::Attribute)
			s += Format(" (%s)", Attribute::attributes[tp.value].id);
		else if(info.value_type == PerkInfo::Skill)
			s += Format(" (%s)", Skill::skills[tp.value].id);
	}
	Msg(s.c_str());
}

void CommandParser::ListStats(Unit* u)
{
	Msg("Health bonus: %+g", u->GetEffectSum(EffectId::Health));
	Msg("Regeneration bonus: %+g/sec", u->GetEffectSum(EffectId::Regeneration));
	Msg("Natural healing mod: x%g", u->GetEffectMul(EffectId::NaturalHealingMod));
	Msg("Attack bonus, melee: %+g,  ranged: %+g", u->GetEffectSum(EffectId::MeleeAttack), u->GetEffectSum(EffectId::RangedAttack));
	Msg("Defense bonus: %+g", u->GetEffectSum(EffectId::Defense));
	Msg("Mobility: %d (bonus %+g)", (int)u->CalculateMobility(), u->GetEffectSum(EffectId::Mobility));
	Msg("Carry mod: x%g", u->GetEffectMul(EffectId::Carry));
	Msg("Magic resistance: %d%%", (int)((1.f - u->CalculateMagicResistance()) * 100));
	Msg("Poison resistance: %d%%", (int)((1.f - u->GetEffectMul(EffectId::PoisonResistance)) * 100));
}