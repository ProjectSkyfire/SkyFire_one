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
SDName: Searing_Gorge
SD%Complete: 80
SDComment: Quest support: 3377, 3441 (More accurate info on Kalaran needed). Lothos Riftwaker teleport to Molten Core.
SDCategory: Searing Gorge
EndScriptData */

/* ContentData
npc_kalaran_windblade
npc_lothos_riftwaker
npc_zamael_lunthistle
EndContentData */

#include "ScriptPCH.h"

/*######
## npc_kalaran_windblade
######*/

#define GOSSIP_HELLO_KW    "Tell me what drives this vengance?"
#define GOSSIP_SELECT_KW1  "Continue please"
#define GOSSIP_SELECT_KW2  "Let me confer with my colleagues"

bool GossipHello_npc_kalaran_windblade(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(3441) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_KW, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_kalaran_windblade(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_KW1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(1954, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_KW2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(1955, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3441);
            break;
    }
    return true;
}

/*######
## npc_lothos_riftwaker
######*/

#define GOSSIP_HELLO_LR "Teleport me to the Molten Core"

bool GossipHello_npc_lothos_riftwaker(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestRewardStatus(7487) || player->GetQuestRewardStatus(7848))
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_LR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lothos_riftwaker(Player* player, Creature* /*creature*/, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(409, 1096, -467, -104.6f, 3.64f);
    }

    return true;
}

/*######
## npc_zamael_lunthistle
######*/

#define GOSSIP_HELLO_ZL     "Tell me your story"
#define GOSSIP_SELECT_ZL1   "Please continue..."
#define GOSSIP_SELECT_ZL2   "Goodbye"

bool GossipHello_npc_zamael_lunthistle(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(3377) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_ZL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(1920, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_zamael_lunthistle(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_ZL1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(1921, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_ZL2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(1922, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3377);
            break;
    }
    return true;
}

/*######
##
######*/

void AddSC_searing_gorge()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_kalaran_windblade";
    newscript->pGossipHello =  &GossipHello_npc_kalaran_windblade;
    newscript->pGossipSelect = &GossipSelect_npc_kalaran_windblade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lothos_riftwaker";
    newscript->pGossipHello          = &GossipHello_npc_lothos_riftwaker;
    newscript->pGossipSelect         = &GossipSelect_npc_lothos_riftwaker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_zamael_lunthistle";
    newscript->pGossipHello =  &GossipHello_npc_zamael_lunthistle;
    newscript->pGossipSelect = &GossipSelect_npc_zamael_lunthistle;
    newscript->RegisterSelf();
}
