/*
 * Copyright (C) 2010-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2010-2012 Oregon <http://www.oregoncore.com/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
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

#include "Player.h"
#include "Battleground.h"
#include "BattlegroundAA.h"
#include "Language.h"

BattleGroundAA::BattleGroundAA()
{
    m_StartDelayTimes[BG_STARTING_EVENT_FIRST]  = BG_START_DELAY_1M;
    m_StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_30S;
    m_StartDelayTimes[BG_STARTING_EVENT_THIRD]  = BG_START_DELAY_15S;
    m_StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;
    //we must set messageIds
    m_StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_ARENA_ONE_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_ARENA_THIRTY_SECONDS;
    m_StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_ARENA_FIFTEEN_SECONDS;
    m_StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_ARENA_HAS_BEGUN;
}

BattleGroundAA::~BattleGroundAA()
{
}

void BattleGroundAA::Update(time_t diff)
{
    BattleGround::Update(diff);
}

void BattleGroundAA::StartingEventCloseDoors()
{
}

void BattleGroundAA::StartingEventOpenDoors()
{
}

void BattleGroundAA::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundAAScore* sc = new BattleGroundAAScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundAA::RemovePlayer(Player * /*plr*/, uint64 /*guid*/)
{
}

void BattleGroundAA::HandleKillPlayer(Player* player, Player* killer)
{
    BattleGround::HandleKillPlayer(player, killer);
}

void BattleGroundAA::HandleAreaTrigger(Player * /*Source*/, uint32 /*Trigger*/)
{
}

bool BattleGroundAA::SetupBattleGround()
{
    return true;
}

