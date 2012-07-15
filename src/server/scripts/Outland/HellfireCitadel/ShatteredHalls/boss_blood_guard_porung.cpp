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
SDName: Boss_Blood_Guard_Porung
SD%Complete: 99
SDComment:
SDCategory: Hellfire Citadel, Shattered Halls
EndScriptData */

/* ContentData
boss_blood_guard_porung
EndContentData */

#include "ScriptPCH.h"
#include "shattered_halls.h"

#define SPELL_CLEAVE    15496

struct boss_blood_guard_porungAI : public ScriptedAI
{
    boss_blood_guard_porungAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* instance;
    bool Heroic;

    uint32 Cleave_Timer;

    void Reset()
    {
        Cleave_Timer = 15000;

        if (instance)
            instance->SetData(DATA_PORUNG, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (instance)
            instance->SetData(DATA_PORUNG, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        if (instance)
            instance->SetData(DATA_PORUNG, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
           return;

        if (Cleave_Timer <= diff)
        {
            DoCast(me->getVictim(),SPELL_CLEAVE);
            Cleave_Timer = 10000+rand()%5000;
        } else Cleave_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_blood_guard_porungAI(Creature* creature)
{
    return new boss_blood_guard_porungAI (creature);
}

void AddSC_boss_blood_guard_porung()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_blood_guard_porung";
    newscript->GetAI = &GetAI_boss_blood_guard_porungAI;
    newscript->RegisterSelf();
}