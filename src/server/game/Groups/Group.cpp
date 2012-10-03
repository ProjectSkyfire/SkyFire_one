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
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "Formulas.h"
#include "ObjectAccessor.h"
#include "Battleground.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "MapInstanced.h"
#include "Util.h"

Group::Group()
{
    m_leaderGuid        = 0;
    m_mainTank          = 0;
    m_mainAssistant     = 0;
    m_groupType         = GroupType(0);
    m_bgGroup           = NULL;
    m_lootMethod        = LootMethod(0);
    m_looterGuid        = 0;
    m_lootThreshold     = ITEM_QUALITY_UNCOMMON;
    m_subGroupsCounts   = NULL;

    for (uint8 i = 0; i < TARGETICONCOUNT; ++i)
        m_targetIcons[i] = 0;
}

Group::~Group()
{
    if (m_bgGroup)
    {
        sLog->outDebug("Group::~Group: battleground group being deleted.");
        if (m_bgGroup->GetBgRaid(ALLIANCE) == this) m_bgGroup->SetBgRaid(ALLIANCE, NULL);
        else if (m_bgGroup->GetBgRaid(HORDE) == this) m_bgGroup->SetBgRaid(HORDE, NULL);
        else sLog->outError("Group::~Group: battleground group is not linked to the correct battleground.");
    }
    Rolls::iterator itr;
    while (!RollId.empty())
    {
        itr = RollId.begin();
        Roll *r = *itr;
        RollId.erase(itr);
        delete(r);
    }

    // it is undefined whether objectmgr (which stores the groups) or instancesavemgr
    // will be unloaded first so we must be prepared for both cases
    // this may unload some instance saves
    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
            itr->second.save->RemoveGroup(this);

    // Sub group counters clean up
    if (m_subGroupsCounts)
        delete[] m_subGroupsCounts;
}

bool Group::Create(const uint64 &guid, const char * name)
{
    m_leaderGuid = guid;
    m_leaderName = name;

    m_groupType  = isBGGroup() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;

    if (m_groupType == GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = GROUP_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_looterGuid = guid;

    m_difficulty = DIFFICULTY_NORMAL;
    if (!isBGGroup())
    {
        Player *leader = ObjectAccessor::FindPlayer(guid);
        if (leader)
            m_difficulty = leader->GetDifficulty();

        Player::ConvertInstancesToGroup(leader, this, guid);

        // store group in database
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE leaderGuid ='%u'", GUID_LOPART(m_leaderGuid));
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE leaderGuid ='%u'", GUID_LOPART(m_leaderGuid));
        CharacterDatabase.PExecute("INSERT INTO groups(leaderGuid, mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, difficulty) "
            "VALUES('%u', '%u', '%u', '%u', '%u', '%u', '" UI64FMTD "', '" UI64FMTD "', '" UI64FMTD "', '" UI64FMTD "', '" UI64FMTD "', '" UI64FMTD "', '" UI64FMTD "', '" UI64FMTD "', '%u', '%u')",
            GUID_LOPART(m_leaderGuid), GUID_LOPART(m_mainTank), GUID_LOPART(m_mainAssistant), uint32(m_lootMethod),
            GUID_LOPART(m_looterGuid), uint32(m_lootThreshold), m_targetIcons[0], m_targetIcons[1], m_targetIcons[2], m_targetIcons[3], m_targetIcons[4], m_targetIcons[5], m_targetIcons[6], m_targetIcons[7], isRaidGroup(), m_difficulty);
    }

    if (!AddMember(guid, name))
        return false;

    if (!isBGGroup()) CharacterDatabase.CommitTransaction();

    return true;
}

bool Group::LoadGroupFromDB(const uint64 &leaderGuid, QueryResult_AutoPtr result, bool loadMembers)
{
    if (isBGGroup())
        return false;

    bool external = true;
    if (!result)
    {
        external = false;
        //                                       0          1              2           3           4              5      6      7      8      9      10     11     12     13      14
        result = CharacterDatabase.PQuery("SELECT mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, difficulty FROM groups WHERE leaderGuid ='%u'", GUID_LOPART(leaderGuid));
        if (!result)
            return false;
    }

    m_leaderGuid = leaderGuid;

    // group leader not exist
    if (!sObjectMgr->GetPlayerNameByGUID(m_leaderGuid, m_leaderName))
        return false;

    m_groupType  = (*result)[13].GetBool() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;

    if (m_groupType == GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    m_difficulty = (*result)[14].GetUInt8();
    m_mainTank = (*result)[0].GetUInt64();
    m_mainAssistant = (*result)[1].GetUInt64();
    m_lootMethod = (LootMethod)(*result)[2].GetUInt8();
    m_looterGuid = MAKE_NEW_GUID((*result)[3].GetUInt32(), 0, HIGHGUID_PLAYER);
    m_lootThreshold = (ItemQualities)(*result)[4].GetUInt16();

    for (int i = 0; i < TARGETICONCOUNT; i++)
        m_targetIcons[i] = (*result)[5+i].GetUInt64();

    if (loadMembers)
    {
        result = CharacterDatabase.PQuery("SELECT memberGuid, assistant, subgroup FROM group_member WHERE leaderGuid ='%u'", GUID_LOPART(leaderGuid));
        if (!result)
            return false;

        do
        {
            LoadMemberFromDB((*result)[0].GetUInt32(), (*result)[2].GetUInt8(), (*result)[1].GetBool());
        } while (result->NextRow());

        if (GetMembersCount() < 2)                          // group too small
            return false;
    }

    return true;
}

bool Group::LoadMemberFromDB(uint32 guidLow, uint8 subgroup, bool assistant)
{
    MemberSlot member;
    member.guid = MAKE_NEW_GUID(guidLow, 0, HIGHGUID_PLAYER);

    // skip non-existed member
    if (!sObjectMgr->GetPlayerNameByGUID(member.guid, member.name))
        return false;

    member.group = subgroup;
    member.assistant = assistant;

    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subgroup);

    return true;
}

void Group::ConvertToRaid()
{
    m_groupType = GROUPTYPE_RAID;

    _initRaidSubGroupsCounter();

    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET isRaid = 1 WHERE leaderGuid='%u'", GUID_LOPART(m_leaderGuid));
    SendUpdate();
}

bool Group::AddInvite(Player* player)
{
    if (!player || player->GetGroupInvite())
        return false;
    Group* group = player->GetGroup();
    if (group && group->isBGGroup())
        group = player->GetOriginalGroup();
    if (group)
        return false;

    RemoveInvite(player);

    m_invitees.insert(player);

    player->SetGroupInvite(this);

    return true;
}

bool Group::AddLeaderInvite(Player* player)
{
    if (!AddInvite(player))
        return false;

    m_leaderGuid = player->GetGUID();
    m_leaderName = player->GetName();
    return true;
}

uint32 Group::RemoveInvite(Player* player)
{
    m_invitees.erase(player);

    player->SetGroupInvite(NULL);
    return GetMembersCount();
}

void Group::RemoveAllInvites()
{
    for (InvitesList::iterator itr=m_invitees.begin(); itr != m_invitees.end(); ++itr)
        (*itr)->SetGroupInvite(NULL);

    m_invitees.clear();
}

Player* Group::GetInvited(const uint64& guid) const
{
    for (InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if ((*itr)->GetGUID() == guid)
            return (*itr);
    }
    return NULL;
}

Player* Group::GetInvited(const std::string& name) const
{
    for (InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if ((*itr)->GetName() == name)
            return (*itr);
    }
    return NULL;
}

bool Group::AddMember(const uint64 &guid, const char* name)
{
    if (!_addMember(guid, name))
        return false;
    SendUpdate();

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (player)
    {
        if (!IsLeader(player->GetGUID()) && !isBGGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid/heroic instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN);

            if (player->getLevel() >= LEVELREQUIREMENT_HEROIC && player->GetDifficulty() != GetDifficulty())
            {
                player->SetDifficulty(m_difficulty);
                player->SendDungeonDifficulty(true);
            }
        }
        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        UpdatePlayerOutOfRange(player);
    }

    return true;
}

uint32 Group::RemoveMember(const uint64 &guid, const uint8 &method)
{
    BroadcastGroupUpdate();

    // remove member and change leader (if need) only if strong more 2 members _before_ member remove
    if (GetMembersCount() > (isBGGroup() ? 1 : 2))           // in BG group case allow 1 members group
    {
        bool leaderChanged = _removeMember(guid);

        if (Player* player = ObjectAccessor::FindPlayer(guid))
        {
            WorldPacket data;

            if (method == 1)
            {
                data.Initialize(SMSG_GROUP_UNINVITE, 0);
                player->GetSession()->SendPacket(&data);
            }

            // we already removed player from group and in player->GetGroup() is his original group!
            if (Group* group = player->GetGroup())
                group->SendUpdate();
            else
            {
                data.Initialize(SMSG_GROUP_LIST, 24);
                data << uint64(0) << uint64(0) << uint64(0);
                player->GetSession()->SendPacket(&data);
            }

            _homebindIfInstance(player);
        }

        if (leaderChanged)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_memberSlots.front().name.size()+1));
            data << m_memberSlots.front().name;
            BroadcastPacket(&data, true);
        }

        SendUpdate();
    }
    // if group before remove <= 2 disband it
    else
        Disband(true);

    return m_memberSlots.size();
}

void Group::ChangeLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);

    if (slot == m_memberSlots.end())
        return;

    _setLeader(guid);

    WorldPacket data(SMSG_GROUP_SET_LEADER, slot->name.size()+1);
    data << slot->name;
    BroadcastPacket(&data, true);
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    Player* player;

    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = ObjectAccessor::FindPlayer(citr->guid);
        if (!player)
            continue;

        // we cannot call _removeMember because it would invalidate member iterator
        // if we are removing player from battleground raid
        if (isBGGroup())
            player->RemoveFromBattlegroundRaid();
        else
        {
            // we can remove player who is in battleground from his original group
            if (player->GetOriginalGroup() == this)
                player->SetOriginalGroup(NULL);
            else
                player->SetGroup(NULL);
        }

        if (!player->GetSession())
            continue;

        WorldPacket data;
        if (!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(&data);
        }

        // we already removed player from group and in player->GetGroup() is his original group, send update
        if (Group* group = player->GetGroup())
            group->SendUpdate();
        else
        {
            data.Initialize(SMSG_GROUP_LIST, 24);
            data << uint64(0) << uint64(0) << uint64(0);
            player->GetSession()->SendPacket(&data);
        }

        _homebindIfInstance(player);
    }
    RollId.clear();
    m_memberSlots.clear();

    RemoveAllInvites();

    if (!isBGGroup())
    {
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE leaderGuid='%u'", GUID_LOPART(m_leaderGuid));
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE leaderGuid='%u'", GUID_LOPART(m_leaderGuid));
        CharacterDatabase.CommitTransaction();
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, NULL);
    }

    m_leaderGuid = 0;
    m_leaderName = "";
}

/*********************************************************/
/***                   LOOT SYSTEM                     ***/
/*********************************************************/

void Group::SendLootStartRoll(uint32 CountDown, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_START_ROLL, (8+4+4+4+4+4));
    data << uint64(r.itemGUID);                             // guid of rolled item
    data << uint32(r.totalPlayersRolling);                  // maybe the number of players rolling for it???
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // item random property ID
    data << uint32(CountDown);                              // the countdown time to choose "need" or "greed"

    for (Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = ObjectAccessor::FindPlayer(itr->first);
        if (!p || !p->GetSession())
            continue;

        if (itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket(&data);
    }
}

void Group::SendLootRoll(const uint64& SourceGuid, const uint64& TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL, (8+4+8+4+4+4+1+1+1));
    data << uint64(SourceGuid);                             // guid of the item rolled
    data << uint32(0);                                      // unknown, maybe amount of players
    data << uint64(TargetGuid);
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint8(RollNumber);                              // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << uint8(RollType);                                // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll
    data << uint8(0);                                       // auto pass on NeedBeforeGreed loot because player cannot use the object

    for (Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = ObjectAccessor::FindPlayer(itr->first);
        if (!p || !p->GetSession())
            continue;

        if (itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket(&data);
    }
}

void Group::SendLootRollWon(const uint64& SourceGuid, const uint64& TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL_WON, (8+4+4+4+4+8+1+1));
    data << uint64(SourceGuid);                             // guid of the item rolled
    data << uint32(0);                                      // unknown, maybe amount of players
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // Item random property
    data << uint64(TargetGuid);                             // guid of the player who won.
    data << uint8(RollNumber);                              // rollnumber realted to SMSG_LOOT_ROLL
    data << uint8(RollType);                                // Rolltype related to SMSG_LOOT_ROLL

    for (Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = ObjectAccessor::FindPlayer(itr->first);
        if (!p || !p->GetSession())
            continue;

        if (itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket(&data);
    }
}

void Group::SendLootAllPassed(uint32 NumberOfPlayers, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ALL_PASSED, (8+4+4+4+4));
    data << uint64(r.itemGUID);                             // Guid of the item rolled
    data << uint32(NumberOfPlayers);                        // The number of players rolling for it???
    data << uint32(r.itemid);                               // The itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint32(r.itemRandomSuffix);                     // Item random suffix ID

    for (Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = ObjectAccessor::FindPlayer(itr->first);
        if (!p || !p->GetSession())
            continue;

        if (itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket(&data);
    }
}

void Group::GroupLoot(const uint64& playerGUID, Loot *loot, WorldObject* object)
{
    std::vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
    Player* player = ObjectAccessor::FindPlayer(playerGUID);
    Group *group = player->GetGroup();

    for (i = loot->items.begin(); i != loot->items.end(); ++i, ++itemSlot)
    {
        item = sObjectMgr->GetItemPrototype(i->itemid);
        if (!item)
        {
            //sLog->outDebug("Group::GroupLoot: missing item prototype for item with id: %d", i->itemid);
            continue;
        }

        //roll for over-threshold item if it's one-player loot
        if (item->Quality >= uint32(m_lootThreshold) && !i->freeforall)
        {
            uint64 newitemGUID = MAKE_NEW_GUID(sObjectMgr->GenerateLowGuid(HIGHGUID_ITEM), 0, HIGHGUID_ITEM);
            Roll* r = new Roll(newitemGUID, *i);

            //a vector is filled with only near party members
            for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->getSource();
                if (!member || !member->GetSession())
                    continue;
                if (i->AllowedForPlayer(member))
                {
                    if (member->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
                    {
                        r->playerVote[member->GetGUID()] = ROLL_NOT_EMITED_YET;
                        ++r->totalPlayersRolling;
                    }
                }
            }

            r->setLoot(loot);
            r->itemSlot = itemSlot;

            group->SendLootStartRoll(60000, *r);

            loot->items[itemSlot].is_blocked = true;
            object->m_groupLootTimer = 60000;
            object->lootingGroupLeaderGUID = GetLeaderGUID();

            RollId.push_back(r);
        }
        else
            i->is_underthreshold=1;
    }
}

void Group::NeedBeforeGreed(const uint64& playerGUID, Loot *loot, WorldObject* object)
{
    ItemPrototype const *item;
    Player* player = ObjectAccessor::FindPlayer(playerGUID);
    Group *group = player->GetGroup();

    uint8 itemSlot = 0;
    for (std::vector<LootItem>::iterator i=loot->items.begin(); i != loot->items.end(); ++i, ++itemSlot)
    {
        item = sObjectMgr->GetItemPrototype(i->itemid);

        //only roll for one-player items, not for ones everyone can get
        if (item->Quality >= uint32(m_lootThreshold) && !i->freeforall)
        {
            uint64 newitemGUID = MAKE_NEW_GUID(sObjectMgr->GenerateLowGuid(HIGHGUID_ITEM), 0, HIGHGUID_ITEM);
            Roll* r=new Roll(newitemGUID, *i);

            for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *playerToRoll = itr->getSource();
                if (!playerToRoll || !playerToRoll->GetSession())
                    continue;

                if (playerToRoll->CanUseItem(item) && i->AllowedForPlayer(playerToRoll))
                {
                    if (playerToRoll->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
                    {
                        r->playerVote[playerToRoll->GetGUID()] = ROLL_NOT_EMITED_YET;
                        ++r->totalPlayersRolling;
                    }
                }
            }

            if (r->totalPlayersRolling > 0)
            {
                r->setLoot(loot);
                r->itemSlot = itemSlot;

                group->SendLootStartRoll(60000, *r);

                loot->items[itemSlot].is_blocked = true;

                RollId.push_back(r);
            }
            else
            {
                delete r;
            }
        }
        else
            i->is_underthreshold=1;
    }
}

void Group::MasterLoot(const uint64& playerGUID, Loot* /*loot*/, WorldObject* object)
{
    Player* player = ObjectAccessor::FindPlayer(playerGUID);
    if (!player)
        return;

    sLog->outDebug("Group::MasterLoot (SMSG_LOOT_MASTER_LIST, 330) player = [%s].", player->GetName());

    uint32 real_count = 0;

    WorldPacket data(SMSG_LOOT_MASTER_LIST, 330);
    data << (uint8)GetMembersCount();

    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *looter = itr->getSource();
        if (!looter->IsInWorld())
            continue;

        if (looter->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
        {
            data << uint64(looter->GetGUID());
            ++real_count;
        }
    }

    data.put<uint8>(0, real_count);

    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *looter = itr->getSource();
        if (looter->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
            looter->GetSession()->SendPacket(&data);
    }
}

void Group::CountRollVote(const uint64& playerGUID, const uint64& Guid, uint32 NumberOfPlayers, uint8 Choice)
{
    Rolls::iterator rollI = GetRoll(Guid);
    if (rollI == RollId.end())
        return;
    Roll* roll = *rollI;

    Roll::PlayerVote::iterator itr = roll->playerVote.find(playerGUID);
    // this condition means that player joins to the party after roll begins
    if (itr == roll->playerVote.end())
        return;

    if (roll->getLoot())
        if (roll->getLoot()->items.empty())
            return;

    switch (Choice)
    {
        case ROLL_PASS:                                     // Player choose pass
        {
            SendLootRoll(0, playerGUID, 128, 128, *roll);
            ++roll->totalPass;
            itr->second = ROLL_PASS;
            break;
        }
        case ROLL_NEED:                                     // player choose Need
        {
            SendLootRoll(0, playerGUID, 0, 0, *roll);
            ++roll->totalNeed;
            itr->second = ROLL_NEED;
            break;
        }
        case ROLL_GREED:                                    // player choose Greed
        {
            SendLootRoll(0, playerGUID, 128, 2, *roll);
            ++roll->totalGreed;
            itr->second = ROLL_GREED;
            break;
        }
        default:                                            // Roll removed case
            break;
    }

    if (roll->totalPass + roll->totalGreed + roll->totalNeed >= roll->totalPlayersRolling)
    {
        CountTheRoll(rollI, NumberOfPlayers);
    }
}

//called when roll timer expires
void Group::EndRoll()
{
    Rolls::iterator itr;
    while (!RollId.empty())
    {
        //need more testing here, if rolls disappear
        itr = RollId.begin();
        CountTheRoll(itr, GetMembersCount());               //i don't have to edit player votes, who didn't vote ... he will pass
    }
}

void Group::CountTheRoll(Rolls::iterator rollI, uint32 NumberOfPlayers)
{
    Roll* roll = *rollI;
    if (!roll->isValid())                                    // is loot already deleted ?
    {
        RollId.erase(rollI);
        delete roll;
        return;
    }
    //end of the roll
    if (roll->totalNeed > 0)
    {
        if (!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            uint64 maxguid  = (*roll->playerVote.begin()).first;
            Player* player;

            for (Roll::PlayerVote::const_iterator itr=roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
            {
                if (itr->second != ROLL_NEED)
                    continue;

                uint8 randomN = urand(1, 100);
                SendLootRoll(0, itr->first, randomN, ROLL_NEED, *roll);
                if (maxresul < randomN)
                {
                    maxguid  = itr->first;
                    maxresul = randomN;
                }
            }
            SendLootRollWon(0, maxguid, maxresul, ROLL_NEED, *roll);
            player = ObjectAccessor::FindPlayer(maxguid);

            if (player && player->GetSession())
            {
                ItemPosCountVec dest;
                LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count);
                if (msg == EQUIP_ERR_OK)
                {
                    item->is_looted = true;
                    roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                    --roll->getLoot()->unlootedCount;
                    player->StoreNewItem(dest, roll->itemid, true, item->randomPropertyId);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError(msg, NULL, NULL);
                }
            }
        }
    }
    else if (roll->totalGreed > 0)
    {
        if (!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            uint64 maxguid = (*roll->playerVote.begin()).first;
            Player* player;

            Roll::PlayerVote::iterator itr;
            for (itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
            {
                if (itr->second != ROLL_GREED)
                    continue;

                uint8 randomN = urand(1, 100);
                SendLootRoll(0, itr->first, randomN, itr->second, *roll);
                if (maxresul < randomN)
                {
                    maxguid  = itr->first;
                    maxresul = randomN;
                }
            }
            SendLootRollWon(0, maxguid, maxresul, ROLL_GREED, *roll);
            player = ObjectAccessor::FindPlayer(maxguid);

            if (player && player->GetSession())
            {
                ItemPosCountVec dest;
                LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count);
                if (msg == EQUIP_ERR_OK)
                {
                    item->is_looted = true;
                    roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                    --roll->getLoot()->unlootedCount;
                    player->StoreNewItem(dest, roll->itemid, true, item->randomPropertyId);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError(msg, NULL, NULL);
                }
            }
        }
    }
    else
    {
        SendLootAllPassed(NumberOfPlayers, *roll);

        // remove is_blocked so that the item is lootable by all players
        LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
        if (item)
            item->is_blocked = false;
    }
    RollId.erase(rollI);
    delete roll;
}

void Group::SetTargetIcon(uint8 id, uint64 guid)
{
    if (id >= TARGETICONCOUNT)
        return;

    // clean other icons
    if (guid != 0)
        for (int i = 0; i < TARGETICONCOUNT; i++)
            if (m_targetIcons[i] == guid)
                SetTargetIcon(i, 0);

    m_targetIcons[id] = guid;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1+1+8));
    data << (uint8)0;
    data << uint8(id);
    data << uint64(guid);
    BroadcastPacket(&data, true);
}

void Group::GetDataForXPAtKill(Unit const* victim, uint32& count, uint32& sum_level, Player* & member_with_max_level, Player* & not_gray_member_with_max_level)
{
    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member || !member->isAlive())                   // only for alive
            continue;

        if (!member->IsAtGroupRewardDistance(victim))        // at req. distance
            continue;

        ++count;
        sum_level += member->getLevel();
        if (!member_with_max_level || member_with_max_level->getLevel() < member->getLevel())
            member_with_max_level = member;

        uint32 gray_level = Skyfire::XP::GetGrayLevel(member->getLevel());
        // if the victim is higher level than the gray level of the currently examined group member,
        // then set not_gray_member_with_max_level if needed.
        if (victim->getLevel() > gray_level && (!not_gray_member_with_max_level
           || not_gray_member_with_max_level->getLevel() < member->getLevel()))
            not_gray_member_with_max_level = member;
    }
}

void Group::SendTargetIconList(WorldSession *session)
{
    if (!session)
        return;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1+TARGETICONCOUNT*9));
    data << uint8(1);                                       // list targets

    for (int i = 0; i < TARGETICONCOUNT; ++i)
    {
        if (m_targetIcons[i] == 0)
            continue;

        data << uint8(i);
        data << uint64(m_targetIcons[i]);
    }

    session->SendPacket(&data);
}

void Group::SendUpdate()
{
    Player* player;
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = ObjectAccessor::FindPlayer(citr->guid);
        if (!player || !player->GetSession() || player->GetGroup() != this)
            continue;

                                                            // guess size
        WorldPacket data(SMSG_GROUP_LIST, (1+1+1+1+8+4+GetMembersCount()*20));
        data << (uint8)m_groupType;                         // group type
        data << (uint8)(isBGGroup() ? 1 : 0);               // 2.0.x, isBattlegroundGroup?
        data << (uint8)(citr->group);                       // groupid
        data << (uint8)(citr->assistant?0x01:0);            // 0x2 main assist, 0x4 main tank
        data << uint64(0x50000000FFFFFFFELL);               // related to voice chat?
        data << uint32(GetMembersCount()-1);
        for (member_citerator citr2 = m_memberSlots.begin(); citr2 != m_memberSlots.end(); ++citr2)
        {
            if (citr->guid == citr2->guid)
                continue;

            Player* member = ObjectAccessor::FindPlayer(citr2->guid);
            uint8 onlineState = (member) ? MEMBER_STATUS_ONLINE : MEMBER_STATUS_OFFLINE;
            onlineState = onlineState | ((isBGGroup()) ? MEMBER_STATUS_PVP : 0);

            data << citr2->name;
            data << (uint64)citr2->guid;
                                                            // online-state
            data << (uint8)(onlineState);
            data << (uint8)(citr2->group);                  // groupid
            data << (uint8)(citr2->assistant?0x01:0);       // 0x2 main assist, 0x4 main tank
        }

        data << uint64(m_leaderGuid);                       // leader guid
        if (GetMembersCount()-1)
        {
            data << uint8(m_lootMethod);                    // loot method
            data << uint64(m_looterGuid);                   // looter guid
            data << uint8(m_lootThreshold);                 // loot threshold
            data << uint8(m_difficulty);                    // Heroic Mod Group
        }
        player->GetSession()->SendPacket(&data);
    }
}

void Group::UpdatePlayerOutOfRange(Player* player)
{
    if (!player || !player->IsInWorld())
        return;

    Player* member;
    WorldPacket data;
    player->GetSession()->BuildPartyMemberStatsChangedPacket(player, &data);

    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        member = itr->getSource();
        if (member && member != player && !player->isVisibleFor(member))
            member->GetSession()->SendPacket(&data);
    }
}

void Group::BroadcastPacket(WorldPacket *packet, bool ignorePlayersInBGRaid, int group, uint64 ignore)
{
    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();
        if (!pl || (ignore != 0 && pl->GetGUID() == ignore) || (ignorePlayersInBGRaid && pl->GetGroup() != this))
            continue;

        if (pl->GetSession() && (group == -1 || itr->getSubGroup() == group))
            pl->GetSession()->SendPacket(packet);
    }
}

void Group::BroadcastReadyCheck(WorldPacket *packet)
{
    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();
        if (pl && pl->GetSession())
            if (IsLeader(pl->GetGUID()) || IsAssistant(pl->GetGUID()))
                pl->GetSession()->SendPacket(packet);
    }
}

void Group::OfflineReadyCheck()
{
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player *pl = ObjectAccessor::FindPlayer(citr->guid);
        if (!pl || !pl->GetSession())
        {
            WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
            data << uint64(citr->guid);
            data << uint8(0);
            BroadcastReadyCheck(&data);
        }
    }
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant)
{
    // get first not-full group
    uint8 groupid = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; groupid < MAXRAIDSIZE/MAXGROUPSIZE; ++groupid)
        {
            if (m_subGroupsCounts[groupid] < MAXGROUPSIZE)
            {
                groupFound = true;
                break;
            }
        }
        // We are raid group and no one slot is free
        if (!groupFound)
            return false;
    }

    return _addMember(guid, name, isAssistant, groupid);
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group)
{
    if (IsFull())
        return false;

    if (!guid)
        return false;

    Player* player = ObjectAccessor::FindPlayer(guid);

    MemberSlot member;
    member.guid      = guid;
    member.name      = name;
    member.group     = group;
    member.assistant = isAssistant;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(group);

    if (player)
    {
        player->SetGroupInvite(NULL);
        // if player is in group and he is being added to BG raid group, then call SetBattlegroundRaid()
        if (player->GetGroup() && isBGGroup())
            player->SetBattlegroundRaid(this, group);
        // if player is in bg raid and we are adding him to normal group, then call SetOriginalGroup()
        else if (player->GetGroup())
            player->SetOriginalGroup(this, group);
        // if player is not in group, then call set group
        else
            player->SetGroup(this, group);
        // if the same group invites the player back, cancel the homebind timer
        InstanceGroupBind *bind = GetBoundInstance(player);
        if (bind && bind->save->GetInstanceId() == player->GetInstanceId())
            player->m_InstanceValid = true;
    }

    if (!isRaidGroup())                                      // reset targetIcons for non-raid-groups
    {
        for (uint8 i = 0; i < TARGETICONCOUNT; ++i)
            m_targetIcons[i] = 0;
    }

    if (!isBGGroup())
    {
        // insert into group table
        CharacterDatabase.PExecute("INSERT INTO group_member(leaderGuid, memberGuid, assistant, subgroup) VALUES('%u', '%u', '%u', '%u')", GUID_LOPART(m_leaderGuid), GUID_LOPART(member.guid), ((member.assistant == 1)?1:0), member.group);
    }

    return true;
}

bool Group::_removeMember(const uint64 &guid)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    if (player)
    {
        // if we are removing player from battleground raid
        if (isBGGroup())
            player->RemoveFromBattlegroundRaid();
        else
        {
            // we can remove player who is in battleground from his original group
            if (player->GetOriginalGroup() == this)
                player->SetOriginalGroup(NULL);
            else
                player->SetGroup(NULL);
        }
    }

    _removeRolls(guid);

    member_witerator slot = _getMemberWSlot(guid);
    if (slot != m_memberSlots.end())
    {
        SubGroupCounterDecrease(slot->group);

        m_memberSlots.erase(slot);
    }

    if (!isBGGroup())
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid='%u'", GUID_LOPART(guid));

    if (m_leaderGuid == guid)                                // leader was removed
    {
        if (GetMembersCount() > 0)
            _setLeader(m_memberSlots.front().guid);
        return true;
    }

    return false;
}

void Group::_setLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    if (!isBGGroup())
    {
        // TODO: set a time limit to have this function run rarely cause it can be slow
        CharacterDatabase.BeginTransaction();

        // update the group's bound instances when changing leaders

        // remove all permanent binds from the group
        // in the DB also remove solo binds that will be replaced with permbinds
        // from the new leader
        CharacterDatabase.PExecute(
            "DELETE FROM group_instance WHERE leaderguid='%u' AND (permanent = 1 OR "
            "instance IN (SELECT instance FROM character_instance WHERE guid = '%u')"
            ")", GUID_LOPART(m_leaderGuid), GUID_LOPART(slot->guid)
        );

        Player* player = ObjectAccessor::FindPlayer(slot->guid);
        if (player)
        {
            for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
            {
                for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end();)
                {
                    if (itr->second.perm)
                    {
                        itr->second.save->RemoveGroup(this);
                        m_boundInstances[i].erase(itr++);
                    }
                    else
                        ++itr;
                }
            }
        }

        // update the group's solo binds to the new leader
        CharacterDatabase.PExecute("UPDATE group_instance SET leaderGuid='%u' WHERE leaderGuid = '%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));

        // copy the permanent binds from the new leader to the group
        // overwriting the solo binds with permanent ones if necessary
        // in the DB those have been deleted already
        Player::ConvertInstancesToGroup(player, this, slot->guid);

        // update the group leader
        CharacterDatabase.PExecute("UPDATE groups SET leaderGuid='%u' WHERE leaderGuid='%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));
        CharacterDatabase.PExecute("UPDATE group_member SET leaderGuid='%u' WHERE leaderGuid='%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));
        CharacterDatabase.CommitTransaction();
    }

    m_leaderGuid = slot->guid;
    m_leaderName = slot->name;
}

void Group::_removeRolls(const uint64 &guid)
{
    for (Rolls::iterator it = RollId.begin(); it < RollId.end(); ++it)
    {
        Roll* roll = *it;
        Roll::PlayerVote::iterator itr2 = roll->playerVote.find(guid);
        if (itr2 == roll->playerVote.end())
            continue;

        if (itr2->second == ROLL_GREED)
            --roll->totalGreed;
        if (itr2->second == ROLL_NEED)
            --roll->totalNeed;
        if (itr2->second == ROLL_PASS)
            --roll->totalPass;
        if (itr2->second != ROLL_NOT_VALID)
            --roll->totalPlayersRolling;

        roll->playerVote.erase(itr2);

        CountRollVote(guid, roll->itemGUID, GetMembersCount()-1, 3);
    }
}

bool Group::_setMembersGroup(const uint64 &guid, const uint8 &group)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->group = group;

    SubGroupCounterIncrease(group);

    if (!isBGGroup()) CharacterDatabase.PExecute("UPDATE group_member SET subgroup='%u' WHERE memberGuid='%u'", group, GUID_LOPART(guid));

    return true;
}

bool Group::_setAssistantFlag(const uint64 &guid, const bool &state)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->assistant = state;
    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET assistant='%u' WHERE memberGuid='%u'", (state == true)?1:0, GUID_LOPART(guid));
    return true;
}

bool Group::_setMainTank(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    if (m_mainAssistant == guid)
        _setMainAssistant(0);
    m_mainTank = guid;
    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainTank='%u' WHERE leaderGuid='%u'", GUID_LOPART(m_mainTank), GUID_LOPART(m_leaderGuid));
    return true;
}

bool Group::_setMainAssistant(const uint64 &guid)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    if (m_mainTank == guid)
        _setMainTank(0);
    m_mainAssistant = guid;
    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainAssistant='%u' WHERE leaderGuid='%u'", GUID_LOPART(m_mainAssistant), GUID_LOPART(m_leaderGuid));
    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if (!member1 || !member2) return false;
    if (member1->GetGroup() != this || member2->GetGroup() != this) return false;
    else return member1->GetSubGroup() == member2->GetSubGroup();
}

// allows setting subgroup for offline members
void Group::ChangeMembersGroup(const uint64 &guid, const uint8 &group)
{
    if (!isRaidGroup())
        return;
    Player* player = ObjectAccessor::FindPlayer(guid);

    if (player)
    {
        uint8 prevSubGroup = player->GetSubGroup();
        if (player->GetGroup() == this)
            player->GetGroupRef().setSubGroup(group);
        // if player is in BG raid, it is possible that he is also in normal raid - and that normal raid is stored in m_originalGroup reference
        else
        {
            prevSubGroup = player->GetOriginalSubGroup();
            player->GetOriginalGroupRef().setSubGroup(group);
        }

        SubGroupCounterDecrease(prevSubGroup);

        if (_setMembersGroup(guid, group))
            SendUpdate();
    }
    else
        // This methods handles itself groupcounter decrease
        ChangeMembersGroup(player, group);
}

// only for online members
void Group::ChangeMembersGroup(Player* player, const uint8 &group)
{
    if (!player || !isRaidGroup())
        return;
    if (_setMembersGroup(player->GetGUID(), group))
    {
        uint8 prevSubGroup;
        prevSubGroup = player->GetSubGroup();

        SubGroupCounterDecrease(prevSubGroup);

        player->GetGroupRef().setSubGroup(group);
        SendUpdate();
    }
}

void Group::UpdateLooterGuid(WorldObject* object, bool ifneed)
{
    switch (GetLootMethod())
    {
        case MASTER_LOOT:
        case FREE_FOR_ALL:
            return;
        default:
            // round robin style looting applies for all low
            // quality items in each loot method except free for all and master loot
            break;
    }

    member_citerator guid_itr = _getMemberCSlot(GetLooterGuid());
    if (guid_itr != m_memberSlots.end())
    {
        if (ifneed)
        {
            // not update if only update if need and ok
            Player* looter = ObjectAccessor::FindPlayer(guid_itr->guid);
            if (looter && looter->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
                return;
        }
        ++guid_itr;
    }

    // search next after current
    if (guid_itr != m_memberSlots.end())
    {
        for (member_citerator itr = guid_itr; itr != m_memberSlots.end(); ++itr)
        {
            if (Player* pl = ObjectAccessor::FindPlayer(itr->guid))
            {
                if (pl->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
                {
                    bool refresh = pl->GetLootGUID() == object->GetGUID();

                    //if (refresh)                             // update loot for new looter
                    //    pl->GetSession()->DoLootRelease(pl->GetLootGUID());
                    SetLooterGuid(pl->GetGUID());
                    SendUpdate();
                    if (refresh)                             // update loot for new looter
                        pl->SendLoot(object->GetGUID(), LOOT_CORPSE);
                    return;
                }
            }
        }
    }

    // search from start
    for (member_citerator itr = m_memberSlots.begin(); itr != guid_itr; ++itr)
    {
        if (Player* pl = ObjectAccessor::FindPlayer(itr->guid))
        {
            if (pl->GetDistance2d(object) < sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE))
            {
                bool refresh = pl->GetLootGUID() == object->GetGUID();

                //if (refresh)                               // update loot for new looter
                //    pl->GetSession()->DoLootRelease(pl->GetLootGUID());
                SetLooterGuid(pl->GetGUID());
                SendUpdate();
                if (refresh)                                 // update loot for new looter
                    pl->SendLoot(object->GetGUID(), LOOT_CORPSE);
                return;
            }
        }
    }

    SetLooterGuid(0);
    SendUpdate();
}

uint32 Group::CanJoinBattlegroundQueue(uint32 bgTypeId, uint32 bgQueueType, uint32 MinPlayerCount, uint32 MaxPlayerCount, bool isRated, uint32 arenaSlot)
{
    // check for min / max count
    uint32 memberscount = GetMembersCount();
    if (memberscount < MinPlayerCount)
        return BG_JOIN_ERR_GROUP_NOT_ENOUGH;
    if (memberscount > MaxPlayerCount)
        return BG_JOIN_ERR_GROUP_TOO_MANY;

    // get a player as reference, to compare other players' stats to (arena team id, queue id based on level, etc.)
    Player * reference = GetFirstMember()->getSource();
    // no reference found, can't join this way
    if (!reference)
        return BG_JOIN_ERR_OFFLINE_MEMBER;

    uint32 bgQueueId = reference->GetBattlegroundQueueIdFromLevel();
    uint32 arenaTeamId = reference->GetArenaTeamId(arenaSlot);
    uint32 team = reference->GetTeam();

    // check every member of the group to be able to join
    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *member = itr->getSource();
        // offline member? don't let join
        if (!member)
            return BG_JOIN_ERR_OFFLINE_MEMBER;
        // don't allow cross-faction join as group
        if (member->GetTeam() != team)
            return BG_JOIN_ERR_MIXED_FACTION;
        // not in the same battleground level braket, don't let join
        if (member->GetBattlegroundQueueIdFromLevel() != bgQueueId)
            return BG_JOIN_ERR_MIXED_LEVELS;
        // don't let join rated matches if the arena team id doesn't match
        if (isRated && member->GetArenaTeamId(arenaSlot) != arenaTeamId)
            return BG_JOIN_ERR_MIXED_ARENATEAM;
        // don't let join if someone from the group is already in that bg queue
        if (member->InBattlegroundQueueForBattlegroundQueueType(bgQueueType))
            return BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE;
        // check for deserter debuff in case not arena queue
        if (bgTypeId != BATTLEGROUND_AA && !member->CanJoinToBattleground())
            return BG_JOIN_ERR_GROUP_DESERTER;
        // check if member can join any more battleground queues
        if (!member->HasFreeBattlegroundQueueId())
            return BG_JOIN_ERR_ALL_QUEUES_USED;
    }
    return BG_JOIN_ERR_OK;
}

//===================================================
//============== Roll ===============================
//===================================================

void Roll::targetObjectBuildLink()
{
    // called from link()
    getTarget()->addLootValidatorRef(this);
}

void Group::SetDifficulty(uint8 difficulty)
{
    m_difficulty = difficulty;
    if (!isBGGroup()) CharacterDatabase.PExecute("UPDATE groups SET difficulty = %u WHERE leaderGuid ='%u'", m_difficulty, GUID_LOPART(m_leaderGuid));

    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (!player->GetSession() || player->getLevel() < LEVELREQUIREMENT_HEROIC)
            continue;
        player->SetDifficulty(difficulty);
        player->SendDungeonDifficulty(true);
        //send player to recall positio nis a dungeon (to avoid an exploit)
        if (sMapStore.LookupEntry(player->GetMap()->IsDungeon()))
            player->TeleportTo(player->m_recallMap, player->m_recallX, player->m_recallY, player->m_recallZ, player->m_recallO);
    }
}

bool Group::InCombatToInstance(uint32 instanceId)
{
    for (GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (player && player->getAttackers().size() && player->GetInstanceId() == instanceId && (player->GetMap()->IsRaid() || player->GetMap()->IsHeroic()))
            for (std::set<Unit*>::const_iterator i = player->getAttackers().begin(); i != player->getAttackers().end(); ++i)
          if ((*i) && (*i)->GetTypeId() == TYPEID_UNIT && (*i)->ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
                    return true;
    }
    return false;
}

void Group::ResetInstances(uint8 method, Player* SendMsgTo)
{
    if (isBGGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_DISBAND

    // we assume that when the difficulty changes, all instances that can be reset will be
    uint8 diff = GetDifficulty();

    for (BoundInstancesMap::iterator itr = m_boundInstances[diff].begin(); itr != m_boundInstances[diff].end();)
    {
        InstanceSave *p = itr->second.save;
        const MapEntry *entry = sMapStore.LookupEntry(itr->first);
        if (!entry || (!p->CanReset() && method != INSTANCE_RESET_GROUP_DISBAND))
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (diff == DIFFICULTY_HEROIC || entry->IsRaid())
            {
                ++itr;
                continue;
            }
        }

        bool isEmpty = true;
        // if the map is loaded, reset it
        Map *map = sMapMgr->FindMap(p->GetMapId(), p->GetInstanceId());
        if (map && map->IsDungeon())
        {
            if (p->CanReset())
                isEmpty = ((InstanceMap*)map)->Reset(method);
            else
                isEmpty = !map->HavePlayers();
        }

        if (SendMsgTo)
        {
            if (isEmpty) SendMsgTo->SendResetInstanceSuccess(p->GetMapId());
            else SendMsgTo->SendResetInstanceFailed(0, p->GetMapId());
        }

        if (isEmpty || method == INSTANCE_RESET_GROUP_DISBAND || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // do not reset the instance, just unbind if others are permanently bound to it
            if (p->CanReset()) p->DeleteFromDB();
            else CharacterDatabase.PExecute("DELETE FROM group_instance WHERE instance = '%u'", p->GetInstanceId());
            // i don't know for sure if hash_map iterators
            m_boundInstances[diff].erase(itr);
            itr = m_boundInstances[diff].begin();
            // this unloads the instance save unless online players are bound to it
            // (eg. permanent binds or GM solo binds)
            p->RemoveGroup(this);
        }
        else
            ++itr;
    }
}

InstanceGroupBind* Group::GetBoundInstance(Player* player)
{
    uint32 mapid = player->GetMapId();
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry)
        return NULL;

    uint8 difficulty = player->GetDifficulty();

    // some instances only have one difficulty
    if (!mapEntry->SupportsHeroicMode())
        difficulty = DIFFICULTY_NORMAL;

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    if (itr != m_boundInstances[difficulty].end())
        return &itr->second;
    else
        return NULL;
}

InstanceGroupBind* Group::GetBoundInstance(Map* aMap)
{
    uint8 difficulty = GetDifficulty();

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(aMap->GetId());
    if (itr != m_boundInstances[difficulty].end())
        return &itr->second;
    else
        return NULL;
}

InstanceGroupBind* Group::BindToInstance(InstanceSave *save, bool permanent, bool load)
{
    if (save && !isBGGroup())
    {
        InstanceGroupBind& bind = m_boundInstances[save->GetDifficulty()][save->GetMapId()];
        if (bind.save)
        {
            // when a boss is killed or when copying the player's binds to the group
            if (permanent != bind.perm || save != bind.save)
                if (!load) CharacterDatabase.PExecute("UPDATE group_instance SET instance = '%u', permanent = '%u' WHERE leaderGuid = '%u' AND instance = '%u'", save->GetInstanceId(), permanent, GUID_LOPART(GetLeaderGUID()), bind.save->GetInstanceId());
        }
        else
            if (!load) CharacterDatabase.PExecute("INSERT INTO group_instance (leaderGuid, instance, permanent) VALUES ('%u', '%u', '%u')", GUID_LOPART(GetLeaderGUID()), save->GetInstanceId(), permanent);

        if (bind.save != save)
        {
            if (bind.save) bind.save->RemoveGroup(this);
            save->AddGroup(this);
        }

        bind.save = save;
        bind.perm = permanent;
        if (!load) sLog->outDebug("Group::BindToInstance: %d is now bound to map %d, instance %d, difficulty %d", GUID_LOPART(GetLeaderGUID()), save->GetMapId(), save->GetInstanceId(), save->GetDifficulty());
        return &bind;
    }
    else
        return NULL;
}

void Group::UnbindInstance(uint32 mapid, uint8 difficulty, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    if (itr != m_boundInstances[difficulty].end())
    {
        if (!unload)
            CharacterDatabase.PExecute("DELETE FROM group_instance WHERE leaderGuid = '%u' AND instance = '%u'", GUID_LOPART(GetLeaderGUID()), itr->second.save->GetInstanceId());
        itr->second.save->RemoveGroup(this);                // save can become invalid
        m_boundInstances[difficulty].erase(itr);
    }
}

void Group::_homebindIfInstance(Player* player)
{
    if (player && !player->isGameMaster() && sMapStore.LookupEntry(player->GetMapId())->IsDungeon())
    {
        // leaving the group in an instance, the homebind timer is started
        // unless the player is permanently saved to the instance
        InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(player->GetInstanceId());
        InstancePlayerBind *playerBind = save ? player->GetBoundInstance(save->GetMapId(), save->GetDifficulty()) : NULL;
        if (!playerBind || !playerBind->perm)
            player->m_InstanceValid = false;
    }
}

void Group::BroadcastGroupUpdate(void)
{
    // FG: HACK: force flags update on group leave - for values update hack
    // -- not very efficient but safe
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player *pp = ObjectAccessor::FindPlayer(citr->guid);
        if (pp && pp->IsInWorld())
        {
            pp->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
            pp->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTIONTEMPLATE);
            sLog->outDebug("-- Forced group value update for '%s'", pp->GetName());
        }
    }
}

