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

#include "ScriptPCH.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "ObjectMgr.h"

#include "ScriptLoader.h"
#include "ScriptSystem.h"

int num_sc_scripts;
Script *m_scripts[MAX_SCRIPTS];

void FillSpellSummary();
void LoadOverridenSQLData();

void ScriptMgr::LoadDatabase()
{
    pSystemMgr.LoadScriptTexts();
    pSystemMgr.LoadScriptTextsCustom();
    pSystemMgr.LoadScriptWaypoints();
}

struct TSpellSummary {
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
}extern *SpellSummary;

ScriptMgr::ScriptMgr()
{
}

ScriptMgr::~ScriptMgr()
{
    // Free Spell Summary
    delete []SpellSummary;

    // Free resources before library unload
    for (uint16 i =0; i < MAX_SCRIPTS; ++i)
        delete m_scripts[i];

    num_sc_scripts = 0;
}

void ScriptMgr::ScriptsInit()
{
    //Load database (must be called after SD2Config.SetSource).
    LoadDatabase();

    sLog->outString("TSCR: Loading C++ scripts");
    sLog->outString("");

    for (uint16 i =0; i < MAX_SCRIPTS; ++i)
        m_scripts[i]=NULL;

    FillSpellSummary();

    AddScripts();

    sLog->outString(">> Loaded %i C++ Scripts.", num_sc_scripts);

    sLog->outString(">> Load Overriden SQL Data.");
    LoadOverridenSQLData();
}

//*********************************
//*** Functions used globally ***

void DoScriptText(int32 iTextEntry, WorldObject* pSource, Unit* pTarget)
{
    if (!pSource)
    {
        sLog->outError("TSCR: DoScriptText entry %i, invalid Source pointer.", iTextEntry);
        return;
    }

    if (iTextEntry >= 0)
    {
        sLog->outError("TSCR: DoScriptText with source entry %u (TypeId=%u, guid=%u) attempts to process text entry %i, but text entry must be negative.", pSource->GetEntry(), pSource->GetTypeId(), pSource->GetGUIDLow(), iTextEntry);
        return;
    }

    const StringTextData* pData = pSystemMgr.GetTextData(iTextEntry);

    if (!pData)
    {
        sLog->outError("TSCR: DoScriptText with source entry %u (TypeId=%u, guid=%u) could not find text entry %i.", pSource->GetEntry(), pSource->GetTypeId(), pSource->GetGUIDLow(), iTextEntry);
        return;
    }

    sLog->outDebug("TSCR: DoScriptText: text entry=%i, Sound=%u, Type=%u, Language=%u, Emote=%u", iTextEntry, pData->uiSoundId, pData->uiType, pData->uiLanguage, pData->uiEmote);

    if (pData->uiSoundId)
    {
        if (GetSoundEntriesStore()->LookupEntry(pData->uiSoundId))
        {
            pSource->SendPlaySound(pData->uiSoundId, false);
        }
        else
            sLog->outError("TSCR: DoScriptText entry %i tried to process invalid sound id %u.", iTextEntry, pData->uiSoundId);
    }

    if (pData->uiEmote)
    {
        if (pSource->GetTypeId() == TYPEID_UNIT || pSource->GetTypeId() == TYPEID_PLAYER)
            ((Unit*)pSource)->HandleEmoteCommand(pData->uiEmote);
        else
            sLog->outError("TSCR: DoScriptText entry %i tried to process emote for invalid TypeId (%u).", iTextEntry, pSource->GetTypeId());
    }

    switch (pData->uiType)
    {
        case CHAT_TYPE_SAY:
            pSource->MonsterSay(iTextEntry, pData->uiLanguage, pTarget ? pTarget->GetGUID() : 0);
            break;
        case CHAT_TYPE_YELL:
            pSource->MonsterYell(iTextEntry, pData->uiLanguage, pTarget ? pTarget->GetGUID() : 0);
            break;
        case CHAT_TYPE_TEXT_EMOTE:
            pSource->MonsterTextEmote(iTextEntry, pTarget ? pTarget->GetGUID() : 0);
            break;
        case CHAT_TYPE_BOSS_EMOTE:
            pSource->MonsterTextEmote(iTextEntry, pTarget ? pTarget->GetGUID() : 0, true);
            break;
        case CHAT_TYPE_WHISPER:
            {
                if (pTarget && pTarget->GetTypeId() == TYPEID_PLAYER)
                    pSource->MonsterWhisper(iTextEntry, pTarget->GetGUID());
                else
                    sLog->outError("TSCR: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", iTextEntry);
            }
            break;
        case CHAT_TYPE_BOSS_WHISPER:
            {
                if (pTarget && pTarget->GetTypeId() == TYPEID_PLAYER)
                    pSource->MonsterWhisper(iTextEntry, pTarget->GetGUID(), true);
                else
                    sLog->outError("TSCR: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", iTextEntry);
            }
            break;
        case CHAT_TYPE_ZONE_YELL:
            pSource->MonsterYellToZone(iTextEntry, pData->uiLanguage, pTarget ? pTarget->GetGUID() : 0);
            break;
    }
}

void Script::RegisterSelf()
{
    // try to find scripts which try to use another script's allocated memory
    // that means didn't allocate memory for script
    for (uint16 i = 0; i < MAX_SCRIPTS; ++i)
    {
        // somebody forgot to allocate memory for a script by a method like this: newscript = new Script
        if (m_scripts[i] == this)
        {
            sLog->outError("ScriptName: '%s' - Forgot to allocate memory, so this script and/or the script before that can't work.", Name.c_str());
            // don't register it
            // and don't delete it because its memory is used for another script
            return;
        }
    }

    int id = GetScriptId(Name.c_str());
    if (id)
    {
        // try to find the script in assigned scripts
        bool IsExist = false;
        for (uint16 i = 0; i < MAX_SCRIPTS; ++i)
        {
            if (m_scripts[i])
            {
                // if the assigned script's name and the new script's name is the same
                if (m_scripts[i]->Name == Name)
                {
                    IsExist = true;
                    break;
                }
            }
        }

        // if the script doesn't assigned -> assign it!
        if (!IsExist)
        {
            m_scripts[id] = this;
            ++num_sc_scripts;
        }
        // if the script is already assigned -> delete it!
        else
        {
            // TODO: write a better error message than this one :)
            sLog->outError("ScriptName: '%s' already assigned with the same ScriptName, so the script can't work.", Name.c_str());
            delete this;
        }
    }
    else
    {
        if (Name.find("example") == std::string::npos)
            sLog->outErrorDb("TSCR: RegisterSelf, but script named %s does not have ScriptName assigned in database.", (this)->Name.c_str());
        delete this;
    }
}

void ScriptMgr::OnLogin(Player* player)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnLogin) return;
    tmpscript->pOnLogin(player);
}

void ScriptMgr::OnLogout(Player* player)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnLogout) return;
    tmpscript->pOnLogout(player);
}

void ScriptMgr::OnPVPKill(Player* killer, Player *killed)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnPVPKill) return;
    tmpscript->pOnPVPKill(killer, killed);
}

char const* ScriptMgr::ScriptsVersion()
{
    return "Integrated Skyfire Scripts";
}

bool ScriptMgr::GossipHello (Player * player, Creature* creature)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pGossipHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGossipHello(player, creature);
}

bool ScriptMgr::GossipSelect(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    sLog->outDebug("TSCR: Gossip selection, sender: %d, action: %d", uiSender, uiAction);

    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pGossipSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGossipSelect(player, creature, uiSender, uiAction);
}

bool ScriptMgr::GossipSelectWithCode(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction, const char* sCode)
{
    sLog->outDebug("TSCR: Gossip selection with code, sender: %d, action: %d", uiSender, uiAction);

    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pGossipSelectWithCode) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGossipSelectWithCode(player, creature, uiSender, uiAction, sCode);
}

bool ScriptMgr::GOSelect(Player* player, GameObject* pGO, uint32 uiSender, uint32 uiAction)
{
    if (!pGO)
    return false;
    sLog->outDebug("TSCR: Gossip selection, sender: %d, action: %d", uiSender, uiAction);

    Script *tmpscript = m_scripts[pGO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOSelect(player, pGO, uiSender, uiAction);
}

bool ScriptMgr::GOSelectWithCode(Player* player, GameObject* pGO, uint32 uiSender, uint32 uiAction, const char* sCode)
{
    if (!pGO)
    return false;
    sLog->outDebug("TSCR: Gossip selection, sender: %d, action: %d", uiSender, uiAction);

    Script *tmpscript = m_scripts[pGO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOSelectWithCode) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOSelectWithCode(player, pGO, uiSender , uiAction, sCode);
}

bool ScriptMgr::QuestAccept(Player* player, Creature* creature, Quest const* pQuest)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestAccept(player, creature, pQuest);
}

bool ScriptMgr::QuestSelect(Player* player, Creature* creature, Quest const* pQuest)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestSelect(player, creature, pQuest);
}

bool ScriptMgr::QuestComplete(Player* player, Creature* creature, Quest const* pQuest)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestComplete) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestComplete(player, creature, pQuest);
}

bool ScriptMgr::ChooseReward(Player* player, Creature* creature, Quest const* pQuest, uint32 opt)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pChooseReward) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pChooseReward(player, creature, pQuest, opt);
}

uint32 ScriptMgr::NPCDialogStatus(Player* player, Creature* creature)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pNPCDialogStatus) return 100;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pNPCDialogStatus(player, creature);
}

uint32 ScriptMgr::GODialogStatus(Player* player, GameObject* pGO)
{
    Script *tmpscript = m_scripts[pGO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGODialogStatus) return 100;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGODialogStatus(player, pGO);
}

bool ScriptMgr::ItemHello(Player* player, Item* pItem, Quest const* pQuest)
{
    Script *tmpscript = m_scripts[pItem->GetProto()->ScriptId];
    if (!tmpscript || !tmpscript->pItemHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pItemHello(player, pItem, pQuest);
}

bool ScriptMgr::ItemQuestAccept(Player* player, Item* pItem, Quest const* pQuest)
{
    Script *tmpscript = m_scripts[pItem->GetProto()->ScriptId];
    if (!tmpscript || !tmpscript->pItemQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pItemQuestAccept(player, pItem, pQuest);
}

bool ScriptMgr::GOHello(Player* player, GameObject* pGO)
{
    Script *tmpscript = m_scripts[pGO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOHello(player, pGO);
}

bool ScriptMgr::GOQuestAccept(Player* player, GameObject* pGO, Quest const* pQuest)
{
    Script *tmpscript = m_scripts[pGO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOQuestAccept(player, pGO, pQuest);
}

bool ScriptMgr::GOChooseReward(Player* player, GameObject* pGO, Quest const* pQuest, uint32 opt)
{
    Script *tmpscript = m_scripts[pGO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOChooseReward) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOChooseReward(player, pGO, pQuest, opt);
}

bool ScriptMgr::AreaTrigger(Player* player, AreaTriggerEntry const* atEntry)
{
    Script *tmpscript = m_scripts[GetAreaTriggerScriptId(atEntry->id)];
    if (!tmpscript || !tmpscript->pAreaTrigger) return false;

    return tmpscript->pAreaTrigger(player, atEntry);
}

CreatureAI* ScriptMgr::GetAI(Creature* creature)
{
    Script *tmpscript = m_scripts[creature->GetScriptId()];
    if (!tmpscript || !tmpscript->GetAI) return NULL;

    return tmpscript->GetAI(creature);
}

bool ScriptMgr::ItemUse(Player* player, Item* pItem, SpellCastTargets const& targets)
{
    Script *tmpscript = m_scripts[pItem->GetProto()->ScriptId];
    if (!tmpscript || !tmpscript->pItemUse) return false;

    return tmpscript->pItemUse(player, pItem, targets);
}

bool ScriptMgr::EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget)
{
    Script *tmpscript = m_scripts[crTarget->GetScriptId()];

    if (!tmpscript || !tmpscript->pEffectDummyCreature) return false;

    return tmpscript->pEffectDummyCreature(caster, spellId, effIndex, crTarget);
}

InstanceScript* ScriptMgr::CreateInstanceData(Map *map)
{
    if (!map->IsDungeon()) return NULL;

    Script *tmpscript = m_scripts[((InstanceMap*)map)->GetScriptId()];
    if (!tmpscript || !tmpscript->GetInstanceScript) return NULL;

    return tmpscript->GetInstanceScript(map);
}

