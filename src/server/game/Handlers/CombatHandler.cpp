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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectGuid.h"

void WorldSession::HandleAttackSwingOpcode(WorldPacket & recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;

    sLog->outDebug("WORLD: Recvd CMSG_ATTACKSWING Message %s", guid.GetString().c_str());

    Unit *pEnemy = ObjectAccessor::GetUnit(*_player, guid.GetRawValue());

    if (!pEnemy)
    {
        if (!guid.IsUnit())
            sLog->outError("WORLD: %s isn't player, pet or creature", guid.GetString().c_str());
        else
            sLog->outError("WORLD: Enemy %s not found", guid.GetString().c_str());

        // stop attack state at client
        SendAttackStop(NULL);
        return;
    }

    if (!_player->canAttack(pEnemy))
    {
        sLog->outError("WORLD: Enemy %s is friendly", guid.GetString().c_str());

        // stop attack state at client
        SendAttackStop(pEnemy);
        return;
    }

    _player->Attack(pEnemy, true);
}

void WorldSession::HandleAttackStopOpcode(WorldPacket & /*recv_data*/)
{
    GetPlayer()->AttackStop();
}

void WorldSession::HandleSetSheathedOpcode(WorldPacket & recv_data)
{
    uint32 sheathed;
    recv_data >> sheathed;

    if (sheathed >= MAX_SHEATH_STATE)
    {
        sLog->outError("Unknown sheath state %u ??", sheathed);
        return;
    }

    GetPlayer()->SetSheath(SheathState(sheathed));
}

void WorldSession::SendAttackStop(Unit const* enemy)
{
    WorldPacket data(SMSG_ATTACKSTOP, (4+20));              // we guess size
    data << GetPlayer()->GetPackGUID();
    data << (enemy ? enemy->GetPackGUID() : PackedGuid());  // must be packed guid
    data << uint32(0);                                      // unk, can be 1 also
    SendPacket(&data);
}

