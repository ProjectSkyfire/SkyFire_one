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
SDName: Hyjal
SD%Complete: 80
SDComment: gossip text id's unknown
SDCategory: Caverns of Time, Mount Hyjal
EndScriptData */

/* ContentData
npc_jaina_proudmoore
npc_thrall
npc_tyrande_whisperwind
EndContentData */

#include "ScriptPCH.h"
#include "hyjalAI.h"

#define GOSSIP_ITEM_BEGIN_ALLY      "My companions and I are with you, Lady Proudmoore."
#define GOSSIP_ITEM_ANETHERON       "We are ready for whatever Archimonde might send our way, Lady Proudmoore."

#define GOSSIP_ITEM_BEGIN_HORDE     "I am with you, Thrall."
#define GOSSIP_ITEM_AZGALOR         "We have nothing to fear."

#define GOSSIP_ITEM_RETREAT         "We can't keep this up. Let's retreat!"

#define GOSSIP_ITEM_TYRANDE         "Aid us in defending Nordrassil"
#define ITEM_TEAR_OF_GODDESS        24494

#define GOSSIP_ITEM_GM1             "[GM] Toggle Debug Timers"

CreatureAI* GetAI_npc_jaina_proudmoore(Creature* creature)
{
    hyjalAI* ai = new hyjalAI(creature);

    ai->Reset();
    ai->EnterEvadeMode();

    ai->Spell[0].SpellId = SPELL_BLIZZARD;
    ai->Spell[0].Cooldown = 15000 + rand()%20000;
    ai->Spell[0].TargetType = TARGETTYPE_RANDOM;

    ai->Spell[1].SpellId = SPELL_PYROBLAST;
    ai->Spell[1].Cooldown = 5500 + rand()%4000;
    ai->Spell[1].TargetType = TARGETTYPE_RANDOM;

    ai->Spell[2].SpellId = SPELL_SUMMON_ELEMENTALS;
    ai->Spell[2].Cooldown = 15000 + rand()%30000;
    ai->Spell[2].TargetType = TARGETTYPE_SELF;

    return ai;
}

bool GossipHello_npc_jaina_proudmoore(Player* player, Creature* creature)
{
    hyjalAI* ai = CAST_AI(hyjalAI, creature->AI());
    if (ai->EventBegun)
        return false;

    uint32 RageEncounter = ai->GetInstanceScript(DATA_RAGEWINTERCHILLEVENT);
    uint32 AnetheronEncounter = ai->GetInstanceScript(DATA_ANETHERONEVENT);
    if (RageEncounter == NOT_STARTED)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BEGIN_ALLY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    else if (RageEncounter == DONE && AnetheronEncounter == NOT_STARTED)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ANETHERON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    else if (RageEncounter == DONE && AnetheronEncounter == DONE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_RETREAT, GOSSIP_SENDER_MAIN,    GOSSIP_ACTION_INFO_DEF + 3);

    if (player->isGameMaster())
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_ITEM_GM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(907, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_jaina_proudmoore(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    hyjalAI* ai = CAST_AI(hyjalAI, creature->AI());
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            ai->StartEvent(player);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            ai->FirstBossDead = true;
            ai->WaveCount = 9;
            ai->StartEvent(player);
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            ai->Retreat();
            break;
         case GOSSIP_ACTION_INFO_DEF:
            ai->Debug = !ai->Debug;
            sLog->outDebug("TSCR: HyjalAI - Debug mode has been toggled");
            break;
    }
    return true;
}

CreatureAI* GetAI_npc_thrall(Creature* creature)
{
    hyjalAI* ai = new hyjalAI(creature);

    ai->Reset();
    ai->EnterEvadeMode();

    ai->Spell[0].SpellId = SPELL_CHAIN_LIGHTNING;
    ai->Spell[0].Cooldown = 3000 + rand()%5000;
    ai->Spell[0].TargetType = TARGETTYPE_VICTIM;

    ai->Spell[1].SpellId = SPELL_SUMMON_DIRE_WOLF;
    ai->Spell[1].Cooldown = 6000 + rand()%35000;
    ai->Spell[1].TargetType = TARGETTYPE_RANDOM;

    return ai;
}

bool GossipHello_npc_thrall(Player* player, Creature* creature)
{
    hyjalAI* ai = CAST_AI(hyjalAI, creature->AI());
    if (ai->EventBegun)
        return false;

    uint32 AnetheronEvent = ai->GetInstanceScript(DATA_ANETHERONEVENT);
    // Only let them start the Horde phases if Anetheron is dead.
    if (AnetheronEvent == DONE && ai->GetInstanceScript(DATA_ALLIANCE_RETREAT))
    {
        uint32 KazrogalEvent = ai->GetInstanceScript(DATA_KAZROGALEVENT);
        uint32 AzgalorEvent  = ai->GetInstanceScript(DATA_AZGALOREVENT);
        if (KazrogalEvent == NOT_STARTED)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BEGIN_HORDE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        else if (KazrogalEvent == DONE && AzgalorEvent != DONE && AzgalorEvent != IN_PROGRESS)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_AZGALOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        else if (AzgalorEvent == DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_RETREAT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    }

    if (player->isGameMaster())
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_ITEM_GM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(907, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_thrall(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    hyjalAI* ai = CAST_AI(hyjalAI, creature->AI());
    ai->DeSpawnVeins();//despawn the alliance veins
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            ai->StartEvent(player);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            ai->FirstBossDead = true;
            ai->WaveCount = 9;
            ai->StartEvent(player);
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            ai->Retreat();
            break;
        case GOSSIP_ACTION_INFO_DEF:
            ai->Debug = !ai->Debug;
            sLog->outDebug("TSCR: HyjalAI - Debug mode has been toggled");
            break;
    }
    return true;
}

CreatureAI* GetAI_npc_tyrande_whisperwind(Creature* creature)
{
    hyjalAI* ai = new hyjalAI(creature);
    ai->Reset();
    ai->EnterEvadeMode();
    return ai;
}

bool GossipHello_npc_tyrande_whisperwind(Player* player, Creature* creature)
{
    hyjalAI* ai = CAST_AI(hyjalAI, creature->AI());
    uint32 AzgalorEvent = ai->GetInstanceScript(DATA_AZGALOREVENT);

    // Only let them get item if Azgalor is dead.
    if (AzgalorEvent == DONE && !player->HasItemCount(ITEM_TEAR_OF_GODDESS, 1))
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TYRANDE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    player->SEND_GOSSIP_MENU(907, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_tyrande_whisperwind(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TEAR_OF_GODDESS, 1);
            if (msg == EQUIP_ERR_OK)
            {
                 Item* item = player->StoreNewItem(dest, ITEM_TEAR_OF_GODDESS, true);
                 if (item && player)
                     player->SendNewItem(item, 1, true, false, true);
            }
            player->SEND_GOSSIP_MENU(907, creature->GetGUID());
            CAST_AI(hyjalAI, creature->AI());
    }
    return true;
}

void AddSC_hyjal()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_jaina_proudmoore";
    newscript->GetAI = &GetAI_npc_jaina_proudmoore;
    newscript->pGossipHello = &GossipHello_npc_jaina_proudmoore;
    newscript->pGossipSelect = &GossipSelect_npc_jaina_proudmoore;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_thrall";
    newscript->GetAI = &GetAI_npc_thrall;
    newscript->pGossipHello = &GossipHello_npc_thrall;
    newscript->pGossipSelect = &GossipSelect_npc_thrall;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tyrande_whisperwind";
    newscript->pGossipHello = &GossipHello_npc_tyrande_whisperwind;
    newscript->pGossipSelect = &GossipSelect_npc_tyrande_whisperwind;
    newscript->GetAI = &GetAI_npc_tyrande_whisperwind;
    newscript->RegisterSelf();
}

