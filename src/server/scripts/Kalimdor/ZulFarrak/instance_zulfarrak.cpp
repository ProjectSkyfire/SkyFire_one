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

#include "ScriptPCH.h"

#define NPC_GAHZRILLA 7273

struct instance_zulfarrak : public ScriptedInstance
{
    instance_zulfarrak(Map* pMap) : ScriptedInstance(pMap) {Initialize();}

    uint32 GahzRillaEncounter;

    void Initialize()
    {
        GahzRillaEncounter = NOT_STARTED;
    }

    void OnCreatureCreate(Creature* creature, bool /*add*/)
    {
        if (creature->GetEntry() == NPC_GAHZRILLA)
        {
            if (GahzRillaEncounter >= IN_PROGRESS)
                creature->DisappearAndDie();
            else
                GahzRillaEncounter = IN_PROGRESS;
        }
    }
};

InstanceScript* GetInstanceData_instance_zulfarrak(Map* pMap)
{
    return new instance_zulfarrak(pMap);
}

void AddSC_instance_zulfarrak()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_zulfarrak";
    newscript->GetInstanceScript = &GetInstanceData_instance_zulfarrak;
    newscript->RegisterSelf();
}
