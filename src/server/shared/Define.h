/*
 * Copyright (C) 2010-2013 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2010-2013 Oregon <http://www.oregoncore.com/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2013 MaNGOS <http://getmangos.com/>
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

#ifndef SKYFIRE_DEFINE_H
#define SKYFIRE_DEFINE_H

#include <sys/types.h>

#include <ace/Basic_Types.h>
#include <ace/ACE_export.h>

#include "CompilerDefs.h"

#define SKYFIRE_LITTLEENDIAN 0
#define SKYFIRE_BIGENDIAN    1

#if !defined(SKYFIRE_ENDIAN)
#  if defined (ACE_BIG_ENDIAN)
#    define SKYFIRE_ENDIAN SKYFIRE_BIGENDIAN
#  else //ACE_BYTE_ORDER != ACE_BIG_ENDIAN
#    define SKYFIRE_ENDIAN SKYFIRE_LITTLEENDIAN
#  endif //ACE_BYTE_ORDER
#endif //SKYFIRE_ENDIAN

#if PLATFORM == PLATFORM_WINDOWS
#  define SKYFIRE_PATH_MAX MAX_PATH
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#else //PLATFORM != PLATFORM_WINDOWS
#  define SKYFIRE_PATH_MAX PATH_MAX
#  define DECLSPEC_NORETURN
#endif //PLATFORM

#if !defined(COREDEBUG)
#  define SKYFIRE_INLINE inline
#else //COREDEBUG
#  if !defined(SKYFIRE_DEBUG)
#    define SKYFIRE_DEBUG
#  endif //SKYFIRE_DEBUG
#  define SKYFIRE_INLINE
#endif //!COREDEBUG

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F, V) __attribute__ ((format (printf, F, V)))
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F, V)
#endif //COMPILER == COMPILER_GNU

typedef ACE_INT64 int64;
typedef ACE_INT32 int32;
typedef ACE_INT16 int16;
typedef ACE_INT8 int8;
typedef ACE_UINT64 uint64;
typedef ACE_UINT32 uint32;
typedef ACE_UINT16 uint16;
typedef ACE_UINT8 uint8;

#if COMPILER != COMPILER_MICROSOFT
typedef uint16      WORD;
typedef uint32      DWORD;
#endif //COMPILER

typedef uint64 OBJECT_HANDLE;

#endif //SKYFIRE_DEFINE_H

