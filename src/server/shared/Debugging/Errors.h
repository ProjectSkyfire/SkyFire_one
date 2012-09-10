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

#ifndef SKYFIRE_ERRORS_H
#define SKYFIRE_ERRORS_H

#include "Common.h"

#ifdef HAVE_ACE_STACK_TRACE_H                               // old versions ACE not have Stack_Trace.h but used at some oS for better compatibility
#define WPAssert( assertion ) { if (!(assertion)) { ACE_Stack_Trace st; fprintf( stderr, "\n%s:%i in %s ASSERTION FAILED:\n  %s\n%s\n", __FILE__, __LINE__, __FUNCTION__,  #assertion, st.c_str()); assert( #assertion &&0 ); } }
#else
#define WPAssert( assertion ) { if (!(assertion)) { fprintf( stderr, "\n%s:%i in %s ASSERTION FAILED2:\n  %s\n", __FILE__, __LINE__, __FUNCTION__,  #assertion); assert( #assertion &&0 ); } }
#endif
#define WPError( assertion, errmsg ) if ( ! (assertion) ) { sLog->outError( "%\n%s:%i in %s ERROR:\n  %s\n", __FILE__, __LINE__, __FUNCTION__, (char *)errmsg ); assert( false ); }
#define WPWarning( assertion, errmsg ) if ( ! (assertion) ) { sLog->outError( "\n%s:%i in %s WARNING:\n  %s\n", __FILE__, __LINE__, __FUNCTION__, (char *)errmsg ); }

#define WPFatal( assertion, errmsg ) if ( ! (assertion) ) { sLog->outError( "\n%s:%i in %s FATAL ERROR:\n  %s\n", __FILE__, __LINE__, __FUNCTION__, (char *)errmsg ); assert( #assertion &&0 ); abort(); }

#define ASSERT WPAssert

#endif
