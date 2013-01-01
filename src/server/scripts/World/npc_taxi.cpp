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
SDName: Npc_Taxi
SD%Complete: 0%
SDComment: To be used for taxi NPCs that are located globally.
SDCategory: NPCs
EndScriptData
*/

#include "ScriptPCH.h"

#define GOSSIP_SUSURRUS         "I am ready."
#define GOSSIP_NETHER_DRAKE     "I'm ready to fly! Take me up, dragon!"
#define GOSSIP_BRAZEN           "I am ready to go to Durnholde Keep."
#define GOSSIP_DABIREE1         "Fly me to Murketh and Shaadraz Gateways"
#define GOSSIP_DABIREE2         "Fly me to Shatter Point"
#define GOSSIP_WINDBELLOW1      "Fly me to The Abyssal Shelf"
#define GOSSIP_WINDBELLOW2      "Fly me to Honor Point"
#define GOSSIP_BRACK1           "Fly me to Murketh and Shaadraz Gateways"
#define GOSSIP_BRACK2           "Fly me to The Abyssal Shelf"
#define GOSSIP_BRACK3           "Fly me to Spinebreaker Post"
#define GOSSIP_IRENA            "Fly me to Skettis please"
#define GOSSIP_CLOUDBREAKER1    "Speaking of action, I've been ordered to undertake an air strike."
#define GOSSIP_CLOUDBREAKER2    "I need to intercept the Dawnblade reinforcements."
#define GOSSIP_DRAGONHAWK       "<Ride the dragonhawk to Sun's Reach>"
#define GOSSIP_VERONIA          "Fly me to Manaforge Coruu please"
#define GOSSIP_DEESAK           "Fly me to Ogri'la please"
#define GOSSIP_CRIMSONWING      "<Ride the gryphons to Survey Alcaz Island>"
#define GOSSIP_WILLIAMKEILAR1   "Take me to Northpass Tower."
#define GOSSIP_WILLIAMKEILAR2   "Take me to Eastwall Tower."
#define GOSSIP_WILLIAMKEILAR3   "Take me to Crown Guard Tower."

bool GossipHello_npc_taxi(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    switch (creature->GetEntry()) {
    case 17435: // Azuremyst Isle - Susurrus
        if (player->HasItemCount(23843, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SUSURRUS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        break;
    case 20903: // Netherstorm - Protectorate Nether Drake
        if (player->GetQuestStatus(10438) == QUEST_STATUS_INCOMPLETE && player->HasItemCount(29778, 1))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_NETHER_DRAKE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        break;
    case 18725: // Old Hillsbrad Foothills - Brazen
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_BRAZEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        break;
    case 19409: // Hellfire Peninsula - Wing Commander Dabir'ee
        //Mission: The Murketh and Shaadraz Gateways
        if (player->GetQuestStatus(10146) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DABIREE1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        //Shatter Point
        if (!player->GetQuestRewardStatus(10340))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DABIREE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        break;
    case 20235: // Hellfire Peninsula - Gryphoneer Windbellow
        //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
        if (player->GetQuestStatus(10163) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10346) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WINDBELLOW1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

        //Go to the Front
        if (player->GetQuestStatus(10382) != QUEST_STATUS_NONE && !player->GetQuestRewardStatus(10382))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WINDBELLOW2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
        break;
    case 19401: // Hellfire Peninsula - Wing Commander Brack
        //Mission: The Murketh and Shaadraz Gateways
        if (player->GetQuestStatus(10129) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_BRACK1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);

        //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
        if (player->GetQuestStatus(10162) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10347) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_BRACK2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);

        //Spinebreaker Post
        if (player->GetQuestStatus(10242) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(10242))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_BRACK3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
        break;
    case 23413: // Blade's Edge Mountains - Skyguard Handler Irena
        if (player->GetReputationRank(1031) >= REP_HONORED)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_IRENA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
        break;
    case 25059: // Isle of Quel'Danas - Ayren Cloudbreaker
        if (player->GetQuestStatus(11532) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11533) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CLOUDBREAKER1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);

        if (player->GetQuestStatus(11542) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11543) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CLOUDBREAKER2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
        break;
    case 25236: // Isle of Quel'Danas - Unrestrained Dragonhawk
        if (player->GetQuestStatus(11542) == QUEST_STATUS_COMPLETE || player->GetQuestStatus(11543) == QUEST_STATUS_COMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DRAGONHAWK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
        break;
    case 20162: // Netherstorm - Veronia
        //Behind Enemy Lines
        if (player->GetQuestStatus(10652) && !player->GetQuestRewardStatus(10652))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_VERONIA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
        break;
    case 23415: // Terokkar Forest - Skyguard Handler Deesak
        if (player->GetReputationRank(1031) >= REP_HONORED)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEESAK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
        break;
    case 23704: // Dustwallow Marsh - Cassa Crimsonwing
        if (player->GetQuestStatus(11142) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CRIMSONWING, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);
        break;
    case 17209:
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WILLIAMKEILAR1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 17);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WILLIAMKEILAR2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 18);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WILLIAMKEILAR3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 19);
        break;
    }

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_taxi(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    std::vector<uint32> nodes;

    switch (uiAction) {
    case GOSSIP_ACTION_INFO_DEF:
        //spellId is correct, however it gives flight a somewhat funny effect //TaxiPath 506.
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 32474, true);
        break;
    case GOSSIP_ACTION_INFO_DEF + 1:
        player->CLOSE_GOSSIP_MENU();

        nodes.resize(2);
        nodes[0] = 152;                                      //from drake
        nodes[1] = 153;                                      //end at drake
        player->ActivateTaxiPathTo(nodes);                  //TaxiPath 627 (possibly 627+628(152->153->154->155) )
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:
        if (!player->HasItemCount(25853, 1)) {
            player->SEND_GOSSIP_MENU(9780, creature->GetGUID());
        } else {
            player->CLOSE_GOSSIP_MENU();

            nodes.resize(2);
            nodes[0] = 115;                                  //from brazen
            nodes[1] = 116;                                  //end outside durnholde
            player->ActivateTaxiPathTo(nodes);              //TaxiPath 534
        }
        break;
    case GOSSIP_ACTION_INFO_DEF + 3:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 33768, true);               //TaxiPath 585 (Gateways Murket and Shaadraz)
        break;
    case GOSSIP_ACTION_INFO_DEF + 4:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 35069, true);               //TaxiPath 612 (Taxi - Hellfire Peninsula - Expedition Point to Shatter Point)
        break;
    case GOSSIP_ACTION_INFO_DEF + 5:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 33899, true);               //TaxiPath 589 (Aerial Assault Flight (Alliance))
        break;
    case GOSSIP_ACTION_INFO_DEF + 6:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 35065, true);               //TaxiPath 607 (Taxi - Hellfire Peninsula - Shatter Point to Beach Head)
        break;
    case GOSSIP_ACTION_INFO_DEF + 7:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 33659, true);               //TaxiPath 584 (Gateways Murket and Shaadraz)
        break;
    case GOSSIP_ACTION_INFO_DEF + 8:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 33825, true);               //TaxiPath 587 (Aerial Assault Flight (Horde))
        break;
    case GOSSIP_ACTION_INFO_DEF + 9:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 34578, true);               //TaxiPath 604 (Taxi - Reaver's Fall to Spinebreaker Ridge)
        break;
    case GOSSIP_ACTION_INFO_DEF + 10:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 41278, true);               //TaxiPath 706
        break;
    case GOSSIP_ACTION_INFO_DEF + 11:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 45071, true);               //TaxiPath 779
        break;
    case GOSSIP_ACTION_INFO_DEF + 12:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 45113, true);               //TaxiPath 784
        break;
    case GOSSIP_ACTION_INFO_DEF + 13:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 45353, true);               //TaxiPath 788
        break;
    case GOSSIP_ACTION_INFO_DEF + 14:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 34905, true);               //TaxiPath 606
        break;
    case GOSSIP_ACTION_INFO_DEF + 15:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 41279, true);               //TaxiPath 705 (Taxi - Skettis to Skyguard Outpost)
        break;
    case GOSSIP_ACTION_INFO_DEF + 16:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 42295, true);
        break;
    case GOSSIP_ACTION_INFO_DEF + 17:
        player->CLOSE_GOSSIP_MENU();

        nodes.resize(2);
        nodes[0] = 84;
        nodes[1] = 85;
        player->ActivateTaxiPathTo(nodes);
        break;
    case GOSSIP_ACTION_INFO_DEF + 18:
        player->CLOSE_GOSSIP_MENU();

        nodes.resize(2);
        nodes[0] = 84;
        nodes[1] = 86;
        player->ActivateTaxiPathTo(nodes);
        break;
    case GOSSIP_ACTION_INFO_DEF + 19:
        player->CLOSE_GOSSIP_MENU();

        nodes.resize(2);
        nodes[0] = 84;
        nodes[1] = 87;
        player->ActivateTaxiPathTo(nodes);
        break;
    }

    return true;
}

void AddSC_npc_taxi()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_taxi";
    newscript->pGossipHello = &GossipHello_npc_taxi;
    newscript->pGossipSelect = &GossipSelect_npc_taxi;
    newscript->RegisterSelf();
}
