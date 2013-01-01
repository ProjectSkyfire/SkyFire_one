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

/* ScriptData
SDName: Zulfarrak
SD%Complete: 50
SDComment: Consider it temporary, no instance script made for this instance yet.
SDCategory: Zul'Farrak
EndScriptData */

/* ContentData
npc_sergeant_bly
npc_weegli_blastfuse
EndContentData */

#include "ScriptPCH.h"

/*######
## npc_sergeant_bly
######*/

#define FACTION_HOSTILE             14
#define FACTION_FRIENDLY            35

#define SPELL_SHIELD_BASH           11972
#define SPELL_REVENGE               12170

#define GOSSIP_BLY                  "[PH] In that case, I will take my reward!"

struct npc_sergeant_blyAI : public ScriptedAI
{
    npc_sergeant_blyAI(Creature *c) : ScriptedAI(c)
    {
        //instance = c->GetInstanceScript();
    }

    //ScriptedInstance* instance;

    uint32 ShieldBash_Timer;
    uint32 Revenge_Timer;                                   //this is wrong, spell should never be used unless me->getVictim() dodge, parry or block attack. Skyfire support required.

    void Reset()
    {
        ShieldBash_Timer = 5000;
        Revenge_Timer = 8000;

        me->setFaction(FACTION_FRIENDLY);

        /*if (instance)
            instance->SetData(0, NOT_STARTED);*/
    }

    void EnterCombat(Unit * /*who*/)
    {
        /*if (instance)
            instance->SetData(0, IN_PROGRESS);*/
    }

    void JustDied(Unit * /*victim*/)
    {
        /*if (instance)
            instance->SetData(0, DONE);*/
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ShieldBash_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_SHIELD_BASH);
            ShieldBash_Timer = 15000;
        } else ShieldBash_Timer -= diff;

        if (Revenge_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_REVENGE);
            Revenge_Timer = 10000;
        } else Revenge_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_sergeant_bly(Creature* creature)
{
    return new npc_sergeant_blyAI (creature);
}

bool GossipHello_npc_sergeant_bly(Player* player, Creature* creature)
{
    /*if (instance->GetData(0) == DONE)
    {*/
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_BLY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(1517, creature->GetGUID());
    /*}
    else if (instance->GetData(0) == IN_PROGRESS)
        player->SEND_GOSSIP_MENU(1516, creature->GetGUID());
    else
        player->SEND_GOSSIP_MENU(1515, creature->GetGUID());*/

    return true;
}

bool GossipSelect_npc_sergeant_bly(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        creature->setFaction(FACTION_HOSTILE);
        CAST_AI(npc_sergeant_blyAI, creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## npc_weegli_blastfuse
######*/

#define SPELL_BOMB                  8858
#define SPELL_GOBLIN_LAND_MINE      21688
#define SPELL_SHOOT                 6660
#define SPELL_WEEGLIS_BARREL        10772

#define GOSSIP_WEEGLI               "[PH] Please blow up the door."

struct npc_weegli_blastfuseAI : public ScriptedAI
{
    npc_weegli_blastfuseAI(Creature *c) : ScriptedAI(c)
    {
        //instance = c->GetInstanceScript();
    }

    //ScriptedInstance* instance;

    void Reset()
    {
        /*if (instance)
            instance->SetData(0, NOT_STARTED);*/
    }

    void EnterCombat(Unit * /*who*/)
    {
        /*if (instance)
            instance->SetData(0, IN_PROGRESS);*/
    }

    void JustDied(Unit * /*victim*/)
    {
        /*if (instance)
            instance->SetData(0, DONE);*/
    }

    void UpdateAI(const uint32 /*diff*/)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_weegli_blastfuse(Creature* creature)
{
    return new npc_weegli_blastfuseAI (creature);
}

bool GossipHello_npc_weegli_blastfuse(Player* player, Creature* creature)
{
    //event not implemented yet, this is only placeholder for future developement
    /*if (instance->GetData(0) == DONE)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WEEGLI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(1514, creature->GetGUID());//if event can proceed to end
    }
    else if (instance->GetData(0) == IN_PROGRESS)
        player->SEND_GOSSIP_MENU(1513, creature->GetGUID());//if event are in progress
    else*/
    player->SEND_GOSSIP_MENU(1511, creature->GetGUID());   //if event not started
    return true;
}

bool GossipSelect_npc_weegli_blastfuse(Player* player, Creature* /*creature*/, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        //here we make him run to door, set the charge and run away off to nowhere
    }
    return true;
}

/*######
## go_shallow_grave
######*/

enum {
    ZOMBIE = 7286,
    DEAD_HERO = 7276,
    ZOMBIE_CHANCE = 65,
    DEAD_HERO_CHANCE = 10
};

bool GOHello_go_shallow_grave(Player* /*player*/, GameObject* pGo)
{
    // randomly summon a zombie or dead hero the first time a grave is used
    if (pGo->GetUseCount() == 0)
    {
        uint32 randomchance = urand(0, 100);
        if (randomchance < ZOMBIE_CHANCE)
            pGo->SummonCreature(ZOMBIE, pGo->GetPositionX(), pGo->GetPositionY(), pGo->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
        else if ((randomchance-ZOMBIE_CHANCE) < DEAD_HERO_CHANCE)
            pGo->SummonCreature(DEAD_HERO, pGo->GetPositionX(), pGo->GetPositionY(), pGo->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
    }
    pGo->AddUse();
    return false;
}

/*######
## at_zumrah
######*/

enum {
    ZUMRAH_ID = 7271,
    ZUMRAH_HOSTILE_FACTION = 37
};

bool AreaTrigger_at_zumrah(Player* player, const AreaTriggerEntry * /*at*/)
{
    Creature* Zumrah = player->FindNearestCreature(ZUMRAH_ID, 30.0f);

    if (!Zumrah)
        return false;

    Zumrah->setFaction(ZUMRAH_HOSTILE_FACTION);
    Zumrah->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
    return true;
}

void AddSC_zulfarrak()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_sergeant_bly";
    newscript->GetAI = &GetAI_npc_sergeant_bly;
    newscript->pGossipHello =  &GossipHello_npc_sergeant_bly;
    newscript->pGossipSelect = &GossipSelect_npc_sergeant_bly;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_weegli_blastfuse";
    newscript->GetAI = &GetAI_npc_weegli_blastfuse;
    newscript->pGossipHello =  &GossipHello_npc_weegli_blastfuse;
    newscript->pGossipSelect = &GossipSelect_npc_weegli_blastfuse;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_shallow_grave";
    newscript->pGOHello = &GOHello_go_shallow_grave;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_zumrah";
    newscript->pAreaTrigger = &AreaTrigger_at_zumrah;
    newscript->RegisterSelf();
}
