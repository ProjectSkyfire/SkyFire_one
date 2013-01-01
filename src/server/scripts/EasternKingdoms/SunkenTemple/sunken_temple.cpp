/*
 * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2010-2013 Oregon <http://www.oregoncore.com/>
 * Copyright (C) 2006-2013 kb_z
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
SDName: Sunken_Temple
SD%Complete: 100
SDComment: Area Trigger + Puzzle event support
SDCategory: Sunken Temple
EndScriptData */

/* ContentData
at_malfurion_Stormrage_trigger
EndContentData */

#include "ScriptPCH.h"
#include "sunken_temple.h"

/*#####
# at_malfurion_Stormrage_trigger
#####*/

bool AreaTrigger_at_malfurion_stormrage(Player* player, const AreaTriggerEntry * /*at*/)
{
    if (ScriptedInstance* instance = player->GetInstanceScript())
    {
        if (!player->FindNearestCreature(15362, 15))
            player->SummonCreature(15362, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), -1.52, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 100000);
        return false;
    }
return false;
}
/*#####
# go_atalai_statue
#####*/

bool GOHello_go_atalai_statue(Player* player, GameObject* pGo)
{
    ScriptedInstance* instance = player->GetInstanceScript();
    if (!instance)
        return false;
    instance->SetData(EVENT_STATE, pGo->GetEntry());
    return false;
}

void AddSC_sunken_temple()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "at_malfurion_stormrage";
    newscript->pAreaTrigger = &AreaTrigger_at_malfurion_stormrage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_atalai_statue";
    newscript->pGOHello = &GOHello_go_atalai_statue;
    newscript->RegisterSelf();
}
