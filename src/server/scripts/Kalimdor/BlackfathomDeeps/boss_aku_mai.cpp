 /*
  * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
  * Copyright (C) 2010-2013 Oregon <http://www.oregoncore.com/>
  * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
  * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptPCH.h"
#include "blackfathom_deeps.h"

enum Spells
{
    SPELL_POISON_CLOUD                                     = 3815,
    SPELL_FRENZIED_RAGE                                    = 3490
};

struct boss_aku_maiAI : public ScriptedAI
{
    boss_aku_maiAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    uint32 uiPoisonCloudTimer;
    bool bIsEnraged;

    ScriptedInstance *instance;

    void Reset()
    {
        uiPoisonCloudTimer = urand(5000, 9000);
        bIsEnraged = false;
        if (instance)
            instance->SetData(TYPE_AKU_MAI, NOT_STARTED);
    }

    void EnterCombat(Unit* /*who*/)
    {
        if (instance)
            instance->SetData(TYPE_AKU_MAI, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/)
    {
        if (instance)
            instance->SetData(TYPE_AKU_MAI, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (uiPoisonCloudTimer < diff)
        {
            DoCastVictim(SPELL_POISON_CLOUD);
            uiPoisonCloudTimer = urand(25000, 50000);
        } else uiPoisonCloudTimer -= diff;

        if (!bIsEnraged && HealthBelowPct(30))
        {
            DoCast(me, SPELL_FRENZIED_RAGE);
            bIsEnraged = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_aku_mai(Creature* creature)
{
    return new boss_aku_maiAI (creature);
}

void AddSC_boss_aku_mai()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_aku_mai";
    newscript->GetAI = &GetAI_boss_aku_mai;
    newscript->RegisterSelf();
}
