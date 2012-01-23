# Copyright (C) 2010-2012 Oregon <http://www.oregoncore.com/>
# Copyright (C) 2008-2012 Trinity <http://www.trinitycore.org/>
# Copyright (C) 2005-2012 MaNGOS <http://www.getmangos.com/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Enable precompiled headers when using the GCC compiler.
if( USE_SCRIPTPCH )
  include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

file(GLOB_RECURSE scripts_custom Custom/*.cpp Custom/*.h)
file(GLOB_RECURSE scripts_easternplaguelands EasternKingdoms/*.cpp EasternKingdoms/*.h)
file(GLOB_RECURSE scripts_examples Examples/*.cpp Examples/*.h)
file(GLOB_RECURSE scripts_kalimdor Kalimdor/*.cpp Kalimdor/*.h)
file(GLOB_RECURSE scripts_outland Outland/*.cpp Outland/*.h)
file(GLOB_RECURSE scripts_world World/*.cpp World/*.h)

if( USE_SCRIPTPCH AND MSVC )
  set(scripts_STAT_SRCS
    PrecompiledHeaders/ScriptPCH.cpp
    PrecompiledHeaders/ScriptPCH.h
  )
endif()

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ../game/ScriptedEscortAI.cpp
  ../game/ScriptedCreature.cpp
  ../game/ScriptedFollowerAI.cpp
  ../game/ScriptedGuardAI.cpp
  ../game/ScriptedSimpleAI.cpp
)

if(SCRIPTS)
  set(scripts_STAT_SRCS
    ${scripts_STAT_SRCS}
    ${scripts_custom}
    ${scripts_easternplaguelands}
    ${scripts_examples}
    ${scripts_kalimdor}
    ${scripts_outland}
    ${scripts_world}
  )
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/SFMT
  ${CMAKE_SOURCE_DIR}/dep/mersennetwister
  ${CMAKE_SOURCE_DIR}/dep/zlib
  ${CMAKE_SOURCE_DIR}/src/server/framework
  ${CMAKE_SOURCE_DIR}/src/server/game
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography/Authentication  
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

add_library(scripts STATIC ${scripts_STAT_SRCS})

add_dependencies(scripts revision.h)

# Generate precompiled header
if( USE_SCRIPTPCH )
  if(CMAKE_COMPILER_IS_GNUCXX)
    add_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/ScriptPCH.h)
  elseif(MSVC)
    add_native_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/ScriptPCH)
  endif()
endif()
