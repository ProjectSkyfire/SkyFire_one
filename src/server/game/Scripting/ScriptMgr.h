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

#ifndef SC_SCRIPTMGR_H
#define SC_SCRIPTMGR_H

#include "Common.h"
#include "CompilerDefs.h"
#include "DBCStructure.h"

#include <ace/Singleton.h>
//#include <ace/Atomic_Op.h>

class Player;
class Creature;
class CreatureAI;
class InstanceScript;
class Quest;
class Item;
class GameObject;
class SpellCastTargets;
class Map;
class Unit;
class WorldObject;
struct ItemPrototype;

#define MAX_SCRIPTS         5000                            //72 bytes each (approx 351kb)
#define VISIBLE_RANGE       (166.0f)                        //MAX visible range (size of grid)
#define DEFAULT_TEXT        "<Skyfire Script Text Entry Missing!>"

struct Script
{
    Script() :
        pOnLogin(NULL), pOnLogout(NULL), pOnPVPKill(NULL),
        pGossipHello(NULL), pQuestAccept(NULL), pGossipSelect(NULL), pGossipSelectWithCode(NULL),
        pQuestSelect(NULL), pQuestComplete(NULL), pNPCDialogStatus(NULL), pGODialogStatus(NULL),
        pChooseReward(NULL), pItemHello(NULL), pGOHello(NULL), pAreaTrigger(NULL), pItemQuestAccept(NULL),
        pGOQuestAccept(NULL), pGOChooseReward(NULL), pItemUse(NULL), pEffectDummyCreature(NULL),
        GetAI(NULL), GetInstanceScript(NULL)
    {}

    std::string Name;

    //Methods to be scripted
    void (*pOnLogin             )(Player*);
    void (*pOnLogout            )(Player*);
    void (*pOnPVPKill           )(Player*, Player*);
    bool (*pGossipHello         )(Player*, Creature*);
    bool (*pQuestAccept         )(Player*, Creature*, Quest const*);
    bool (*pGossipSelect        )(Player*, Creature*, uint32 , uint32);
    bool (*pGossipSelectWithCode)(Player*, Creature*, uint32 , uint32 , const char*);
    bool (*pGOSelect            )(Player*, GameObject*, uint32 , uint32);
    bool (*pGOSelectWithCode    )(Player*, GameObject*, uint32 , uint32 , const char*);
    bool (*pQuestSelect         )(Player*, Creature*, Quest const*);
    bool (*pQuestComplete       )(Player*, Creature*, Quest const*);
    uint32 (*pNPCDialogStatus   )(Player*, Creature*);
    uint32 (*pGODialogStatus    )(Player*, GameObject * _GO);
    bool (*pChooseReward        )(Player*, Creature*, Quest const*, uint32);
    bool (*pItemHello           )(Player*, Item*, Quest const*);
    bool (*pGOHello             )(Player*, GameObject*);
    bool (*pAreaTrigger         )(Player*, AreaTriggerEntry const*);
    bool (*pItemQuestAccept     )(Player*, Item *, Quest const*);
    bool (*pGOQuestAccept       )(Player*, GameObject*, Quest const*);
    bool (*pGOChooseReward      )(Player*, GameObject*, Quest const*, uint32);
    bool (*pItemUse             )(Player*, Item*, SpellCastTargets const&);
    bool (*pEffectDummyCreature )(Unit*, uint32, uint32, Creature*);

    CreatureAI* (*GetAI)(Creature*);
    InstanceScript* (*GetInstanceScript)(Map*);

    void RegisterSelf();
};

class ScriptMgr
{
    friend class ACE_Singleton<ScriptMgr, ACE_Null_Mutex>;
    friend class ScriptObject;

private:

    ScriptMgr();
    virtual ~ScriptMgr();

public: /* Initialization */

        void ScriptsInit();
        void LoadDatabase();
        char const* ScriptsVersion();

    //event handlers
        void OnLogin(Player* player);
        void OnLogout(Player* player);
        void OnPVPKill(Player* killer, Player *killed);
        bool GossipHello (Player * player, Creature* creature);
        bool GossipSelect(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction);
        bool GossipSelectWithCode(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool GOSelect(Player* player, GameObject* pGO, uint32 uiSender, uint32 uiAction);
        bool GOSelectWithCode(Player* player, GameObject* pGO, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool QuestAccept(Player* player, Creature* creature, Quest const* pQuest);
        bool QuestSelect(Player* player, Creature* creature, Quest const* pQuest);
        bool QuestComplete(Player* player, Creature* creature, Quest const* pQuest);
        bool ChooseReward(Player* player, Creature* creature, Quest const* pQuest, uint32 opt);
        uint32 NPCDialogStatus(Player* player, Creature* creature);
        uint32 GODialogStatus(Player* player, GameObject* pGO);
        bool ItemHello(Player* player, Item* pItem, Quest const* pQuest);
        bool ItemQuestAccept(Player* player, Item* pItem, Quest const* pQuest);
        bool GOHello(Player* player, GameObject* pGO);
        bool GOQuestAccept(Player* player, GameObject* pGO, Quest const* pQuest);
        bool GOChooseReward(Player* player, GameObject* pGO, Quest const* pQuest, uint32 opt);
        bool AreaTrigger(Player* player, AreaTriggerEntry const* atEntry);
        CreatureAI* GetAI(Creature* creature);
        bool ItemUse(Player* player, Item* pItem, SpellCastTargets const& targets);
        bool EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget);
        InstanceScript* CreateInstanceData(Map *map);
};

//Generic scripting text function
void DoScriptText(int32 textEntry, WorldObject* pSource, Unit *pTarget = NULL);

#if COMPILER == COMPILER_GNU
#define FUNC_PTR(name, callconvention, returntype, parameters)    typedef returntype(*name)parameters __attribute__ ((callconvention));
#else
#define FUNC_PTR(name, callconvention, returntype, parameters)    typedef returntype(callconvention *name)parameters;
#endif

#define sScriptMgr ACE_Singleton<ScriptMgr, ACE_Null_Mutex>::instance()
#endif

