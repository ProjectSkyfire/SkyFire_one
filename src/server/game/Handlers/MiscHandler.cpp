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
#include "Language.h"
#include "DatabaseEnv.h"
#include "DatabaseImpl.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "GossipDef.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "BigNumber.h"
#include "SHA1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "zlib.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "Battleground.h"
#include "OutdoorPvP.h"
#include "SpellAuras.h"
#include "Pet.h"
#include "SocialMgr.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "ScriptMgr.h"

void WorldSession::HandleRepopRequestOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Recvd CMSG_REPOP_REQUEST Message");

    recv_data.read_skip<uint8>();

    if (GetPlayer()->isAlive()||GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if (GetPlayer()->getDeathState() == JUST_DIED)
    {
        sLog->outDebug("HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        GetPlayer()->KillPlayer();
    }

    //this is spirit release confirm?
    GetPlayer()->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true);
    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 gossipListId;
    uint32 menuId;
    uint64 guid;
    std::string code = "";

    recv_data >> guid >> menuId >> gossipListId;

    if (_player->PlayerTalkClass->GossipOptionCoded(gossipListId))
    {
        // recheck
        sLog->outBasic("reading string");
        recv_data >> code;
        sLog->outBasic("string read: %s", code.c_str());
    }

    Creature *unit = NULL;
    GameObject *go = NULL;
    if (IS_CREATURE_GUID(guid))
    {
        unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
        if (!unit)
        {
            sLog->outDebug("WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else if (IS_GAMEOBJECT_GUID(guid))
    {
        go = _player->GetMap()->GetGameObject(guid);
        if (!go)
        {
            sLog->outDebug("WORLD: HandleGossipSelectOptionOpcode - GameObject (GUID: %u) not found.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else
    {
        sLog->outDebug("WORLD: HandleGossipSelectOptionOpcode - unsupported GUID type for highguid %u. lowpart %u.", uint32(GUID_HIPART(guid)), uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    if (!code.empty())
    {
        if (unit)
        {
            if (!sScriptMgr->GossipSelectWithCode(_player, unit, _player->PlayerTalkClass->GossipOptionSender(gossipListId), _player->PlayerTalkClass->GossipOptionAction(gossipListId), code.c_str()))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
            sScriptMgr->GOSelectWithCode(_player, go, _player->PlayerTalkClass->GossipOptionSender(gossipListId), _player->PlayerTalkClass->GossipOptionAction(gossipListId), code.c_str());
    }
    else
    {
        if (unit)
        {
            if (!sScriptMgr->GossipSelect(_player, unit, _player->PlayerTalkClass->GossipOptionSender(gossipListId), _player->PlayerTalkClass->GossipOptionAction(gossipListId)))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
            sScriptMgr->GOSelect(_player, go, _player->PlayerTalkClass->GossipOptionSender(gossipListId), _player->PlayerTalkClass->GossipOptionAction(gossipListId));
    }
}

void WorldSession::HandleWhoOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Recvd CMSG_WHO Message");
    //recv_data.hexlike();

    uint32 clientcount = 0;

    uint32 level_min, level_max, racemask, classmask, zones_count, str_count;
    uint32 zoneids[10];                                     // 10 is client limit
    std::string player_name, guild_name;

    recv_data >> level_min;                                 // maximal player level, default 0
    recv_data >> level_max;                                 // minimal player level, default 100 (MAX_LEVEL)
    recv_data >> player_name;                               // player name, case sensitive...

    recv_data >> guild_name;                                // guild name, case sensitive...

    recv_data >> racemask;                                  // race mask
    recv_data >> classmask;                                 // class mask
    recv_data >> zones_count;                               // zones count, client limit = 10 (2.0.10)

    if (zones_count > 10)
        return;                                             // can't be received from real client or broken packet

    for (uint32 i = 0; i < zones_count; ++i)
    {
        uint32 temp;
        recv_data >> temp;                                  // zone id, 0 if zone is unknown...
        zoneids[i] = temp;
        sLog->outDebug("Zone %u: %u", i, zoneids[i]);
    }

    recv_data >> str_count;                                 // user entered strings count, client limit=4 (checked on 2.0.10)

    if (str_count > 4)
        return;                                             // can't be received from real client or broken packet

    sLog->outDebug("Minlvl %u, maxlvl %u, name %s, guild %s, racemask %u, classmask %u, zones %u, strings %u", level_min, level_max, player_name.c_str(), guild_name.c_str(), racemask, classmask, zones_count, str_count);

    std::wstring str[4];                                    // 4 is client limit
    for (uint32 i = 0; i < str_count; ++i)
    {
        std::string temp;
        recv_data >> temp;                                  // user entered string, it used as universal search pattern(guild+player name)?

        if (!Utf8toWStr(temp, str[i]))
            continue;

        wstrToLower(str[i]);

        sLog->outDebug("String %u: %s", i, temp.c_str());
    }

    std::wstring wplayer_name;
    std::wstring wguild_name;
    if (!(Utf8toWStr(player_name, wplayer_name) && Utf8toWStr(guild_name, wguild_name)))
        return;
    wstrToLower(wplayer_name);
    wstrToLower(wguild_name);

    // client send in case not set max level value 100 but Trinity supports 255 max level,
    // update it to show GMs with characters after 100 level
    if (level_max >= MAX_LEVEL)
        level_max = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    uint32 security = GetSecurity();
    bool allowTwoSideWhoList = sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    bool gmInWhoList         = sWorld->getConfig(CONFIG_GM_IN_WHO_LIST);

    WorldPacket data(SMSG_WHO, 50);                         // guess size
    data << uint32(clientcount);                            // clientcount place holder, listed count
    data << uint32(clientcount);                            // clientcount place holder, online count

    ACE_GUARD(ACE_Thread_Mutex, g, *HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (security == SEC_PLAYER)
        {
            // player can see member of other team only if CONFIG_ALLOW_TWO_SIDE_WHO_LIST
            if (itr->second->GetTeam() != team && !allowTwoSideWhoList)
                continue;

            // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
            if ((itr->second->GetSession()->GetSecurity() > SEC_PLAYER && !gmInWhoList))
                continue;
        }

        //do not process players which are not in world
        if (!(itr->second->IsInWorld()))
            continue;

        // check if target is globally visible for player
        if (!(itr->second->IsVisibleGloballyFor(_player)))
            continue;

        // check if target's level is in level range
        uint32 lvl = itr->second->getLevel();
        if (lvl < level_min || lvl > level_max)
            continue;

        // check if class matches classmask
        uint32 class_ = itr->second->getClass();
        if (!(classmask & (1 << class_)))
            continue;

        // check if race matches racemask
        uint32 race = itr->second->getRace();
        if (!(racemask & (1 << race)))
            continue;

        uint32 pzoneid = itr->second->GetZoneId();
        uint8 gender = itr->second->getGender();

        bool z_show = true;
        for (uint32 i = 0; i < zones_count; ++i)
        {
            if (zoneids[i] == pzoneid)
            {
                z_show = true;
                break;
            }

            z_show = false;
        }
        if (!z_show)
            continue;

        std::string pname = itr->second->GetName();
        std::wstring wpname;
        if (!Utf8toWStr(pname, wpname))
            continue;
        wstrToLower(wpname);

        if (!(wplayer_name.empty() || wpname.find(wplayer_name) != std::wstring::npos))
            continue;

        std::string gname = sObjectMgr->GetGuildNameById(itr->second->GetGuildId());
        std::wstring wgname;
        if (!Utf8toWStr(gname, wgname))
            continue;
        wstrToLower(wgname);

        if (!(wguild_name.empty() || wgname.find(wguild_name) != std::wstring::npos))
            continue;

        std::string aname;
        if (AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(itr->second->GetZoneId()))
            aname = areaEntry->area_name[GetSessionDbcLocale()];

        bool s_show = true;
        for (uint32 i = 0; i < str_count; ++i)
        {
            if (!str[i].empty())
            {
                if (wgname.find(str[i]) != std::wstring::npos ||
                    wpname.find(str[i]) != std::wstring::npos ||
                    Utf8FitTo(aname, str[i]))
                {
                    s_show = true;
                    break;
                }
                s_show = false;
            }
        }
        if (!s_show)
            continue;

        data << pname;                                      // player name
        data << gname;                                      // guild name
        data << uint32(lvl);                                // player level
        data << uint32(class_);                             // player class
        data << uint32(race);                               // player race
        data << uint8(gender);                              // player gender
        data << uint32(pzoneid);                            // player zone id

        // 49 is maximum player count sent to client - can be overridden
        // through config, but is unstable
        if ((++clientcount) == sWorld->getConfig(CONFIG_MAX_WHO))
            break;
    }

    data.put(0, clientcount);                // insert right count, listed count
    data.put(4, clientcount);                // insert right count, online count

    SendPacket(&data);
    sLog->outDebug("WORLD: Send SMSG_WHO Message");
}

void WorldSession::HandleLogoutRequestOpcode(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: Recvd CMSG_LOGOUT_REQUEST Message, security - %u", GetSecurity());

    if (uint64 lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);

    //Can not logout if...
    if (GetPlayer()->isInCombat() ||                        //...is in combat
        GetPlayer()->duel         ||                        //...is in Duel
        GetPlayer()->HasAura(9454, 0)         ||             //...is frozen by GM via freeze command
                                                            //...is jumping ...is falling
        GetPlayer()->HasUnitMovementFlag(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR))
    {
        WorldPacket data(SMSG_LOGOUT_RESPONSE, (2+4)) ;
        data << (uint8)0xC;
        data << uint32(0);
        data << uint8(0);
        SendPacket(&data);
        LogoutRequest(0);
        return;
    }

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in TrinityCore.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || GetPlayer()->isInFlight() ||
        GetSecurity() >= sWorld->getConfig(CONFIG_INSTANT_LOGOUT))
    {
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, (8+4));    // guess size
        data << GetPlayer()->GetPackGUID();
        data << (uint32)2;
        SendPacket(&data);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    WorldPacket data(SMSG_LOGOUT_RESPONSE, 5);
    data << uint32(0);
    data << uint8(0);
    SendPacket(&data);
    LogoutRequest(time(NULL));
}

void WorldSession::HandlePlayerLogoutOpcode(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: Recvd CMSG_PLAYER_LOGOUT Message");
}

void WorldSession::HandleLogoutCancelOpcode(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: Recvd CMSG_LOGOUT_CANCEL Message");

    LogoutRequest(0);

    WorldPacket data(SMSG_LOGOUT_CANCEL_ACK, 0);
    SendPacket(&data);

    // not remove flags if can't free move - its not set in Logout request code.
    if (GetPlayer()->CanFreeMove())
    {
        //!we can move again
        data.Initialize(SMSG_FORCE_MOVE_UNROOT, 8);       // guess size
        data << GetPlayer()->GetPackGUID();
        data << uint32(0);
        SendPacket(&data);

        //! Stand Up
        GetPlayer()->SetStandState(UNIT_STAND_STATE_STAND);

        //! DISABLE_ROTATE
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    sLog->outDebug("WORLD: sent SMSG_LOGOUT_CANCEL_ACK Message");
}

void WorldSession::HandleTogglePvP(WorldPacket & recv_data)
{
    // this opcode can be used in two ways: Either set explicit new status or toggle old status
    if (recv_data.size() == 1)
    {
        bool newPvPStatus;
        recv_data >> newPvPStatus;
        GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, newPvPStatus);
    }
    else
    {
        GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
    }

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
    {
        if (!GetPlayer()->IsPvP() || GetPlayer()->pvpInfo.endTimer != 0)
            GetPlayer()->UpdatePvP(true, true);
    }
    else
    {
        if (!GetPlayer()->pvpInfo.inHostileArea && GetPlayer()->IsPvP())
            GetPlayer()->pvpInfo.endTimer = time(NULL);     // start toggle-off
    }
}

void WorldSession::HandleZoneUpdateOpcode(WorldPacket & recv_data)
{
    uint32 newZone;
    recv_data >> newZone;

    sLog->outDebug("WORLD: Recvd ZONE_UPDATE: %u", newZone);

    GetPlayer()->UpdateZone(newZone);

    GetPlayer()->SendInitWorldStates(true, newZone);
}

void WorldSession::HandleSetTargetOpcode(WorldPacket & recv_data)
{
    // When this packet send?
    uint64 guid ;
    recv_data >> guid;

    _player->SetUInt32Value(UNIT_FIELD_TARGET, guid);

    // update reputation list if need
    Unit* unit = ObjectAccessor::GetUnit(*_player, guid);
    if (!unit)
        return;

    _player->SetFactionVisibleForFactionTemplateId(unit->getFaction());
}

void WorldSession::HandleSetSelectionOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    recv_data >> guid;

    _player->SetSelection(guid);

    // update reputation list if need
    Unit* unit = ObjectAccessor::GetUnit(*_player, guid);
    if (!unit)
        return;

    _player->SetFactionVisibleForFactionTemplateId(unit->getFaction());
}

void WorldSession::HandleStandStateChangeOpcode(WorldPacket & recv_data)
{
    // sLog->outDebug("WORLD: Received CMSG_STANDSTATECHANGE"); -- too many spam in log at lags/debug stop
    uint32 animstate;
    recv_data >> animstate;

    _player->SetStandState(animstate);
}

void WorldSession::HandleFriendListOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_CONTACT_LIST");
    uint32 unk;
    recv_data >> unk;
    sLog->outDebug("unk value is %u", unk);
    _player->GetSocial()->SendSocialList();
}

void WorldSession::HandleAddFriendOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_ADD_FRIEND");

    std::string friendName = GetSkyFireString(LANG_FRIEND_IGNORE_UNKNOWN);
    std::string friendNote;

    recv_data >> friendName;

    recv_data >> friendNote;

    if (!normalizePlayerName(friendName))
        return;

    CharacterDatabase.EscapeString(friendName);            // prevent SQL injection - normal name don't must changed by this call

    sLog->outDebug("WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName(), friendName.c_str());

    CharacterDatabase.AsyncPQuery(&WorldSession::HandleAddFriendOpcodeCallBack, GetAccountId(), friendNote, "SELECT guid, race, account FROM characters WHERE name = '%s'", friendName.c_str());
}

void WorldSession::HandleAddFriendOpcodeCallBack(QueryResult_AutoPtr result, uint32 accountId, std::string friendNote)
{
    uint64 friendGuid;
    uint64 friendAcctid;
    uint32 team;
    FriendsResult friendResult;

    WorldSession * session = sWorld->FindSession(accountId);

    if (!session || !session->GetPlayer())
        return;

    friendResult = FRIEND_NOT_FOUND;
    friendGuid = 0;

    if (result)
    {
        friendGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        team = Player::TeamForRace((*result)[1].GetUInt8());
        friendAcctid = (*result)[2].GetUInt32();

        if (session->GetSecurity() >= SEC_MODERATOR || sWorld->getConfig(CONFIG_ALLOW_GM_FRIEND) || sAccountMgr->GetSecurity(friendAcctid) < SEC_MODERATOR)
        {
            if (friendGuid)
            {
                if (friendGuid == session->GetPlayer()->GetGUID())
                    friendResult = FRIEND_SELF;
                else if (session->GetPlayer()->GetTeam() != team && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && session->GetSecurity() < SEC_MODERATOR)
                    friendResult = FRIEND_ENEMY;
                else if (session->GetPlayer()->GetSocial()->HasFriend(GUID_LOPART(friendGuid)))
                    friendResult = FRIEND_ALREADY;
                else
                {
                    Player* pFriend = ObjectAccessor::FindPlayer(friendGuid);
                    if (pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyFor(session->GetPlayer()))
                        friendResult = FRIEND_ADDED_ONLINE;
                    else
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if (!session->GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(friendGuid), false))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        sLog->outDebug("WORLD: %s's friend list is full.", session->GetPlayer()->GetName());
                    }
                }
                session->GetPlayer()->GetSocial()->SetFriendNote(GUID_LOPART(friendGuid), friendNote);
            }
        }
    }

    sSocialMgr->SendFriendStatus(session->GetPlayer(), friendResult, GUID_LOPART(friendGuid), false);

    sLog->outDebug("WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelFriendOpcode(WorldPacket & recv_data)
{
    uint64 FriendGUID;

    sLog->outDebug("WORLD: Received CMSG_DEL_FRIEND");

    recv_data >> FriendGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(FriendGUID), false);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_REMOVED, GUID_LOPART(FriendGUID), false);

    sLog->outDebug("WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleAddIgnoreOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_ADD_IGNORE");

    std::string IgnoreName = GetSkyFireString(LANG_FRIEND_IGNORE_UNKNOWN);

    recv_data >> IgnoreName;

    if (!normalizePlayerName(IgnoreName))
        return;

    CharacterDatabase.EscapeString(IgnoreName);            // prevent SQL injection - normal name don't must changed by this call

    sLog->outDebug("WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName(), IgnoreName.c_str());

    CharacterDatabase.AsyncPQuery(&WorldSession::HandleAddIgnoreOpcodeCallBack, GetAccountId(), "SELECT guid FROM characters WHERE name = '%s'", IgnoreName.c_str());
}

void WorldSession::HandleAddIgnoreOpcodeCallBack(QueryResult_AutoPtr result, uint32 accountId)
{
    uint64 IgnoreGuid;
    FriendsResult ignoreResult;

    WorldSession * session = sWorld->FindSession(accountId);

    if (!session || !session->GetPlayer())
        return;

    ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    IgnoreGuid = 0;

    if (result)
    {
        IgnoreGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);

        if (IgnoreGuid)
        {
            if (IgnoreGuid == session->GetPlayer()->GetGUID())              //not add yourself
                ignoreResult = FRIEND_IGNORE_SELF;
            else if (session->GetPlayer()->GetSocial()->HasIgnore(GUID_LOPART(IgnoreGuid)))
                ignoreResult = FRIEND_IGNORE_ALREADY;
            else
            {
                ignoreResult = FRIEND_IGNORE_ADDED;

                // ignore list full
                if (!session->GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(IgnoreGuid), true))
                    ignoreResult = FRIEND_IGNORE_FULL;
            }
        }
    }

    sSocialMgr->SendFriendStatus(session->GetPlayer(), ignoreResult, GUID_LOPART(IgnoreGuid), false);

    sLog->outDebug("WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelIgnoreOpcode(WorldPacket & recv_data)
{
    uint64 IgnoreGUID;

    sLog->outDebug("WORLD: Received CMSG_DEL_IGNORE");

    recv_data >> IgnoreGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(IgnoreGUID), true);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_IGNORE_REMOVED, GUID_LOPART(IgnoreGUID), false);

    sLog->outDebug("WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleSetFriendNoteOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    std::string note;
    recv_data >> guid >> note;
    _player->GetSocial()->SetFriendNote(guid, note);
}

void WorldSession::HandleBugOpcode(WorldPacket & recv_data)
{
    uint32 suggestion, contentlen, typelen;
    std::string content, type;

    recv_data >> suggestion >> contentlen >> content;
    recv_data >> typelen >> type;

    if (suggestion == 0)
        sLog->outDebug("WORLD: Received CMSG_BUG [Bug Report] \n %s \n %s", type.c_str(), content.c_str());
    else
        sLog->outDebug("WORLD: Received CMSG_BUG [Suggestion]");

    CharacterDatabase.EscapeString(type);
    CharacterDatabase.EscapeString(content);
    CharacterDatabase.PExecute("INSERT INTO bugreport (type, content) VALUES('%s', '%s')", type.c_str(), content.c_str());
}

void WorldSession::HandleCorpseReclaimOpcode(WorldPacket &recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_RECLAIM_CORPSE");

    uint64 guid;
    recv_data >> guid;

    if (GetPlayer()->isAlive())
        return;

    // do not allow corpse reclaim in arena
    if (GetPlayer()->InArena())
        return;

    // body not released yet
    if (!GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    Corpse *corpse = GetPlayer()->GetCorpse();

    if (!corpse)
        return;

    // prevent resurrect before 30-sec delay after body release not finished
    if (corpse->GetGhostTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP) > time(NULL))
        return;

    if (!corpse->IsWithinDistInMap(GetPlayer(), CORPSE_RECLAIM_RADIUS, true))
        return;

    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleGround() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_RESURRECT_RESPONSE");

    uint64 guid;
    uint8 status;
    recv_data >> guid;
    recv_data >> status;

    if (GetPlayer()->isAlive())
        return;

    if (status == 0)
    {
        GetPlayer()->clearResurrectRequestData();           // reject
        return;
    }

    if (!GetPlayer()->isRessurectRequestedBy(guid))
        return;

    GetPlayer()->ResurectUsingRequestData();
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_AREATRIGGER");

    uint32 Trigger_ID;

    recv_data >> Trigger_ID;
    sLog->outDebug("Trigger ID:%u", Trigger_ID);

    if (GetPlayer()->isInFlight())
    {
        sLog->outDebug("Player '%s' (GUID: %u) in flight, ignore Area Trigger ID:%u", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow(), Trigger_ID);
        return;
    }

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
    if (!atEntry)
    {
        sLog->outDebug("Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID:%u", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow(), Trigger_ID);
        return;
    }

    if (GetPlayer()->GetMapId() != atEntry->mapid)
    {
        sLog->outDebug("Player '%s' (GUID: %u) too far (trigger map: %u player map: %u), ignore Area Trigger ID: %u", GetPlayer()->GetName(), atEntry->mapid, GetPlayer()->GetMapId(), GetPlayer()->GetGUIDLow(), Trigger_ID);
        return;
    }

    // delta is safe radius
    const float delta = 5.0f;
    // check if player in the range of areatrigger
    Player* pl = GetPlayer();

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist = pl->GetDistance(atEntry->x, atEntry->y, atEntry->z);
        if (dist > atEntry->radius + delta)
        {
            sLog->outDebug("Player '%s' (GUID: %u) too far (radius: %f distance: %f), ignore Area Trigger ID: %u",
                pl->GetName(), pl->GetGUIDLow(), atEntry->radius, dist, Trigger_ID);
            return;
        }
    }
    else
    {
        // we have only extent
        float dx = pl->GetPositionX() - atEntry->x;
        float dy = pl->GetPositionY() - atEntry->y;
        float dz = pl->GetPositionZ() - atEntry->z;
        double es = sin(atEntry->box_orientation);
        double ec = cos(atEntry->box_orientation);
        // calc rotated vector based on extent axis
        double rotateDx = dx*ec - dy*es;
        double rotateDy = dx*es + dy*ec;

        if ((fabs(rotateDx) > atEntry->box_x/2 + delta) ||
            (fabs(rotateDy) > atEntry->box_y/2 + delta) ||
            (fabs(dz) > atEntry->box_z/2 + delta))
        {
            sLog->outDebug("Player '%s' (GUID: %u) too far (1/2 box X: %f 1/2 box Y: %f 1/2 box Z: %f rotate dX: %f rotate dY: %f dZ:%f), ignore Area Trigger ID: %u",
                pl->GetName(), pl->GetGUIDLow(), atEntry->box_x/2, atEntry->box_y/2, atEntry->box_z/2, rotateDx, rotateDy, dz, Trigger_ID);
            return;
        }
    }

    if (sScriptMgr->AreaTrigger(GetPlayer(), atEntry))
        return;

    uint32 quest_id = sObjectMgr->GetQuestForAreaTrigger(Trigger_ID);
    if (quest_id && GetPlayer()->isAlive() && GetPlayer()->IsActiveQuest(quest_id))
    {
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(quest_id);
        if (pQuest)
        {
            if (GetPlayer()->GetQuestStatus(quest_id) == QUEST_STATUS_INCOMPLETE)
                GetPlayer()->AreaExploredOrEventHappens(quest_id);
        }
    }

    if (sObjectMgr->IsTavernAreaTrigger(Trigger_ID))
    {
        // set resting flag we are in the inn
        GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        GetPlayer()->InnEnter(time(NULL), atEntry->mapid, atEntry->x, atEntry->y, atEntry->z);
        GetPlayer()->SetRestType(REST_TYPE_IN_TAVERN);

        if (sWorld->IsFFAPvPRealm())
            GetPlayer()->SetFFAPvP(false);

        return;
    }

    if (GetPlayer()->InBattleGround())
    {
        BattleGround* bg = GetPlayer()->GetBattleGround();
        if (bg)
            if (bg->GetStatus() == STATUS_IN_PROGRESS)
                bg->HandleAreaTrigger(GetPlayer(), Trigger_ID);

        return;
    }

    if (OutdoorPvP * pvp = GetPlayer()->GetOutdoorPvP())
    {
        if (pvp->HandleAreaTrigger(_player, Trigger_ID))
            return;
    }

    // NULL if all values default (non teleport trigger)
    AreaTrigger const* at = sObjectMgr->GetAreaTrigger(Trigger_ID);
    if (!at)
        return;

    if (!GetPlayer()->Satisfy(sObjectMgr->GetAccessRequirement(at->access_id), at->target_mapId, true))
        return;

    GetPlayer()->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation, TELE_TO_NOT_LEAVE_TRANSPORT);
}

void WorldSession::HandleUpdateAccountData(WorldPacket &recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_UPDATE_ACCOUNT_DATA");
    // not yet implemented. Needs to be backported from TC2
    recv_data.rpos(recv_data.wpos());                       // prevent warnings spam
}

void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");
    // not yet implemented. Needs to be backported from TC2
    recv_data.rpos(recv_data.wpos());                       // prevent warnings spam
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recv_data)
{
    sLog->outDebug("WORLD: Received CMSG_SET_ACTION_BUTTON");
    uint8 button, misc, type;
    uint16 action;
    recv_data >> button >> action >> misc >> type;
    sLog->outDebug("BUTTON: %u ACTION: %u TYPE: %u MISC: %u", button, action, type, misc);
    if (action == 0)
    {
        sLog->outDebug("MISC: Remove action from button %u", button);
        GetPlayer()->removeActionButton(button);
    }
    else
    {
        if (type == ACTION_BUTTON_MACRO || type == ACTION_BUTTON_CMACRO)
        {
            sLog->outDebug("MISC: Added Macro %u into button %u", action, button);
            GetPlayer()->addActionButton(button, action, type, misc);
        }
        else if (type == ACTION_BUTTON_SPELL)
        {
            sLog->outDebug("MISC: Added Action %u into button %u", action, button);
            GetPlayer()->addActionButton(button, action, type, misc);
        }
        else if (type == ACTION_BUTTON_ITEM)
        {
            sLog->outDebug("MISC: Added Item %u into button %u", action, button);
            GetPlayer()->addActionButton(button, action, type, misc);
        }
        else
            sLog->outError("MISC: Unknown action button type %u for action %u into button %u", type, action, button);
    }
}

void WorldSession::HandleCompleteCinema(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: Player is watching cinema");
}

void WorldSession::HandleNextCinematicCamera(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: Which movie to play");
}

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket & recv_data)
{
    /*  WorldSession::Update(getMSTime());*/
    sLog->outDebug("WORLD: Time Lag/Synchronization Resent/Update");

    recv_data.read_skip<uint64>();
    recv_data.read_skip<uint32>();
    /*
        uint64 guid;
        uint32 time_skipped;
        recv_data >> guid;
        recv_data >> time_skipped;
        sLog->outDebug("WORLD: CMSG_MOVE_TIME_SKIPPED");

        // TODO
        must be need use in Trinity
        We substract server Lags to move time (AntiLags)
        for exmaple
        GetPlayer()->ModifyLastMoveTime(-int32(time_skipped));
    */
}

void WorldSession::HandleFeatherFallAck(WorldPacket &recv_data)
{
    sLog->outDebug("WORLD: CMSG_MOVE_FEATHER_FALL_ACK");

    // no used
    recv_data.rpos(recv_data.wpos());                       // prevent warnings spam
}

void WorldSession::HandleMoveUnRootAck(WorldPacket& recv_data)
{
    // no used
    recv_data.rpos(recv_data.wpos());                       // prevent warnings spam

    /*
        recv_data.hexlike();

        recv_data >> guid;
        recv_data >> unknown1;
        recv_data >> unknown2;
        recv_data >> PositionX;
        recv_data >> PositionY;
        recv_data >> PositionZ;
        recv_data >> Orientation;
    */
}

void WorldSession::HandleMoveRootAck(WorldPacket& recv_data)
{
    // no used
    recv_data.rpos(recv_data.wpos());                       // prevent warnings spam

    /*
        sLog->outDebug("WORLD: CMSG_FORCE_MOVE_ROOT_ACK");

        recv_data >> guid;
        recv_data >> unknown1;
        recv_data >> unknown2;
        recv_data >> PositionX;
        recv_data >> PositionY;
        recv_data >> PositionZ;
        recv_data >> Orientation;
    */
}

void WorldSession::HandleSetActionBar(WorldPacket& recv_data)
{
    uint8 ActionBar;
    recv_data >> ActionBar;

    if (!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if (ActionBar != 0)
            sLog->outError("WorldSession::HandleSetActionBar in not logged state with value: %u, ignored", uint32(ActionBar));
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, 2, ActionBar);
}

void WorldSession::HandlePlayedTime(WorldPacket& /*recv_data*/)
{
    WorldPacket data(SMSG_PLAYED_TIME, 4 + 4);
    data << uint32(_player->GetTotalPlayedTime());
    data << uint32(_player->GetLevelPlayedTime());
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode(WorldPacket& recv_data)
{
    uint64 guid;
    recv_data >> guid;
    sLog->outDebug("Inspected guid is (GUID: %u TypeId: %u)", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));

    _player->SetSelection(guid);

    Player *plr = sObjectMgr->GetPlayer(guid);
    if (!plr)                                                // wrong player
        return;

    uint32 talent_points = 0x3D;
    uint32 guid_size = plr->GetPackGUID().size();
    WorldPacket data(SMSG_INSPECT_TALENT, 4+talent_points);
    data << plr->GetPackGUID();
    data << uint32(talent_points);

    // fill by 0 talents array
    for (uint32 i = 0; i < talent_points; ++i)
        data << uint8(0);

    if (sWorld->getConfig(CONFIG_TALENTS_INSPECTING) || _player->isGameMaster())
    {
        // find class talent tabs (all players have 3 talent tabs)
        uint32 const* talentTabIds = GetTalentTabPages(plr->getClass());

        uint32 talentTabPos = 0;                            // pos of first talent rank in tab including all prev tabs
        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 talentTabId = talentTabIds[i];

            // fill by real data
            for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
            {
                TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);
                if (!talentInfo)
                    continue;

                // skip another tab talents
                if (talentInfo->TalentTab != talentTabId)
                    continue;

                // find talent rank
                uint32 curtalent_maxrank = 0;
                for (uint32 k = 5; k > 0; --k)
                {
                    if (talentInfo->RankID[k-1] && plr->HasSpell(talentInfo->RankID[k-1]))
                    {
                        curtalent_maxrank = k;
                        break;
                    }
                }

                // not learned talent
                if (!curtalent_maxrank)
                    continue;

                // 1 rank talent bit index
                uint32 curtalent_index = talentTabPos + GetTalentInspectBitPosInTab(talentId);

                uint32 curtalent_rank_index = curtalent_index+curtalent_maxrank-1;

                // slot/offset in 7-bit bytes
                uint32 curtalent_rank_slot7   = curtalent_rank_index / 7;
                uint32 curtalent_rank_offset7 = curtalent_rank_index % 7;

                // rank pos with skipped 8 bit
                uint32 curtalent_rank_index2 = curtalent_rank_slot7 * 8 + curtalent_rank_offset7;

                // slot/offset in 8-bit bytes with skipped high bit
                uint32 curtalent_rank_slot = curtalent_rank_index2 / 8;
                uint32 curtalent_rank_offset =  curtalent_rank_index2 % 8;

                // apply mask
                uint32 val = data.read<uint8>(guid_size + 4 + curtalent_rank_slot);
                val |= (1 << curtalent_rank_offset);
                data.put<uint8>(guid_size + 4 + curtalent_rank_slot, val & 0xFF);
            }

            talentTabPos += GetTalentTabInspectBitSize(talentTabId);
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recv_data)
{
    uint64 guid;
    recv_data >> guid;

    Player* player = sObjectMgr->GetPlayer(guid);

    if (!player)
    {
        sLog->outError("InspectHonorStats: WTF, player not found...");
        return;
    }

    WorldPacket data(MSG_INSPECT_HONOR_STATS, 8+1+4*4);
    data << uint64(player->GetGUID());
    data << uint8(player->GetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_KILLS));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
    SendPacket(&data);
}

void WorldSession::HandleWorldTeleportOpcode(WorldPacket& recv_data)
{
    // write in client console: worldport 469 452 6454 2536 180 or /console worldport 469 452 6454 2536 180
    // Received opcode CMSG_WORLD_TELEPORT
    // Time is ***, map=469, x=452.000000, y=6454.000000, z=2536.000000, orient=3.141593

    uint32 time;
    uint32 mapid;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recv_data >> time;                                      // time in m.sec.
    recv_data >> mapid;
    recv_data >> PositionX;
    recv_data >> PositionY;
    recv_data >> PositionZ;
    recv_data >> Orientation;                               // o (3.141593 = 180 degrees)

    sLog->outDebug("Received opcode CMSG_WORLD_TELEPORT");

    if (GetPlayer()->isInFlight())
    {
        sLog->outDebug("Player '%s' (GUID: %u) in flight, ignore worldport command.", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        return;
    }

    sLog->outDebug("Time %u sec, map=%u, x=%f, y=%f, z=%f, orient=%f", time/IN_MILLISECONDS, mapid, PositionX, PositionY, PositionZ, Orientation);

    if (GetSecurity() >= SEC_ADMINISTRATOR)
        GetPlayer()->TeleportTo(mapid, PositionX, PositionY, PositionZ, Orientation);
    else
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);

    sLog->outDebug("Received worldport command from player %s", GetPlayer()->GetName());
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recv_data)
{
    sLog->outDebug("Received opcode CMSG_WHOIS");
    std::string charname;
    recv_data >> charname;

    if (GetSecurity() < SEC_ADMINISTRATOR)
    {
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if (charname.empty() || !normalizePlayerName (charname))
    {
        SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player *plr = sObjectMgr->GetPlayer(charname.c_str());

    if (!plr)
    {
        SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, charname.c_str());
        return;
    }

    uint32 accid = plr->GetSession()->GetAccountId();

    QueryResult_AutoPtr result = LoginDatabase.PQuery("SELECT username, email, last_ip FROM account WHERE id=%u", accid);
    if (!result)
    {
        SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, charname.c_str());
        return;
    }

    Field *fields = result->Fetch();
    std::string acc = fields[0].GetCppString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetCppString();
    if (email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetCppString();
    if (lastip.empty())
        lastip = "Unknown";

    std::string msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

    WorldPacket data(SMSG_WHOIS, msg.size()+1);
    data << msg;
    _player->GetSession()->SendPacket(&data);

    sLog->outDebug("Received whois command from player %s for character %s", GetPlayer()->GetName(), charname.c_str());
}

void WorldSession::HandleReportSpamOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: CMSG_REPORT_SPAM");
    recv_data.hexlike();

    uint8 spam_type;                                        // 0 - mail, 1 - chat
    uint64 spammer_guid;
    uint32 unk1 = 0;
    uint32 unk2 = 0;
    uint32 unk3 = 0;
    uint32 unk4 = 0;
    std::string description = "";
    recv_data >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recv_data >> spammer_guid;                              // player guid
    switch (spam_type)
    {
        case 0:
            recv_data >> unk1;                              // const 0
            recv_data >> unk2;                              // probably mail id
            recv_data >> unk3;                              // const 0
            break;
        case 1:
            recv_data >> unk1;                              // probably language
            recv_data >> unk2;                              // message type?
            recv_data >> unk3;                              // probably channel id
            recv_data >> unk4;                              // unk random value
            recv_data >> description;                       // spam description string (messagetype, channel name, player name, message)
            break;
    }

    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_COMPLAIN_RESULT, 1);
    data << uint8(0);
    SendPacket(&data);

    sLog->outDebug("REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s", spam_type, GUID_LOPART(spammer_guid), unk1, unk2, unk3, unk4, description.c_str());
}

void WorldSession::HandleRealmStateRequestOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("CMSG_REALM_SPLIT");

    uint32 unk;
    std::string split_date = "01/01/01";
    recv_data >> unk;

    WorldPacket data(SMSG_REALM_SPLIT, 4+4+split_date.size()+1);
    data << unk;
    data << uint32(0x00000000);                             // realm split state
    // split states:
    // 0x0 realm normal
    // 0x1 realm split
    // 0x2 realm split pending
    data << split_date;
    SendPacket(&data);
    //sLog->outDebug("response sent %u", unk);
}

void WorldSession::HandleFarSightOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("WORLD: CMSG_FAR_SIGHT");

    uint8 apply;
    recv_data >> apply;

    switch (apply)
    {
        case 0:
            sLog->outDebug("Player %u set vision to self", _player->GetGUIDLow());
            _player->SetSeer(_player);
            break;
        case 1:
            sLog->outDebug("Added FarSight " I64FMT " to player %u", _player->GetUInt64Value(PLAYER_FARSIGHT), _player->GetGUIDLow());
            if (WorldObject *target = _player->GetViewpoint())
                _player->SetSeer(target);
            else
                sLog->outError("Player %s requests invalid seer", _player->GetName());
            break;
        default:
            sLog->outDebug("Unhandled mode in CMSG_FAR_SIGHT: %u", apply);
            return;
    }

    GetPlayer()->UpdateVisibilityForPlayer();
}

void WorldSession::HandleChooseTitleOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("CMSG_SET_TITLE");

    int32 title;
    recv_data >> title;

    // -1 at none
    if (title > 0 && title < MAX_TITLE_INDEX)
    {
        if (!GetPlayer()->HasTitle(title))
            return;
    }
    else
        title = 0;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
}

void WorldSession::HandleTimeSyncResp(WorldPacket & recv_data)
{
    sLog->outDebug("CMSG_TIME_SYNC_RESP");

    uint32 counter, clientTicks;
    recv_data >> counter >> clientTicks;

    if (counter != _player->m_timeSyncCounter - 1)
        sLog->outDebug("Wrong time sync counter from player %s (cheater?)", _player->GetName());

    sLog->outDebug("Time sync received: counter %u, client ticks %u, time since last sync %u", counter, clientTicks, clientTicks - _player->m_timeSyncClient);

    uint32 ourTicks = clientTicks + (getMSTime() - _player->m_timeSyncServer);

    // diff should be small
    sLog->outDebug("Our ticks: %u, diff %u, latency %u", ourTicks, ourTicks - clientTicks, GetLatency());

    _player->m_timeSyncClient = clientTicks;
}

void WorldSession::HandleResetInstancesOpcode(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: CMSG_RESET_INSTANCES");

    if (Group *pGroup = _player->GetGroup())
    {
        if (pGroup->IsLeader(_player->GetGUID()))
            pGroup->ResetInstances(INSTANCE_RESET_ALL, _player);
    }
    else
        _player->ResetInstances(INSTANCE_RESET_ALL);
}

void WorldSession::HandleDungeonDifficultyOpcode(WorldPacket & recv_data)
{
    sLog->outDebug("MSG_SET_DUNGEON_DIFFICULTY");

    uint32 mode;
    recv_data >> mode;

    if (mode == _player->GetDifficulty())
        return;

    if (mode > DIFFICULTY_HEROIC)
    {
        sLog->outDebug("WorldSession::HandleDungeonDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    // cannot reset while in an instance
    Map *map = _player->GetMap();
    if (map && map->IsDungeon())
    {
        sLog->outDebug("WorldSession::HandleDungeonDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
        return;
    }

    if (_player->getLevel() < LEVELREQUIREMENT_HEROIC)
        return;

    Group *pGroup = _player->GetGroup();
    if (pGroup)
    {
        if (pGroup->IsLeader(_player->GetGUID()))
        {
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            pGroup->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, _player);
            pGroup->SetDifficulty(mode);
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY);
        _player->SetDifficulty(mode);
    }
}

void WorldSession::HandleDismountOpcode(WorldPacket & /*recv_data*/)
{
    sLog->outDebug("WORLD: CMSG_CANCEL_MOUNT_AURA");

    //If player is not mounted, so go out :)
    if (!_player->IsMounted())                              // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_CHAR_NON_MOUNTED);
        return;
    }

    if (_player->isInFlight())                               // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_YOU_IN_FLIGHT);
        return;
    }

    _player->Unmount();
    _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
}

void WorldSession::HandleRequestPetInfoOpcode(WorldPacket & /*recv_data */)
{
    /*
        sLog->outDebug("WORLD: CMSG_REQUEST_PET_INFO");
        recv_data.hexlike();
    */
}

void WorldSession::HandleSetTaxiBenchmarkOpcode(WorldPacket & recv_data)
{
    uint8 mode;
    recv_data >> mode;

    sLog->outDebug("Client used \"/timetest %d\" command", mode);
}

