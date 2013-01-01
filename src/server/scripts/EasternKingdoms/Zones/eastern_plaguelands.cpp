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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 5211, 5742. Special vendor Augustus the Touched
SDCategory: Eastern Plaguelands
EndScriptData */

/* ContentData
mobs_ghoul_flayer
npc_augustus_the_touched
npc_darrowshire_spirit
npc_tirion_fordring
EndContentData */

#include "ScriptPCH.h"

//id8530 - cannibal ghoul
//id8531 - gibbering ghoul
//id8532 - diseased flayer

struct mobs_ghoul_flayerAI : public ScriptedAI
{
    mobs_ghoul_flayerAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}

    void EnterCombat(Unit* /*who*/) {}

    void JustDied(Unit* Killer)
    {
        if (Killer->GetTypeId() == TYPEID_PLAYER)
            me->SummonCreature(11064, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);
    }
};

CreatureAI* GetAI_mobs_ghoul_flayer(Creature* creature)
{
    return new mobs_ghoul_flayerAI (creature);
}

/*######
## npc_augustus_the_touched
######*/

bool GossipHello_npc_augustus_the_touched(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (creature->isVendor() && player->GetQuestRewardStatus(6164))
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_augustus_the_touched(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST(creature->GetGUID());
    return true;
}

/*######
## npc_darrowshire_spirit
######*/

#define SPELL_SPIRIT_SPAWNIN    17321

struct npc_darrowshire_spiritAI : public ScriptedAI
{
    npc_darrowshire_spiritAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        DoCast(me, SPELL_SPIRIT_SPAWNIN);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit * /*who*/) {}
};
CreatureAI* GetAI_npc_darrowshire_spirit(Creature* creature)
{
    return new npc_darrowshire_spiritAI (creature);
}

bool GossipHello_npc_darrowshire_spirit(Player* player, Creature* creature)
{
    player->SEND_GOSSIP_MENU(3873, creature->GetGUID());
    player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    return true;
}

/*######
## npc_tirion_fordring
######*/

#define GOSSIP_HELLO    "I am ready to hear your tale, Tirion."
#define GOSSIP_SELECT1  "Thank you, Tirion.  What of your identity?"
#define GOSSIP_SELECT2  "That is terrible."
#define GOSSIP_SELECT3  "I will, Tirion."

bool GossipHello_npc_tirion_fordring(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(5742) == QUEST_STATUS_INCOMPLETE && player->getStandState() == UNIT_STAND_STATE_SIT)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_tirion_fordring(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(4493, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(4494, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(4495, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(5742);
            break;
    }
    return true;
}

void AddSC_eastern_plaguelands()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mobs_ghoul_flayer";
    newscript->GetAI = &GetAI_mobs_ghoul_flayer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_augustus_the_touched";
    newscript->pGossipHello = &GossipHello_npc_augustus_the_touched;
    newscript->pGossipSelect = &GossipSelect_npc_augustus_the_touched;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_darrowshire_spirit";
    newscript->GetAI = &GetAI_npc_darrowshire_spirit;
    newscript->pGossipHello = &GossipHello_npc_darrowshire_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tirion_fordring";
    newscript->pGossipHello =  &GossipHello_npc_tirion_fordring;
    newscript->pGossipSelect = &GossipSelect_npc_tirion_fordring;
    newscript->RegisterSelf();
}

