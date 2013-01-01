 /*
  * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
  *
  * This program is free software; you can redistribute it and/or modify it
  * under the terms of the GNU General Public License as published by the
  * Free Software Foundation; either version 3 of the License, or (at your
  * option) any later version.
  *
  * This program is distributed in the hope that it will be useful, but WITHOUT
  * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
  * more details.
  *
  * You should have received a copy of the GNU General Public License along
  * with this program. If not, see <http://www.gnu.org/licenses/>.
  */

/* ScriptData
Name: Boss_Viscidus
%Complete: 75%
Comment:
Category: Temple of Ahn'Qiraj
EndScriptData */

#include "ScriptPCH.h"
#include "temple_of_ahnqiraj.h"

enum Spells
{
    SPELL_TOXIN_CLOUD               = 26575, // Aura

    SPELL_POISON_SHOCK              = 25993,
    SPELL_POISONBOLT_VOLLEY         = 25991,

    SPELL_VISCIDUS_FREEZE_1         = 26034,
    SPELL_VISCIDUS_FREEZE_2         = 26036,
    SPELL_VISCIDUS_FREEZE_3         = 25937,
};

float Spell_Summon_glob[20] =
{
    25865, //HP > 5%
    25866, //HP > 10%
    25867, //HP > 15%
    25868, //HP > 20%
    25869, //HP > 25%
    25870, //HP > 30%
    25871, //HP > 35%
    25872, //HP > 40%
    25873, //HP > 45%
    25874, //HP > 50%
    25875, //HP > 55%
    25876, //HP > 45%
    25877, //HP > 65%
    25878, //HP > 70%
    25879, //HP > 75%
    25880, //HP > 80%
    25881, //HP > 85%
    25882, //HP > 90%
    25883, //HP > 95%
    25884, //HP == 100% ???
};

#define EMOTE_VISCIDUS_FREEZE_1         "begins to slow."
#define EMOTE_VISCIDUS_FREEZE_2         "begins to freeze."
#define EMOTE_VISCIDUS_FREEZE_3         "is frozen solid."

#define EMOTE_VISCIDUS_CRACK_1          "begins to crack."
#define EMOTE_VISCIDUS_CRACK_2          "looks ready to shatter."
#define EMOTE_VISCIDUS_CRACK_3          "explodes."

enum ViscidusState
{
    NORMAL_STATE_1 = 0,
    NORMAL_STATE_2,
    NORMAL_STATE_3,
    FROZEN_STATE_1,
    FROZEN_STATE_2,
    FROZEN_STATE_3,
    SHATTRED_STATE,
};

struct boss_viscidusAI : public ScriptedAI
{
    boss_viscidusAI(Creature *creature) : ScriptedAI(creature)
    {
        instance = ((ScriptedInstance*)creature->GetInstanceScript());
    }

    ScriptedInstance* instance;
    ViscidusState state;
    uint32 HitCounter;

    void Reset()
    {
        state = NORMAL_STATE_1;
    }

    void Aggro(Unit *who){}

    void DamageTaken(Unit* Done_by, uint32 &Damage)
    {
        if (Damage > 0) Damage = Damage/2;
    }

    void NextState(ViscidusState &state)
    {
        switch (state)
        {
        case NORMAL_STATE_1:
            state = NORMAL_STATE_2;
            break;
        case NORMAL_STATE_2:
            state = NORMAL_STATE_3;
            break;
        case NORMAL_STATE_3:
            state = FROZEN_STATE_1;
            break;
        case FROZEN_STATE_2:
            state = FROZEN_STATE_3;
            break;
        case FROZEN_STATE_3:
            state = SHATTRED_STATE;
            break;
        case SHATTRED_STATE:
            state = NORMAL_STATE_1;
            break;
        }
    }

    void ElementalDamageTaken(Unit* Done_by, uint32 &Damage, SpellSchoolMask damageSchoolMask)
    {
        if (Done_by->GetTypeId() == TYPEID_PLAYER)
        {
            switch (state)
            {
            case NORMAL_STATE_1:
            case NORMAL_STATE_2:
            case NORMAL_STATE_3:
                if (damageSchoolMask == SPELL_SCHOOL_MASK_FROST) //Only frost damage
                {
                    if (++HitCounter >= 200)
                    {
                        NextState(state);
                        HitCounter = 0;
                        ChangePhase(state);
                    }
                }
                break;
            case FROZEN_STATE_1:
            case FROZEN_STATE_2:
            case FROZEN_STATE_3:
                if (damageSchoolMask == SPELL_SCHOOL_MASK_NORMAL) //Only Physical Damage
                {
                    if (++HitCounter >= 200)
                    {
                        NextState(state);
                        HitCounter = 0;
                        ChangePhase(state);
                    }
                }
                break;
            }
        }
    }

    void ChangePhase(ViscidusState NextPhase)
    {
        switch (NextPhase)
        {
            case NORMAL_STATE_1:
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_1);
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_2);
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_3);
                break;
            case NORMAL_STATE_2:
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_2);
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_3);

                me->CastSpell(me, SPELL_VISCIDUS_FREEZE_1, true);
                me->MonsterTextEmote(EMOTE_VISCIDUS_FREEZE_1, 0, true);
                break;
            case NORMAL_STATE_3:
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_1);
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_3);

                me->CastSpell(me,SPELL_VISCIDUS_FREEZE_2, true);
                me->MonsterTextEmote(EMOTE_VISCIDUS_FREEZE_2, 0, true);
                break;
            case FROZEN_STATE_1:
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_1);
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE_2);

                me->CastSpell(me,SPELL_VISCIDUS_FREEZE_3, true);
                me->MonsterTextEmote(EMOTE_VISCIDUS_FREEZE_3, 0, true);
                break;
            case FROZEN_STATE_2:
                me->MonsterTextEmote(EMOTE_VISCIDUS_CRACK_1, 0, true);
                break;
            case FROZEN_STATE_3:
                me->MonsterTextEmote(EMOTE_VISCIDUS_CRACK_2, 0, true);
                break;
            case SHATTRED_STATE:
                me->MonsterTextEmote(EMOTE_VISCIDUS_CRACK_3, 0, true);
                DoSummonGlobs();
                break;
        }
    }

    void DoSummonGlobs()
    {
        uint32 hpborder = 5;
        for (int i = 0; i < 20; i++)
        {
            if ((me->GetHealth()*100) / me->GetMaxHealth() >= hpborder)
            {
                me->CastSpell(me, Spell_Summon_glob[i], true);
                hpborder += 5;
            }
            break;
        }
    }

    void JustDied(Unit* Killer){}

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_viscidus(Creature* creature)
{
    return new boss_viscidusAI (creature);
}

void AddSC_boss_viscidus()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_viscidus";
    newscript->GetAI = &GetAI_boss_viscidus;
    newscript->RegisterSelf();
}
