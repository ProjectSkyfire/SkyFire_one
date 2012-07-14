 /*
  * Copyright (C) 2010-2012 Project SkyFire <http://www.projectskyfire.org/>
  * Copyright (C) 2010-2012 Oregon <http://www.oregoncore.com/>
  * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
  * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
  *
  * This program is free software; you can redistribute it and/or modify it
  * under the terms of the GNU General Public License as published by the
  * Free Software Foundation; either version 2 of the License, or (at your
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
SDName: Boss_Anzu
SD%Complete: 95
SDComment:Summon event is missing.
SDCategory: Auchindoun, Sethekk Halls
EndScriptData */

#include "ScriptPCH.h"
#include "sethekk_halls.h"

#define SAY_INTRO1          -1900169
#define SAY_INTRO2          -1900170
#define SAY_HELP            -1900171

#define SPELL_CYCLONE       40321
#define SPELL_RIP           40199
#define SPELL_PARALYZING    40184
#define SPELL_BOMB          40303
#define SPELL_BANISH        42354

#define GO_CLAW             185554

#define NPC_BROOD           23132

struct boss_anzuAI : public ScriptedAI
{
    boss_anzuAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
        HeroicMode = me->GetMap()->IsHeroic();
        DoScriptText(SAY_INTRO1, me);
        DoScriptText(SAY_INTRO2, me);
    }

    ScriptedInstance* instance;

    bool HeroicMode;

    uint32 Cyclone_Timer;
    uint32 Rip_Timer;
    uint32 Paralyzing_Timer;
    uint32 Bomb_Timer;
    uint32 SummonedCount;

    bool Summon1;
    bool Summon2;
    bool Paralyzing;

    void Reset()
    {
        if (GameObject* pClaw = me->FindNearestGameObject(GO_CLAW , 20))
            pClaw->RemoveFromWorld();

        if (instance)
            instance->SetData(DATA_ANZUEVENT, NOT_STARTED);

        Cyclone_Timer = 10000;
        Rip_Timer = 8000;
        Paralyzing_Timer = 1000;
        Bomb_Timer = 22000;
        Summon1 = false;
        Summon2 = false;
        Paralyzing = false;
        SummonedCount = 5;
    }

    void EnterCombat(Unit *who)
    {
        if (instance)
            instance->SetData(DATA_ANZUEVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        if (instance)
            instance->SetData(DATA_ANZUEVENT, DONE);
    }

    void JustSummoned(Creature *summoned)
    {
        if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
            summoned->AI()->AttackStart(pTarget);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (((me->GetHealth()*100) / me->GetMaxHealth() < 66) && !Summon1)
        {
            DoCast(me, SPELL_BANISH);
            DoScriptText(SAY_HELP, me);
            Paralyzing = true;
            Creature* Summoned;
            for (uint32 i = 0; i < SummonedCount; i++)
            {
                Summoned = me->SummonCreature(NPC_BROOD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            }
            Summon1 = true;
        }

        if (((me->GetHealth()*100) / me->GetMaxHealth() < 33) && !Summon2)
        {
            DoCast(me, SPELL_BANISH);
            DoScriptText(SAY_HELP, me);
            Paralyzing = true;
            Creature* Summoned;
            for (uint32 i = 0; i < SummonedCount; i++)
            {
                Summoned = me->SummonCreature(NPC_BROOD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            }
            Summon2 = true;
        }

        if (Paralyzing)
        {
            if (Paralyzing_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_PARALYZING);
                Paralyzing_Timer = 1000;
                Paralyzing = false;
            } else Paralyzing_Timer -= diff;
        }

        if (!me->HasAura(SPELL_BANISH, 0))
        {
            if (Cyclone_Timer <= diff)
            {
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                {
                    DoCast(pTarget, SPELL_CYCLONE);
                    Cyclone_Timer = 20000+rand()%1000;
                }
            } else Cyclone_Timer -= diff;

            if (Bomb_Timer <= diff)
            {
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                {
                    DoCast(pTarget, SPELL_BOMB);
                    Bomb_Timer = 30000+rand()%1000;
                }
            } else Bomb_Timer -= diff;

            if (Rip_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_RIP);
                Rip_Timer = 25000+rand()%2000;
            } else Rip_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_boss_anzu(Creature* creature)
{
    return new boss_anzuAI (creature);
}

void AddSC_boss_anzu()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_anzu";
    newscript->GetAI = &GetAI_boss_anzu;
    newscript->RegisterSelf();
}
