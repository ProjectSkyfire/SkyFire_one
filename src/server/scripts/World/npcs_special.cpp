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
SDName: Npcs_Special
SD%Complete: 100
SDComment: To be used for special NPCs that are located globally.
SDCategory: NPCs
EndScriptData
*/

/* ContentData
npc_lunaclaw_spirit      80%    support for quests 6001/6002 (Body and Heart)
npc_chicken_cluck       100%    support for quest 3861 (Cluck!)
npc_dancing_flames      100%    midsummer event NPC
npc_guardian            100%    guardianAI used to prevent players from accessing off-limits areas. Not in use by SD2
npc_garments_of_quests   80%    NPC's related to all Garments of-quests 5621, 5624, 5625, 5648, 5650
npc_injured_patient     100%    patients for triage-quests (6622 and 6624)
npc_doctor              100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_mount_vendor        100%    Regular mount vendors all over the world. Display gossip if player doesn't meet the requirements to buy
npc_rogue_trainer        80%    Scripted trainers, so they are able to offer item 17126 for class quest 6681
npc_sayge               100%    Darkmoon event fortune teller, buff player based on answers given
npc_snake_trap_serpents  80%    AI for snakes that summoned by Snake Trap
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

/*######
## npc_lunaclaw_spirit
######*/

enum eLunaclaw
{
    QUEST_BODY_HEART_A      = 6001,
    QUEST_BODY_HEART_H      = 6002,

    TEXT_ID_DEFAULT         = 4714,
    TEXT_ID_PROGRESS        = 4715
};

#define GOSSIP_ITEM_GRANT   "You have thought well, spirit. I ask you to grant me the strength of your body and the strength of your heart."

bool GossipHello_npc_lunaclaw_spirit(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(QUEST_BODY_HEART_A) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_BODY_HEART_H) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GRANT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(TEXT_ID_DEFAULT, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_lunaclaw_spirit(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->SEND_GOSSIP_MENU(TEXT_ID_PROGRESS, creature->GetGUID());
        player->AreaExploredOrEventHappens(player->GetTeam() == ALLIANCE ? QUEST_BODY_HEART_A : QUEST_BODY_HEART_H);
    }
    return true;
}

/*########
# npc_chicken_cluck
#########*/

enum eChicken
{
    EMOTE_A_HELLO           = -1000204,
    EMOTE_H_HELLO           = -1000205,
    EMOTE_CLUCK_TEXT2       = -1000206,

    QUEST_CLUCK             = 3861,
    FACTION_FRIENDLY        = 35,
    FACTION_CHICKEN         = 31
};

struct npc_chicken_cluckAI : public ScriptedAI
{
    npc_chicken_cluckAI(Creature *c) : ScriptedAI(c) {}

    uint32 ResetFlagTimer;

    void Reset()
    {
        ResetFlagTimer = 120000;
        me->setFaction(FACTION_CHICKEN);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void EnterCombat(Unit * /*who*/) {}

    void UpdateAI(const uint32 diff)
    {
        // Reset flags after a certain time has passed so that the next player has to start the 'event' again
        if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
        {
            if (ResetFlagTimer <= diff)
            {
                EnterEvadeMode();
                return;
            } else ResetFlagTimer -= diff;
        }

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

    void ReceiveEmote(Player* player, uint32 emote)
    {
        if (emote == TEXTEMOTE_CHICKEN)
        {
            if (player->GetTeam() == ALLIANCE)
            {
                if (rand()%30 == 1)
                {
                    if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_A_HELLO, me);
                    }
                }
            }
            else
                DoScriptText(EMOTE_H_HELLO, me);
        }
        else if ((emote == TEXTEMOTE_CHEER && player->GetTeam() == ALLIANCE) &&
            (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->setFaction(FACTION_FRIENDLY);
            DoScriptText(EMOTE_CLUCK_TEXT2, me);
        }
    }
};

CreatureAI* GetAI_npc_chicken_cluck(Creature* creature)
{
    return new npc_chicken_cluckAI(creature);
}

bool QuestAccept_npc_chicken_cluck(Player* /*player*/, Creature* creature, const Quest *_Quest)
{
    if (_Quest->GetQuestId() == QUEST_CLUCK)
        CAST_AI(npc_chicken_cluckAI, creature->AI())->Reset();

    return true;
}

bool QuestComplete_npc_chicken_cluck(Player* /*player*/, Creature* creature, const Quest *_Quest)
{
    if (_Quest->GetQuestId() == QUEST_CLUCK)
        CAST_AI(npc_chicken_cluckAI, creature->AI())->Reset();

    return true;
}

/*######
## npc_dancing_flames
######*/

#define SPELL_BRAZIER       45423
#define SPELL_SEDUCTION     47057
#define SPELL_FIERY_AURA    45427

struct npc_dancing_flamesAI : public ScriptedAI
{
    npc_dancing_flamesAI(Creature *c) : ScriptedAI(c) {}

    bool active;
    uint32 can_iteract;

    void Reset()
    {
        active = true;
        can_iteract = 3500;
        DoCast(me, SPELL_BRAZIER, true);
        DoCast(me, SPELL_FIERY_AURA, false);
        float x, y, z;
        me->GetPosition(x, y, z);
        me->GetMap()->CreatureRelocation(me, x, y, z + 0.94f, 0.0f);
        me->AddUnitMovementFlag(MOVEFLAG_ONTRANSPORT | MOVEFLAG_LEVITATING);
        me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
        WorldPacket data;                       //send update position to client
        me->BuildHeartBeatMsg(&data);
        me->SendMessageToSet(&data, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!active)
        {
            if (can_iteract <= diff){
                active = true;
                can_iteract = 3500;
                me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
            } else can_iteract -= diff;
        }
    }

    void EnterCombat(Unit* /*who*/){}

    void ReceiveEmote(Player* player, uint32 emote)
    {
        if (me->IsWithinLOS(player->GetPositionX(),player->GetPositionY(),player->GetPositionZ()) && me->IsWithinDistInMap(player, 30.0f))
        {
            me->SetInFront(player);
            active = false;

            WorldPacket data;
            me->BuildHeartBeatMsg(&data);
            me->SendMessageToSet(&data, true);
            switch (emote)
            {
                case TEXTEMOTE_KISS:    me->HandleEmoteCommand(EMOTE_ONESHOT_SHY); break;
                case TEXTEMOTE_WAVE:    me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE); break;
                case TEXTEMOTE_BOW:     me->HandleEmoteCommand(EMOTE_ONESHOT_BOW); break;
                case TEXTEMOTE_JOKE:    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH); break;
                case TEXTEMOTE_DANCE:
                {
                    if (!player->HasAura(SPELL_SEDUCTION, 0))
                        DoCast(player, SPELL_SEDUCTION, true);
                }
                break;
            }
        }
    }
};

CreatureAI* GetAI_npc_dancing_flames(Creature* creature)
{
    return new npc_dancing_flamesAI(creature);
}

/*######
## Triage quest
######*/

enum eDoctor
{
    SAY_DOC1         = -1000201,
    SAY_DOC2         = -1000202,
    SAY_DOC3         = -1000203,

    DOCTOR_ALLIANCE  = 12939,
    DOCTOR_HORDE     = 12920,
    ALLIANCE_COORDS  = 7,
    HORDE_COORDS     = 6
};

struct Location
{
    float x, y, z, o;
};

static Location AllianceCoords[]=
{
    {-3757.38f, -4533.05f, 14.16f, 3.62f},                     // Top-far-right bunk as seen from entrance
    {-3754.36f, -4539.13f, 14.16f, 5.13f},                     // Top-far-left bunk
    {-3749.54f, -4540.25f, 14.28f, 3.34f},                     // Far-right bunk
    {-3742.10f, -4536.85f, 14.28f, 3.64f},                     // Right bunk near entrance
    {-3755.89f, -4529.07f, 14.05f, 0.57f},                     // Far-left bunk
    {-3749.51f, -4527.08f, 14.07f, 5.26f},                     // Mid-left bunk
    {-3746.37f, -4525.35f, 14.16f, 5.22f},                     // Left bunk near entrance
};

//alliance run to where
#define A_RUNTOX -3742.96f
#define A_RUNTOY -4531.52f
#define A_RUNTOZ 11.91f

static Location HordeCoords[]=
{
    {-1013.75f, -3492.59f, 62.62f, 4.34f},                     // Left, Behind
    {-1017.72f, -3490.92f, 62.62f, 4.34f},                     // Right, Behind
    {-1015.77f, -3497.15f, 62.82f, 4.34f},                     // Left, Mid
    {-1019.51f, -3495.49f, 62.82f, 4.34f},                     // Right, Mid
    {-1017.25f, -3500.85f, 62.98f, 4.34f},                     // Left, front
    {-1020.95f, -3499.21f, 62.98f, 4.34f}                       // Right, Front
};

//horde run to where
#define H_RUNTOX -1016.44f
#define H_RUNTOY -3508.48f
#define H_RUNTOZ 62.96f

const uint32 AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

const uint32 HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/

struct npc_doctorAI : public ScriptedAI
{
    npc_doctorAI(Creature *c) : ScriptedAI(c) {}

    uint64 PlayerGUID;

    uint32 SummonPatient_Timer;
    uint32 SummonPatientCount;
    uint32 PatientDiedCount;
    uint32 PatientSavedCount;

    bool Event;

    std::list<uint64> Patients;
    std::vector<Location*> Coordinates;

    void Reset()
    {
        PlayerGUID = 0;

        SummonPatient_Timer = 10000;
        SummonPatientCount = 0;
        PatientDiedCount = 0;
        PatientSavedCount = 0;

        Patients.clear();
        Coordinates.clear();

        Event = false;

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void BeginEvent(Player* player);
    void PatientDied(Location* Point);
    void PatientSaved(Creature* soldier, Player* player, Location* Point);
    void UpdateAI(const uint32 diff);

    void EnterCombat(Unit* /*who*/){}
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

struct npc_injured_patientAI : public ScriptedAI
{
    npc_injured_patientAI(Creature *c) : ScriptedAI(c) {}

    uint64 Doctorguid;
    Location* Coord;

    void Reset()
    {
        Doctorguid = 0;
        Coord = NULL;

        //no select
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        //no regen health
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

        //to make them lay with face down
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);

        uint32 mobId = me->GetEntry();

        switch (mobId)
        {                                                   //lower max health
            case 12923:
            case 12938:                                     //Injured Soldier
                me->SetHealth(uint32(me->GetMaxHealth()*.75));
                break;
            case 12924:
            case 12936:                                     //Badly injured Soldier
                me->SetHealth(uint32(me->GetMaxHealth()*.50));
                break;
            case 12925:
            case 12937:                                     //Critically injured Soldier
                me->SetHealth(uint32(me->GetMaxHealth()*.25));
                break;
        }
    }

    void EnterCombat(Unit* /*who*/){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (caster->GetTypeId() == TYPEID_PLAYER && me->isAlive() && spell->Id == 20804)
        {
            if ((CAST_PLR(caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (CAST_PLR(caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                if (Doctorguid)
                    if (Creature* Doctor = Unit::GetCreature(*me, Doctorguid))
                        CAST_AI(npc_doctorAI, Doctor->AI())->PatientSaved(me, CAST_PLR(caster), Coord);

            //make not selectable
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            //regen health
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

            //stand up
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);

            DoScriptText(RAND(SAY_DOC1, SAY_DOC2, SAY_DOC3), me);

            uint32 mobId = me->GetEntry();
            me->RemoveUnitMovementFlag(MOVEFLAG_WALK_MODE);

            switch (mobId)
            {
                case 12923:
                case 12924:
                case 12925:
                    me->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                    break;
                case 12936:
                case 12937:
                case 12938:
                    me->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                    break;
            }
        }
    }

    void UpdateAI(const uint32 /*diff*/)
    {
        //lower HP on every world tick makes it a useful counter, not officlone though
        if (me->isAlive() && me->GetHealth() > 6)
        {
            me->SetHealth(uint32(me->GetHealth()-5));
        }

        if (me->isAlive() && me->GetHealth() <= 6)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->setDeathState(JUST_DIED);
            me->SetFlag(UNIT_DYNAMIC_FLAGS, 32);

            if (Doctorguid)
            {
                if (Creature* Doctor = Unit::GetCreature((*me), Doctorguid))
                    CAST_AI(npc_doctorAI, Doctor->AI())->PatientDied(Coord);
            }
        }
    }
};

CreatureAI* GetAI_npc_injured_patient(Creature* creature)
{
    return new npc_injured_patientAI (creature);
}

/*
npc_doctor (continue)
*/

void npc_doctorAI::BeginEvent(Player* player)
{
    PlayerGUID = player->GetGUID();

    SummonPatient_Timer = 10000;
    SummonPatientCount = 0;
    PatientDiedCount = 0;
    PatientSavedCount = 0;

    switch (me->GetEntry())
    {
        case DOCTOR_ALLIANCE:
            for (uint8 i = 0; i < ALLIANCE_COORDS; ++i)
                Coordinates.push_back(&AllianceCoords[i]);
            break;
        case DOCTOR_HORDE:
            for (uint8 i = 0; i < HORDE_COORDS; ++i)
                Coordinates.push_back(&HordeCoords[i]);
            break;
    }

    Event = true;
    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
}

void npc_doctorAI::PatientDied(Location* Point)
{
    Player* player = Unit::GetPlayer(*me, PlayerGUID);
    if (player && ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
    {
        ++PatientDiedCount;

        if (PatientDiedCount > 5 && Event)
        {
            if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(6624);
            else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(6622);

            Reset();
            return;
        }

        Coordinates.push_back(Point);
    }
    else
        // If no player or player abandon quest in progress
        Reset();
}

void npc_doctorAI::PatientSaved(Creature* /*soldier*/, Player* player, Location* Point)
{
    if (player && PlayerGUID == player->GetGUID())
    {
        if ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
        {
            ++PatientSavedCount;

            if (PatientSavedCount == 15)
            {
                if (!Patients.empty())
                {
                    std::list<uint64>::const_iterator itr;
                    for (itr = Patients.begin(); itr != Patients.end(); ++itr)
                    {
                        if (Creature* Patient = Unit::GetCreature((*me), *itr))
                            Patient->setDeathState(JUST_DIED);
                    }
                }

                if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                    player->AreaExploredOrEventHappens(6624);
                else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                    player->AreaExploredOrEventHappens(6622);

                Reset();
                return;
            }

            Coordinates.push_back(Point);
        }
    }
}

void npc_doctorAI::UpdateAI(const uint32 diff)
{
    if (Event && SummonPatientCount >= 20)
    {
        Reset();
        return;
    }

    if (Event)
    {
        if (SummonPatient_Timer <= diff)
        {
            Creature* Patient = NULL;
            Location* Point = NULL;

            if (Coordinates.empty())
                return;

            std::vector<Location*>::iterator itr = Coordinates.begin()+rand()%Coordinates.size();
            uint32 patientEntry = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE: patientEntry = AllianceSoldierId[rand()%3]; break;
                case DOCTOR_HORDE:    patientEntry = HordeSoldierId[rand()%3]; break;
                default:
                    sLog->outError("TSCR: Invalid entry for Triage doctor. Please check your database");
                    return;
            }

            Point = *itr;

            Patient = me->SummonCreature(patientEntry, Point->x, Point->y, Point->z, Point->o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);

            if (Patient)
            {
                Patients.push_back(Patient->GetGUID());
                CAST_AI(npc_injured_patientAI, Patient->AI())->Doctorguid = me->GetGUID();

                if (Point)
                    CAST_AI(npc_injured_patientAI, Patient->AI())->Coord = Point;

                Coordinates.erase(itr);
            }
            SummonPatient_Timer = 10000;
            ++SummonPatientCount;
        } else SummonPatient_Timer -= diff;
    }
}

bool QuestAccept_npc_doctor(Player* player, Creature* creature, Quest const *quest)
{
    if ((quest->GetQuestId() == 6624) || (quest->GetQuestId() == 6622))
        CAST_AI(npc_doctorAI, creature->AI())->BeginEvent(player);

    return true;
}

CreatureAI* GetAI_npc_doctor(Creature* creature)
{
    return new npc_doctorAI (creature);
}

/*######
## npc_garments_of_quests
######*/

//TODO: get text for each NPC

enum eGarments
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    SAY_COMMON_HEALED       = -1000231,
    SAY_DG_KEL_THANKS       = -1000232,
    SAY_DG_KEL_GOODBYE      = -1000233,
    SAY_ROBERTS_THANKS      = -1000256,
    SAY_ROBERTS_GOODBYE     = -1000257,
    SAY_KORJA_THANKS        = -1000258,
    SAY_KORJA_GOODBYE       = -1000259,
    SAY_DOLF_THANKS         = -1000260,
    SAY_DOLF_GOODBYE        = -1000261,
    SAY_SHAYA_THANKS        = -1000262,
    SAY_SHAYA_GOODBYE       = -1000263,
};

struct npc_garments_of_questsAI : public npc_escortAI
{
    npc_garments_of_questsAI(Creature *c) : npc_escortAI(c) {Reset();}

    uint64 caster;

    bool bIsHealed;
    bool bCanRun;

    uint32 RunAwayTimer;

    void Reset()
    {
        caster = 0;

        bIsHealed = false;
        bCanRun = false;

        RunAwayTimer = 5000;

        me->SetStandState(UNIT_STAND_STATE_KNEEL);
        //expect database to have RegenHealth=0
        me->SetHealth(int(me->GetMaxHealth()*0.7));
    }

    void EnterCombat(Unit * /*who*/) {}

    void SpellHit(Unit* pCaster, const SpellEntry *Spell)
    {
        if (Spell->Id == SPELL_LESSER_HEAL_R2 || Spell->Id == SPELL_FORTITUDE_R1)
        {
            //not while in combat
            if (me->isInCombat())
                return;

            //nothing to be done now
            if (bIsHealed && bCanRun)
                return;

            if (pCaster->GetTypeId() == TYPEID_PLAYER)
            {
                switch (me->GetEntry())
                {
                    case ENTRY_SHAYA:
                        if (CAST_PLR(pCaster)->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (bIsHealed && !bCanRun && Spell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_SHAYA_THANKS, me, pCaster);
                                bCanRun = true;
                            }
                            else if (!bIsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                caster = pCaster->GetGUID();
                                me->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, me, pCaster);
                                bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_ROBERTS:
                        if (CAST_PLR(pCaster)->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (bIsHealed && !bCanRun && Spell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_ROBERTS_THANKS, me, pCaster);
                                bCanRun = true;
                            }
                            else if (!bIsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                caster = pCaster->GetGUID();
                                me->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, me, pCaster);
                                bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_DOLF:
                        if (CAST_PLR(pCaster)->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (bIsHealed && !bCanRun && Spell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_DOLF_THANKS, me, pCaster);
                                bCanRun = true;
                            }
                            else if (!bIsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                caster = pCaster->GetGUID();
                                me->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, me, pCaster);
                                bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_KORJA:
                        if (CAST_PLR(pCaster)->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (bIsHealed && !bCanRun && Spell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_KORJA_THANKS, me, pCaster);
                                bCanRun = true;
                            }
                            else if (!bIsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                caster = pCaster->GetGUID();
                                me->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, me, pCaster);
                                bIsHealed = true;
                            }
                        }
                        break;
                    case ENTRY_DG_KEL:
                        if (CAST_PLR(pCaster)->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (bIsHealed && !bCanRun && Spell->Id == SPELL_FORTITUDE_R1)
                            {
                                DoScriptText(SAY_DG_KEL_THANKS, me, pCaster);
                                bCanRun = true;
                            }
                            else if (!bIsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                            {
                                caster = pCaster->GetGUID();
                                me->SetStandState(UNIT_STAND_STATE_STAND);
                                DoScriptText(SAY_COMMON_HEALED, me, pCaster);
                                bIsHealed = true;
                            }
                        }
                        break;
                }

                //give quest credit, not expect any special quest objectives
                if (bCanRun)
                    CAST_PLR(pCaster)->TalkedToCreature(me->GetEntry(),me->GetGUID());
            }
        }
    }

    void WaypointReached(uint32 /*uiPoint*/)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (bCanRun && !me->isInCombat())
        {
            if (RunAwayTimer <= diff)
            {
                if (Unit *pUnit = Unit::GetUnit(*me, caster))
                {
                    switch (me->GetEntry())
                    {
                        case ENTRY_SHAYA: DoScriptText(SAY_SHAYA_GOODBYE, me, pUnit); break;
                        case ENTRY_ROBERTS: DoScriptText(SAY_ROBERTS_GOODBYE, me, pUnit); break;
                        case ENTRY_DOLF: DoScriptText(SAY_DOLF_GOODBYE, me, pUnit); break;
                        case ENTRY_KORJA: DoScriptText(SAY_KORJA_GOODBYE, me, pUnit); break;
                        case ENTRY_DG_KEL: DoScriptText(SAY_DG_KEL_GOODBYE, me, pUnit); break;
                    }

                    Start(false, true, true);
                }
                else
                    EnterEvadeMode();                       //something went wrong

                RunAwayTimer = 30000;
            } else RunAwayTimer -= diff;
        }

    npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_garments_of_quests(Creature* creature)
{
    return new npc_garments_of_questsAI(creature);
}

/*######
## npc_guardian
######*/

#define SPELL_DEATHTOUCH                5
#define SAY_AGGRO                        "This area is closed!"

struct npc_guardianAI : public ScriptedAI
{
    npc_guardianAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void EnterCombat(Unit * /*who*/)
    {
        me->MonsterYell(SAY_AGGRO, LANG_UNIVERSAL, 0);
    }

    void UpdateAI(const uint32 /*diff*/)
    {
        if (!UpdateVictim())
            return;

        if (me->isAttackReady())
        {
            DoCast(me->getVictim(), SPELL_DEATHTOUCH, true);
            me->resetAttackTimer();
        }
    }
};

CreatureAI* GetAI_npc_guardian(Creature* creature)
{
    return new npc_guardianAI (creature);
}

/*######
## npc_mount_vendor
######*/

bool GossipHello_npc_mount_vendor(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    bool canBuy;
    canBuy = false;
    uint32 vendor = creature->GetEntry();
    uint8 race = player->getRace();

    switch (vendor)
    {
        case 384:                                           //Katie Hunter
        case 1460:                                          //Unger Statforth
        case 2357:                                          //Merideth Carlson
        case 4885:                                          //Gregor MacVince
            if (player->GetReputationRank(72) != REP_EXALTED && race != RACE_HUMAN)
                player->SEND_GOSSIP_MENU(5855, creature->GetGUID());
            else canBuy = true;
            break;
        case 1261:                                          //Veron Amberstill
            if (player->GetReputationRank(47) != REP_EXALTED && race != RACE_DWARF)
                player->SEND_GOSSIP_MENU(5856, creature->GetGUID());
            else canBuy = true;
            break;
        case 3362:                                          //Ogunaro Wolfrunner
            if (player->GetReputationRank(76) != REP_EXALTED && race != RACE_ORC)
                player->SEND_GOSSIP_MENU(5841, creature->GetGUID());
            else canBuy = true;
            break;
        case 3685:                                          //Harb Clawhoof
            if (player->GetReputationRank(81) != REP_EXALTED && race != RACE_TAUREN)
                player->SEND_GOSSIP_MENU(5843, creature->GetGUID());
            else canBuy = true;
            break;
        case 4730:                                          //Lelanai
            if (player->GetReputationRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
                player->SEND_GOSSIP_MENU(5844, creature->GetGUID());
            else canBuy = true;
            break;
        case 4731:                                          //Zachariah Post
            if (player->GetReputationRank(68) != REP_EXALTED && race != RACE_UNDEAD_PLAYER)
                player->SEND_GOSSIP_MENU(5840, creature->GetGUID());
            else canBuy = true;
            break;
        case 7952:                                          //Zjolnir
            if (player->GetReputationRank(530) != REP_EXALTED && race != RACE_TROLL)
                player->SEND_GOSSIP_MENU(5842, creature->GetGUID());
            else canBuy = true;
            break;
        case 7955:                                          //Milli Featherwhistle
            if (player->GetReputationRank(54) != REP_EXALTED && race != RACE_GNOME)
                player->SEND_GOSSIP_MENU(5857, creature->GetGUID());
            else canBuy = true;
            break;
        case 16264:                                         //Winaestra
            if (player->GetReputationRank(911) != REP_EXALTED && race != RACE_BLOODELF)
                player->SEND_GOSSIP_MENU(10305, creature->GetGUID());
            else canBuy = true;
            break;
        case 17584:                                         //Torallius the Pack Handler
            if (player->GetReputationRank(930) != REP_EXALTED && race != RACE_DRAENEI)
                player->SEND_GOSSIP_MENU(10239, creature->GetGUID());
            else canBuy = true;
            break;
    }

    if (canBuy)
    {
        if (creature->isVendor())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_mount_vendor(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST(creature->GetGUID());

    return true;
}

/*######
## npc_rogue_trainer
######*/

#define GOSSIP_HELLO_ROGUE1 "I wish to unlearn my talents"
#define GOSSIP_HELLO_ROGUE2 "<Take the letter>"
#define GOSSIP_HELLO_ROGUE3 "Purchase a Dual Talent Specialization."

bool GossipHello_npc_rogue_trainer(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (creature->isTrainer())
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

    if (creature->isCanTrainingAndResetTalentsOf(player))
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE1, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_UNLEARNTALENTS);

    if (player->getClass() == CLASS_ROGUE && player->getLevel() >= 24 && !player->HasItemCount(17126, 1) && !player->GetQuestRewardStatus(6681))
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_ROGUE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(5996, creature->GetGUID());
    } else
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_rogue_trainer(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player, 21100, false);
            break;
        case GOSSIP_ACTION_TRAIN:
            player->SEND_TRAINERLIST(creature->GetGUID());
            break;
        case GOSSIP_OPTION_UNLEARNTALENTS:
            player->CLOSE_GOSSIP_MENU();
            player->SendTalentWipeConfirm(creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_sayge
######*/

#define SPELL_DMG       23768                               //dmg
#define SPELL_RES       23769                               //res
#define SPELL_ARM       23767                               //arm
#define SPELL_SPI       23738                               //spi
#define SPELL_INT       23766                               //int
#define SPELL_STM       23737                               //stm
#define SPELL_STR       23735                               //str
#define SPELL_AGI       23736                               //agi
#define SPELL_FORTUNE   23765                               //faire fortune

#define GOSSIP_HELLO_SAYGE  "Yes"
#define GOSSIP_SENDACTION_SAYGE1    "Slay the Man"
#define GOSSIP_SENDACTION_SAYGE2    "Turn him over to liege"
#define GOSSIP_SENDACTION_SAYGE3    "Confiscate the corn"
#define GOSSIP_SENDACTION_SAYGE4    "Let him go and have the corn"
#define GOSSIP_SENDACTION_SAYGE5    "Execute your friend painfully"
#define GOSSIP_SENDACTION_SAYGE6    "Execute your friend painlessly"
#define GOSSIP_SENDACTION_SAYGE7    "Let your friend go"
#define GOSSIP_SENDACTION_SAYGE8    "Confront the diplomat"
#define GOSSIP_SENDACTION_SAYGE9    "Show not so quiet defiance"
#define GOSSIP_SENDACTION_SAYGE10   "Remain quiet"
#define GOSSIP_SENDACTION_SAYGE11   "Speak against your brother openly"
#define GOSSIP_SENDACTION_SAYGE12   "Help your brother in"
#define GOSSIP_SENDACTION_SAYGE13   "Keep your brother out without letting him know"
#define GOSSIP_SENDACTION_SAYGE14   "Take credit, keep gold"
#define GOSSIP_SENDACTION_SAYGE15   "Take credit, share the gold"
#define GOSSIP_SENDACTION_SAYGE16   "Let the knight take credit"
#define GOSSIP_SENDACTION_SAYGE17   "Thanks"

bool GossipHello_npc_sayge(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->HasSpellCooldown(SPELL_INT) ||
        player->HasSpellCooldown(SPELL_ARM) ||
        player->HasSpellCooldown(SPELL_DMG) ||
        player->HasSpellCooldown(SPELL_RES) ||
        player->HasSpellCooldown(SPELL_STR) ||
        player->HasSpellCooldown(SPELL_AGI) ||
        player->HasSpellCooldown(SPELL_STM) ||
        player->HasSpellCooldown(SPELL_SPI))
        player->SEND_GOSSIP_MENU(7393, creature->GetGUID());
    else
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_SAYGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(7339, creature->GetGUID());
    }

    return true;
}

void SendAction_npc_sayge(Player* player, Creature* creature, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE1,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE2,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE3,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE4,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(7340, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE5,            GOSSIP_SENDER_MAIN+1, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE6,            GOSSIP_SENDER_MAIN+2, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE7,            GOSSIP_SENDER_MAIN+3, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(7341, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE8,            GOSSIP_SENDER_MAIN+4, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE9,            GOSSIP_SENDER_MAIN+5, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE10,           GOSSIP_SENDER_MAIN+2, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(7361, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE11,           GOSSIP_SENDER_MAIN+6, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE12,           GOSSIP_SENDER_MAIN+7, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE13,           GOSSIP_SENDER_MAIN+8, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(7362, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE14,           GOSSIP_SENDER_MAIN+5, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE15,           GOSSIP_SENDER_MAIN+4, GOSSIP_ACTION_INFO_DEF);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE16,           GOSSIP_SENDER_MAIN+3, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(7363, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE17,           GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(7364, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            creature->CastSpell(player, SPELL_FORTUNE, false);
            player->SEND_GOSSIP_MENU(7365, creature->GetGUID());
            break;
    }
}

bool GossipSelect_npc_sayge(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    switch (uiSender)
    {
        case GOSSIP_SENDER_MAIN:
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+1:
            creature->CastSpell(player, SPELL_DMG, false);
            player->AddSpellCooldown(SPELL_DMG, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+2:
            creature->CastSpell(player, SPELL_RES, false);
            player->AddSpellCooldown(SPELL_RES, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+3:
            creature->CastSpell(player, SPELL_ARM, false);
            player->AddSpellCooldown(SPELL_ARM, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+4:
            creature->CastSpell(player, SPELL_SPI, false);
            player->AddSpellCooldown(SPELL_SPI, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+5:
            creature->CastSpell(player, SPELL_INT, false);
            player->AddSpellCooldown(SPELL_INT, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+6:
            creature->CastSpell(player, SPELL_STM, false);
            player->AddSpellCooldown(SPELL_STM, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+7:
            creature->CastSpell(player, SPELL_STR, false);
            player->AddSpellCooldown(SPELL_STR, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
        case GOSSIP_SENDER_MAIN+8:
            creature->CastSpell(player, SPELL_AGI, false);
            player->AddSpellCooldown(SPELL_AGI, 0, time(NULL) + 7200);
            SendAction_npc_sayge(player, creature, uiAction);
            break;
    }
    return true;
}

struct npc_steam_tonkAI : public ScriptedAI
{
    npc_steam_tonkAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}
    void EnterCombat(Unit * /*who*/) {}

    void OnPossess(bool apply)
    {
        if (apply)
        {
            // Initialize the action bar without the melee attack command
            me->InitCharmInfo();
            me->GetCharmInfo()->InitEmptyActionBar(false);

            me->SetReactState(REACT_PASSIVE);
        }
        else
            me->SetReactState(REACT_AGGRESSIVE);
    }
};

CreatureAI* GetAI_npc_steam_tonk(Creature* creature)
{
    return new npc_steam_tonkAI(creature);
}

#define SPELL_TONK_MINE_DETONATE 25099

struct npc_tonk_mineAI : public ScriptedAI
{
    npc_tonk_mineAI(Creature *c) : ScriptedAI(c)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 ExplosionTimer;

    void Reset()
    {
        ExplosionTimer = 3000;
    }

    void EnterCombat(Unit * /*who*/) {}
    void AttackStart(Unit * /*who*/) {}
    void MoveInLineOfSight(Unit * /*who*/) {}

    void UpdateAI(const uint32 diff)
    {
        if (ExplosionTimer <= diff)
        {
            DoCast(me, SPELL_TONK_MINE_DETONATE, true);
            me->setDeathState(DEAD); // unsummon it
        } else
            ExplosionTimer -= diff;
    }
};

CreatureAI* GetAI_npc_tonk_mine(Creature* creature)
{
    return new npc_tonk_mineAI(creature);
}

/*####
## npc_brewfest_reveler
####*/

struct npc_brewfest_revelerAI : public ScriptedAI
{
    npc_brewfest_revelerAI(Creature* c) : ScriptedAI(c) {}
    void ReceiveEmote(Player* player, uint32 emote)
    {
        if (emote == TEXTEMOTE_DANCE)
            me->CastSpell(player, 41586, false);
    }
};

CreatureAI* GetAI_npc_brewfest_reveler(Creature* creature)
{
    return new npc_brewfest_revelerAI(creature);
}

/*####
## npc_winter_reveler
####*/

struct npc_winter_revelerAI : public ScriptedAI
{
    npc_winter_revelerAI(Creature* c) : ScriptedAI(c) {}
    void ReceiveEmote(Player* player, uint32 emote)
    {
        //TODO: check auralist.
        if (player->HasAura(26218, 1))
            return;

        if (emote == TEXTEMOTE_KISS)
        {
            me->CastSpell(me, 26218, false);
            player->CastSpell(player, 26218, false);
            switch (urand(0, 2))
            {
                case 0: me->CastSpell(player, 26207, false); break;
                case 1: me->CastSpell(player, 26206, false); break;
                case 2: me->CastSpell(player, 45036, false); break;
            }
        }
    }
};

CreatureAI* GetAI_npc_winter_reveler(Creature* creature)
{
    return new npc_winter_revelerAI(creature);
}

/*####
## npc_snake_trap_serpents
####*/

enum SnakeTrap
{
    SPELL_MIND_NUMBING_POISON     = 25810,	// Viper poison 1
    SPELL_DEADLY_POISON           = 34655,	// Viper poison 2
    SPELL_CRIPPLING_POISON        = 30981,	// Viper poison 3

    NPC_VIPER                     = 19921   // Viper
};

#define VENOMOUS_SNAKE_TIMER 1500
#define VIPER_TIMER 3000
#define RAND 5

struct npc_snake_trap_serpentsAI : public ScriptedAI
{
    npc_snake_trap_serpentsAI(Creature* creature) : ScriptedAI(creature) {}

    uint32 SpellTimer;
    bool IsViper;

    void EnterCombat(Unit* /*who*/) {}

    void Reset()
    {
        SpellTimer = 0;

        CreatureTemplate const* Info = me->GetCreatureTemplate();

        if (Info->Entry == NPC_VIPER)
            IsViper = true;
        else
            IsViper = false;

        me->SetMaxHealth(uint32(107 * (me->getLevel() - 40) * 0.025f));
        // Add delta to make them not all hit the same time
        uint32 delta = (rand() % 7) * 100;
        me->SetStatFloatValue(UNIT_FIELD_BASEATTACKTIME, float(Info->baseattacktime + delta));
        me->SetStatFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER , float(Info->attackpower));

        // Start attacking attacker of owner on first ai update after spawn - move in line of sight may choose better target
        if (!me->getVictim() && me->isSummon())
            if (Unit * Owner = CAST_SUM(me)->GetSummoner())
                if (Owner->getAttackerForHelper())
                    AttackStart(Owner->getAttackerForHelper());
    }

    // Redefined for random target selection:
    void MoveInLineOfSight(Unit *who)
    {
        if (!me->getVictim() && who->isTargetableForAttack() && (me->IsHostileTo(who)) && who->isInAccessiblePlaceFor(me))
        {
            if (me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;

            float attackRadius = me->GetAttackDistance(who);
            if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
            {
                if (!(rand() % RAND))
                {
                    me->setAttackTimer(BASE_ATTACK, (rand() % 10) * 100);
                    SpellTimer = (rand() % 10) * 100;
                    AttackStart(who);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpellTimer <= diff)
        {
            if (IsViper) // Viper
            {
                if (urand(0, 2) == 0) // 33% chance to cast
                {
                    uint32 spell;
                    if (urand(0, 1) == 0)
                        spell = SPELL_MIND_NUMBING_POISON;
                    else
                        spell = SPELL_CRIPPLING_POISON;

                    DoCast(me->getVictim(), spell);
                }

                SpellTimer = VIPER_TIMER;
            }
            else // Venomous Snake
            {
                if (urand(0, 2) == 0) // 33% chance to cast
                    DoCast(me->getVictim(), SPELL_DEADLY_POISON);
                SpellTimer = VENOMOUS_SNAKE_TIMER + (rand() %5)*100;
            }
        } else SpellTimer -= diff;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_snake_trap_serpents(Creature* creature)
{
    return new npc_snake_trap_serpentsAI(creature);
}

#define SAY_RANDOM_MOJO0    "Now that's what I call froggy-style!"
#define SAY_RANDOM_MOJO1    "Your lily pad or mine?"
#define SAY_RANDOM_MOJO2    "This won't take long, did it?"
#define SAY_RANDOM_MOJO3    "I thought you'd never ask!"
#define SAY_RANDOM_MOJO4    "I promise not to give you warts..."
#define SAY_RANDOM_MOJO5    "Feelin' a little froggy, are ya?"
#define SAY_RANDOM_MOJO6a   "Listen, "
#define SAY_RANDOM_MOJO6b   ", I know of a little swamp not too far from here...."
#define SAY_RANDOM_MOJO7    "There's just never enough Mojo to go around..."

struct mob_mojoAI : public ScriptedAI
{
    mob_mojoAI(Creature *c) : ScriptedAI(c) {Reset();}
    uint32 hearts;
    uint64 victimGUID;
    void Reset()
    {
        victimGUID = 0;
        hearts = 15000;
        if (Unit* own = me->GetOwner())
            me->GetMotionMaster()->MoveFollow(own, 0, 0);
    }
    void Aggro(Unit * /*who*/){}
    void UpdateAI(const uint32 diff)
    {
        if (me->HasAura(20372, 0))
        {
            if (hearts <= diff)
            {
                me->RemoveAurasDueToSpell(20372);
                hearts = 15000;
            } hearts -= diff;
        }
    }
    void ReceiveEmote(Player* player, uint32 emote)
    {
        me->HandleEmoteCommand(emote);
        Unit* own = me->GetOwner();
        if (!own || own->GetTypeId() != TYPEID_PLAYER || CAST_PLR(own)->GetTeam() != player->GetTeam())
            return;
        if (emote == TEXTEMOTE_KISS)
        {
            std::string whisp = "";
            switch (rand()%8)
            {
                case 0:whisp.append(SAY_RANDOM_MOJO0);break;
                case 1:whisp.append(SAY_RANDOM_MOJO1);break;
                case 2:whisp.append(SAY_RANDOM_MOJO2);break;
                case 3:whisp.append(SAY_RANDOM_MOJO3);break;
                case 4:whisp.append(SAY_RANDOM_MOJO4);break;
                case 5:whisp.append(SAY_RANDOM_MOJO5);break;
                case 6:
                    whisp.append(SAY_RANDOM_MOJO6a);
                    whisp.append(player->GetName());
                    whisp.append(SAY_RANDOM_MOJO6b);
                    break;
                case 7:whisp.append(SAY_RANDOM_MOJO7);break;
            }
            me->MonsterWhisper(whisp.c_str(),player->GetGUID());
            if (victimGUID)
            {
                Player* victim = Unit::GetPlayer(*me, victimGUID);
                if (victim && victim->HasAura(43906, 0))
                    victim->RemoveAurasDueToSpell(43906); // remove polymorph frog thing
            }
            me->AddAura(43906, player);//add polymorph frog thing
            victimGUID = player->GetGUID();
            DoCast(me, 20372, true);//tag.hearts
            me->GetMotionMaster()->MoveFollow(player, 0, 0);
            hearts = 15000;
        }
    }
};

CreatureAI* GetAI_mob_mojo(Creature* creature)
{
    return new mob_mojoAI (creature);
}

void AddSC_npcs_special()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_lunaclaw_spirit";
    newscript->pGossipHello =  &GossipHello_npc_lunaclaw_spirit;
    newscript->pGossipSelect = &GossipSelect_npc_lunaclaw_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_chicken_cluck";
    newscript->GetAI = &GetAI_npc_chicken_cluck;
    newscript->pQuestAccept =   &QuestAccept_npc_chicken_cluck;
    newscript->pQuestComplete = &QuestComplete_npc_chicken_cluck;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dancing_flames";
    newscript->GetAI = &GetAI_npc_dancing_flames;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_injured_patient";
    newscript->GetAI = &GetAI_npc_injured_patient;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_doctor";
    newscript->GetAI = &GetAI_npc_doctor;
    newscript->pQuestAccept = &QuestAccept_npc_doctor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_garments_of_quests";
    newscript->GetAI = &GetAI_npc_garments_of_quests;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_guardian";
    newscript->GetAI = &GetAI_npc_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mount_vendor";
    newscript->pGossipHello =  &GossipHello_npc_mount_vendor;
    newscript->pGossipSelect = &GossipSelect_npc_mount_vendor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_rogue_trainer";
    newscript->pGossipHello =  &GossipHello_npc_rogue_trainer;
    newscript->pGossipSelect = &GossipSelect_npc_rogue_trainer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_sayge";
    newscript->pGossipHello = &GossipHello_npc_sayge;
    newscript->pGossipSelect = &GossipSelect_npc_sayge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_steam_tonk";
    newscript->GetAI = &GetAI_npc_steam_tonk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tonk_mine";
    newscript->GetAI = &GetAI_npc_tonk_mine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_winter_reveler";
    newscript->GetAI = &GetAI_npc_winter_reveler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_brewfest_reveler";
    newscript->GetAI = &GetAI_npc_winter_reveler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_snake_trap_serpents";
    newscript->GetAI = &GetAI_npc_snake_trap_serpents;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_mojo";
    newscript->GetAI = &GetAI_mob_mojo;
    newscript->RegisterSelf();
}

