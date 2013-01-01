/*
 * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2010-2013 Oregon <http://www.oregoncore.com/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2013 MaNGOS <http://getmangos.com/>
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

#include "InstanceScript.h"
#include "DatabaseEnv.h"
#include "Map.h"
#include "GameObject.h"
#include "Creature.h"
#include "CreatureAI.h"

void InstanceScript::SaveToDB()
{
    std::string data = GetSaveData();
    if (data.empty())
        return;
    CharacterDatabase.EscapeString(data);
    CharacterDatabase.PExecute("UPDATE instance SET data = '%s' WHERE id = '%d'", data.c_str(), instance->GetInstanceId());
}

void InstanceScript::HandleGameObject(uint64 GUID, bool open, GameObject *go)
{
    if (!go)
        go = instance->GetGameObject(GUID);
    if (go)
        go->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
    else
        sLog->outDebug("TSCR: InstanceScript: HandleGameObject failed");
}

bool InstanceScript::IsEncounterInProgress() const
{
    for (std::vector<BossInfo>::const_iterator itr = bosses.begin(); itr != bosses.end(); ++itr)
        if (itr->state == IN_PROGRESS)
            return true;

    return false;
}

void InstanceScript::LoadMinionData(const MinionData *data)
{
    while (data->entry)
    {
        if (data->bossId < bosses.size())
            minions.insert(std::make_pair(data->entry, MinionInfo(&bosses[data->bossId])));

        ++data;
    }
    sLog->outDebug("InstanceScript::LoadMinionData: %u minions loaded.", doors.size());
}

void InstanceScript::LoadDoorData(const DoorData *data)
{
    while (data->entry)
    {
        if (data->bossId < bosses.size())
            doors.insert(std::make_pair(data->entry, DoorInfo(&bosses[data->bossId], data->type)));

        ++data;
    }
    sLog->outDebug("InstanceScript::LoadDoorData: %u doors loaded.", doors.size());
}

void InstanceScript::UpdateMinionState(Creature *minion, EncounterState state)
{
    switch (state)
    {
        case NOT_STARTED:
            if (!minion->isAlive())
                minion->Respawn();
            else if (minion->isInCombat())
                minion->AI()->EnterEvadeMode();
            break;
        case IN_PROGRESS:
            if (!minion->isAlive())
                minion->Respawn();
            else if (!minion->getVictim())
                minion->AI()->DoZoneInCombat();
            break;
    }
}

void InstanceScript::UpdateDoorState(GameObject *door)
{
    DoorInfoMap::iterator lower = doors.lower_bound(door->GetEntry());
    DoorInfoMap::iterator upper = doors.upper_bound(door->GetEntry());
    if (lower == upper)
        return;

    bool open = true;
    for (DoorInfoMap::iterator itr = lower; itr != upper; ++itr)
    {
        if (itr->second.type == DOOR_TYPE_ROOM)
        {
            if (itr->second.bossInfo->state == IN_PROGRESS)
            {
                open = false;
                break;
            }
        }
        else if (itr->second.type == DOOR_TYPE_PASSAGE)
        {
            if (itr->second.bossInfo->state != DONE)
            {
                open = false;
                break;
            }
        }
    }

    door->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
}

void InstanceScript::AddDoor(GameObject *door, bool add)
{
    DoorInfoMap::iterator lower = doors.lower_bound(door->GetEntry());
    DoorInfoMap::iterator upper = doors.upper_bound(door->GetEntry());
    if (lower == upper)
        return;

    for (DoorInfoMap::iterator itr = lower; itr != upper; ++itr)
    {
        if (add)
            itr->second.bossInfo->door[itr->second.type].insert(door);
        else
            itr->second.bossInfo->door[itr->second.type].erase(door);
    }

    if (add)
        UpdateDoorState(door);
}

void InstanceScript::AddMinion(Creature *minion, bool add)
{
    MinionInfoMap::iterator itr = minions.find(minion->GetEntry());
    if (itr == minions.end())
        return;

    if (add)
        itr->second.bossInfo->minion.insert(minion);
    else
        itr->second.bossInfo->minion.erase(minion);
}

bool InstanceScript::SetBossState(uint32 id, EncounterState state)
{
    if (id < bosses.size())
    {
        BossInfo *bossInfo = &bosses[id];
        if (bossInfo->state == TO_BE_DECIDED) // loading
        {
            bossInfo->state = state;
            //sLog->outError("Inialize boss %u state as %u.", id, (uint32)state);
            return false;
        }
        else
        {
            if (bossInfo->state == state)
                return false;
            bossInfo->state = state;
            SaveToDB();
        }

        for (uint32 type = 0; type < MAX_DOOR_TYPES; ++type)
            for (DoorSet::iterator i = bossInfo->door[type].begin(); i != bossInfo->door[type].end(); ++i)
                UpdateDoorState(*i);

        for (MinionSet::iterator i = bossInfo->minion.begin(); i != bossInfo->minion.end(); ++i)
            UpdateMinionState(*i, state);

        return true;
    }
    return false;
}

std::string InstanceScript::LoadBossState(const char * data)
{
    if (!data)
        return NULL;
    std::istringstream loadStream(data);
    uint32 buff;
    uint32 bossId = 0;
    for (std::vector<BossInfo>::iterator i = bosses.begin(); i != bosses.end(); ++i, ++bossId)
    {
        loadStream >> buff;
        if (buff < TO_BE_DECIDED)
            SetBossState(bossId, (EncounterState)buff);
    }
    return loadStream.str();
}

std::string InstanceScript::GetBossSaveData()
{
    std::ostringstream saveStream;
    for (std::vector<BossInfo>::iterator i = bosses.begin(); i != bosses.end(); ++i)
        saveStream << (uint32)i->state << " ";
    return saveStream.str();
}

void InstanceScript::DoUseDoorOrButton(uint64 uiGuid, uint32 uiWithRestoreTime, bool bUseAlternativeState)
{
    if (!uiGuid)
        return;

    GameObject* pGo = instance->GetGameObject(uiGuid);

    if (pGo)
    {
        if (pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR || pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
        {
            if (pGo->getLootState() == GO_READY)
                pGo->UseDoorOrButton(uiWithRestoreTime, bUseAlternativeState);
            else if (pGo->getLootState() == GO_ACTIVATED)
                pGo->ResetDoorOrButton();
        }
        else
            sLog->outError("TSCR: Script call DoUseDoorOrButton, but gameobject entry %u is type %u.", pGo->GetEntry(), pGo->GetGoType());
    }
}

void InstanceScript::DoRespawnGameObject(uint64 uiGuid, uint32 uiTimeToDespawn)
{
    if (GameObject* pGo = instance->GetGameObject(uiGuid))
    {
        //not expect any of these should ever be handled
        if (pGo->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE || pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR ||
            pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON || pGo->GetGoType() == GAMEOBJECT_TYPE_TRAP)
            return;

        if (pGo->isSpawned())
            return;

        pGo->SetRespawnTime(uiTimeToDespawn);
    }
}

