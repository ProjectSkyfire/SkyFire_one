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

#ifndef SKYFIRE_MAPMANAGER_H
#define SKYFIRE_MAPMANAGER_H

#include "Define.h"
#include "Common.h"
#include "Map.h"
#include "GridStates.h"
#include "MapUpdater.h"

#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

class Transport;

class MapManager
{
    friend class ACE_Singleton<MapManager, ACE_Thread_Mutex>;
    typedef UNORDERED_MAP<uint32, Map*> MapMapType;
    typedef std::pair<UNORDERED_MAP<uint32, Map*>::iterator, bool>  MapMapPair;

    public:

        Map* CreateMap(uint32, const WorldObject* obj, uint32 instanceId);
        Map const* CreateBaseMap(uint32 id) const { return const_cast<MapManager*>(this)->_createBaseMap(id); }
        Map* FindMap(uint32 mapid, uint32 instanceId = 0) const;

        uint16 GetAreaFlag(uint32 mapid, float x, float y, float z) const
        {
            Map const* m = CreateBaseMap(mapid);
            return m->GetAreaFlag(x, y, z);
        }
        uint32 GetAreaId(uint32 mapid, float x, float y, float z) { return Map::GetAreaId(GetAreaFlag(mapid, x, y, z), mapid); }
        uint32 GetZoneId(uint32 mapid, float x, float y, float z) { return Map::GetZoneId(GetAreaFlag(mapid, x, y, z), mapid); }

        void Initialize(void);
        void Update(time_t);

        void SetGridCleanUpDelay(uint32 t)
        {
            if (t < MIN_GRID_DELAY)
                i_gridCleanUpDelay = MIN_GRID_DELAY;
            else
                i_gridCleanUpDelay = t;
        }

        void SetMapUpdateInterval(uint32 t)
        {
            if (t > MIN_MAP_UPDATE_DELAY)
                t = MIN_MAP_UPDATE_DELAY;

            i_timer.SetInterval(t);
            i_timer.Reset();
        }

        //void LoadGrid(int mapid, float x, float y, const WorldObject* obj, bool no_unload = false);
        void UnloadAll();

        static bool ExistMapAndVMap(uint32 mapid, float x, float y);
        static bool IsValidMAP(uint32 mapid);

        static bool IsValidMapCoord(uint32 mapid, float x, float y)
        {
            return IsValidMAP(mapid) && SkyFire::IsValidMapCoord(x, y);
        }

        static bool IsValidMapCoord(uint32 mapid, float x, float y, float z)
        {
            return IsValidMAP(mapid) && SkyFire::IsValidMapCoord(x, y, z);
        }

        static bool IsValidMapCoord(uint32 mapid, float x, float y, float z, float o)
        {
            return IsValidMAP(mapid) && SkyFire::IsValidMapCoord(x, y, z, o);
        }

        static bool IsValidMapCoord(WorldLocation const& loc)
        {
            return IsValidMapCoord(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation());
        }

        // modulos a radian orientation to the range of 0..2PI
        static float NormalizeOrientation(float o)
        {
            // fmod only supports positive numbers. Thus we have
            // to emulate negative numbers
            if (o < 0)
            {
                float mod = o *-1;
                mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
                mod = -mod + 2.0f * M_PI;
                return mod;
            }
            return fmod(o, 2.0f * static_cast<float>(M_PI));
        }

        void DoDelayedMovesAndRemoves();

        void LoadTransports();

        typedef std::set<Transport *> TransportSet;
        TransportSet m_Transports;

        typedef std::map<uint32, TransportSet> TransportMap;
        TransportMap m_TransportsByMap;

        bool CanPlayerEnter(uint32 mapid, Player* player);
        uint32 GenerateInstanceId() { return ++i_MaxInstanceId; }
        void InitMaxInstanceId();
        void InitializeVisibilityDistanceInfo();

        /* statistics */
        uint32 GetNumInstances();
        uint32 GetNumPlayersInInstances();

    private:
        // debugging code, should be deleted some day
        void checkAndCorrectGridStatesArray();              // just for debugging to find some memory overwrites
        GridState* i_GridStates[MAX_GRID_STATE];            // shadow entries to the global array in Map.cpp
        int i_GridStateErrorCount;
    private:
        MapManager();
        ~MapManager();

        MapManager(const MapManager &);
        MapManager& operator=(const MapManager &);

        Map* _createBaseMap(uint32 id);
        Map* _findMap(uint32 id) const
        {
            MapMapType::const_iterator iter = i_maps.find(id);
            return (iter == i_maps.end() ? NULL : iter->second);
        }

        ACE_Thread_Mutex Lock;
        uint32 i_gridCleanUpDelay;
        MapMapType i_maps;
        IntervalTimer i_timer;

        uint32 i_MaxInstanceId;
        MapUpdater m_updater;
};

#define sMapMgr ACE_Singleton<MapManager, ACE_Thread_Mutex>::instance()

#endif
