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
SDName: Boss_Warlord_Kalithres
SD%Complete: 65
SDComment: Contains workarounds regarding warlord's rage spells not acting as expected. Both scripts here require review and fine tuning.
SDCategory: Coilfang Resevoir, The Steamvault
EndScriptData */

#include "ScriptPCH.h"
#include "steam_vault.h"

#define SAY_INTRO                   -1545016
#define SAY_REGEN                   -1545017
#define SAY_AGGRO1                  -1545018
#define SAY_AGGRO2                  -1545019
#define SAY_AGGRO3                  -1545020
#define SAY_SLAY1                   -1545021
#define SAY_SLAY2                   -1545022
#define SAY_DEATH                   -1545023

#define SPELL_SPELL_REFLECTION      31534
#define SPELL_IMPALE                39061
#define SPELL_WARLORDS_RAGE         37081
#define SPELL_WARLORDS_RAGE_NAGA    31543

#define SPELL_WARLORDS_RAGE_PROC    36453

struct mob_naga_distillerAI : public ScriptedAI
{
    mob_naga_distillerAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    ScriptedInstance *instance;

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        //hack, due to really weird spell behaviour :(
        if (instance)
        {
            if (instance->GetData(TYPE_DISTILLER) == IN_PROGRESS)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }
    }

    void EnterCombat(Unit *who) { }

    void StartRageGen(Unit *caster)
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        DoCast(me, SPELL_WARLORDS_RAGE_NAGA, true);

        if (instance)
            instance->SetData(TYPE_DISTILLER, IN_PROGRESS);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (me->GetHealth() <= damage)
            if (instance)
                instance->SetData(TYPE_DISTILLER, DONE);
    }
};

struct boss_warlord_kalithreshAI : public ScriptedAI
{
    boss_warlord_kalithreshAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceScript();
    }

    ScriptedInstance *instance;

    uint32 Reflection_Timer;
    uint32 Impale_Timer;
    uint32 Rage_Timer;
    bool CanRage;

    void Reset()
    {
        Reflection_Timer = 10000;
        Impale_Timer = 7000+rand()%7000;
        Rage_Timer = 45000;
        CanRage = false;

        if (instance)
            instance->SetData(TYPE_WARLORD_KALITHRESH, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        switch (rand()%3)
        {
            case 0: DoScriptText(SAY_AGGRO1, me); break;
            case 1: DoScriptText(SAY_AGGRO2, me); break;
            case 2: DoScriptText(SAY_AGGRO3, me); break;
        }

        if (instance)
            instance->SetData(TYPE_WARLORD_KALITHRESH, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        switch (rand()%2)
        {
            case 0: DoScriptText(SAY_SLAY1, me); break;
            case 1: DoScriptText(SAY_SLAY2, me); break;
        }
    }

    Creature* SelectCreatureInGrid(uint32 entry, float range)
    {
        Creature* creature = NULL;

        CellPair pair(SkyFire::ComputeCellPair(me->GetPositionX(), me->GetPositionY()));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        SkyFire::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*me, entry, true, range);
        SkyFire::CreatureLastSearcher<SkyFire::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);
        TypeContainerVisitor<SkyFire::CreatureLastSearcher<SkyFire::NearestCreatureEntryWithLiveStateInObjectRangeCheck>, GridTypeMapContainer> creature_searcher(searcher);
        cell.Visit(pair, creature_searcher,*(me->GetMap()));

        return creature;
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        //hack :(
        if (spell->Id == SPELL_WARLORDS_RAGE_PROC)
            if (instance)
                if (instance->GetData(TYPE_DISTILLER) == DONE)
                    me->RemoveAurasDueToSpell(SPELL_WARLORDS_RAGE_PROC);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);

        if (instance)
            instance->SetData(TYPE_WARLORD_KALITHRESH, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Rage_Timer <= diff)
        {
            Creature* distiller = SelectCreatureInGrid(17954, 100);
            if (distiller)
            {
                DoScriptText(SAY_REGEN, me);
                DoCast(me, SPELL_WARLORDS_RAGE);
                ((mob_naga_distillerAI*)distiller->AI())->StartRageGen(me);
            }
            Rage_Timer = 3000+rand()%15000;
        } else Rage_Timer -= diff;

        //Reflection_Timer
        if (Reflection_Timer <= diff)
        {
            DoCast(me, SPELL_SPELL_REFLECTION);
            Reflection_Timer = 15000+rand()%10000;
        } else Reflection_Timer -= diff;

        //Impale_Timer
        if (Impale_Timer <= diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(pTarget, SPELL_IMPALE);

            Impale_Timer = 7500+rand()%5000;
        } else Impale_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_naga_distiller(Creature* creature)
{
    return new mob_naga_distillerAI (creature);
}

CreatureAI* GetAI_boss_warlord_kalithresh(Creature* creature)
{
    return new boss_warlord_kalithreshAI (creature);
}

void AddSC_boss_warlord_kalithresh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_naga_distiller";
    newscript->GetAI = &GetAI_mob_naga_distiller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_warlord_kalithresh";
    newscript->GetAI = &GetAI_boss_warlord_kalithresh;
    newscript->RegisterSelf();
}

