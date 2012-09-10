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
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Creature.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Player.h"
#include "PoolMgr.h"
#include "Opcodes.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "WaypointMovementGenerator.h"
#include "InstanceScript.h"
#include "Battleground.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "OutdoorPvPMgr.h"
#include "GameEventMgr.h"
#include "CreatureGroups.h"
// apply implementation of the singletons

void TrainerSpellData::Clear()
{
    for (TrainerSpellList::iterator itr = spellList.begin(); itr != spellList.end(); ++itr)
        delete (*itr);
    spellList.empty();
}

TrainerSpell const* TrainerSpellData::Find(uint32 spell_id) const
{
    for (TrainerSpellList::const_iterator itr = spellList.begin(); itr != spellList.end(); ++itr)
        if ((*itr)->spell == spell_id)
            return *itr;

    return NULL;
}

bool VendorItemData::RemoveItem(uint32 item_id)
{
    for (VendorItemList::iterator i = m_items.begin(); i != m_items.end(); ++i)
    {
        if ((*i)->item == item_id)
        {
            m_items.erase(i);
            return true;
        }
    }
    return false;
}

size_t VendorItemData::FindItemSlot(uint32 item_id) const
{
    for (size_t i = 0; i < m_items.size(); ++i)
        if (m_items[i]->item == item_id)
            return i;
    return m_items.size();
}

VendorItem const* VendorItemData::FindItem(uint32 item_id) const
{
    for (VendorItemList::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
        if ((*i)->item == item_id)
            return *i;
    return NULL;
}

uint32 CreatureTemplate::GetRandomValidModelId() const
{
    uint32 c = 0;
    uint32 modelIDs[4];

    if (Modelid1) modelIDs[c++] = Modelid1;
    if (Modelid2) modelIDs[c++] = Modelid2;
    if (Modelid3) modelIDs[c++] = Modelid3;
    if (Modelid4) modelIDs[c++] = Modelid4;

    return ((c>0) ? modelIDs[urand(0, c-1)] : 0);
}

uint32 CreatureTemplate::GetFirstValidModelId() const
{
    if(Modelid1) return Modelid1;
    if(Modelid2) return Modelid2;
    if(Modelid3) return Modelid3;
    if(Modelid4) return Modelid4;
    return 0;
}

bool AssistDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    if (Unit* victim = Unit::GetUnit(m_owner, m_victim))
    {
        while (!m_assistants.empty())
        {
            Creature* assistant = Unit::GetCreature(m_owner, *m_assistants.begin());
            m_assistants.pop_front();

            if (assistant && assistant->CanAssistTo(&m_owner, victim))
            {
                assistant->SetNoCallAssistance(true);
                assistant->CombatStart(victim);
                if (assistant->IsAIEnabled)
                    assistant->AI()->AttackStart(victim);
            }
        }
    }
    return true;
}

bool ForcedDespawnDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.ForcedDespawn();
    return true;
}

Creature::Creature() :
Unit(),
lootForPickPocketed(false), lootForBody(false), m_lootMoney(0), m_lootRecipient(0),
m_corpseRemoveTime(0), m_respawnTime(0), m_respawnDelay(25), m_corpseDelay(60), m_respawnradius(0.0f),
m_emoteState(0), m_reactState(REACT_AGGRESSIVE),
m_regenTimer(2000), m_defaultMovementType(IDLE_MOTION_TYPE), m_equipmentId(0), m_AlreadyCallAssistance(false),
m_regenHealth(true), m_AI_locked(false), m_isDeadByDefault(false),
m_meleeDamageSchoolMask(SPELL_SCHOOL_MASK_NORMAL), m_creatureInfo(NULL), m_DBTableGuid(0), m_formation(NULL), m_PlayerDamageReq(0), m_summonMask(SUMMON_MASK_NONE)
, m_AlreadySearchedAssistance(false)
, m_creatureData(NULL)
, m_group(NULL)
{
    m_valuesCount = UNIT_END;

    for (int i = 0; i < CREATURE_MAX_SPELLS; ++i)
        m_spells[i] = 0;

    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();
    m_GlobalCooldown = 0;
    DisableReputationGain = false;
    //m_unit_movement_flags = MOVEFLAG_WALK_MODE;

    m_SightDistance = sWorld->getConfig(CONFIG_SIGHT_MONSTER);
    m_CombatDistance = MELEE_RANGE;
}

Creature::~Creature()
{
    m_vendorItemCounts.clear();

    delete i_AI;
    i_AI = NULL;
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if (!IsInWorld())
    {
        if (m_zoneScript)
            m_zoneScript->OnCreatureCreate(this);
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        SearchFormation();
        AIM_Initialize();
        //if (IsVehicle())
            //GetVehicleKit()->Install();
    }
}

void Creature::RemoveFromWorld()
{
    if (IsInWorld())
    {
        if (m_zoneScript)
            m_zoneScript->OnCreatureRemove(this);
        if (m_formation)
            FormationMgr::RemoveCreatureFromGroup(m_formation, this);
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

void Creature::DisappearAndDie()
{
    DestroyForNearbyPlayers();
    if (isAlive())
        setDeathState(JUST_DIED);
    RemoveCorpse(false);
}

void Creature::SearchFormation()
{
    if (isPet())
        return;

    uint32 lowguid = GetDBTableGUIDLow();
    if (!lowguid)
        return;

    CreatureGroupInfoType::iterator frmdata = CreatureGroupMap.find(lowguid);
    if (frmdata != CreatureGroupMap.end())
        FormationMgr::AddCreatureToGroup(frmdata->second->leaderGUID, this);
}

void Creature::RemoveCorpse(bool setSpawnTime)
{
    if ((getDeathState() != CORPSE && !m_isDeadByDefault) || (getDeathState() != ALIVE && m_isDeadByDefault))
        return;

    m_corpseRemoveTime = time(NULL);
    setDeathState(DEAD);
    UpdateObjectVisibility();
    loot.clear();
    // Should get removed later, just keep "compatibility" with scripts
    if (setSpawnTime)
        m_respawnTime = time(NULL) + m_respawnDelay;

    float x, y, z, o;
    GetRespawnCoord(x, y, z, &o);
    SetHomePosition(x, y, z, o);
    GetMap()->CreatureRelocation(this, x, y, z, o);
}

/**
 * change the entry of creature until respawn
 */
bool Creature::InitEntry(uint32 Entry, uint32 team, const CreatureData *data)
{
    CreatureTemplate const* normalInfo = sObjectMgr->GetCreatureTemplate(Entry);
    if (!normalInfo)
    {
        sLog->outErrorDb("Creature::UpdateEntry creature entry %u does not exist.", Entry);
        return false;
    }

    // get heroic mode entry
    uint32 actualEntry = Entry;
    CreatureTemplate const* cinfo = normalInfo;
    if (normalInfo->HeroicEntry)
    {
        //we already have valid Map pointer for current creature!
        if (GetMap()->IsHeroic())
        {
            cinfo = sObjectMgr->GetCreatureTemplate(normalInfo->HeroicEntry);
            if (!cinfo)
            {
                sLog->outErrorDb("Creature::UpdateEntry creature heroic entry %u does not exist.", actualEntry);
                return false;
            }
        }
    }

    SetEntry(Entry);                                        // normal entry always
    m_creatureInfo = cinfo;                                 // map mode related always

    // Cancel load if no model defined
    if (!(cinfo->GetFirstValidModelId()))
    {
        sLog->outErrorDb("Creature (Entry: %u) has no model defined in table creature_template, can't load. ", Entry);
        return false;
    }

    uint32 display_id = sObjectMgr->ChooseDisplayId(team, GetCreatureTemplate(), data);
    CreatureModelInfo const *minfo = sObjectMgr->GetCreatureModelRandomGender(display_id);
    if (!minfo)                                             // Cancel load if no model defined
    {
        sLog->outErrorDb("Creature (Entry: %u) has model %u not found in table creature_model_info, can't load. ", Entry, display_id);
        return false;
    }

    display_id = minfo->modelid;                            // it can be different (for another gender)

    SetDisplayId(display_id);
    SetNativeDisplayId(display_id);
    SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);

    // Load creature equipment
    if (!data || data->equipmentId == 0)
    {                                                       // use default from the template
        LoadEquipment(cinfo->equipmentId);
    }
    else if (data && data->equipmentId != -1)
    {                                                       // override, -1 means no equipment
        LoadEquipment(data->equipmentId);
    }

    SetName(normalInfo->Name);                              // at normal entry always

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, minfo->bounding_radius);
    SetFloatValue(UNIT_FIELD_COMBATREACH, minfo->combat_reach);

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    SetSpeed(MOVE_WALK,     cinfo->speed);
    SetSpeed(MOVE_RUN,      cinfo->speed);
    SetSpeed(MOVE_SWIM,     cinfo->speed);

    SetFloatValue(OBJECT_FIELD_SCALE_X, cinfo->scale);

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    if (!m_respawnradius && m_defaultMovementType == RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        m_spells[i] = GetCreatureTemplate()->spells[i];

    return true;
}

bool Creature::UpdateEntry(uint32 Entry, uint32 team, const CreatureData *data)
{
    if (!InitEntry(Entry, team, data))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    m_regenHealth = cInfo->RegenHealth;

    // creatures always have melee weapon ready if any
    SetSheath(SHEATH_STATE_MELEE);
    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_AURAS);

    SelectLevel(cInfo);
    if (team == HORDE)
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, cInfo->faction_H);
    else
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, cInfo->faction_A);

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        SetUInt32Value(UNIT_NPC_FLAGS, cInfo->npcflag | sGameEventMgr->GetNPCFlag(this));
    else
        SetUInt32Value(UNIT_NPC_FLAGS, cInfo->npcflag);

    SetAttackTime(BASE_ATTACK,  cInfo->baseattacktime);
    SetAttackTime(OFF_ATTACK,   cInfo->baseattacktime);
    SetAttackTime(RANGED_ATTACK, cInfo->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS, cInfo->unit_flags);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS, cInfo->dynamicflags);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    SetMeleeDamageSchool(SpellSchools(cInfo->dmgschool));
    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(cInfo->armor));

    if (cInfo->resistance1 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_HOLY, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, float(cInfo->resistance1));

    if (cInfo->resistance2 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, float(cInfo->resistance2));

    if (cInfo->resistance3 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(cInfo->resistance3));

    if (cInfo->resistance4 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, float(cInfo->resistance4));

    if (cInfo->resistance5 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(cInfo->resistance5));

    if (cInfo->resistance6 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(cInfo->resistance6));

    SetCanModifyStats(true);
    UpdateAllStats();

    // checked and error show at loading templates
    if (FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction_A))
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction);
        if (factionEntry)
            if (!(cInfo->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN) &&
                (factionEntry->team == ALLIANCE || factionEntry->team == HORDE))
                SetPvP(true);
    }

    // trigger creature is always not selectable and can not be attacked
    if (isTrigger())
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    if (isTotem() || isTrigger()
        || GetCreatureType() == CREATURE_TYPE_CRITTER)
        SetReactState(REACT_PASSIVE);
    /*else if (isCivilian())
        SetReactState(REACT_DEFENSIVE);*/
    else
        SetReactState(REACT_AGGRESSIVE);

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_NO_TAUNT)
    {
        ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
    }

    // TODO: In fact monster move flags should be set - not movement flags.
    if (cInfo->InhabitType & INHABIT_AIR)
        AddUnitMovementFlag(MOVEFLAG_FLYING | MOVEFLAG_FLYING2);

    if (cInfo->InhabitType & INHABIT_WATER)
        AddUnitMovementFlag(MOVEFLAG_SWIMMING);

    return true;
}

void Creature::Update(uint32 diff)
{
    if (m_GlobalCooldown <= diff)
        m_GlobalCooldown = 0;
    else
        m_GlobalCooldown -= diff;

    switch (m_deathState)
    {
        case JUST_ALIVED:
            // Don't must be called, see Creature::setDeathState JUST_ALIVED -> ALIVE promoting.
            sLog->outError("Creature (GUIDLow: %u Entry: %u) in wrong state: JUST_ALIVED (4)", GetGUIDLow(), GetEntry());
            break;
        case JUST_DIED:
            // Don't must be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            sLog->outError("Creature (GUIDLow: %u Entry: %u) in wrong state: JUST_DEAD (1)", GetGUIDLow(), GetEntry());
            break;
        case DEAD:
        {
            if (m_respawnTime <= time(NULL))
            {
                if (!GetLinkedCreatureRespawnTime()) // Can respawn
                    Respawn();
                else // the master is dead
                {
                    if (uint32 targetGuid = sObjectMgr->GetLinkedRespawnGuid(m_DBTableGuid))
                    {
                        if (targetGuid == m_DBTableGuid) // if linking self, never respawn (check delayed to next day)
                            SetRespawnTime(DAY);
                        else
                            m_respawnTime = (time(NULL)>GetLinkedCreatureRespawnTime()? time(NULL):GetLinkedCreatureRespawnTime())+urand(5, MINUTE); // else copy time from master and add a little
                        SaveRespawnTime(); // also save to DB immediately
                    }
                    else
                        Respawn();
                }
            }
            break;
        }
        case CORPSE:
        {
            if (m_isDeadByDefault)
                break;

            /*if (GetGroup() && GetGroup()->IsAllowedToRespawn(this))
            {
                Respawn();
                break;
            }*/

            if (m_corpseRemoveTime <= time(NULL))
            {
                RemoveCorpse(false);
                sLog->outDebug("Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
            }
            else
            {
                if (m_groupLootTimer && lootingGroupLeaderGUID)
                {
                    if (diff <= m_groupLootTimer)
                    {
                        m_groupLootTimer -= diff;
                    }
                    else
                    {
                        Group* group = sObjectMgr->GetGroupByLeader(lootingGroupLeaderGUID);
                        if (group)
                            group->EndRoll();
                        m_groupLootTimer = 0;
                        lootingGroupLeaderGUID = 0;
                    }
                }
            }

            break;
        }
        case ALIVE:
        {
            if (m_isDeadByDefault)
            {
                if (m_corpseRemoveTime <= time(NULL))
                {
                    RemoveCorpse(false);
                    sLog->outDebug("Removing alive corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
                }
            }

            Unit::Update(diff);

            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;

            // if creature is charmed, switch to charmed AI
            if (NeedChangeAI)
            {
                UpdateCharmAI();
                NeedChangeAI = false;
                IsAIEnabled = true;
            }

            if (!IsInEvadeMode() && IsAIEnabled)
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                i_AI->UpdateAI(diff);
                m_AI_locked = false;
            }

            // creature can be dead after UpdateAI call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;
            if (m_regenTimer > 0)
            {
                if (diff >= m_regenTimer)
                    m_regenTimer = 0;
                else
                    m_regenTimer -= diff;
            }
            if (m_regenTimer != 0)
                break;

            if (!isInCombat())
            {
                if (HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER))
                    SetUInt32Value(UNIT_DYNAMIC_FLAGS, GetCreatureTemplate()->dynamicflags);
                RegenerateHealth();
            }
            else if (IsPolymorphed())
                    RegenerateHealth();

            RegenerateMana();

            m_regenTimer = 2000;
            break;
        }
        case DEAD_FALLING:
            GetMotionMaster()->UpdateMotion(diff);
            break;
        default:
            break;
    }
}

void Creature::RegenerateMana()
{
    uint32 curValue = GetPower(POWER_MANA);
    uint32 maxValue = GetMaxPower(POWER_MANA);

    if (curValue >= maxValue)
        return;

    uint32 addvalue = 0;

    // Combat and any controlled creature
    if (isInCombat() || GetCharmerOrOwnerGUID())
    {
        if (!IsUnderLastManaUseEffect())
        {
            float ManaIncreaseRate = sWorld->getRate(RATE_POWER_MANA);
            float Spirit = GetStat(STAT_SPIRIT);

            addvalue = uint32((Spirit/5.0f + 17.0f) * ManaIncreaseRate);
        }
    }
    else
        addvalue = maxValue/3;

    ModifyPower(POWER_MANA, addvalue);
}

void Creature::RegenerateHealth()
{
    if (!isRegeneratingHealth())
        return;

    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    uint32 addvalue = 0;

    // Not only pet, but any controlled creature
    if (GetCharmerOrOwnerGUID())
    {
        float HealthIncreaseRate = sWorld->getRate(RATE_HEALTH);
        float Spirit = GetStat(STAT_SPIRIT);

        if (GetPower(POWER_MANA) > 0)
            addvalue = uint32(Spirit * 0.25 * HealthIncreaseRate);
        else
            addvalue = uint32(Spirit * 0.80 * HealthIncreaseRate);
    }
    else
        addvalue = maxValue/3;

    ModifyHealth(addvalue);
}

bool Creature::AIM_Initialize(CreatureAI* ai)
{
    // make sure nothing can change the AI during AI update
    if (m_AI_locked)
    {
        sLog->outDebug("AIM_Initialize: failed to init, locked.");
        return false;
    }

    UnitAI *oldAI = i_AI;
    i_motionMaster.Initialize();
    i_AI = ai ? ai : FactorySelector::selectAI(this);
    delete oldAI;
    IsAIEnabled = true;
    i_AI->InitializeAI();
    return true;
}

bool Creature::Create(uint32 guidlow, Map *map, uint32 Entry, uint32 team, float x, float y, float z, float ang, const CreatureData *data)
{
    ASSERT(map);
    SetMap(map);

    Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        sLog->outError("Creature (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)", guidlow, Entry, x, y);
        return false;
    }

    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    const bool bResult = CreateFromProto(guidlow, Entry, team, data);

    if (bResult)
    {
        switch (GetCreatureTemplate()->rank)
        {
            case CREATURE_ELITE_RARE:
                m_corpseDelay = sWorld->getConfig(CONFIG_CORPSE_DECAY_RARE);
                break;
            case CREATURE_ELITE_ELITE:
                m_corpseDelay = sWorld->getConfig(CONFIG_CORPSE_DECAY_ELITE);
                break;
            case CREATURE_ELITE_RAREELITE:
                m_corpseDelay = sWorld->getConfig(CONFIG_CORPSE_DECAY_RAREELITE);
                break;
            case CREATURE_ELITE_WORLDBOSS:
                m_corpseDelay = sWorld->getConfig(CONFIG_CORPSE_DECAY_WORLDBOSS);
                break;
            default:
                m_corpseDelay = sWorld->getConfig(CONFIG_CORPSE_DECAY_NORMAL);
                break;
        }
        LoadCreaturesAddon();

        if (GetCreatureTemplate()->InhabitType & INHABIT_AIR)
        {
            if (GetDefaultMovementType() == IDLE_MOTION_TYPE)
                AddUnitMovementFlag(MOVEFLAG_FLYING);
            else
                SetFlying(true);
        }

        if (GetCreatureTemplate()->InhabitType & INHABIT_WATER)
        {
            AddUnitMovementFlag(MOVEFLAG_SWIMMING);
        }
    }

    return bResult;
}

bool Creature::isCanTrainingOf(Player* player, bool msg) const
{
    if (!isTrainer())
        return false;

    TrainerSpellData const* trainer_spells = GetTrainerSpells();

    if (!trainer_spells || trainer_spells->spellList.empty())
    {
        sLog->outErrorDb("Creature %u (Entry: %u) has UNIT_NPC_FLAG_TRAINER but trainer spell list is empty.",
            GetGUIDLow(), GetEntry());
        return false;
    }

    switch (GetCreatureTemplate()->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if (player->getClass() != GetCreatureTemplate()->classNum)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureTemplate()->classNum)
                    {
                        case CLASS_DRUID:  player->PlayerTalkClass->SendGossipMenu(4913, GetGUID()); break;
                        case CLASS_HUNTER: player->PlayerTalkClass->SendGossipMenu(10090, GetGUID()); break;
                        case CLASS_MAGE:   player->PlayerTalkClass->SendGossipMenu( 328, GetGUID()); break;
                        case CLASS_PALADIN:player->PlayerTalkClass->SendGossipMenu(1635, GetGUID()); break;
                        case CLASS_PRIEST: player->PlayerTalkClass->SendGossipMenu(4436, GetGUID()); break;
                        case CLASS_ROGUE:  player->PlayerTalkClass->SendGossipMenu(4797, GetGUID()); break;
                        case CLASS_SHAMAN: player->PlayerTalkClass->SendGossipMenu(5003, GetGUID()); break;
                        case CLASS_WARLOCK:player->PlayerTalkClass->SendGossipMenu(5836, GetGUID()); break;
                        case CLASS_WARRIOR:player->PlayerTalkClass->SendGossipMenu(4985, GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if (player->getClass() != CLASS_HUNTER)
            {
                player->PlayerTalkClass->ClearMenus();
                player->PlayerTalkClass->SendGossipMenu(3620, GetGUID());
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if (GetCreatureTemplate()->race && player->getRace() != GetCreatureTemplate()->race)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureTemplate()->classNum)
                    {
                        case RACE_DWARF:        player->PlayerTalkClass->SendGossipMenu(5865, GetGUID()); break;
                        case RACE_GNOME:        player->PlayerTalkClass->SendGossipMenu(4881, GetGUID()); break;
                        case RACE_HUMAN:        player->PlayerTalkClass->SendGossipMenu(5861, GetGUID()); break;
                        case RACE_NIGHTELF:     player->PlayerTalkClass->SendGossipMenu(5862, GetGUID()); break;
                        case RACE_ORC:          player->PlayerTalkClass->SendGossipMenu(5863, GetGUID()); break;
                        case RACE_TAUREN:       player->PlayerTalkClass->SendGossipMenu(5864, GetGUID()); break;
                        case RACE_TROLL:        player->PlayerTalkClass->SendGossipMenu(5816, GetGUID()); break;
                        case RACE_UNDEAD_PLAYER:player->PlayerTalkClass->SendGossipMenu(624, GetGUID()); break;
                        case RACE_BLOODELF:     player->PlayerTalkClass->SendGossipMenu(5862, GetGUID()); break;
                        case RACE_DRAENEI:      player->PlayerTalkClass->SendGossipMenu(5864, GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if (GetCreatureTemplate()->trainer_spell && !player->HasSpell(GetCreatureTemplate()->trainer_spell))
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    player->PlayerTalkClass->SendGossipMenu(11031, GetGUID());
                }
                return false;
            }
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::isCanInteractWithBattleMaster(Player* player, bool msg) const
{
    if (!isBattleMaster())
        return false;

    uint32 bgTypeId = sObjectMgr->GetBattleMasterBG(GetEntry());
    if (!msg)
        return player->GetBGAccessByLevel(bgTypeId);

    if (!player->GetBGAccessByLevel(bgTypeId))
    {
        player->PlayerTalkClass->ClearMenus();
        switch (bgTypeId)
        {
            case BATTLEGROUND_AV:  player->PlayerTalkClass->SendGossipMenu(7616, GetGUID()); break;
            case BATTLEGROUND_WS:  player->PlayerTalkClass->SendGossipMenu(7599, GetGUID()); break;
            case BATTLEGROUND_AB:  player->PlayerTalkClass->SendGossipMenu(7642, GetGUID()); break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_NA:
            case BATTLEGROUND_BE:
            case BATTLEGROUND_AA:
            case BATTLEGROUND_RL:  player->PlayerTalkClass->SendGossipMenu(10024, GetGUID()); break;
            break;
        }
        return false;
    }
    return true;
}

bool Creature::isCanTrainingAndResetTalentsOf(Player* player) const
{
    return player->getLevel() >= 10
        && GetCreatureTemplate()->trainer_type == TRAINER_TYPE_CLASS
        && player->getClass() == GetCreatureTemplate()->classNum;
}

void Creature::AI_SendMoveToPacket(float x, float y, float z, uint32 time, uint32 /*MovementFlags*/, uint8 /*type*/)
{
    /*    uint32 timeElap = getMSTime();
        if ((timeElap - m_startMove) < m_moveTime)
        {
            oX = (dX - oX) * ((timeElap - m_startMove) / m_moveTime);
            oY = (dY - oY) * ((timeElap - m_startMove) / m_moveTime);
        }
        else
        {
            oX = dX;
            oY = dY;
        }

        dX = x;
        dY = y;
        m_orientation = atan2((oY - dY), (oX - dX));

        m_startMove = getMSTime();
        m_moveTime = time;*/
    SendMonsterMove(x, y, z, time);
}

Player *Creature::GetLootRecipient() const
{
    if (!m_lootRecipient) return NULL;
    else return ObjectAccessor::FindPlayer(m_lootRecipient);
}

void Creature::SetLootRecipient(Unit *unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears

    if (!unit)
    {
        m_lootRecipient = 0;
        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
        return;
    }

    Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!player)                                             // normal creature, no player involved
        return;

    m_lootRecipient = player->GetGUID();
    SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
}

void Creature::SaveToDB()
{
    // this should only be used when the creature has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    CreatureData const *data = sObjectMgr->GetCreatureData(m_DBTableGuid);
    if (!data)
    {
        sLog->outError("Creature::SaveToDB failed, cannot get creature data!");
        return;
    }

    SaveToDB(GetMapId(), data->spawnMask);
}

void Creature::SaveToDB(uint32 mapid, uint8 spawnMask)
{
    // update in loaded data
    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUIDLow();
    CreatureData& data = sObjectMgr->NewOrExistCreatureData(m_DBTableGuid);

    uint32 displayId = GetNativeDisplayId();
    uint32 npcflag = GetUInt32Value(UNIT_NPC_FLAGS);
    uint32 unit_flags = GetUInt32Value(UNIT_FIELD_FLAGS);
    uint32 dynamicflags = GetUInt32Value(UNIT_DYNAMIC_FLAGS);

    // check if it's a custom model and if not, use 0 for displayId
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo)
    {
        if (displayId == cinfo->Modelid1 || displayId == cinfo->Modelid2 ||
            displayId == cinfo->Modelid3 || displayId == cinfo->Modelid4)
            displayId = 0;

        if (npcflag == cinfo->npcflag)
            npcflag = 0;

        if (unit_flags == cinfo->unit_flags)
            unit_flags = 0;

        if (dynamicflags == cinfo->dynamicflags)
            dynamicflags = 0;
    }

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.displayid = displayId;
    data.equipmentId = GetEquipmentId();
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZ();
    data.orientation = GetOrientation();
    data.spawntimesecs = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType() == IDLE_MOTION_TYPE ? 0 : m_respawnradius;
    data.currentwaypoint = 0;
    data.curhealth = GetHealth();
    data.curmana = GetPower(POWER_MANA);
    data.is_dead = m_isDeadByDefault;
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType() == RANDOM_MOTION_TYPE
        ? IDLE_MOTION_TYPE : GetDefaultMovementType();
    data.spawnMask = spawnMask;
    data.npcflag = npcflag;
    data.unit_flags = unit_flags;
    data.dynamicflags = dynamicflags;

    // updated in DB
    WorldDatabase.BeginTransaction();

    WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid = '%u'", m_DBTableGuid);

    std::ostringstream ss;
    ss << "INSERT INTO creature VALUES ("
        << m_DBTableGuid << ", "
        << GetEntry() << ", "
        << mapid <<", "
        << (uint32)spawnMask << ", "
        << displayId <<", "
        << GetEquipmentId() <<", "
        << GetPositionX() << ", "
        << GetPositionY() << ", "
        << GetPositionZ() << ", "
        << GetOrientation() << ", "
        << m_respawnDelay << ", "                            //respawn time
        << (float) m_respawnradius << ", "                   //spawn distance (float)
        << (uint32) (0) << ", "                              //currentwaypoint
        << GetHealth() << ", "                               //curhealth
        << GetPower(POWER_MANA) << ", "                      //curmana
        << (m_isDeadByDefault ? 1 : 0) << ", "               //is_dead
        << GetDefaultMovementType() << ")";                 //default movement generator type

    WorldDatabase.PExecuteLog(ss.str().c_str());

    WorldDatabase.CommitTransaction();
}

void Creature::SelectLevel(const CreatureTemplate *cinfo)
{
    uint32 rank = isPet()? 0 : cinfo->rank;

    // level
    uint32 minlevel = std::min(cinfo->maxlevel, cinfo->minlevel);
    uint32 maxlevel = std::max(cinfo->maxlevel, cinfo->minlevel);
    uint32 level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);
    SetLevel(level);

    float rellevel = maxlevel == minlevel ? 0 : (float(level - minlevel))/(maxlevel - minlevel);

    // health
    float healthmod = _GetHealthMod(rank);

    uint32 minhealth = std::min(cinfo->maxhealth, cinfo->minhealth);
    uint32 maxhealth = std::max(cinfo->maxhealth, cinfo->minhealth);
    uint32 health = uint32(healthmod * (minhealth + uint32(rellevel*(maxhealth - minhealth))));

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);
    ResetPlayerDamageReq();

    // mana
    uint32 minmana = std::min(cinfo->maxmana, cinfo->minmana);
    uint32 maxmana = std::max(cinfo->maxmana, cinfo->minmana);
    uint32 mana = minmana + uint32(rellevel*(maxmana - minmana));

    SetCreateMana(mana);
    SetMaxPower(POWER_MANA, mana);                          //MAX Mana
    SetPower(POWER_MANA, mana);

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, health);
    SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, mana);

    // damage
    float damagemod = _GetDamageMod(rank);

    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg * damagemod);
    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg * damagemod);
    SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, cinfo->mindmg * damagemod);
    SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, cinfo->maxdmg * damagemod);
    SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, cinfo->minrangedmg * damagemod);
    SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, cinfo->maxrangedmg * damagemod);

    // this value is not accurate, but should be close to the real value
    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, level * 5);
    SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, level * 5);
    //SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, cinfo->attackpower * damagemod);
    //SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, cinfo->rangedattackpower * damagemod);
}

float Creature::_GetHealthMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_HP);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_HP);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
    }
}

float Creature::_GetDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_DAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_DAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_DAMAGE);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
    }
}

float Creature::GetSpellDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_SPELLDAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_SPELLDAMAGE);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
    }
}

bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data)
{
    SetZoneScript();
    if (m_zoneScript && data)
    {
        Entry = m_zoneScript->GetCreatureEntry(guidlow, data);
        if (!Entry)
            return false;
    }

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(Entry);
    if (!cinfo)
    {
        sLog->outErrorDb("Creature entry %u does not exist.", Entry);
        return false;
    }

    m_originalEntry = Entry;

    Object::_Create(guidlow, Entry, HIGHGUID_UNIT);

    if (!UpdateEntry(Entry, team, data))
        return false;

    return true;
}

bool Creature::LoadFromDB(uint32 guid, Map *map)
{
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);

    if (!data)
    {
        sLog->outErrorDb("Creature (GUID: %u) not found in table creature, can't load. ", guid);
        return false;
    }

    m_DBTableGuid = guid;
    if (map->GetInstanceId() != 0)
        guid = sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT);

    uint16 team = 0;
    if (!Create(guid, map, data->id, team, data->posX, data->posY, data->posZ, data->orientation, data))
        return false;

    //We should set first home position, because then AI calls home movement
    SetHomePosition(data->posX, data->posY, data->posZ, data->orientation);

    m_respawnradius = data->spawndist;

    m_respawnDelay = data->spawntimesecs;
    m_isDeadByDefault = data->is_dead;
    m_deathState = m_isDeadByDefault ? DEAD : ALIVE;

    m_respawnTime  = sObjectMgr->GetCreatureRespawnTime(m_DBTableGuid, GetInstanceId());
    if (m_respawnTime)                          // respawn on Update
    {
        m_deathState = DEAD;
        if (canFly())
        {
            float tz = GetMap()->GetHeight(data->posX, data->posY, data->posZ, false);
            if (data->posZ - tz > 0.1)
                Relocate(data->posX, data->posY, tz);
        }
    }

    uint32 curhealth = data->curhealth;
    if (curhealth)
    {
        curhealth = uint32(curhealth*_GetHealthMod(GetCreatureTemplate()->rank));
        if (curhealth < 1)
            curhealth = 1;
    }

    SetHealth(m_deathState == ALIVE ? curhealth : 0);
    SetPower(POWER_MANA, data->curmana);

    // checked at creature_template loading
    m_defaultMovementType = MovementGeneratorType(data->movementType);

    m_creatureData = data;

    return true;
}

void Creature::LoadEquipment(uint32 equip_entry, bool force)
{
    if (equip_entry == 0)
    {
        if (force)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, 0);
                SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2), 0);
                SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, 0);
            }
            m_equipmentId = 0;
        }
        return;
    }

    EquipmentInfo const *einfo = sObjectMgr->GetEquipmentInfo(equip_entry);
    if (!einfo)
        return;

    m_equipmentId = equip_entry;
    for (uint8 i = 0; i < 3; ++i)
    {
        SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, einfo->equipmodel[i]);
        SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2), einfo->equipinfo[i]);
        SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, einfo->equipslot[i]);
    }
}

bool Creature::hasQuest(uint32 quest_id) const
{
    QuestRelations const& qr = sObjectMgr->mCreatureQuestRelations;
    for (QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelations const& qr = sObjectMgr->mCreatureQuestInvolvedRelations;
    for (QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

void Creature::DeleteFromDB()
{
    if (!m_DBTableGuid)
    {
        sLog->outDebug("Trying to delete not saved creature!");
        return;
    }

    sObjectMgr->SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);
    sObjectMgr->DeleteCreatureData(m_DBTableGuid);

    WorldDatabase.BeginTransaction();
    WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid = '%u'", m_DBTableGuid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_addon WHERE guid = '%u'", m_DBTableGuid);
    WorldDatabase.PExecuteLog("DELETE FROM game_event_creature WHERE guid = '%u'", m_DBTableGuid);
    WorldDatabase.PExecuteLog("DELETE FROM game_event_model_equip WHERE guid = '%u'", m_DBTableGuid);
    WorldDatabase.CommitTransaction();
}

bool Creature::canSeeOrDetect(Unit const* u, bool detect, bool inVisibleList, bool is3dDistance) const
{
    // not in world
    if (!IsInWorld() || !u->IsInWorld())
        return false;

    // all dead creatures/players not visible for any creatures
    if (!u->isAlive() || !isAlive())
        return false;

    // Always can see self
    if (u == this)
        return true;

    // always seen by owner
    if (GetGUID() == u->GetCharmerOrOwnerGUID())
        return true;

    if (u->GetVisibility() == VISIBILITY_OFF) //GM
        return false;

    // invisible aura
    if ((m_invisibilityMask || u->m_invisibilityMask) && !canDetectInvisibilityOf(u))
        return false;

    // unit got in stealth in this moment and must ignore old detected state
    //if (m_Visibility == VISIBILITY_GROUP_NO_DETECT)
    //    return false;

    // GM invisibility checks early, invisibility if any detectable, so if not stealth then visible
    if (u->GetVisibility() == VISIBILITY_GROUP_STEALTH)
    {
        //do not know what is the use of this detect
        if (!detect || !canDetectStealthOf(u, GetDistance(u)))
            return false;
    }

    // Now check is target visible with LoS
    //return u->IsWithinLOS(GetPositionX(), GetPositionY(), GetPositionZ());
    return true;
}

bool Creature::canStartAttack(Unit const* who) const
{
    if (isCivilian()
        || !who->isInAccessiblePlaceFor(this)
        || !canFly() && GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE
        || !IsWithinDistInMap(who, GetAttackDistance(who)))
        return false;

    if (!canAttack(who, false))
        return false;

    return IsWithinLOSInMap(who);
}

float Creature::GetAttackDistance(Unit const* pl) const
{
    float aggroRate = sWorld->getRate(RATE_CREATURE_AGGRO);
    if (aggroRate == 0)
        return 0.0f;

    int32 playerlevel   = pl->getLevelForTarget(this);
    int32 creaturelevel = getLevelForTarget(pl);

    int32 leveldif       = playerlevel - creaturelevel;

    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 30 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    if (leveldif < - 25)
        leveldif = -25;

    // "The aggro radius of a mob having the same level as the player is roughly 20 yards"
    float RetDistance = 20;

    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    RetDistance -= (float)leveldif;

    if (creaturelevel+5 <= sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        // detect range auras
        RetDistance += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);

        // detected range auras
        RetDistance += pl->GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);
    }

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if (RetDistance < 5)
        RetDistance = 5;

    return (RetDistance*aggroRate);
}

void Creature::setDeathState(DeathState s)
{
    if ((s == JUST_DIED && !m_isDeadByDefault)||(s == JUST_ALIVED && m_isDeadByDefault))
    {
        m_corpseRemoveTime = time(NULL) + m_corpseDelay;
        m_respawnTime = time(NULL) + m_respawnDelay + m_corpseDelay;

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld->getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY) || isWorldBoss())
            SaveRespawnTime();

        SetNoSearchAssistance(false);

        //Dismiss group if is leader
        if (m_formation && m_formation->getLeader() == this)
            m_formation->FormationReset(true);

        if (m_zoneScript)
            m_zoneScript->OnCreatureDeath(this);

        if (canFly() && FallGround())
            return;
    }
    Unit::setDeathState(s);

    if (s == JUST_DIED)
    {
        SetUInt64Value (UNIT_FIELD_TARGET, 0);               // remove target selection in any cases (can be set at aura remove in Unit::setDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, 0);
        //if (!isPet())
            setActive(false);

        if (!isPet() && GetCreatureTemplate()->SkinLootId)
            if (LootTemplates_Skinning.HaveLootFor(GetCreatureTemplate()->SkinLootId))
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        if (canFly() && FallGround())
            return;

        Unit::setDeathState(CORPSE);
    }
    if (s == JUST_ALIVED)
    {
        SetHealth(GetMaxHealth());
        SetLootRecipient(NULL);
        ResetPlayerDamageReq();
        Unit::setDeathState(ALIVE);
        CreatureTemplate const* cinfo = GetCreatureTemplate();
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        AddUnitMovementFlag(MOVEFLAG_WALK_MODE);
        if (GetCreatureTemplate()->InhabitType & INHABIT_AIR)
            AddUnitMovementFlag(MOVEFLAG_FLYING | MOVEFLAG_FLYING2);
        if (GetCreatureTemplate()->InhabitType & INHABIT_WATER)
            AddUnitMovementFlag(MOVEFLAG_SWIMMING);
        SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);
        clearUnitState(UNIT_STAT_ALL_STATE);
        i_motionMaster.Initialize();
        SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));
        LoadCreaturesAddon(true);
    }
}

bool Creature::FallGround()
{
    // Let's abort after we called this function one time
    if (getDeathState() == DEAD_FALLING)
        return false;

    float x, y, z;
    GetPosition(x, y, z);

    // use larger distance for vmap height search than in most other cases
    float ground_Z = GetMap()->GetHeight(x, y, z, true, MAX_FALL_DISTANCE);

    // Abort too if the ground is very near
    if (fabs(z - ground_Z) < 0.1f)
        return false;

    GetMotionMaster()->MoveFall(ground_Z, EVENT_FALL_GROUND);
    Unit::setDeathState(DEAD_FALLING);
    return true;
}

void Creature::Respawn(bool force)
{
    DestroyForNearbyPlayers();

    if (force)
    {
        if (isAlive())
            setDeathState(JUST_DIED);
        else if (getDeathState() != CORPSE)
            setDeathState(CORPSE);
    }

    RemoveCorpse(false);

    if (getDeathState() == DEAD)
    {
        if (m_DBTableGuid)
            sObjectMgr->SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);

        sLog->outDebug("Respawning...");
        m_respawnTime = 0;
        lootForPickPocketed = false;
        lootForBody         = false;

        if (m_originalEntry != GetEntry())
            UpdateEntry(m_originalEntry);

        CreatureTemplate const* cinfo = GetCreatureTemplate();
        SelectLevel(cinfo);

        if (m_isDeadByDefault)
        {
            setDeathState(JUST_DIED);
            SetHealth(0);
            i_motionMaster.Clear();
            clearUnitState(UNIT_STAT_ALL_STATE);
            LoadCreaturesAddon(true);
        }
        else
            setDeathState(JUST_ALIVED);

        //Call AI respawn virtual function
        AI()->JustRespawned();

        uint16 poolid = GetDBTableGUIDLow() ? sPoolMgr->IsPartOfAPool<Creature>(GetDBTableGUIDLow()) : 0;
        if (poolid)
            sPoolMgr->UpdatePool<Creature>(poolid, GetDBTableGUIDLow());
    }

    UpdateObjectVisibility();
}

void Creature::ForcedDespawn(uint32 timeMSToDespawn)
{
    if (timeMSToDespawn)
    {
        ForcedDespawnDelayEvent *pEvent = new ForcedDespawnDelayEvent(*this);

        m_Events.AddEvent(pEvent, m_Events.CalculateTime(timeMSToDespawn));
        return;
    }

    if (isAlive())
        setDeathState(JUST_DIED);

    RemoveCorpse(false);
}

bool Creature::IsImmunedToSpell(SpellEntry const* spellInfo, bool useCharges)
{
    if (!spellInfo)
        return false;

    if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->Mechanic - 1)))
        return true;

    return Unit::IsImmunedToSpell(spellInfo, useCharges);
}

bool Creature::IsImmunedToSpellEffect(SpellEntry const* spellInfo, uint32 index) const
{
    if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->EffectMechanic[index] - 1)))
        return true;

    return Unit::IsImmunedToSpellEffect(spellInfo, index);
}

SpellEntry const *Creature::reachWithSpellAttack(Unit *pVictim)
{
    if (!pVictim)
        return NULL;

    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        if (!m_spells[i])
            continue;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(m_spells[i]);
        if (!spellInfo)
        {
            sLog->outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j = 0; j < 3; j++)
        {
            if ((spellInfo->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE)        ||
                (spellInfo->Effect[j] == SPELL_EFFECT_INSTAKILL)            ||
                (spellInfo->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) ||
                (spellInfo->Effect[j] == SPELL_EFFECT_HEALTH_LEECH))
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue) continue;

        if (spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetSpellMaxRange(srange);
        float minrange = GetSpellMinRange(srange);
        float dist = GetDistance(pVictim);
        //if (!isInFront(pVictim, range) && spellInfo->AttributesEx)
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

SpellEntry const *Creature::reachWithSpellCure(Unit *pVictim)
{
    if (!pVictim)
        return NULL;

    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        if (!m_spells[i])
            continue;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(m_spells[i]);
        if (!spellInfo)
        {
            sLog->outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j = 0; j < 3; j++)
        {
            if ((spellInfo->Effect[j] == SPELL_EFFECT_HEAL))
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue) continue;

        if (spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetSpellMaxRange(srange);
        float minrange = GetSpellMinRange(srange);
        float dist = GetDistance(pVictim);
        //if (!isInFront(pVictim, range) && spellInfo->AttributesEx)
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

bool Creature::IsVisibleInGridForPlayer(Player const* pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if (pl->isGameMaster())
        return true;

    // Live player (or with not release body see live creatures or death creatures with corpse disappearing time > 0
    if (pl->isAlive() || pl->GetDeathTimer() > 0)
    {
        if (GetEntry() == VISUAL_WAYPOINT && !pl->isGameMaster())
            return false;
        return isAlive() || m_corpseRemoveTime > time(NULL) || m_isDeadByDefault && m_deathState == CORPSE;
    }

    // Dead player see creatures near own corpse
    Corpse *corpse = pl->GetCorpse();
    if (corpse)
    {
        // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
        if (corpse->IsWithinDistInMap(this, (20 + 25) * sWorld->getRate(RATE_CREATURE_AGGRO)))
            return true;
    }

    // Dead player see Spirit Healer or Spirit Guide
    if (isSpiritService())
        return true;

    // and not see any other
    return false;
}

void Creature::DoFleeToGetAssistance()
{
    if (!getVictim())
        return;

    if (HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    float radius = sWorld->getConfig(CONFIG_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS);
    if (radius >0)
    {
        Creature* creature = NULL;

        CellPair p(SkyFire::ComputeCellPair(GetPositionX(), GetPositionY()));
        Cell cell(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();
        SkyFire::NearestAssistCreatureInCreatureRangeCheck u_check(this, getVictim(), radius);
        SkyFire::CreatureLastSearcher<SkyFire::NearestAssistCreatureInCreatureRangeCheck> searcher(creature, u_check);

        TypeContainerVisitor<SkyFire::CreatureLastSearcher<SkyFire::NearestAssistCreatureInCreatureRangeCheck>, GridTypeMapContainer > grid_creature_searcher(searcher);

        cell.Visit(p, grid_creature_searcher, *GetMap(), *this, radius);

        SetNoSearchAssistance(true);
        if (!creature)
            SetControlled(true, UNIT_STAT_FLEEING);
        else
            GetMotionMaster()->MoveSeekAssistance(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
    }
}

Unit* Creature::SelectNearestTarget(float dist) const
{
    CellPair p(SkyFire::ComputeCellPair(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Unit *target = NULL;

    {
        SkyFire::NearestHostileUnitInAttackDistanceCheck u_check(this, dist);
        SkyFire::UnitLastSearcher<SkyFire::NearestHostileUnitInAttackDistanceCheck> searcher(target, u_check);

        TypeContainerVisitor<SkyFire::UnitLastSearcher<SkyFire::NearestHostileUnitInAttackDistanceCheck>, WorldTypeMapContainer > world_unit_searcher(searcher);
        TypeContainerVisitor<SkyFire::UnitLastSearcher<SkyFire::NearestHostileUnitInAttackDistanceCheck>, GridTypeMapContainer >  grid_unit_searcher(searcher);

        cell.Visit(p, world_unit_searcher, *GetMap(), *this, ATTACK_DISTANCE);
        cell.Visit(p, grid_unit_searcher, *GetMap(), *this, ATTACK_DISTANCE);
    }

    return target;
}

void Creature::CallAssistance()
{
    if (!m_AlreadyCallAssistance && getVictim() && !isPet() && !isCharmed())
    {
        SetNoCallAssistance(true);

        float radius = sWorld->getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS);
        if (radius > 0)
        {
            std::list<Creature*> assistList;

            {
                CellPair p(SkyFire::ComputeCellPair(GetPositionX(), GetPositionY()));
                Cell cell(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                SkyFire::AnyAssistCreatureInRangeCheck u_check(this, getVictim(), radius);
                SkyFire::CreatureListSearcher<SkyFire::AnyAssistCreatureInRangeCheck> searcher(assistList, u_check);

                TypeContainerVisitor<SkyFire::CreatureListSearcher<SkyFire::AnyAssistCreatureInRangeCheck>, GridTypeMapContainer >  grid_creature_searcher(searcher);

                cell.Visit(p, grid_creature_searcher, *GetMap(), *this, radius);
            }

            if (!assistList.empty())
            {
                AssistDelayEvent *e = new AssistDelayEvent(getVictim()->GetGUID(), *this);
                while (!assistList.empty())
                {
                    // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
                    e->AddAssistant((*assistList.begin())->GetGUID());
                    assistList.pop_front();
                }
                m_Events.AddEvent(e, m_Events.CalculateTime(sWorld->getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY)));
            }
        }
    }
}

void Creature::CallForHelp(float fRadius)
{
    if (fRadius <= 0.0f || !getVictim() || isPet() || isCharmed())
        return;

    CellPair p(SkyFire::ComputeCellPair(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    SkyFire::CallOfHelpCreatureInRangeDo u_do(this, getVictim(), fRadius);
    SkyFire::CreatureWorker<SkyFire::CallOfHelpCreatureInRangeDo> worker(u_do);

    TypeContainerVisitor<SkyFire::CreatureWorker<SkyFire::CallOfHelpCreatureInRangeDo>, GridTypeMapContainer >  grid_creature_searcher(worker);

    cell.Visit(p, grid_creature_searcher, *GetMap(), *this, fRadius);
}

bool Creature::CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction /*= true*/) const
{
    // is it true?
    if (!HasReactState(REACT_AGGRESSIVE))
        return false;

    // we don't need help from zombies :)
    if (!isAlive())
        return false;

    // skip fighting creature
    if (isInCombat())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getFaction() != u->getFaction())
            return false;
    }
    else
    {
        if (!IsFriendlyTo(u))
            return false;
    }

    // only free creature
    if (GetCharmerOrOwnerGUID())
        return false;

    // skip non hostile to caster enemy creatures
    if (!IsHostileTo(enemy))
        return false;

    return true;
}

void Creature::SaveRespawnTime()
{
    if (isPet() || !m_DBTableGuid || m_creatureData && !m_creatureData->dbData)
        return;

    sObjectMgr->SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), m_respawnTime);
}

bool Creature::IsOutOfThreatArea(Unit* pVictim) const
{
    if (!pVictim)
        return true;

    if (!pVictim->IsInMap(this))
        return true;

    if (!pVictim->isTargetableForAttack())
        return true;

    if (!pVictim->isInAccessiblePlaceFor(this))
        return true;

    if (!pVictim->isVisibleForOrDetect(this, this, false))
        return true;

    if (sMapStore.LookupEntry(GetMapId())->IsDungeon())
        return false;

    float length = pVictim->GetDistance(m_homePosition);
    float AttackDist = GetAttackDistance(pVictim);
    uint32 ThreatRadius = sWorld->getConfig(CONFIG_THREAT_RADIUS);

    //Use AttackDistance in distance check if threat radius is lower. This prevents creature bounce in and out of combat every update tick.
    return (length > (ThreatRadius > AttackDist ? ThreatRadius : AttackDist));
}

CreatureDataAddon const* Creature::GetCreatureAddon() const
{
    if (m_DBTableGuid)
    {
        if (CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(m_DBTableGuid))
            return addon;
    }

    // dependent from heroic mode entry
    return ObjectMgr::GetCreatureTemplateAddon(GetCreatureTemplate()->Entry);
}

//creature_addon table
bool Creature::LoadCreaturesAddon(bool reload)
{
    CreatureDataAddon const *cainfo = GetCreatureAddon();
    if (!cainfo)
        return false;

    if (cainfo->mount != 0)
        Mount(cainfo->mount);

    if (cainfo->bytes0 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);

    if (cainfo->bytes1 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_1, cainfo->bytes1);

    if (cainfo->bytes2 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_2, cainfo->bytes2);

    if (cainfo->emote != 0)
        SetUInt32Value(UNIT_NPC_EMOTESTATE, cainfo->emote);

    if (cainfo->move_flags != 0)
        SetUnitMovementFlags(cainfo->move_flags);

    //Load Path
    if (cainfo->path_id != 0)
        m_path_id = cainfo->path_id;

    if (cainfo->isActive)
        setActive(true);

    if (cainfo->auras)
    {
        for (CreatureDataAddonAura const* cAura = cainfo->auras; cAura->spell_id; ++cAura)
        {
            SpellEntry const *AdditionalSpellInfo = sSpellStore.LookupEntry(cAura->spell_id);
            if (!AdditionalSpellInfo)
            {
                sLog->outErrorDb("Creature (GUIDLow: %u Entry: %u) has wrong spell %u defined in auras field.", GetGUIDLow(), GetEntry(), cAura->spell_id);
                continue;
            }

            // skip already applied aura
            if (HasAura(cAura->spell_id, cAura->effect_idx))
            {
                if (!reload)
                    sLog->outErrorDb("Creature (GUIDLow: %u Entry: %u) has duplicate aura (spell %u effect %u) in auras field.", GetGUIDLow(), GetEntry(), cAura->spell_id, cAura->effect_idx);

                continue;
            }

            Aura* AdditionalAura = CreateAura(AdditionalSpellInfo, cAura->effect_idx, NULL, this, this, 0);
            AddAura(AdditionalAura);
            sLog->outDebug("Spell: %u with Aura %u added to creature (GUIDLow: %u Entry: %u)", cAura->spell_id, AdditionalSpellInfo->EffectApplyAuraName[0], GetGUIDLow(), GetEntry());
        }
    }
    return true;
}

// Send a message to LocalDefense channel for players opposition team in the zone
void Creature::SendZoneUnderAttackMessage(Player* attacker)
{
    uint32 enemy_team = attacker->GetTeam();

    WorldPacket data(SMSG_ZONE_UNDER_ATTACK, 4);
    data << (uint32)GetZoneId();
    sWorld->SendGlobalMessage(&data, NULL, (enemy_team == ALLIANCE ? HORDE : ALLIANCE));
}

void Creature::SetInCombatWithZone()
{
    if (!CanHaveThreatList())
    {
        sLog->outError("Creature entry %u call SetInCombatWithZone but creature cannot have threat list.", GetEntry());
        return;
    }

    Map* pMap = GetMap();

    if (!pMap->IsDungeon())
    {
        sLog->outError("Creature entry %u call SetInCombatWithZone for map (id: %u) that isn't an instance.", GetEntry(), pMap->GetId());
        return;
    }

    Map::PlayerList const &PlList = pMap->GetPlayers();

    if (PlList.isEmpty())
        return;

    for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
    {
        if (Player* player = i->getSource())
        {
            if (player->isGameMaster())
                continue;

            if (player->isAlive())
            {
                player->SetInCombatWith(this);
                AddThreat(player, 0.0f);
            }
        }
    }
}

void Creature::_AddCreatureSpellCooldown(uint32 spell_id, time_t end_time)
{
    m_CreatureSpellCooldowns[spell_id] = end_time;
}

void Creature::_AddCreatureCategoryCooldown(uint32 category, time_t apply_time)
{
    m_CreatureCategoryCooldowns[category] = apply_time;
}

void Creature::AddCreatureSpellCooldown(uint32 spellid)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellid);
    if (!spellInfo)
        return;

    uint32 cooldown = GetSpellRecoveryTime(spellInfo);
    if (cooldown)
        _AddCreatureSpellCooldown(spellid, time(NULL) + cooldown/IN_MILLISECONDS);

    if (spellInfo->Category)
        _AddCreatureCategoryCooldown(spellInfo->Category, time(NULL));

    m_GlobalCooldown = spellInfo->StartRecoveryTime;
}

bool Creature::HasCategoryCooldown(uint32 spell_id) const
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell_id);
    if (!spellInfo)
        return false;

    // check global cooldown if spell affected by it
    if (spellInfo->StartRecoveryCategory > 0 && m_GlobalCooldown > 0)
        return true;

    CreatureSpellCooldowns::const_iterator itr = m_CreatureCategoryCooldowns.find(spellInfo->Category);
    return(itr != m_CreatureCategoryCooldowns.end() && time_t(itr->second + (spellInfo->CategoryRecoveryTime / IN_MILLISECONDS)) > time(NULL));
}

bool Creature::HasSpellCooldown(uint32 spell_id) const
{
    CreatureSpellCooldowns::const_iterator itr = m_CreatureSpellCooldowns.find(spell_id);
    return (itr != m_CreatureSpellCooldowns.end() && itr->second > time(NULL)) || HasCategoryCooldown(spell_id);
}

bool Creature::IsInEvadeMode() const
{
    return /*!i_motionMaster.empty() &&*/ i_motionMaster.GetCurrentMovementGeneratorType() == HOME_MOTION_TYPE;
}

bool Creature::HasSpell(uint32 spellID) const
{
    uint8 i;
    for (i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if (spellID == m_spells[i])
            break;
    return i < CREATURE_MAX_SPELLS;                         //broke before end of iteration of known spells
}

time_t Creature::GetRespawnTimeEx() const
{
    time_t now = time(NULL);
    if (m_respawnTime > now)
        return m_respawnTime;
    else
        return now;
}

void Creature::GetRespawnCoord(float &x, float &y, float &z, float* ori, float* dist) const
{
    if (m_DBTableGuid)
    {
        if (CreatureData const* data = sObjectMgr->GetCreatureData(GetDBTableGUIDLow()))
        {
            x = data->posX;
            y = data->posY;
            z = data->posZ;
            if (ori)
                *ori = data->orientation;
            if (dist)
                *dist = data->spawndist;

            return;
        }
    }

    x = GetPositionX();
    y = GetPositionY();
    z = GetPositionZ();
    if (ori)
        *ori = GetOrientation();
    if (dist)
        *dist = 0;
}

void Creature::AllLootRemovedFromCorpse()
{
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        time_t now = time(NULL);
        if (m_corpseRemoveTime <= now)
            return;

        float decayRate;
        CreatureTemplate const* cinfo = GetCreatureTemplate();

        // corpse was not skinnable -> apply corpse looted timer
        if (!cinfo || !cinfo->SkinLootId)
            decayRate = sWorld->getRate(RATE_CORPSE_DECAY_LOOTED);
        // corpse skinnable, but without skinning flag, and then skinned, corpse will despawn next update
        else
            decayRate = 0.0f;

        uint32 diff = (m_corpseRemoveTime - now) * decayRate;

        m_corpseRemoveTime -= diff;
        m_respawnTime -= diff;
    }
}

uint32 Creature::getLevelForTarget(Unit const* target) const
{
    if (!isWorldBoss())
        return Unit::getLevelForTarget(target);

    uint32 level = target->getLevel()+sWorld->getConfig(CONFIG_WORLD_BOSS_LEVEL_DIFF);
    if (level < 1)
        return 1;
    if (level > 255)
        return 255;
    return level;
}

std::string Creature::GetAIName() const
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->AIName;
}

std::string Creature::GetScriptName()
{
    return sObjectMgr->GetScriptName(GetScriptId());
}

uint32 Creature::GetScriptId()
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->ScriptID;
}

VendorItemData const* Creature::GetVendorItems() const
{
    return sObjectMgr->GetNpcVendorItemList(GetEntry());
}

uint32 Creature::GetVendorItemCurrentCount(VendorItem const* vItem)
{
    if (!vItem->maxcount)
        return vItem->maxcount;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
        return vItem->maxcount;

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if (vCount->lastIncrementTime + vItem->incrtime <= ptime)
    {
        ItemPrototype const* pProto = sObjectMgr->GetItemPrototype(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) >= vItem->maxcount)
        {
            m_vendorItemCounts.erase(itr);
            return vItem->maxcount;
        }

        vCount->count += diff * pProto->BuyCount;
        vCount->lastIncrementTime = ptime;
    }

    return vCount->count;
}

uint32 Creature::UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count)
{
    if (!vItem->maxcount)
        return 0;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
    {
        uint32 new_count = vItem->maxcount > used_count ? vItem->maxcount-used_count : 0;
        m_vendorItemCounts.push_back(VendorItemCount(vItem->item, new_count));
        return new_count;
    }

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if (vCount->lastIncrementTime + vItem->incrtime <= ptime)
    {
        ItemPrototype const* pProto = sObjectMgr->GetItemPrototype(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) < vItem->maxcount)
            vCount->count += diff * pProto->BuyCount;
        else
            vCount->count = vItem->maxcount;
    }

    vCount->count = vCount->count > used_count ? vCount->count-used_count : 0;
    vCount->lastIncrementTime = ptime;
    return vCount->count;
}

TrainerSpellData const* Creature::GetTrainerSpells() const
{
    return sObjectMgr->GetNpcTrainerSpells(GetEntry());
}

// overwrite WorldObject function for proper name localization
const char* Creature::GetNameForLocaleIdx(int32 loc_idx) const
{
    if (loc_idx >= 0)
    {
        CreatureLocale const *cl = sObjectMgr->GetCreatureLocale(GetEntry());
        if (cl)
        {
            if (cl->Name.size() > loc_idx && !cl->Name[loc_idx].empty())
                return cl->Name[loc_idx].c_str();
        }
    }

    return GetName();
}

const CreatureData* Creature::GetLinkedRespawnCreatureData() const
{
    if (!m_DBTableGuid) // only hard-spawned creatures from DB can have a linked master
        return NULL;

    if (uint32 targetGuid = sObjectMgr->GetLinkedRespawnGuid(m_DBTableGuid))
        return sObjectMgr->GetCreatureData(targetGuid);

    return NULL;
}

// returns master's remaining respawn time if any
time_t Creature::GetLinkedCreatureRespawnTime() const
{
    if (!m_DBTableGuid) // only hard-spawned creatures from DB can have a linked master
        return 0;

    if (uint32 targetGuid = sObjectMgr->GetLinkedRespawnGuid(m_DBTableGuid))
    {
        Map* targetMap = NULL;
        if (const CreatureData* data = sObjectMgr->GetCreatureData(targetGuid))
        {
            if (data->mapid == GetMapId())   // look up on the same map
                targetMap = GetMap();
            else                            // it shouldn't be instanceable map here
                targetMap = sMapMgr->FindMap(data->mapid);
        }
        if (targetMap)
            return sObjectMgr->GetCreatureRespawnTime(targetGuid, targetMap->GetInstanceId());
    }

    return 0;
}

