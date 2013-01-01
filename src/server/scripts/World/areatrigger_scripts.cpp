 /*
  * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
  * Copyright (C) 2010-2013 Oregon <http://www.oregoncore.com/>
  * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
  * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
SDName: Areatrigger_Scripts
SD%Complete: 100
SDComment: Scripts for areatriggers
SDCategory: Areatrigger
EndScriptData */

/* ContentData
at_coilfang_waterfall           4591
at_legion_teleporter            4560 Teleporter TO Invasion Point: Cataclysm
at_ravenholdt
EndContentData */

#include "ScriptPCH.h"

/*######
## at_coilfang_waterfall
######*/

enum eCoilfangGOs
{
    GO_COILFANG_WATERFALL   = 184212
};

bool AreaTrigger_at_coilfang_waterfall(Player* player, const AreaTriggerEntry * /*pAt*/)
{
    if (GameObject* pGo = GetClosestGameObjectWithEntry(player, GO_COILFANG_WATERFALL, 35.0f))
        if (pGo->getLootState() == GO_READY)
            pGo->UseDoorOrButton();

    return false;
}

/*#####
## at_legion_teleporter
#####*/

enum eLegionTeleporter
{
    SPELL_TELE_A_TO         = 37387,
    QUEST_GAINING_ACCESS_A  = 10589,

    SPELL_TELE_H_TO         = 37389,
    QUEST_GAINING_ACCESS_H  = 10604
};

bool AreaTrigger_at_legion_teleporter(Player* player, const AreaTriggerEntry * /*pAt*/)
{
    if (player->isAlive() && !player->isInCombat())
    {
        if (player->GetTeam() == ALLIANCE && player->GetQuestRewardStatus(QUEST_GAINING_ACCESS_A))
        {
            player->CastSpell(player, SPELL_TELE_A_TO, false);
            return true;
        }

        if (player->GetTeam() == HORDE && player->GetQuestRewardStatus(QUEST_GAINING_ACCESS_H))
        {
            player->CastSpell(player, SPELL_TELE_H_TO, false);
            return true;
        }

        return false;
    }
    return false;
}

enum eRavenholdt
{
    QUEST_MANOR_RAVENHOLDT  = 6681,
    NPC_RAVENHOLDT          = 13936
};

bool AreaTrigger_at_ravenholdt(Player* player, const AreaTriggerEntry* /*pAt*/)
{
    if (player->GetQuestStatus(QUEST_MANOR_RAVENHOLDT) == QUEST_STATUS_INCOMPLETE)
        player->KilledMonsterCredit(NPC_RAVENHOLDT, 0);

    return false;
}

void AddSC_areatrigger_scripts()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "at_coilfang_waterfall";
    newscript->pAreaTrigger = &AreaTrigger_at_coilfang_waterfall;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_legion_teleporter";
    newscript->pAreaTrigger = &AreaTrigger_at_legion_teleporter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_ravenholdt";
    newscript->pAreaTrigger = &AreaTrigger_at_ravenholdt;
    newscript->RegisterSelf();
}
