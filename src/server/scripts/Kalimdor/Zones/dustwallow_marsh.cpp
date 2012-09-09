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
SDName: Dustwallow_Marsh
SD%Complete: 95
SDComment: Quest support: 11180, 558, 11126, 1173, 1273, 1324, 11183, 11142, 11209, Vendor Nat Pagle
SDCategory: Dustwallow Marsh
EndScriptData */

/* ContentData
mobs_risen_husk_spirit
npc_restless_apparition
npc_deserter_agitator
npc_lady_jaina_proudmoore
npc_nat_pagle
npc_morokk
npc_ogron
npc_private_hendel
npc_zelfrax
npc_cassa_crimsonwing
at_nats_landing
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

/*######
## mobs_risen_husk_spirit
######*/

#define SPELL_SUMMON_RESTLESS_APPARITION    42511
#define SPELL_CONSUME_FLESH                 37933           //Risen Husk
#define SPELL_INTANGIBLE_PRESENCE           43127           //Risen Spirit

struct mobs_risen_husk_spiritAI : public ScriptedAI
{
    mobs_risen_husk_spiritAI(Creature *c) : ScriptedAI(c) {}

    uint32 ConsumeFlesh_Timer;
    uint32 IntangiblePresence_Timer;

    void Reset()
    {
        ConsumeFlesh_Timer = 10000;
        IntangiblePresence_Timer = 5000;
    }

    void EnterCombat(Unit* who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER)
            if (damage >= me->GetHealth() && CAST_PLR(done_by)->GetQuestStatus(11180) == QUEST_STATUS_INCOMPLETE)
                me->CastSpell(done_by, SPELL_SUMMON_RESTLESS_APPARITION, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ConsumeFlesh_Timer <= diff)
        {
            if (me->GetEntry() == 23555)
                DoCast(me->getVictim(),SPELL_CONSUME_FLESH);
            ConsumeFlesh_Timer = 15000;
        } else ConsumeFlesh_Timer -= diff;

        if (IntangiblePresence_Timer <= diff)
        {
            if (me->GetEntry() == 23554)
                DoCast(me->getVictim(),SPELL_INTANGIBLE_PRESENCE);
            IntangiblePresence_Timer = 20000;
        } else IntangiblePresence_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mobs_risen_husk_spirit(Creature* creature)
{
    return new mobs_risen_husk_spiritAI (creature);
}

/*######
## npc_restless_apparition
######*/

bool GossipHello_npc_restless_apparition(Player* player, Creature* creature)
{
    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
    creature->SetInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    return true;
}

/*######
## npc_deserter_agitator
######*/

enum eDeserter
{
    QUEST_TRAITORS_AMONG_US                      = 11126,
    NPC_THERAMORE_DESERTER                       = 23602,
};

const Position DeserterDisappearPos = {-3609.03f, -4332.91f, 9.39354f, 3.73862f};

#define GOSSIP_ITEM_DESERTER "Your propaganda wont`t work on me. Spout your treasonous filth elsewhere traitor!"

struct npc_deserter_agitatorAI : public ScriptedAI
{
    npc_deserter_agitatorAI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
        me->RestoreFaction();
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void MovementInform(uint32 uiType, uint32 uiId)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        if (uiId == 1)
            me->DisappearAndDie();
    }
};

CreatureAI* GetAI_npc_deserter_agitator(Creature* creature)
{
    return new npc_deserter_agitatorAI(creature);
}

bool GossipHello_npc_deserter_agitator(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(QUEST_TRAITORS_AMONG_US) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_DESERTER, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_deserter_agitator(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    player->PlayerTalkClass->ClearMenus();

    if (uiAction == GOSSIP_SENDER_INFO)
    {
        player->CLOSE_GOSSIP_MENU();
        switch (urand(0, 1))
        {
        case 0:
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            creature->setFaction(14);
            creature->AI()->AttackStart(player);
            break;
        case 1:
            player->KilledMonsterCredit(NPC_THERAMORE_DESERTER, 0);
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            creature->SetSpeed(MOVE_RUN, creature->GetSpeedRate(MOVE_RUN), true);
            creature->setFaction(35);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE);
            creature->SetReactState(REACT_PASSIVE);
            creature->GetMotionMaster()->MovePoint(1, DeserterDisappearPos);
            break;
        }
    }

    return true;
}

/*######
## npc_theramore_guard
######*/

enum eTheramoreGuard
{
    SAY_QUEST1                                   = -1000719,
    SAY_QUEST2                                   = -1000720,
    SAY_QUEST3                                   = -1000721,
    SAY_QUEST4                                   = -1000722,
    SAY_QUEST5                                   = -1000723,
    SAY_QUEST6                                   = -1000724,
    SAY_QUEST7                                   = -1000725,
    SAY_QUEST8                                   = -1000726,
    SAY_QUEST9                                   = -1000727,

    QUEST_DISCREDITING_THE_DESERTERS             = 11133,

    NPC_THERAMORE_GUARD                          = 4979,

    SPELL_DOCTORED_LEAFLET                       = 42725,
    SPELL_PROPAGANDIZED                          = 42246,
};

#define GOSSIP_ITEM_THERAMORE_GUARD "You look like an intelligent person. Why don't you read one of these leaflets and give it some thought?"

struct npc_theramore_guardAI : public ScriptedAI
{
    npc_theramore_guardAI(Creature* creature) : ScriptedAI(creature) { }

    uint32 uiYellTimer;
    uint32 uiStep;
    bool bYellTimer;

    void Reset()
    {
        bYellTimer = false;
        uiStep = 0;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!me->HasAura(SPELL_PROPAGANDIZED, 0))
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if (bYellTimer && uiYellTimer <= uiDiff)
        {
            switch (uiStep)
            {
            case 0:
                DoScriptText(RAND(SAY_QUEST2, SAY_QUEST3, SAY_QUEST4, SAY_QUEST5, SAY_QUEST6), me);
                uiYellTimer = 5000;
                ++uiStep;
                    break;
            case 1:
                DoScriptText(RAND(SAY_QUEST7, SAY_QUEST8, SAY_QUEST9), me);
                me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                uiStep = 0;
                bYellTimer = false;
                break;
            }
        }
        else
            uiYellTimer -= uiDiff;
    }
};

CreatureAI *GetAI_npc_theramore_guard(Creature* creature)
{
    return new npc_theramore_guardAI(creature);
}

bool GossipHello_npc_theramore_guard(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(QUEST_DISCREDITING_THE_DESERTERS) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_THERAMORE_GUARD, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_theramore_guard(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    player->PlayerTalkClass->ClearMenus();

    if (uiAction == GOSSIP_SENDER_INFO)
    {
        player->CLOSE_GOSSIP_MENU();
        player->KilledMonsterCredit(NPC_THERAMORE_GUARD, 0);
        DoScriptText(SAY_QUEST1, creature);
        creature->CastSpell(creature, SPELL_DOCTORED_LEAFLET, false);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        CAST_AI(npc_theramore_guardAI, creature->AI())->uiYellTimer = 4000;
        CAST_AI(npc_theramore_guardAI, creature->AI())->bYellTimer = true;
    }

    return true;
}

/*######
## npc_lady_jaina_proudmoore
######*/

#define GOSSIP_ITEM_JAINA "I know this is rather silly but i have a young ward who is a bit shy and would like your autograph."

bool GossipHello_npc_lady_jaina_proudmoore(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(558) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_JAINA, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lady_jaina_proudmoore(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_SENDER_INFO)
    {
        player->SEND_GOSSIP_MENU(7012, creature->GetGUID());
        player->CastSpell(player, 23122, false);
    }
    return true;
}

/*######
## npc_nat_pagle
######*/

bool GossipHello_npc_nat_pagle(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (creature->isVendor() && player->GetQuestRewardStatus(8227))
    {
        player->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(7640, creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(7638, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_nat_pagle(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST(creature->GetGUID());

    return true;
}

/*######
## npc_morokk
######*/

enum
{
    SAY_MOR_CHALLENGE               = -1000499,
    SAY_MOR_SCARED                  = -1000500,

    QUEST_CHALLENGE_MOROKK          = 1173,

    FACTION_MOR_HOSTILE             = 168,
    FACTION_MOR_RUNNING             = 35
};

struct npc_morokkAI : public npc_escortAI
{
    npc_morokkAI(Creature* creature) : npc_escortAI(creature)
    {
        m_bIsSuccess = false;
        Reset();
    }

    bool m_bIsSuccess;

    void Reset() {}

    void WaypointReached(uint32 uiPointId)
    {
        switch (uiPointId)
        {
            case 0:
                SetEscortPaused(true);
                break;
            case 1:
                if (m_bIsSuccess)
                    DoScriptText(SAY_MOR_SCARED, me);
                else
                {
                    me->setDeathState(JUST_DIED);
                    me->Respawn();
                }
                break;
        }
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (me->getVictim())
            return;

        if (me->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (me->GetHealth()*100 < me->GetMaxHealth()*30.0f)
            {
                if (Player* player = GetPlayerForEscort())
                    player->GroupEventHappens(QUEST_CHALLENGE_MOROKK, me);

                me->setFaction(FACTION_MOR_RUNNING);
                SetRun(true);

                m_bIsSuccess = true;
                EnterEvadeMode();

                uiDamage = 0;
            }
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!me->getVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (Player* player = GetPlayerForEscort())
                {
                    m_bIsSuccess = false;
                    DoScriptText(SAY_MOR_CHALLENGE, me, player);
                    me->setFaction(FACTION_MOR_HOSTILE);
                    AttackStart(player);
                }

                SetEscortPaused(false);
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_morokk(Creature* creature)
{
    return new npc_morokkAI(creature);
}

bool QuestAccept_npc_morokk(Player* player, Creature* creature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CHALLENGE_MOROKK)
    {
        if (npc_morokkAI* pEscortAI = CAST_AI(npc_morokkAI, creature->AI()))
            pEscortAI->Start(true, false, player->GetGUID(), pQuest);

        return true;
    }

    return false;
}

/*######
## npc_ogron
######*/

enum
{
    SAY_OGR_START                       = -1000452,
    SAY_OGR_SPOT                        = -1000453,
    SAY_OGR_RET_WHAT                    = -1000454,
    SAY_OGR_RET_SWEAR                   = -1000455,
    SAY_OGR_REPLY_RET                   = -1000456,
    SAY_OGR_RET_TAKEN                   = -1000457,
    SAY_OGR_TELL_FIRE                   = -1000458,
    SAY_OGR_RET_NOCLOSER                = -1000459,
    SAY_OGR_RET_NOFIRE                  = -1000460,
    SAY_OGR_RET_HEAR                    = -1000461,
    SAY_OGR_CAL_FOUND                   = -1000462,
    SAY_OGR_CAL_MERCY                   = -1000463,
    SAY_OGR_HALL_GLAD                   = -1000464,
    EMOTE_OGR_RET_ARROW                 = -1000465,
    SAY_OGR_RET_ARROW                   = -1000466,
    SAY_OGR_CAL_CLEANUP                 = -1000467,
    SAY_OGR_NODIE                       = -1000468,
    SAY_OGR_SURVIVE                     = -1000469,
    SAY_OGR_RET_LUCKY                   = -1000470,
    SAY_OGR_THANKS                      = -1000471,

    QUEST_QUESTIONING                   = 1273,

    FACTION_GENERIC_FRIENDLY            = 35,
    FACTION_THER_HOSTILE                = 151,

    NPC_REETHE                          = 4980,
    NPC_CALDWELL                        = 5046,
    NPC_HALLAN                          = 5045,
    NPC_SKIRMISHER                      = 5044,

    SPELL_FAKE_SHOT                     = 7105,

    PHASE_INTRO                         = 0,
    PHASE_GUESTS                        = 1,
    PHASE_FIGHT                         = 2,
    PHASE_COMPLETE                      = 3
};

static float m_afSpawn[] = {-3383.501953f, -3203.383301f, 36.149f};
static float m_afMoveTo[] = {-3371.414795f, -3212.179932f, 34.210f};

struct npc_ogronAI : public npc_escortAI
{
    npc_ogronAI(Creature* creature) : npc_escortAI(creature)
    {
        lCreatureList.clear();
        m_uiPhase = 0;
        m_uiPhaseCounter = 0;
        Reset();
    }

    std::list<Creature*> lCreatureList;

    uint32 m_uiPhase;
    uint32 m_uiPhaseCounter;
    uint32 m_uiGlobalTimer;

    void Reset()
    {
        m_uiGlobalTimer = 5000;

        /*if (HasEscortState(STATE_ESCORT_PAUSED) && m_uiPhase == PHASE_FIGHT)
            m_uiPhase = PHASE_COMPLETE;*/

        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            lCreatureList.clear();
            m_uiPhase = 0;
            m_uiPhaseCounter = 0;
        }
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) && pWho->GetEntry() == NPC_REETHE && lCreatureList.empty())
            lCreatureList.push_back((Creature*)pWho);

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch (uiPointId)
        {
            case 9:
                DoScriptText(SAY_OGR_SPOT, me);
                break;
            case 10:
                if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                    DoScriptText(SAY_OGR_RET_WHAT, pReethe);
                break;
            case 11:
                SetEscortPaused(true);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        lCreatureList.push_back(pSummoned);

        pSummoned->setFaction(FACTION_GENERIC_FRIENDLY);

        if (pSummoned->GetEntry() == NPC_CALDWELL)
            pSummoned->GetMotionMaster()->MovePoint(0, m_afMoveTo[0], m_afMoveTo[1], m_afMoveTo[2]);
        else
        {
            if (Creature* pCaldwell = me->FindNearestCreature(NPC_CALDWELL, 15.0f))
            {
                //will this conversion work without compile warning/error?
                size_t iSize = lCreatureList.size();
                pSummoned->GetMotionMaster()->MoveFollow(pCaldwell, 0.5f, (M_PI/2)*(int)iSize);
            }
        }
    }

    void DoStartAttackMe()
    {
        if (!lCreatureList.empty())
        {
            for (std::list<Creature*>::iterator itr = lCreatureList.begin(); itr != lCreatureList.end(); ++itr)
            {
                if ((*itr)->GetEntry() == NPC_REETHE)
                    continue;

                if ((*itr)->isAlive())
                {
                    (*itr)->setFaction(FACTION_THER_HOSTILE);
                    (*itr)->AI()->AttackStart(me);
                }
            }
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (m_uiGlobalTimer < uiDiff)
                {
                    m_uiGlobalTimer = 5000;

                    switch (m_uiPhase)
                    {
                        case PHASE_INTRO:
                        {
                            switch (m_uiPhaseCounter)
                            {
                                case 0:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_SWEAR, pReethe);
                                    break;
                                case 1:
                                    DoScriptText(SAY_OGR_REPLY_RET, me);
                                    break;
                                case 2:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_TAKEN, pReethe);
                                    break;
                                case 3:
                                    DoScriptText(SAY_OGR_TELL_FIRE, me);
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_NOCLOSER, pReethe);
                                    break;
                                case 4:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_NOFIRE, pReethe);
                                    break;
                                case 5:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_HEAR, pReethe);

                                    me->SummonCreature(NPC_CALDWELL, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
                                    me->SummonCreature(NPC_HALLAN, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
                                    me->SummonCreature(NPC_SKIRMISHER, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
                                    me->SummonCreature(NPC_SKIRMISHER, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);

                                    m_uiPhase = PHASE_GUESTS;
                                    break;
                            }
                            break;
                        }

                        case PHASE_GUESTS:
                        {
                            switch (m_uiPhaseCounter)
                            {
                                case 6:
                                    if (Creature* pCaldwell = me->FindNearestCreature(NPC_CALDWELL, 15.0f))
                                        DoScriptText(SAY_OGR_CAL_FOUND, pCaldwell);
                                    break;
                                case 7:
                                    if (Creature* pCaldwell = me->FindNearestCreature(NPC_CALDWELL, 15.0f))
                                        DoScriptText(SAY_OGR_CAL_MERCY, pCaldwell);
                                    break;
                                case 8:
                                    if (Creature* pHallan = me->FindNearestCreature(NPC_HALLAN, 15.0f))
                                    {
                                        DoScriptText(SAY_OGR_HALL_GLAD, pHallan);

                                        if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                            pHallan->CastSpell(pReethe, SPELL_FAKE_SHOT, false);
                                    }
                                    break;
                                case 9:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                    {
                                        DoScriptText(EMOTE_OGR_RET_ARROW, pReethe);
                                        DoScriptText(SAY_OGR_RET_ARROW, pReethe);
                                    }
                                    break;
                                case 10:
                                    if (Creature* pCaldwell = me->FindNearestCreature(NPC_CALDWELL, 15.0f))
                                        DoScriptText(SAY_OGR_CAL_CLEANUP, pCaldwell);

                                    DoScriptText(SAY_OGR_NODIE, me);
                                    break;
                                case 11:
                                    DoStartAttackMe();
                                    m_uiPhase = PHASE_COMPLETE;
                                    break;
                            }
                            break;
                        }

                        case PHASE_COMPLETE:
                        {
                            switch (m_uiPhaseCounter)
                            {
                                case 12:
                                    if (Player* player = GetPlayerForEscort())
                                        player->GroupEventHappens(QUEST_QUESTIONING, me);
                                    DoScriptText(SAY_OGR_SURVIVE, me);
                                    break;
                                case 13:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_LUCKY, pReethe);
                                    break;
                                case 14:
                                    if (Creature* pReethe = me->FindNearestCreature(NPC_REETHE, 15.0f))
                                        pReethe->setDeathState(JUST_DIED);
                                    break;
                                case 15:
                                    DoScriptText(SAY_OGR_THANKS, me);
                                    SetRun(true);
                                    SetEscortPaused(false);
                                    break;
                            }
                            break;
                        }
                    }
                        ++m_uiPhaseCounter;
                }
                else
                    m_uiGlobalTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_ogron(Player* player, Creature* creature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_QUESTIONING)
    {
        creature->setFaction(FACTION_ESCORT_N_FRIEND_PASSIVE);
        DoScriptText(SAY_OGR_START, creature, player);

        if (npc_ogronAI* pEscortAI = CAST_AI(npc_ogronAI, (creature->AI())))
            pEscortAI->Start(false, false, player->GetGUID(), pQuest, true);
    }

    return true;
}

CreatureAI* GetAI_npc_ogron(Creature* creature)
{
    return new npc_ogronAI(creature);
}

/*######
## npc_private_hendel
######*/

enum eHendel
{
    SAY_PROGRESS_1_TER          = -1000411,
    SAY_PROGRESS_2_HEN          = -1000412,
    SAY_PROGRESS_3_TER          = -1000413,
    SAY_PROGRESS_4_TER          = -1000414,
    EMOTE_SURRENDER             = -1000415,

    QUEST_MISSING_DIPLO_PT16    = 1324,
    FACTION_HOSTILE             = 168,

    NPC_SENTRY                  = 5184,
    NPC_JAINA                   = 4968,
    NPC_TERVOSH                 = 4967,
    NPC_PAINED                  = 4965,

    PHASE_ATTACK                = 1,
    PHASE_COMPLETED             = 2
};

struct EventLocation
{
    float m_fX, m_fY, m_fZ;
};

EventLocation m_afEventMoveTo[] =
{
    {-2943.92f, -3319.41f, 29.8336f},
    {-2933.01f, -3321.05f, 29.5781f}
};

struct npc_private_hendelAI : public ScriptedAI
{
    npc_private_hendelAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

    std::list<Creature*> lCreatureList;

    uint32 m_uiPhaseCounter;
    uint32 m_uiEventTimer;
    uint32 m_uiPhase;
    uint64 PlayerGUID;

    void Reset()
    {
        PlayerGUID = 0;
        m_uiPhase = 0;
        m_uiEventTimer = 0;
        m_uiPhaseCounter = 0;
        lCreatureList.clear();
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (me->getVictim())
            return;

        if (me->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->RemoveUnitMovementFlag(MOVEFLAG_WALK_MODE);

        if (pSummoned->GetEntry() == NPC_TERVOSH)
        {
            pSummoned->GetMotionMaster()->MovePoint(0, -2889.48f, -3349.37f, 32.0619f);
            return;
        }
        if (pSummoned->GetEntry() == NPC_JAINA)
        {
            pSummoned->GetMotionMaster()->MovePoint(0, -2889.27f, -3347.17f, 32.2615f);
            return;
        }
        pSummoned->GetMotionMaster()->MovePoint(0, -2890.31f,-3345.23f, 32.3087f);
    }

    void DoAttackPlayer()
    {
        Player* player = Unit::GetPlayer(*me, PlayerGUID);
        if (!player)
            return;

        me->setFaction(FACTION_HOSTILE);
        me->AI()->AttackStart(player);

        float x, y, z;

        me->GetPosition(x, y, z);
        CellPair pair(SkyFire::ComputeCellPair(x, y));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        SkyFire::AllCreaturesOfEntryInRange check(me, NPC_SENTRY, 20);
        SkyFire::CreatureListSearcher<SkyFire::AllCreaturesOfEntryInRange> searcher(lCreatureList, check);
        TypeContainerVisitor<SkyFire::CreatureListSearcher<SkyFire::AllCreaturesOfEntryInRange>, GridTypeMapContainer> cSearcher(searcher);
        cell.Visit(pair, cSearcher, *(me->GetMap()));

        if (!lCreatureList.empty())
        {
            for (std::list<Creature*>::iterator itr = lCreatureList.begin(); itr != lCreatureList.end(); ++itr)
            {
                if ((*itr)->isAlive())
                {
                    (*itr)->setFaction(FACTION_HOSTILE);
                    (*itr)->AI()->AttackStart(player);
                }
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim() && m_uiPhase)
        {
            switch (m_uiPhase)
            {
            case PHASE_ATTACK:
                DoAttackPlayer();
                break;

            case PHASE_COMPLETE:
                if (m_uiEventTimer <= uiDiff)
                {
                    m_uiEventTimer = 5000;

                    switch (m_uiPhaseCounter)
                    {
                    case 0:
                        DoScriptText(EMOTE_SURRENDER, me);
                        break;
                    case 1:
                        if (Creature* pTervosh = me->FindNearestCreature(NPC_TERVOSH, 10.0f))
                            DoScriptText(SAY_PROGRESS_1_TER, pTervosh);
                        break;
                    case 2:
                        DoScriptText(SAY_PROGRESS_2_HEN, me);
                        break;
                    case 3:
                        if (Creature* pTervosh = me->FindNearestCreature(NPC_TERVOSH, 10.0f))
                            DoScriptText(SAY_PROGRESS_3_TER, pTervosh);
                        break;
                    case 4:
                        if (Creature* pTervosh = me->FindNearestCreature(NPC_TERVOSH, 10.0f))
                                DoScriptText(SAY_PROGRESS_4_TER, pTervosh);
                        if (Player* player = Unit::GetPlayer(*me, PlayerGUID))
                            player->GroupEventHappens(QUEST_MISSING_DIPLO_PT16, me);
                        Reset();
                        break;
                    }
                    ++m_uiPhaseCounter;
                }
                else
                    m_uiEventTimer -= uiDiff;
            }
        }
        return;
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > me->GetHealth() || ((me->GetHealth() - uiDamage)*100 / me->GetMaxHealth() < 20))
        {
            uiDamage = 0;
            m_uiPhase = PHASE_COMPLETE;
            m_uiEventTimer = 2000;

            me->RestoreFaction();
            me->RemoveAllAuras();
            me->DeleteThreatList();
            me->CombatStop(true);
            me->RemoveUnitMovementFlag(MOVEFLAG_WALK_MODE);
            me->SetHomePosition(-2892.28f,-3347.81f, 31.8609f, 0.160719f);
            me->GetMotionMaster()->MoveTargetedHome();

            if (Player* player = Unit::GetPlayer(*me, PlayerGUID))
                player->CombatStop(true);

            if (!lCreatureList.empty())
            {
                uint16 N = -1;

                for (std::list<Creature*>::iterator itr = lCreatureList.begin(); itr != lCreatureList.end(); ++itr)
                {
                    if ((*itr)->isAlive())
                    {
                        N = N + 1;
                        (*itr)->RestoreFaction();
                        (*itr)->RemoveAllAuras();
                        (*itr)->DeleteThreatList();
                        (*itr)->CombatStop(true);
                        (*itr)->RemoveUnitMovementFlag(MOVEFLAG_WALK_MODE);
                        (*itr)->GetMotionMaster()->MovePoint(0, m_afEventMoveTo[N].m_fX,  m_afEventMoveTo[N].m_fY,  m_afEventMoveTo[N].m_fZ);
                        (*itr)->ForcedDespawn(5000);
                    }
                }
            }
            lCreatureList.clear();

            me->ForcedDespawn(60000);
            me->SummonCreature(NPC_TERVOSH, -2876.66f, -3346.96f, 35.6029f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
            me->SummonCreature(NPC_JAINA, -2876.95f, -3342.78f, 35.6244f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
            me->SummonCreature(NPC_PAINED, -2877.67f, -3338.63f, 35.2548f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
        }
    }
};

bool QuestAccept_npc_private_hendel(Player* player, Creature* creature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLO_PT16)
    {
        CAST_AI(npc_private_hendelAI, creature->AI())->m_uiPhase = PHASE_ATTACK;
        CAST_AI(npc_private_hendelAI, creature->AI())->PlayerGUID = player->GetGUID();
    }

    return true;
}

CreatureAI* GetAI_npc_private_hendel(Creature* creature)
{
    return new npc_private_hendelAI(creature);
}

/*######
## npc_zelfrax
######*/

enum eZelfrax
{
    SAY_ZELFRAX     = -1000605,
    SAY_ZELFRAX_2   = -1000606
};

struct npc_zelfraxAI : public ScriptedAI
{
    npc_zelfraxAI(Creature* creature) : ScriptedAI(creature)
    {
        MoveToDock();
    }

    void AttackStart(Unit* pWho)
    {
        if (!pWho)
            return;

        if (me->Attack(pWho, true))
        {
            me->SetInCombatWith(pWho);
            pWho->SetInCombatWith(me);

            if (IsCombatMovement())
                me->GetMotionMaster()->MoveChase(pWho);
        }
    }

    void MovementInform(uint32 uiType, uint32 /*uiId*/)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        me->SetHomePosition(me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation());
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
        SetCombatMovement(true);

        if (me->isInCombat())
            if (Unit* pUnit = me->getVictim())
                me->GetMotionMaster()->MoveChase(pUnit);
    }

    void MoveToDock()
    {
        SetCombatMovement(false);
        me->GetMotionMaster()->MovePoint(0,-2967.030,-3872.1799, 35.620);
        DoScriptText(SAY_ZELFRAX, me);
        DoScriptText(SAY_ZELFRAX_2, me);
    }

    void UpdateAI(uint32 const /*uiDiff*/)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_zelfrax(Creature* creature)
{
    return new npc_zelfraxAI(creature);
}

/*######
## npc_cassa_crimsonwing
######*/

enum
{
    QUEST_SURVEY_ALCAZ          = 11142,
    SPELL_ALCAZ_SURVEY          = 42295
};

#define GOSSIP_RIDE             "<Ride the gryphons to Survey Alcaz Island>"

bool GossipHello_npc_cassa_crimsonwing(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(QUEST_SURVEY_ALCAZ) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RIDE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_cassa_crimsonwing(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, SPELL_ALCAZ_SURVEY, false);
    }
    return true;
}

/*######
## at_nats_landing
######*/
enum
{
    QUEST_NATS_BARGAIN = 11209,
    SPELL_FISH_PASTE   = 42644,
    NPC_LURKING_SHARK  = 23928
};

bool AreaTrigger_at_nats_landing(Player* player, const AreaTriggerEntry* pAt)
{
    if (player->GetQuestStatus(QUEST_NATS_BARGAIN) == QUEST_STATUS_INCOMPLETE && player->HasAura(SPELL_FISH_PASTE, 0))
    {
        Creature* pShark = GetClosestCreatureWithEntry(player, NPC_LURKING_SHARK, 20.0f);

        if (!pShark)
            pShark = player->SummonCreature(NPC_LURKING_SHARK, -4246.243f, -3922.356f, -7.488f, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100000);

        pShark->AI()->AttackStart(player);
        return false;
    }
    return true;
}

/*#####
## npc_stinky
#####*/

enum eStinky
{
    QUEST_STINKYS_ESCAPE_H                       = 1270,
    QUEST_STINKYS_ESCAPE_A                       = 1222,
    SAY_QUEST_ACCEPTED                           = -1000612,
    SAY_STAY_1                                   = -1000613,
    SAY_STAY_2                                   = -1000614,
    SAY_STAY_3                                   = -1000615,
    SAY_STAY_4                                   = -1000616,
    SAY_STAY_5                                   = -1000617,
    SAY_STAY_6                                   = -1000618,
    SAY_QUEST_COMPLETE                           = -1000619,
    SAY_ATTACKED_1                               = -1000620,
    EMOTE_DISAPPEAR                              = -1000621
};

struct npc_stinkyAI : public npc_escortAI
{
    npc_stinkyAI(Creature* creature) : npc_escortAI(creature) { }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();
        if (!player)
            return;

        switch (i)
        {
        case 7:
            DoScriptText(SAY_STAY_1, me, player);
            break;
        case 11:
            DoScriptText(SAY_STAY_2, me, player);
            break;
        case 25:
            DoScriptText(SAY_STAY_3, me, player);
            break;
        case 26:
            DoScriptText(SAY_STAY_4, me, player);
            break;
        case 27:
            DoScriptText(SAY_STAY_5, me, player);
            break;
        case 28:
            DoScriptText(SAY_STAY_6, me, player);
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            break;
        case 29:
            me->SetStandState(UNIT_STAND_STATE_STAND);
            break;
        case 37:
            DoScriptText(SAY_QUEST_COMPLETE, me, player);
            SetRun();
            if (player && player->GetQuestStatus(QUEST_STINKYS_ESCAPE_H))
                player->GroupEventHappens(QUEST_STINKYS_ESCAPE_H, me);
            if (player && player->GetQuestStatus(QUEST_STINKYS_ESCAPE_A))
                player->GroupEventHappens(QUEST_STINKYS_ESCAPE_A, me);
            break;
        case 39:
            DoScriptText(EMOTE_DISAPPEAR, me);
            break;
        }
    }

    void EnterCombat(Unit* pWho)
    {
        DoScriptText(SAY_ATTACKED_1, me, pWho);
    }

    void Reset() {}

    void JustDied(Unit* /*pKiller*/)
    {
        Player* player = GetPlayerForEscort();

        if (HasEscortState(STATE_ESCORT_ESCORTING) && player)
        {
            if (player->GetQuestStatus(QUEST_STINKYS_ESCAPE_H))
                player->FailQuest(QUEST_STINKYS_ESCAPE_H);
            if (player->GetQuestStatus(QUEST_STINKYS_ESCAPE_A))
                player->FailQuest(QUEST_STINKYS_ESCAPE_A);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        npc_escortAI::UpdateAI(uiDiff);

            if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stinky(Creature* creature)
{
    return new npc_stinkyAI(creature);
}

bool QuestAccept_npc_stinky(Player* player, Creature* creature, Quest const *quest)
{
    if (quest->GetQuestId() == QUEST_STINKYS_ESCAPE_H || QUEST_STINKYS_ESCAPE_A)
    {
        if (npc_stinkyAI* pEscortAI = CAST_AI(npc_stinkyAI, creature->AI()))
        {
            creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
            creature->SetStandState(UNIT_STAND_STATE_STAND);
            DoScriptText(SAY_QUEST_ACCEPTED, creature);
            pEscortAI->Start(false, false, player->GetGUID());
        }
    }
    return true;
}

void AddSC_dustwallow_marsh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mobs_risen_husk_spirit";
    newscript->GetAI = &GetAI_mobs_risen_husk_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_restless_apparition";
    newscript->pGossipHello =   &GossipHello_npc_restless_apparition;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_deserter_agitator";
    newscript->GetAI = &GetAI_npc_deserter_agitator;
    newscript->pGossipHello = &GossipHello_npc_deserter_agitator;
    newscript->pGossipSelect = &GossipSelect_npc_deserter_agitator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_theramore_guard";
    newscript->GetAI = &GetAI_npc_theramore_guard;
    newscript->pGossipHello = &GossipHello_npc_theramore_guard;
    newscript->pGossipSelect = &GossipSelect_npc_theramore_guard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lady_jaina_proudmoore";
    newscript->pGossipHello = &GossipHello_npc_lady_jaina_proudmoore;
    newscript->pGossipSelect = &GossipSelect_npc_lady_jaina_proudmoore;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_nat_pagle";
    newscript->pGossipHello = &GossipHello_npc_nat_pagle;
    newscript->pGossipSelect = &GossipSelect_npc_nat_pagle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_morokk";
    newscript->GetAI = &GetAI_npc_morokk;
    newscript->pQuestAccept = &QuestAccept_npc_morokk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ogron";
    newscript->GetAI = &GetAI_npc_ogron;
    newscript->pQuestAccept = &QuestAccept_npc_ogron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_private_hendel";
    newscript->GetAI = &GetAI_npc_private_hendel;
    newscript->pQuestAccept = &QuestAccept_npc_private_hendel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_zelfrax";
    newscript->GetAI = &GetAI_npc_zelfrax;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_cassa_crimsonwing";
    newscript->pGossipHello = &GossipHello_npc_cassa_crimsonwing;
    newscript->pGossipSelect = &GossipSelect_npc_cassa_crimsonwing;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_nats_landing";
    newscript->pAreaTrigger = &AreaTrigger_at_nats_landing;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stinky";
    newscript->GetAI = &GetAI_npc_stinky;
    newscript->pQuestAccept = &QuestAccept_npc_stinky;
    newscript->RegisterSelf();
}

