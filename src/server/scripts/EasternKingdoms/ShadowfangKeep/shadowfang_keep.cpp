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
SDName: Shadowfang_Keep
SD%Complete: 75
SDComment: npc_shadowfang_prisoner using escortAI for movement to door. Might need additional code in case being attacked. Add proper texts/say().
SDCategory: Shadowfang Keep
EndScriptData */

/* ContentData
npc_shadowfang_prisoner
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "shadowfang_keep.h"

/*######
## npc_shadowfang_prisoner
######*/

enum eEnums
{
    SAY_FREE_AS             = -1033000,
    SAY_OPEN_DOOR_AS        = -1033001,
    SAY_POST_DOOR_AS        = -1033002,
    SAY_FREE_AD             = -1033003,
    SAY_OPEN_DOOR_AD        = -1033004,
    SAY_POST1_DOOR_AD       = -1033005,
    SAY_POST2_DOOR_AD       = -1033006,

    SPELL_UNLOCK            = 6421,
    NPC_ASH                 = 3850,

    SPELL_DARK_OFFERING     = 7154
};

#define GOSSIP_ITEM_DOOR        "Thanks, I'll follow you to the door."

struct npc_shadowfang_prisonerAI : public npc_escortAI
{
    npc_shadowfang_prisonerAI(Creature *c) : npc_escortAI(c)
    {
        instance = c->GetInstanceScript();
        uiNpcEntry = c->GetEntry();
    }

    ScriptedInstance *instance;
    uint32 uiNpcEntry;

    void WaypointReached(uint32 uiPoint)
    {
        switch (uiPoint)
        {
            case 0:
                if (uiNpcEntry == NPC_ASH)
                    DoScriptText(SAY_FREE_AS, me);
                else
                    DoScriptText(SAY_FREE_AD, me);
                break;
            case 10:
                if (uiNpcEntry == NPC_ASH)
                    DoScriptText(SAY_OPEN_DOOR_AS, me);
                else
                    DoScriptText(SAY_OPEN_DOOR_AD, me);
                break;
            case 11:
                if (uiNpcEntry == NPC_ASH)
                    DoCast(me, SPELL_UNLOCK);
                break;
            case 12:
                if (uiNpcEntry == NPC_ASH)
                    DoScriptText(SAY_POST_DOOR_AS, me);
                else
                    DoScriptText(SAY_POST1_DOOR_AD, me);

                if (instance)
                    instance->SetData(TYPE_FREE_NPC, DONE);
                break;
            case 13:
                if (uiNpcEntry != NPC_ASH)
                    DoScriptText(SAY_POST2_DOOR_AD, me);
                break;
        }
    }

    void Reset() {}
    void EnterCombat(Unit* /*who*/) {}
};

CreatureAI* GetAI_npc_shadowfang_prisoner(Creature* creature)
{
    return new npc_shadowfang_prisonerAI(creature);
}

bool GossipHello_npc_shadowfang_prisoner(Player* player, Creature* creature)
{
    ScriptedInstance* instance = creature->GetInstanceScript();

    if (instance && instance->GetData(TYPE_FREE_NPC) != DONE && instance->GetData(TYPE_RETHILGORE) == DONE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_DOOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_shadowfang_prisoner(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();

        if (npc_escortAI* pEscortAI = CAST_AI(npc_shadowfang_prisonerAI, creature->AI()))
            pEscortAI->Start(false, false);
    }
    return true;
}

struct npc_arugal_voidwalkerAI : public ScriptedAI
{
    npc_arugal_voidwalkerAI(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    ScriptedInstance* instance;

    uint32 uiDarkOffering;

    void Reset()
    {
        uiDarkOffering = urand(290, 10);
    }

    void UpdateAI(uint32 const uiDiff)
    {
        if (!UpdateVictim())
            return;

        if (uiDarkOffering <= uiDiff)
        {
            if (Creature* pFriend = me->FindNearestCreature(me->GetEntry(),25.0f, true))
            {
                if (pFriend)
                    DoCast(pFriend, SPELL_DARK_OFFERING);
            }
            else
                DoCast(me, SPELL_DARK_OFFERING);
            uiDarkOffering = urand(4400, 12500);
        } else uiDarkOffering -= uiDiff;

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (instance)
            instance->SetData(TYPE_FENRUS, instance->GetData(TYPE_FENRUS) + 1);
    }
};

CreatureAI* GetAI_npc_arugal_voidwalker(Creature* creature)
{
    return new npc_arugal_voidwalkerAI(creature);
}

void AddSC_shadowfang_keep()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_shadowfang_prisoner";
    newscript->pGossipHello =  &GossipHello_npc_shadowfang_prisoner;
    newscript->pGossipSelect = &GossipSelect_npc_shadowfang_prisoner;
    newscript->GetAI = &GetAI_npc_shadowfang_prisoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_arugal_voidwalker";
    newscript->GetAI = &GetAI_npc_arugal_voidwalker;
    newscript->RegisterSelf();
}

