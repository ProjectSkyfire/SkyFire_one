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

#ifndef SKYFIRE_TEMPSUMMON_H
#define SKYFIRE_TEMPSUMMON_H

#include "Creature.h"

class TempSummon : public Creature
{
    public:
        explicit TempSummon(SummonPropertiesEntry const *properties, Unit *owner);
        virtual ~TempSummon(){};
        void Update(uint32 time);
        virtual void InitStats(uint32 lifetime);
        virtual void InitSummon();
        void UnSummon();
        void RemoveFromWorld();
        void SetTempSummonType(TempSummonType type);
        void SaveToDB();
        Unit* GetSummoner() const;

        SummonPropertiesEntry const *m_Properties;
    private:
        TempSummonType m_type;
        uint32 m_timer;
        uint32 m_lifetime;
        uint64 m_summonerGUID;
};

class Minion : public TempSummon
{
    public:
        Minion(SummonPropertiesEntry const *properties, Unit *owner);
        void InitStats(uint32 duration);
        void RemoveFromWorld();
        Unit *GetOwner() { return m_owner; }
        float GetFollowAngle() const { return m_followAngle; }
        void SetFollowAngle(float angle) { m_followAngle = angle; }
        bool IsGuardianPet() const;
    protected:
        Unit * const m_owner;
        float m_followAngle;
};

class Guardian : public Minion
{
    public:
        Guardian(SummonPropertiesEntry const *properties, Unit *owner);
        void InitStats(uint32 duration);
        bool InitStatsForLevel(uint32 level);
        void InitSummon();

        int32 GetBonusDamage() { return m_bonusdamage; }
        void SetBonusDamage(int32 damage) { m_bonusdamage = damage; }
    protected:
        int32   m_bonusdamage;
};

class Puppet : public Minion
{
    public:
        Puppet(SummonPropertiesEntry const *properties, Unit *owner);
        void InitStats(uint32 duration);
        void InitSummon();
        void Update(uint32 time);
        void RemoveFromWorld();
    protected:
        Player *m_owner;
};

#endif

