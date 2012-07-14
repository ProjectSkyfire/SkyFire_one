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
SDName: Instance_Old_Hillsbrad
SD%Complete: 75
SDComment: If thrall escort fail, all parts will reset. In future, save sub-parts and continue from last known.
SDCategory: Caverns of Time, Old Hillsbrad Foothills
EndScriptData */

#include "ScriptPCH.h"
#include "old_hillsbrad.h"

#define ENCOUNTERS      6

#define THRALL_ENTRY    17876
#define TARETHA_ENTRY   18887
#define EPOCH_ENTRY    18096

#define DRAKE_ENTRY             17848

#define QUEST_ENTRY_DIVERSION   10283
#define LODGE_QUEST_TRIGGER     20155

struct instance_old_hillsbrad : public ScriptedInstance
{
    instance_old_hillsbrad(Map *map) : ScriptedInstance(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    uint32 mBarrelCount;
    uint32 mThrallEventCount;

    uint64 ThrallGUID;
    uint64 TarethaGUID;
    uint64 EpochGUID;

    void Initialize()
    {
        mBarrelCount        = 0;
        mThrallEventCount   = 0;
        ThrallGUID          = 0;
        TarethaGUID         = 0;
    EpochGUID        = 0;

        for (uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
    }

    Player* GetPlayerInMap()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource())
                    return plr;
            }
        }

        sLog->outDebug("TSCR: Instance Old Hillsbrad: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void UpdateOHWorldState()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                {
                    player->SendUpdateWorldState(WORLD_STATE_OH, mBarrelCount);

                    if (mBarrelCount == 5)
                        player->KilledMonsterCredit(LODGE_QUEST_TRIGGER, 0);
                }
            }
        } else
            sLog->outDebug("TSCR: Instance Old Hillsbrad: UpdateOHWorldState, but PlayerList is empty!");
    }

    void OnCreatureCreate(Creature* creature, bool /*add*/)
    {
        switch (creature->GetEntry())
        {
            case THRALL_ENTRY:
                ThrallGUID = creature->GetGUID();
                break;
            case TARETHA_ENTRY:
                TarethaGUID = creature->GetGUID();
                break;
        case EPOCH_ENTRY:
        EpochGUID = creature->GetGUID();
        break;
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        Player* player = GetPlayerInMap();

        if (!player)
        {
            sLog->outDebug("TSCR: Instance Old Hillsbrad: SetData (Type: %u Data %u) cannot find any player.", type, data);
            return;
        }

        switch (type)
        {
            case TYPE_BARREL_DIVERSION:
            {
                if (data == IN_PROGRESS)
                {
                    if (mBarrelCount >= 5)
                        return;

                    ++mBarrelCount;
                    UpdateOHWorldState();

                    sLog->outDebug("TSCR: Instance Old Hillsbrad: go_barrel_old_hillsbrad count %u",mBarrelCount);

                    Encounter[0] = IN_PROGRESS;

                    if (mBarrelCount == 5)
                    {
                    player->SummonCreature(DRAKE_ENTRY, 2128.43f, 71.01f, 64.42f, 1.74f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1800000);
                    Encounter[0] = DONE;
                    }
                }
                break;
            }
            case TYPE_THRALL_EVENT:
            {
                if (data == FAIL)
                {
                    if (mThrallEventCount <= 20)
                    {
                        mThrallEventCount++;
                        Encounter[1] = NOT_STARTED;
                        sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall event failed %u times. Resetting all sub-events.",mThrallEventCount);
                        Encounter[2] = NOT_STARTED;
                        Encounter[3] = NOT_STARTED;
                        Encounter[4] = NOT_STARTED;
                        Encounter[5] = NOT_STARTED;
                    }
                    else if (mThrallEventCount > 20)
                    {
                        Encounter[1] = data;
                        Encounter[2] = data;
                        Encounter[3] = data;
                        Encounter[4] = data;
                        Encounter[5] = data;
                        sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall event failed %u times. Resetting all sub-events.",mThrallEventCount);
                    }
                }
                else
                    Encounter[1] = data;
                sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall escort event adjusted to data %u.",data);
                break;
            }
            case TYPE_THRALL_PART1:
                Encounter[2] = data;
                sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall event part I adjusted to data %u.",data);
                break;
            case TYPE_THRALL_PART2:
                Encounter[3] = data;
                sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall event part II adjusted to data %u.",data);
                break;
            case TYPE_THRALL_PART3:
                Encounter[4] = data;
                sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall event part III adjusted to data %u.",data);
                break;
            case TYPE_THRALL_PART4:
                Encounter[5] = data;
                 sLog->outDebug("TSCR: Instance Old Hillsbrad: Thrall event part IV adjusted to data %u.",data);
                break;
        }
    }

    uint32 GetData(uint32 data)
    {
        switch (data)
        {
            case TYPE_BARREL_DIVERSION:
                return Encounter[0];
            case TYPE_THRALL_EVENT:
                return Encounter[1];
            case TYPE_THRALL_PART1:
                return Encounter[2];
            case TYPE_THRALL_PART2:
                return Encounter[3];
            case TYPE_THRALL_PART3:
                return Encounter[4];
            case TYPE_THRALL_PART4:
                return Encounter[5];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch (data)
        {
            case DATA_THRALL:
                return ThrallGUID;
            case DATA_TARETHA:
                return TarethaGUID;
        case DATA_EPOCH:
        return EpochGUID;
        }
        return 0;
    }
};
InstanceScript* GetInstanceData_instance_old_hillsbrad(Map* map)
{
    return new instance_old_hillsbrad(map);
}

void AddSC_instance_old_hillsbrad()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_old_hillsbrad";
    newscript->GetInstanceScript = &GetInstanceData_instance_old_hillsbrad;
    newscript->RegisterSelf();
}

