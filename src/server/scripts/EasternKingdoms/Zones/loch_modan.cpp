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
SDName: Loch_Modan
SD%Complete: 100
SDComment: Quest support: 3181, 309
SDCategory: Loch Modan
EndScriptData */

/* ContentData
npc_mountaineer_pebblebitty
npc_miran
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

/*######
## npc_mountaineer_pebblebitty
######*/

#define GOSSIP_MP "Open the gate please, i need to get to Searing Gorge"

#define GOSSIP_MP1 "But i need to get there, now open the gate!"
#define GOSSIP_MP2 "Ok, so what is this other way?"
#define GOSSIP_MP3 "Doesn't matter, i'm invulnerable."
#define GOSSIP_MP4 "Yes..."
#define GOSSIP_MP5 "Ok, i'll try to remember that."
#define GOSSIP_MP6 "A key? Ok!"
class npc_mountaineer_pebblebitty : public CreatureScript
{
public:
    npc_mountaineer_pebblebitty() : CreatureScript("npc_mountaineer_pebblebitty") { }

    bool GossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(1833, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->SEND_GOSSIP_MENU(1834, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(1835, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(1836, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+5:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->SEND_GOSSIP_MENU(1837, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+6:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
                player->SEND_GOSSIP_MENU(1838, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+7:
                player->CLOSE_GOSSIP_MENU();
                break;
        }
        return true;
    }

    bool GossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (!player->GetQuestRewardStatus(3181) == 1)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_MP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};

/*#########
##npc_miran
#########*/

enum eMiran
{
    MIRAN_SAY_AMBUSH_ONE        = -1000571,
    DARK_IRON_RAIDER_SAY_AMBUSH = -1000572,
    MIRAN_SAY_AMBUSH_TWO        = -1000573,
    MIRAN_SAY_QUEST_END         = -1000574,

    QUEST_PROTECTING_THE_SHIPMENT  = 309,
    DARK_IRON_RAIDER               = 2149
};
class npc_miran : public CreatureScript
{
public:
    npc_miran() : CreatureScript("npc_miran") { }

    bool QuestAccept(Player* player, Creature* creature, const Quest* pQuest)
    {
        if (pQuest->GetQuestId() == QUEST_PROTECTING_THE_SHIPMENT)
        {
            creature->setFaction(231);

            if (npc_miranAI* pEscortAI = CAST_AI(npc_miranAI, creature->AI()))
                pEscortAI->Start(false, false, player->GetGUID(), pQuest);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_miranAI(creature);
    }

    struct npc_miranAI : public npc_escortAI
    {
        npc_miranAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() { }

        void WaypointReached(uint32 uiPointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (uiPointId)
            {
            case 8:
                DoScriptText(MIRAN_SAY_AMBUSH_ONE, me);
                me->SummonCreature(DARK_IRON_RAIDER, -5697.27f,-3736.36f, 318.54f, 2.02f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
                me->SummonCreature(DARK_IRON_RAIDER, -5697.27f,-3736.36f, 318.54f, 2.07f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
                if (Unit* scoff = me->FindNearestCreature(DARK_IRON_RAIDER, 30))
                    DoScriptText(DARK_IRON_RAIDER_SAY_AMBUSH, scoff);
                DoScriptText(MIRAN_SAY_AMBUSH_TWO, me);
            break;
            case 11:
                DoScriptText(MIRAN_SAY_QUEST_END, me);
                if (Player* player = GetPlayerForEscort())
                    player->GroupEventHappens(QUEST_PROTECTING_THE_SHIPMENT, me);
            break;
            }
        }

        void JustSummoned(Creature* pSummoned)
        {
            pSummoned->AI()->AttackStart(me);
        }
    };
};

void AddSC_loch_modan()
{
    new npc_mountaineer_pebblebitty();
    new npc_miran();
}
