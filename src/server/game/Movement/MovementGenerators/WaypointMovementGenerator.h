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

#ifndef SKYFIRE_WAYPOINTMOVEMENTGENERATOR_H
#define SKYFIRE_WAYPOINTMOVEMENTGENERATOR_H

/** @page PathMovementGenerator is used to generate movements
 * of waypoints and flight paths.  Each serves the purpose
 * of generate activities so that it generates updated
 * packets for the players.
 */

#include "MovementGenerator.h"
#include "DestinationHolder.h"
#include "WaypointManager.h"
#include "Path.h"
#include "Traveller.h"

#include "Player.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  3 * MINUTE * IN_MILLISECONDS          // 3 Minutes
#define TIMEDIFF_NEXT_WP      250

template<class T, class P = Path>
class PathMovementBase
{
    public:
        PathMovementBase() : i_currentNode(0) {}
        virtual ~PathMovementBase() {};

        bool MovementInProgress(void) const { return i_currentNode < i_path.Size(); }

        void LoadPath(T &);
        void ReloadPath(T &);
        uint32 GetCurrentNode() const { return i_currentNode; }

    protected:
        uint32 i_currentNode;
        DestinationHolder< Traveller<T> > i_destinationHolder;
        P i_path;
};

template<class T>

class WaypointMovementGenerator
    : public MovementGeneratorMedium< T, WaypointMovementGenerator<T> >, public PathMovementBase<T>
{
    public:
        WaypointMovementGenerator(uint32 _path_id = 0, bool _repeating = true) :
          i_nextMoveTime(0), path_id(_path_id), repeating(_repeating), StopedByPlayer(false), node(NULL) {}

        void Initialize(T &);
        void Finalize(T &);
        void MovementInform(T &);
        void InitTraveller(T &, const WaypointData &);
        void GeneratePathId(T &);
        void Reset(T &unit);
        bool Update(T &, const uint32 &);
        bool GetDestination(float &x, float &y, float &z) const;
        MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

    private:
        WaypointData *node;
        uint32 path_id;
        TimeTrackerSmall i_nextMoveTime;
        WaypointPath *waypoints;
        bool repeating, StopedByPlayer;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class FlightPathMovementGenerator
: public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
public PathMovementBase<Player>
{
    uint32 i_pathId;
    std::vector<uint32> i_mapIds;
    public:
        explicit FlightPathMovementGenerator(uint32 id, uint32 startNode = 0) : i_pathId(id) { i_currentNode = startNode; }
        void Initialize(Player &);
        void Finalize(Player &);
        void Reset(Player &) {}
        bool Update(Player &, const uint32 &);
        MovementGeneratorType GetMovementGeneratorType() { return FLIGHT_MOTION_TYPE; }

        void LoadPath(Player &);
        void ReloadPath(Player &) { /* don't reload flight path */ }

        Path& GetPath() { return i_path; }
        uint32 GetPathAtMapEnd() const;
        bool HasArrived() const { return (i_currentNode >= i_path.Size()); }
        void SetCurrentNodeAfterTeleport();
        void SkipCurrentNode() { ++i_currentNode; }
        bool GetDestination(float& x, float& y, float& z) const { i_destinationHolder.GetDestination(x, y, z); return true; }

    private:
        // storage for preloading the flightmaster grid at end
        // before reaching final waypoint
        uint32 m_endMapId;
        uint32 m_preloadTargetNode;
        float m_endGridX;
        float m_endGridY;
        void PreloadEndGrid();
};
#endif

