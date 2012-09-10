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

#ifndef SKYFIRE_COMBATAI_H
#define SKYFIRE_COMBATAI_H

#include "CreatureAI.h"
#include "CreatureAIImpl.h"

class Creature;

class AggressorAI : public CreatureAI
{
    public:
        explicit AggressorAI(Creature *c) : CreatureAI(c) {}

        void UpdateAI(const uint32);
        static int Permissible(const Creature *);
};

typedef std::vector<uint32> SpellVct;

class CombatAI : public CreatureAI
{
    public:
        explicit CombatAI(Creature *c) : CreatureAI(c) {}

        void InitializeAI();
        void Reset();
        void EnterCombat(Unit* who);
        void JustDied(Unit *killer);
        void UpdateAI(const uint32 diff);
        static int Permissible(const Creature *);
    protected:
        EventMap events;
        SpellVct spells;
};

class CasterAI : public CombatAI
{
    public:
        explicit CasterAI(Creature *c) : CombatAI(c) { m_attackDist = MELEE_RANGE; }
        void InitializeAI();
        void AttackStart(Unit * victim) { AttackStartCaster(victim, m_attackDist); }
        void UpdateAI(const uint32 diff);
        void EnterCombat(Unit *who);
    private:
        float m_attackDist;
};

#endif

