/*
 * Copyright (C) 2011-2013 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

#ifndef sPreparedStatement

#include "ace/Singleton.h"
#include "DatabaseEnv.h"

class PreparedStatementHolder
{
    public:
        ///- Load prepare statements on database $db and increase $count for every statement
        void LoadCharacters(Database* db, uint32 &count);
        void LoadAuthserver(Database* db, uint32 &count);
        void LoadWorldserver(Database* db, uint32 &count);

        ///- Executes prepared statement that doesn't require feedback with name $name on database $db
        void Execute(Database* db, const char* name);
        ///- Executes prepared statement that doesn't require feedback with name $name and args $args
        ///- on database $db
        void PExecute(Database* db, const char* name, const char* args);

        ///- Executes a prepared statement without args on db $db with name $name and puts the result set in a pointer.
        QueryResult_AutoPtr Query(Database* db, const char* name);
        ///- Executes a prepared statement with args $args on db $db with name $name and put the result set in a pointer.
        QueryResult_AutoPtr PQuery(Database* db, const char* name, const char* args);

    private:
        void _prepareStatement(const char* name, const char* sql, Database* db, uint32 &count);
};
#define sPreparedStatement ACE_Singleton<PreparedStatementHolder, ACE_Null_Mutex>::instance()
#endif