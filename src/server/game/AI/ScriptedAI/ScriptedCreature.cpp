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

#include "ScriptPCH.h"
#include "Item.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"

// Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} *SpellSummary;

void SummonList::DoZoneInCombat(uint32 entry)
{
    for (iterator i = begin(); i != end();)
    {
        Creature *summon = Unit::GetCreature(*me, *i);
        ++i;
        if (summon && summon->IsAIEnabled
            && (!entry || summon->GetEntry() == entry))
            summon->AI()->DoZoneInCombat();
    }
}

void SummonList::DoAction(uint32 entry, uint32 info)
{
    for (iterator i = begin(); i != end();)
    {
        Creature *summon = Unit::GetCreature(*me, *i);
        ++i;
        if (summon && summon->IsAIEnabled
            && (!entry || summon->GetEntry() == entry))
            summon->AI()->DoAction(info);
    }
}

void SummonList::DespawnEntry(uint32 entry)
{
    for (iterator i = begin(); i != end();)
    {
        Creature *summon = Unit::GetCreature(*me, *i);
        if (!summon)
            erase(i++);
        else if (summon->GetEntry() == entry)
        {
            erase(i++);
            summon->setDeathState(JUST_DIED);
            summon->RemoveCorpse();
        }
        else
            ++i;
    }
}

void SummonList::DespawnAll()
{
    while (!empty())
    {
        Creature *summon = Unit::GetCreature(*me, *begin());
        if (!summon)
            erase(begin());
        else
        {
            erase(begin());
            if (summon->isSummon())
            {
                summon->DestroyForNearbyPlayers();
                CAST_SUM(summon)->UnSummon();
            }
            else
                summon->DisappearAndDie();
        }
    }
}

ScriptedAI::ScriptedAI(Creature* creature) : CreatureAI(creature),
    me(creature),
    IsFleeing(false),
    m_bCombatMovement(true),
    m_uiEvadeCheckCooldown(2500)
{
    HeroicMode = me->GetMap()->IsHeroic();
}

void ScriptedAI::AttackStartNoMove(Unit* pWho)
{
    if (!pWho)
        return;

    if (me->Attack(pWho, false))
        DoStartNoMovement(pWho);
}

void ScriptedAI::UpdateAI(const uint32 /*uiDiff*/)
{
    //Check if we have a current target
    if (!UpdateVictim())
        return;

    if (me->isAttackReady())
    {
        //If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim()))
        {
            me->AttackerStateUpdate(me->getVictim());
            me->resetAttackTimer();
        }
    }
}

void ScriptedAI::DoStartMovement(Unit* pVictim, float fDistance, float fAngle)
{
    if (pVictim)
        me->GetMotionMaster()->MoveChase(pVictim, fDistance, fAngle);
}

void ScriptedAI::DoStartNoMovement(Unit* pVictim)
{
    if (!pVictim)
        return;

    me->GetMotionMaster()->MoveIdle();
}

void ScriptedAI::DoStopAttack()
{
    if (me->getVictim())
        me->AttackStop();
}

void ScriptedAI::DoCastSpell(Unit* pTarget, SpellEntry const* pSpellInfo, bool bTriggered)
{
    if (!pTarget || me->IsNonMeleeSpellCasted(false))
        return;

    me->StopMoving();
    me->CastSpell(pTarget, pSpellInfo, bTriggered);
}

void ScriptedAI::DoPlaySoundToSet(WorldObject* pSource, uint32 uiSoundId)
{
    if (!pSource)
        return;

    if (!GetSoundEntriesStore()->LookupEntry(uiSoundId))
    {
        sLog->outError("TSCR: Invalid soundId %u used in DoPlaySoundToSet (Source: TypeId %u, GUID %u)", uiSoundId, pSource->GetTypeId(), pSource->GetGUIDLow());
        return;
    }

    pSource->PlayDirectSound(uiSoundId);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 uiId, float fX, float fY, float fZ, float fAngle, uint32 uiType, uint32 uiDespawntime)
{
    return me->SummonCreature(uiId, me->GetPositionX()+fX, me->GetPositionY()+fY, me->GetPositionZ()+fZ, fAngle, (TempSummonType)uiType, uiDespawntime);
}

Unit* ScriptedAI::SelectUnit(SelectAggroTarget pTarget, uint32 uiPosition)
{
    //ThreatList m_threatlist;
    std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator itr = threatlist.begin();
    std::list<HostileReference*>::reverse_iterator ritr = threatlist.rbegin();

    if (uiPosition >= threatlist.size() || !threatlist.size())
        return NULL;

    switch (pTarget)
    {
    case SELECT_TARGET_RANDOM:
        advance (itr , uiPosition +  (rand() % (threatlist.size() - uiPosition)));
        return Unit::GetUnit((*me), (*itr)->getUnitGuid());
        break;

    case SELECT_TARGET_TOPAGGRO:
        advance (itr , uiPosition);
        return Unit::GetUnit((*me), (*itr)->getUnitGuid());
        break;

    case SELECT_TARGET_BOTTOMAGGRO:
        advance (ritr , uiPosition);
        return Unit::GetUnit((*me), (*ritr)->getUnitGuid());
        break;

    default:
        return UnitAI::SelectTarget(pTarget, uiPosition);
    }
}

SpellEntry const* ScriptedAI::SelectSpell(Unit* pTarget, uint32 uiSchool, uint32 uiMechanic, SelectTargetType selectTargets, uint32 uiPowerCostMin, uint32 uiPowerCostMax, float fRangeMin, float fRangeMax, SelectEffect selectEffects)
{
    //No target so we can't cast
    if (!pTarget)
        return false;

    //Silenced so we can't cast
    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    //Using the extended script system we first create a list of viable spells
    SpellEntry const* apSpell[CREATURE_MAX_SPELLS];
    memset(apSpell, 0, sizeof(SpellEntry*)*CREATURE_MAX_SPELLS);

    uint32 uiSpellCount = 0;

    SpellEntry const* pTempSpell;
    SpellRangeEntry const* pTempRange;

    //Check if each spell is viable(set it to null if not)
    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        pTempSpell = GetSpellStore()->LookupEntry(me->m_spells[i]);

        //This spell doesn't exist
        if (!pTempSpell)
            continue;

        // Targets and Effects checked first as most used restrictions
        //Check the spell targets if specified
        if (selectTargets && !(SpellSummary[me->m_spells[i]].Targets & (1 << (selectTargets-1))))
            continue;

        //Check the type of spell if we are looking for a specific spell type
        if (selectEffects && !(SpellSummary[me->m_spells[i]].Effects & (1 << (selectEffects-1))))
            continue;

        //Check for school if specified
        if (uiSchool && (pTempSpell->SchoolMask & uiSchool) == 0)
            continue;

        //Check for spell mechanic if specified
        if (uiMechanic && pTempSpell->Mechanic != uiMechanic)
            continue;

        //Make sure that the spell uses the requested amount of power
        if (uiPowerCostMin && pTempSpell->manaCost < uiPowerCostMin)
            continue;

        if (uiPowerCostMax && pTempSpell->manaCost > uiPowerCostMax)
            continue;

        //Continue if we don't have the mana to actually cast this spell
        if (pTempSpell->manaCost > me->GetPower((Powers)pTempSpell->powerType))
            continue;

        //Get the Range
        pTempRange = GetSpellRangeStore()->LookupEntry(pTempSpell->rangeIndex);

        //Spell has invalid range store so we can't use it
        if (!pTempRange)
            continue;

        //Check if the spell meets our range requirements
        if (fRangeMin && pTempRange->maxRange < fRangeMin)
            continue;
        if (fRangeMax && pTempRange->maxRange > fRangeMax)
            continue;

        //Check if our target is in range
         if (me->IsWithinDistInMap(pTarget, pTempRange->minRange) || !me->IsWithinDistInMap(pTarget, pTempRange->maxRange))
            continue;

        //All good so lets add it to the spell list
        apSpell[uiSpellCount] = pTempSpell;
        ++uiSpellCount;
    }

    //We got our usable spells so now lets randomly pick one
    if (!uiSpellCount)
        return NULL;

    return apSpell[rand()%uiSpellCount];
}

bool ScriptedAI::CanCast(Unit* pTarget, SpellEntry const* pSpell, bool bTriggered)
{
    //No target so we can't cast
    if (!pTarget || !pSpell)
        return false;

    //Silenced so we can't cast
    if (!bTriggered && me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    //Check for power
    if (!bTriggered && me->GetPower((Powers)pSpell->powerType) < pSpell->manaCost)
        return false;

    SpellRangeEntry const* pTempRange = GetSpellRangeStore()->LookupEntry(pSpell->rangeIndex);

    //Spell has invalid range store so we can't use it
    if (!pTempRange)
        return false;

    //Unit is out of range of this spell
    if (me->IsInRange(pTarget, pTempRange->minRange, pTempRange->maxRange))
        return false;

    return true;
}

void FillSpellSummary()
{
    SpellSummary = new TSpellSummary[GetSpellStore()->GetNumRows()];

    SpellEntry const* pTempSpell;

    for (uint32 i = 0; i < GetSpellStore()->GetNumRows(); ++i)
    {
        SpellSummary[i].Effects = 0;
        SpellSummary[i].Targets = 0;

        pTempSpell = GetSpellStore()->LookupEntry(i);
        //This spell doesn't exist
        if (!pTempSpell)
            continue;

        for (uint32 j = 0; j < 3; ++j)
        {
            //Spell targets self
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SELF-1);

            //Spell targets a single enemy
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ENEMY ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_DST_TARGET_ENEMY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_ENEMY-1);

            //Spell targets AoE at enemy
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_SRC ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_DST ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_DEST_DYNOBJ_ENEMY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_ENEMY-1);

            //Spell targets an enemy
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ENEMY ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_DST_TARGET_ENEMY ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_SRC ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_DST ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_DEST_DYNOBJ_ENEMY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_ENEMY-1);

            //Spell targets a single friend(or self)
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ALLY ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_PARTY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_FRIEND-1);

            //Spell targets aoe friends
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_CASTER ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_TARGET ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_FRIEND-1);

            //Spell targets any friend(or self)
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ALLY ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_PARTY ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_CASTER ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_TARGET ||
                pTempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_FRIEND-1);

            //Make sure that this spell includes a damage effect
            if (pTempSpell->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE ||
                pTempSpell->Effect[j] == SPELL_EFFECT_INSTAKILL ||
                pTempSpell->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
                pTempSpell->Effect[j] == SPELL_EFFECT_HEALTH_LEECH)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_DAMAGE-1);

            //Make sure that this spell includes a healing effect (or an apply aura with a periodic heal)
            if (pTempSpell->Effect[j] == SPELL_EFFECT_HEAL ||
                pTempSpell->Effect[j] == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                pTempSpell->Effect[j] == SPELL_EFFECT_HEAL_MECHANICAL ||
                (pTempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA  && pTempSpell->EffectApplyAuraName[j] == 8))
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_HEALING-1);

            //Make sure that this spell applies an aura
            if (pTempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_AURA-1);
        }
    }
}

void ScriptedAI::DoResetThreat()
{
    if (!me->CanHaveThreatList() || me->getThreatManager().isThreatListEmpty())
    {
        sLog->outError("TSCR: DoResetThreat called for creature that either cannot have threat list or has empty threat list (me entry = %d)", me->GetEntry());
        return;
    }

    std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();

    for (std::list<HostileReference*>::iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
    {
        Unit* pUnit = Unit::GetUnit((*me), (*itr)->getUnitGuid());

        if (pUnit && DoGetThreat(pUnit))
            DoModifyThreatPercent(pUnit, -100);
    }
}

float ScriptedAI::DoGetThreat(Unit* pUnit)
{
    if (!pUnit) return 0.0f;
    return me->getThreatManager().getThreat(pUnit);
}

void ScriptedAI::DoModifyThreatPercent(Unit* pUnit, int32 pct)
{
    if (!pUnit) return;
    me->getThreatManager().modifyThreatPercent(pUnit, pct);
}

void ScriptedAI::DoTeleportTo(float fX, float fY, float fZ, uint32 uiTime)
{
    me->Relocate(fX, fY, fZ);
    me->SendMonsterMove(fX, fY, fZ, uiTime);
}

void ScriptedAI::DoTeleportTo(const float fPos[4])
{
    me->NearTeleportTo(fPos[0], fPos[1], fPos[2], fPos[3]);
}

void ScriptedAI::DoTeleportPlayer(Unit* pUnit, float fX, float fY, float fZ, float fO)
{
    if (!pUnit || pUnit->GetTypeId() != TYPEID_PLAYER)
    {
        if (pUnit)
            sLog->outError("TSCR: Creature %u (Entry: %u) Tried to teleport non-player unit (Type: %u GUID: %u) to x: %f y:%f z: %f o: %f. Aborted.", me->GetGUID(), me->GetEntry(), pUnit->GetTypeId(), pUnit->GetGUID(), fX, fY, fZ, fO);
        return;
    }

    CAST_PLR(pUnit)->TeleportTo(pUnit->GetMapId(), fX, fY, fZ, fO, TELE_TO_NOT_LEAVE_COMBAT);
}

void ScriptedAI::DoTeleportAll(float fX, float fY, float fZ, float fO)
{
    Map *map = me->GetMap();
    if (!map->IsDungeon())
        return;

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        if (Player* i_pl = i->getSource())
            if (i_pl->isAlive())
                i_pl->TeleportTo(me->GetMapId(), fX, fY, fZ, fO, TELE_TO_NOT_LEAVE_COMBAT);
}

Unit* ScriptedAI::DoSelectLowestHpFriendly(float fRange, uint32 uiMinHPDiff)
{
    Unit* pUnit = NULL;
    SkyFire::MostHPMissingInRange u_check(me, fRange, uiMinHPDiff);
    SkyFire::UnitLastSearcher<SkyFire::MostHPMissingInRange> searcher(pUnit, u_check);
    me->VisitNearbyObject(fRange, searcher);

    return pUnit;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyCC(float fRange)
{
    std::list<Creature*> pList;
    SkyFire::FriendlyCCedInRange u_check(me, fRange);
    SkyFire::CreatureListSearcher<SkyFire::FriendlyCCedInRange> searcher(pList, u_check);
    me->VisitNearbyObject(fRange, searcher);
    return pList;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyMissingBuff(float fRange, uint32 uiSpellid)
{
    std::list<Creature*> pList;
    SkyFire::FriendlyMissingBuffInRange u_check(me, fRange, uiSpellid);
    SkyFire::CreatureListSearcher<SkyFire::FriendlyMissingBuffInRange> searcher(pList, u_check);
    me->VisitNearbyObject(fRange, searcher);
    return pList;
}

Player* ScriptedAI::GetPlayerAtMinimumRange(float fMinimumRange)
{
    Player* player = NULL;

    CellPair pair(SkyFire::ComputeCellPair(me->GetPositionX(), me->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    SkyFire::PlayerAtMinimumRangeAway check(me, fMinimumRange);
    SkyFire::PlayerSearcher<SkyFire::PlayerAtMinimumRangeAway> searcher(player, check);
    TypeContainerVisitor<SkyFire::PlayerSearcher<SkyFire::PlayerAtMinimumRangeAway>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *(me->GetMap()));

    return player;
}

void ScriptedAI::SetEquipmentSlots(bool bLoadDefault, int32 uiMainHand, int32 uiOffHand, int32 uiRanged)
{
    if (bLoadDefault)
    {
        if (CreatureTemplate const* pInfo = GetCreatureTemplateStore(me->GetEntry()))
            me->LoadEquipment(pInfo->equipmentId, true);

        return;
    }

    if (uiMainHand >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0, uint32(uiMainHand));

    if (uiOffHand >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, uint32(uiOffHand));

    if (uiRanged >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2, uint32(uiRanged));
}

void ScriptedAI::SetCombatMovement(bool bCombatMove)
{
    m_bCombatMovement = bCombatMove;
}

enum eNPCs
{
    NPC_BROODLORD   = 12017,
    NPC_VOID_REAVER = 19516,
    NPC_JAN_ALAI    = 23578,
    NPC_SARTHARION  = 28860
};

// Hacklike storage used for misc creatures that are expected to evade of outside of a certain area.
// It is assumed the information is found elswehere and can be handled by the core. So far no luck finding such information/way to extract it.
bool ScriptedAI::EnterEvadeIfOutOfCombatArea(const uint32 uiDiff)
{
    if (m_uiEvadeCheckCooldown <= uiDiff)
        m_uiEvadeCheckCooldown = 2500;
    else
    {
        m_uiEvadeCheckCooldown -= uiDiff;
        return false;
    }

    if (me->IsInEvadeMode() || !me->getVictim())
        return false;

    float fX = me->GetPositionX();
    float fY = me->GetPositionY();
    float fZ = me->GetPositionZ();

    switch (me->GetEntry())
    {
        case NPC_BROODLORD:                                         // broodlord (not move down stairs)
            if (fZ > 448.60f)
                return false;
            break;
        case NPC_VOID_REAVER:                                         // void reaver (calculate from center of room)
            if (me->GetDistance2d(432.59f, 371.93f) < 105.0f)
                return false;
            break;
        case NPC_JAN_ALAI:                                         // jan'alai (calculate by Z)
            if (fZ > 12.0f)
                return false;
            break;
        case NPC_SARTHARION:                                         // sartharion (calculate box)
            if (fX > 3218.86f && fX < 3275.69f && fY < 572.40f && fY > 484.68f)
                return false;
            break;
        default:
            sLog->outError("TSCR: EnterEvadeIfOutOfCombatArea used for creature entry %u, but does not have any definition.", me->GetEntry());
            return false;
    }

    EnterEvadeMode();
    return true;
}

void Scripted_NoMovementAI::AttackStart(Unit* pWho)
{
    if (!pWho)
        return;

    if (me->Attack(pWho, true))
    {
        DoStartNoMovement(pWho);
    }
}

BossAI::BossAI(Creature *c, uint32 id) : ScriptedAI(c)
, bossId(id), summons(me), instance(c->GetInstanceScript())
{
}

void BossAI::_Reset()
{
    if (!me->isAlive())
        return;

    events.Reset();
    summons.DespawnAll();
    if (instance)
        instance->SetBossState(bossId, NOT_STARTED);
}

void BossAI::_JustDied()
{
    events.Reset();
    summons.DespawnAll();
    if (instance)
    {
        instance->SetBossState(bossId, DONE);
        instance->SaveToDB();
    }
}

void BossAI::_EnterCombat()
{
    me->setActive(true);
    DoZoneInCombat();
    if (instance)
        instance->SetBossState(bossId, IN_PROGRESS);
}

void BossAI::JustSummoned(Creature *summon)
{
    summons.Summon(summon);
    if (me->isInCombat())
        DoZoneInCombat(summon);
}

void BossAI::SummonedCreatureDespawn(Creature *summon)
{
    summons.Despawn(summon);
}

#define GOBJECT(x) (const_cast<GameObjectInfo*>(GetGameObjectInfo(x)))

void LoadOverridenSQLData()
{
    GameObjectInfo *goInfo;

    // Sunwell Plateau : Kalecgos : Spectral Rift
    goInfo = GOBJECT(187055);
    if (goInfo)
        if (goInfo->type == GAMEOBJECT_TYPE_GOOBER)
            goInfo->goober.lockId = 57; // need LOCKTYPE_QUICK_OPEN

    // Naxxramas : Sapphiron Birth
    goInfo = GOBJECT(181356);
    if (goInfo)
        if (goInfo->type == GAMEOBJECT_TYPE_TRAP)
            goInfo->trap.radius = 50;
}

// SD2 grid searchers.
Creature *GetClosestCreatureWithEntry(WorldObject *pSource, uint32 uiEntry, float fMaxSearchRange, bool bAlive)
{
    return pSource->FindNearestCreature(uiEntry, fMaxSearchRange, bAlive);
}
GameObject *GetClosestGameObjectWithEntry(WorldObject *pSource, uint32 uiEntry, float fMaxSearchRange)
{
    return pSource->FindNearestGameObject(uiEntry, fMaxSearchRange);
}
void GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, WorldObject *pSource, uint32 uiEntry, float fMaxSearchRange)
{
    return pSource->GetCreatureListWithEntryInGrid(lList, uiEntry, fMaxSearchRange);
}
void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList, WorldObject *pSource, uint32 uiEntry, float fMaxSearchRange)
{
    return pSource->GetGameObjectListWithEntryInGrid(lList, uiEntry, fMaxSearchRange);
}

