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
SDName: Orgrimmar
SD%Complete: 100
SDComment: Quest support: 2460, 5727, 6566
SDCategory: Orgrimmar
EndScriptData */

/* ContentData
npc_neeru_fireblade     npc_text + gossip options text missing
npc_shenthul
npc_thrall_warchief
npc_eitrigg
EndContentData */

#include "ScriptPCH.h"

/*######
## npc_neeru_fireblade
######*/

#define QUEST_5727  5727

#define GOSSIP_HNF "You may speak frankly, Neeru..."
#define GOSSIP_SNF "[PH] ..."
bool GossipHello_npc_neeru_fireblade(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(QUEST_5727) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HNF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(4513, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_neeru_fireblade(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SNF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(4513, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(QUEST_5727);
            break;
    }
    return true;
}

/*######
## npc_shenthul
######*/

enum eShenthul
{
    QUEST_SHATTERED_SALUTE  = 2460
};

struct npc_shenthulAI : public ScriptedAI
{
    npc_shenthulAI(Creature* c) : ScriptedAI(c) {}

    bool CanTalk;
    bool CanEmote;
    uint32 Salute_Timer;
    uint32 Reset_Timer;
    uint64 PlayerGUID;

    void Reset()
    {
        CanTalk = false;
        CanEmote = false;
        Salute_Timer = 6000;
        Reset_Timer = 0;
        PlayerGUID = 0;
    }

    void EnterCombat(Unit* /*who*/) {}

    void UpdateAI(const uint32 diff)
    {
        if (CanEmote)
        {
            if (Reset_Timer <= diff)
            {
                if (Player* player = Unit::GetPlayer(*me, PlayerGUID))
                {
                    if (player->GetTypeId() == TYPEID_PLAYER && player->GetQuestStatus(QUEST_SHATTERED_SALUTE) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(QUEST_SHATTERED_SALUTE);
                }
                Reset();
            } else Reset_Timer -= diff;
        }

        if (CanTalk && !CanEmote)
        {
            if (Salute_Timer <= diff)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                CanEmote = true;
                Reset_Timer = 60000;
            } else Salute_Timer -= diff;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void ReceiveEmote(Player* player, uint32 emote)
    {
        if (emote == TEXTEMOTE_SALUTE && player->GetQuestStatus(QUEST_SHATTERED_SALUTE) == QUEST_STATUS_INCOMPLETE)
        {
            if (CanEmote)
            {
                player->AreaExploredOrEventHappens(QUEST_SHATTERED_SALUTE);
                Reset();
            }
        }
    }
};

CreatureAI* GetAI_npc_shenthul(Creature* creature)
{
    return new npc_shenthulAI (creature);
}

bool QuestAccept_npc_shenthul(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_SHATTERED_SALUTE)
    {
        CAST_AI(npc_shenthulAI, creature->AI())->CanTalk = true;
        CAST_AI(npc_shenthulAI, creature->AI())->PlayerGUID = player->GetGUID();
    }
    return true;
}

/*######
## npc_thrall_warchief
######*/

#define QUEST_6566              6566

#define SPELL_CHAIN_LIGHTNING   16033
#define SPELL_SHOCK             16034

#define GOSSIP_HTW "Please share your wisdom with me, Warchief."
#define GOSSIP_STW1 "What discoveries?"
#define GOSSIP_STW2 "Usurper?"
#define GOSSIP_STW3 "With all due respect, Warchief - why not allow them to be destroyed? Does this not strengthen our position?"
#define GOSSIP_STW4 "I... I did not think of it that way, Warchief."
#define GOSSIP_STW5 "I live only to serve, Warchief! My life is empty and meaningless without your guidance."
#define GOSSIP_STW6 "Of course, Warchief!"

//TODO: verify abilities/timers
struct npc_thrall_warchiefAI : public ScriptedAI
{
    npc_thrall_warchiefAI(Creature* c) : ScriptedAI(c) {}

    uint32 ChainLightning_Timer;
    uint32 Shock_Timer;

    void Reset()
    {
        ChainLightning_Timer = 2000;
        Shock_Timer = 8000;
    }

    void EnterCombat(Unit * /*who*/) {}

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ChainLightning_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_CHAIN_LIGHTNING);
            ChainLightning_Timer = 9000;
        } else ChainLightning_Timer -= diff;

        if (Shock_Timer <= diff)
        {
            DoCast(me->getVictim(), SPELL_SHOCK);
            Shock_Timer = 15000;
        } else Shock_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_thrall_warchief(Creature* creature)
{
    return new npc_thrall_warchiefAI (creature);
}

bool GossipHello_npc_thrall_warchief(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(QUEST_6566) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HTW, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_thrall_warchief(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(5733, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(5734, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(5735, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(5736, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(5737, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
            player->SEND_GOSSIP_MENU(5738, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(QUEST_6566);
            break;
    }
    return true;
}

/*######
## npc_eitrigg
######*/

#define GOSSIP_HELLO    "Hello, Eitrigg. I bring news from Blackrock Spire."
#define GOSSIP_SELECT1  "There is only one Warchief, Eitrigg!"
#define GOSSIP_SELECT2  "What do you mean?"
#define GOSSIP_SELECT3  "Hearthglen?? But..."
#define GOSSIP_SELECT4  "I will take you up on that offer, Eitrigg."
#define GOSSIP_SELECT5  "Ah, so that is how they pushed the Dark Iron to the lower parts of the Spire."
#define GOSSIP_SELECT6  "Perhaps there exists a way?"
#define GOSSIP_SELECT7  "As you wish, Eitrigg."

bool GossipHello_npc_eitrigg(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(4941) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(3573, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_eitrigg(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(3574, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(3575, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(3576, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(3577, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(3578, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            player->SEND_GOSSIP_MENU(3579, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->SEND_GOSSIP_MENU(3580, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(4941);
            break;
    }
    return true;
}

void AddSC_orgrimmar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_neeru_fireblade";
    newscript->pGossipHello =  &GossipHello_npc_neeru_fireblade;
    newscript->pGossipSelect = &GossipSelect_npc_neeru_fireblade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shenthul";
    newscript->GetAI = &GetAI_npc_shenthul;
    newscript->pQuestAccept =  &QuestAccept_npc_shenthul;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_thrall_warchief";
    newscript->GetAI = &GetAI_npc_thrall_warchief;
    newscript->pGossipHello =  &GossipHello_npc_thrall_warchief;
    newscript->pGossipSelect = &GossipSelect_npc_thrall_warchief;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_eitrigg";
    newscript->pGossipHello =  &GossipHello_npc_eitrigg;
    newscript->pGossipSelect = &GossipSelect_npc_eitrigg;
    newscript->RegisterSelf();
}

