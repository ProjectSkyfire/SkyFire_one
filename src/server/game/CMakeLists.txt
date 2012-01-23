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

if (USE_COREPCH)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

file(GLOB sources_localdir *.cpp *.h)

# Create game-libary

if (USE_COREPCH AND MSVC)
  set(game_STAT_SRCS
    PrecompiledHeaders/gamePCH.cpp
    PrecompiledHeaders/gamePCH.h
  )
endif()

set(game_STAT_SRCS
  ${game_STAT_SRCS}
  ${sources_localdir}
)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/mersennetwister
  ${CMAKE_SOURCE_DIR}/dep/SFMT
  ${CMAKE_SOURCE_DIR}/dep/zlib
  ${CMAKE_SOURCE_DIR}/src/server/framework
  ${CMAKE_SOURCE_DIR}/src/server/collision
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography/Authentication
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/server/scripts/PrecompiledHeaders
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

add_library(game STATIC ${game_STAT_SRCS})

add_dependencies(game revision.h)

# Generate precompiled header
if (USE_COREPCH)
  if (CMAKE_COMPILER_IS_GNUCXX)
    add_precompiled_header(game ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/gamePCH.h)
  elseif (MSVC)
    add_native_precompiled_header(game ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/gamePCH)
  endif()
endif()
