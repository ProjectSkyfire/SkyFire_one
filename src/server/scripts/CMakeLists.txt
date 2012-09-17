# Copyright (C) 2010-2012 Project SkyFire <http://www.projectskyfire.org/>
# Copyright (C) 2010-2012 Oregon <http://www.oregoncore.com/>
# Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
if ( USE_SCRIPTPCH )
  include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

# ------ Pooling ------
file(GLOB_RECURSE scripts_custom Custom/*.cpp Custom/*.h)
file(GLOB_RECURSE sources_PrecompiledHeaders PrecompiledHeaders/*.cpp PrecompiledHeaders/*.h)
  # Eastern Kingdoms
file(GLOB scripts_easternkingdoms EasternKingdoms/*.cpp EasternKingdoms/*.h)
file(GLOB_RECURSE scripts_EK_AlteracValley EasternKingdoms/AlteracValley/*.cpp EasternKingdoms/AlteracValley/*.h)
file(GLOB_RECURSE scripts_EK_BlackrockDepths EasternKingdoms/BlackrockDepths/*.cpp EasternKingdoms/BlackrockDepths/*.h)
file(GLOB_RECURSE scripts_EK_BlackrockSpire EasternKingdoms/BlackrockSpire/*.cpp EasternKingdoms/BlackrockSpire/*.h)
file(GLOB_RECURSE scripts_EK_BlackwingLair EasternKingdoms/BlackwingLair/*.cpp EasternKingdoms/BlackwingLair/*.h)
file(GLOB_RECURSE scripts_EK_Deadmines EasternKingdoms/Deadmines/*.cpp EasternKingdoms/Deadmines/*.h)
file(GLOB_RECURSE scripts_EK_Gnomeregan EasternKingdoms/Gnomeregan/*.cpp EasternKingdoms/Gnomeregan/*.h)
file(GLOB_RECURSE scripts_EK_Karazhan EasternKingdoms/Karazhan/*.cpp EasternKingdoms/Karazhan/*.h)
file(GLOB_RECURSE scripts_EK_MagistersTerrace EasternKingdoms/MagistersTerrace/*.cpp EasternKingdoms/MagistersTerrace/*.h)
file(GLOB_RECURSE scripts_EK_MoltenCore EasternKingdoms/MoltenCore/*.cpp EasternKingdoms/MoltenCore/*.h)
file(GLOB_RECURSE scripts_EK_Naxxramas EasternKingdoms/Naxxramas/*.cpp EasternKingdoms/Naxxramas/*.h)
file(GLOB_RECURSE scripts_EK_ScarletMonastery EasternKingdoms/ScarletMonastery/*.cpp EasternKingdoms/ScarletMonastery/*.h)
file(GLOB_RECURSE scripts_EK_ShadowfangKeep EasternKingdoms/ShadowfangKeep/*.cpp EasternKingdoms/ShadowfangKeep/*.h)
file(GLOB_RECURSE scripts_EK_Scholomance EasternKingdoms/Scholomance/*.cpp EasternKingdoms/Scholomance/*.h)
file(GLOB_RECURSE scripts_EK_Stratholme EasternKingdoms/Stratholme/*.cpp EasternKingdoms/Stratholme/*.h)
file(GLOB_RECURSE scripts_EK_SunkenTemple EasternKingdoms/SunkenTemple/*.cpp EasternKingdoms/SunkenTemple/*.h)
file(GLOB_RECURSE scripts_EK_SunwellPlateau EasternKingdoms/SunwellPlateau/*.cpp EasternKingdoms/SunwellPlateau/*.h)
file(GLOB_RECURSE scripts_EK_Uldaman EasternKingdoms/Uldaman/*.cpp EasternKingdoms/Uldaman/*.h)
file(GLOB_RECURSE scripts_EK_ZulAman EasternKingdoms/ZulAman/*.cpp EasternKingdoms/ZulAman/*.h)
file(GLOB_RECURSE scripts_EK_ZulGurub EasternKingdoms/ZulGurub/*.cpp EasternKingdoms/ZulGurub/*.h)
file(GLOB_RECURSE scripts_EK_Zones EasternKingdoms/Zones/*.cpp EasternKingdoms/ZulGurub/*.h)
  # Kalimbdor
file(GLOB scripts_kalimdor Kalimdor/*.cpp Kalimdor/*.h)
file(GLOB_RECURSE scripts_K_BlackfathomDeeps Kalimdor/BlackfathomDeeps/*.cpp Kalimdor/BlackfathomDeeps/*.h)
file(GLOB_RECURSE scripts_K_BattleForMountHyjal Kalimdor/CavernsOfTime/BattleForMountHyjal/*.cpp Kalimdor/CavernsOfTime/BattleForMountHyjal/*.h)
file(GLOB_RECURSE scripts_K_DarkPortal Kalimdor/CavernsOfTime/DarkPortal/*.cpp Kalimdor/CavernsOfTime/DarkPortal/*.h)
file(GLOB_RECURSE scripts_K_EscapeFromDurnholdeKeep Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/*.cpp Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/*.h)
file(GLOB_RECURSE scripts_K_Maraudon Kalimdor/Maraudon/*.cpp Kalimdor/Maraudon/*.h)
file(GLOB_RECURSE scripts_K_OnyxiasLair Kalimdor/OnyxiasLair/*.cpp Kalimdor/OnyxiasLair/*.h)
file(GLOB_RECURSE scripts_K_RazorfenDowns Kalimdor/RazorfenDowns/*.cpp Kalimdor/RazorfenDowns/*.h)
file(GLOB_RECURSE scripts_K_RazorfenKraul Kalimdor/RazorfenKraul/*.cpp Kalimdor/RazorfenKraul/*.h)
file(GLOB_RECURSE scripts_K_RuinsOfAhnQiraj Kalimdor/RuinsOfAhnQiraj/*.cpp Kalimdor/RuinsOfAhnQiraj/*.h)
file(GLOB_RECURSE scripts_K_TempleOfAhnQiraj Kalimdor/TempleOfAhnQiraj/*.cpp Kalimdor/TempleOfAhnQiraj/*.h)
file(GLOB_RECURSE scripts_K_WailingCaverns Kalimdor/WailingCaverns/*.cpp Kalimdor/WailingCaverns/*.h)
file(GLOB_RECURSE scripts_K_ZulFarrak Kalimdor/ZulFarrak/*.cpp Kalimdor/ZulFarrak/*.h)
file(GLOB_RECURSE scripts_K_Zones Kalimdor/Zones/*.cpp Kalimdor/ZulGurub/*.h)
  # Outland
file(GLOB scripts_outland Outland/*.cpp Outland/*.h)
file(GLOB_RECURSE scripts_O_AuchenaiCrypts Outland/Auchindoun/AuchenaiCrypts/*.cpp Outland/Auchindoun/AuchenaiCrypts/*.h)
file(GLOB_RECURSE scripts_O_ManaTombs Outland/Auchindoun/ManaTombs/*.cpp Outland/Auchindoun/ManaTombs/*.h)
file(GLOB_RECURSE scripts_O_SethekkHalls Outland/Auchindoun/SethekkHalls/*.cpp Outland/Auchindoun/SethekkHalls/*.h)
file(GLOB_RECURSE scripts_O_ShadowLabyrinth Outland/Auchindoun/ShadowLabyrinth/*.cpp Outland/Auchindoun/ShadowLabyrinth/*.h)
file(GLOB_RECURSE scripts_O_BlackTemple Outland/BlackTemple/*.cpp Outland/BlackTemple/*.h)
file(GLOB_RECURSE scripts_O_SerpentShrine Outland/CoilfangReservoir/SerpentShrine/*.cpp Outland/CoilfangReservoir/SerpentShrine/*.h)
file(GLOB_RECURSE scripts_O_SteamVault Outland/CoilfangReservoir/SteamVault/*.cpp Outland/CoilfangReservoir/SteamVault/*.h)
file(GLOB_RECURSE scripts_O_Underbog Outland/CoilfangReservoir/Underbog/*.cpp Outland/CoilfangReservoir/Underbog/*.h)
file(GLOB_RECURSE scripts_O_GruulsLair Outland/GruulsLair/*.cpp Outland/GruulsLair/*.h)
file(GLOB_RECURSE scripts_O_BloodFurnace Outland/HellfireCitadel/BloodFurnace/*.cpp Outland/HellfireCitadel/BloodFurnace/*.h)
file(GLOB_RECURSE scripts_O_HellfireRamparts Outland/HellfireCitadel/HellfireRamparts/*.cpp Outland/HellfireCitadel/HellfireRamparts/*.h)
file(GLOB_RECURSE scripts_O_MagtheridonsLair Outland/HellfireCitadel/MagtheridonsLair/*.cpp Outland/HellfireCitadel/MagtheridonsLair/*.h)
file(GLOB_RECURSE scripts_O_ShatteredHalls Outland/HellfireCitadel/ShatteredHalls/*.cpp Outland/HellfireCitadel/ShatteredHalls/*.h)
file(GLOB_RECURSE scripts_O_arcatraz Outland/TempestKeep/arcatraz/*.cpp Outland/TempestKeep/arcatraz/*.h)
file(GLOB_RECURSE scripts_O_botanica Outland/TempestKeep/botanica/*.cpp Outland/TempestKeep/botanica/*.h)
file(GLOB_RECURSE scripts_O_Eye Outland/TempestKeep/Eye/*.cpp Outland/TempestKeep/Eye/*.h)
file(GLOB_RECURSE scripts_O_Mechanar Outland/TempestKeep/Mechanar/*.cpp Outland/TempestKeep/Mechanar/*.h)
file(GLOB_RECURSE scripts_O_Zones Outland/Zones/*.cpp Outland/Zones/*.h)
file(GLOB_RECURSE scripts_world World/*.cpp World/*.h)
file(GLOB_RECURSE scripts_examples Examples/*.cpp Examples/*.h)

# ------- Filters ---------
source_group(CMake FILES CMakeLists.txt)
source_group(PrecompiledHeaders FILES ${sources_PrecompiledHeaders})
source_group(Custom FILES ${scripts_custom})
  # Eastern Kingdoms
source_group("Eastern Kingdoms"                       FILES ${scripts_easternkingdoms})
source_group("Eastern Kingdoms\\Alterac Valley"       FILES ${scripts_EK_AlteracValley})
source_group("Eastern Kingdoms\\Blackrock Depths"     FILES ${scripts_EK_BlackrockDepths})
source_group("Eastern Kingdoms\\Blackrock Spire"      FILES ${scripts_EK_BlackrockSpire})
source_group("Eastern Kingdoms\\Blackwing Lair"       FILES ${scripts_EK_BlackwingLair})
source_group("Eastern Kingdoms\\Deadmines"            FILES ${scripts_EK_Deadmines})
source_group("Eastern Kingdoms\\Gnomeregan"           FILES ${scripts_EK_Gnomeregan})
source_group("Eastern Kingdoms\\Karazhan"             FILES ${scripts_EK_Karazhan})
source_group("Eastern Kingdoms\\Magisters Terrace"    FILES ${scripts_EK_MagistersTerrace})
source_group("Eastern Kingdoms\\Molten Core"          FILES ${scripts_EK_MoltenCore})
source_group("Eastern Kingdoms\\Naxxramas"            FILES ${scripts_EK_Naxxramas})
source_group("Eastern Kingdoms\\Scarlet Monastery"    FILES ${scripts_EK_ScarletMonastery})
source_group("Eastern Kingdoms\\Shadowfang Keep"      FILES ${scripts_EK_ShadowfangKeep})
source_group("Eastern Kingdoms\\Scholomance"          FILES ${scripts_EK_Scholomance})
source_group("Eastern Kingdoms\\Stratholme"           FILES ${scripts_EK_Stratholme})
source_group("Eastern Kingdoms\\SunkenTemple"         FILES ${scripts_EK_SunkenTemple})
source_group("Eastern Kingdoms\\SunwellPlateau"       FILES ${scripts_EK_SunwellPlateau})
source_group("Eastern Kingdoms\\Uldaman"              FILES ${scripts_EK_Uldaman})
source_group("Eastern Kingdoms\\ZulAman"              FILES ${scripts_EK_ZulAman})
source_group("Eastern Kingdoms\\ZulGurub"             FILES ${scripts_EK_ZulGurub})
source_group("Eastern Kingdoms\\Zones"                FILES ${scripts_EK_Zones})
  # Kalimbdor
source_group("Kalimdor"                                               FILES ${scripts_kalimdor})
source_group("Kalimdor\\Blackfathom Deeps"                            FILES ${scripts_K_BlackfathomDeeps})
source_group("Kalimdor\\Caverns Of Time\\Battle For Mount Hyjal"      FILES ${scripts_K_BattleForMountHyjal})
source_group("Kalimdor\\Caverns Of Time\\Dark Portal"                 FILES ${scripts_K_DarkPortal})
source_group("Kalimdor\\Caverns Of Time\\Escape From Durnholde Keep"  FILES ${scripts_K_EscapeFromDurnholdeKeep})
source_group("Kalimdor\\Maraudon"                                     FILES ${scripts_K_Maraudon})
source_group("Kalimdor\\Onyxias Lair"                                 FILES ${scripts_K_OnyxiasLair})
source_group("Kalimdor\\Razorfen Downs"                               FILES ${scripts_K_RazorfenDowns})
source_group("Kalimdor\\Razorfen Kraul"                               FILES ${scripts_K_RazorfenKraul})
source_group("Kalimdor\\Ruins Of Ahn Qiraj"                           FILES ${scripts_K_RuinsOfAhnQiraj})
source_group("Kalimdor\\Temple Of Ahn Qiraj"                          FILES ${scripts_K_TempleOfAhnQiraj})
source_group("Kalimdor\\Wailing Caverns"                              FILES ${scripts_K_WailingCaverns})
source_group("Kalimdor\\Zul Farrak"                                   FILES ${scripts_K_ZulFarrak})
source_group("Kalimdor\\Zones"                                        FILES ${scripts_K_Zones})
  # Outland
source_group("Outland"                                                FILES ${scripts_outland})
source_group("Outland\\Auchindoun\\AuchenaiCrypts"                    FILES ${scripts_O_AuchenaiCrypts})
source_group("Outland\\Auchindoun\\ManaTombs"                         FILES ${scripts_O_ManaTombs})
source_group("Outland\\Auchindoun\\SethekkHalls"                      FILES ${scripts_O_SethekkHalls})
source_group("Outland\\Auchindoun\\ShadowLabyrinth"                   FILES ${scripts_O_ShadowLabyrinth})
source_group("Outland\\BlackTemple"                                   FILES ${scripts_O_BlackTemple})
source_group("Outland\\CoilfangReservoir\\SerpentShrine"              FILES ${scripts_O_SerpentShrine})
source_group("Outland\\CoilfangReservoir\\SteamVault"                 FILES ${scripts_O_SteamVault})
source_group("Outland\\CoilfangReservoir\\Underbog"                   FILES ${scripts_O_Underbog})
source_group("Outland\\GruulsLair"                                    FILES ${scripts_O_GruulsLair})
source_group("Outland\\HellfireCitadel\\BloodFurnace"                 FILES ${scripts_O_BloodFurnace})
source_group("Outland\\HellfireCitadel\\HellfireRamparts"             FILES ${scripts_O_HellfireRamparts})
source_group("Outland\\HellfireCitadel\\MagtheridonsLair"             FILES ${scripts_O_MagtheridonsLair})
source_group("Outland\\HellfireCitadel\\ShatteredHalls"               FILES ${scripts_O_ShatteredHalls})
source_group("Outland\\TempestKeep\\arcatraz"                         FILES ${scripts_O_arcatraz})
source_group("Outland\\TempestKeep\\botanica"                         FILES ${scripts_O_botanica})
source_group("Outland\\TempestKeep\\Eye"                              FILES ${scripts_O_Eye})
source_group("Outland\\TempestKeep\\Mechanar"                         FILES ${scripts_O_Mechanar})
source_group("Outland\\Zones"                                         FILES ${scripts_O_Zones})
source_group(World FILES ${scripts_world})
source_group(Examples ${scripts_examples})

 if( USE_SCRIPTPCH AND MSVC )
  file(GLOB_RECURSE sources_PreHeaders PrecompiledHeaders/ScriptPCH.cpp PrecompiledHeaders/ScriptPCH.h)
  source_group(PrecompiledHeaders FILES ${sources_PreHeaders})
  set(scripts_STAT_SRCS
  ${sources_PreHeaders}
  )
 endif()

file(GLOB_RECURSE sources_ScriptedAI 
  ../game/AI/ScriptedAI/ScriptedEscortAI.cpp
  ../game/AI/ScriptedAI/ScriptedGuardAI.cpp
  ../game/AI/ScriptedAI/ScriptedCreature.cpp
  ../game/AI/ScriptedAI/ScriptedFollowerAI.cpp
  ../game/AI/ScriptedAI/ScriptedSimpleAI.cpp
  )

source_group(ScriptedAI FILES ${sources_ScriptedAI})
  
 set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${sources_ScriptedAI}
  )

if (SCRIPTS)
  set(scripts_STAT_SRCS
    ${scripts_STAT_SRCS}
    ${scripts_custom}
    # Eastern Kingdoms
    ${scripts_easternkingdoms}
    ${scripts_EK_AlteracValley}
    ${scripts_EK_BlackrockDepths}
    ${scripts_EK_BlackrockSpire}
    ${scripts_EK_BlackwingLair}
    ${scripts_EK_Deadmines}
    ${scripts_EK_Gnomeregan}
    ${scripts_EK_Karazhan}
    ${scripts_EK_MagistersTerrace}
    ${scripts_EK_MoltenCore}
    ${scripts_EK_Naxxramas}
    ${scripts_EK_ScarletMonastery}
    ${scripts_EK_ShadowfangKeep}
    ${scripts_EK_Scholomance}
    ${scripts_EK_Stratholme}
    ${scripts_EK_SunkenTemple}
    ${scripts_EK_SunwellPlateau}
    ${scripts_EK_Uldaman}
    ${scripts_EK_ZulAman}
    ${scripts_EK_ZulGurub}
	${scripts_EK_Zones}
    # Kalibmdor
    ${scripts_kalimdor}
    ${scripts_K_BlackfathomDeeps}
    ${scripts_K_BattleForMountHyjal}
    ${scripts_K_DarkPortal}
    ${scripts_K_EscapeFromDurnholdeKeep}
    ${scripts_K_Maraudon}
    ${scripts_K_OnyxiasLair}
    ${scripts_K_RazorfenDowns}
    ${scripts_K_RazorfenKraul}
    ${scripts_K_RuinsOfAhnQiraj}
    ${scripts_K_TempleOfAhnQiraj}
    ${scripts_K_WailingCaverns}
    ${scripts_K_ZulFarrak}
	${scripts_K_Zones}
    # Outland
    ${scripts_outland}
    ${scripts_O_AuchenaiCrypts}
    ${scripts_O_ManaTombs}
    ${scripts_O_SethekkHalls}
    ${scripts_O_ShadowLabyrinth}
    ${scripts_O_BlackTemple}
    ${scripts_O_SerpentShrine}
    ${scripts_O_SteamVault}
    ${scripts_O_Underbog}
    ${scripts_O_GruulsLair}
    ${scripts_O_BloodFurnace}
    ${scripts_O_HellfireRamparts}
    ${scripts_O_MagtheridonsLair}
    ${scripts_O_ShatteredHalls}
    ${scripts_O_arcatraz}
    ${scripts_O_botanica}
    ${scripts_O_Eye}
    ${scripts_O_Mechanar}
	${scripts_O_Zones}
    # World
    ${scripts_world}
    ${scripts_examples}
  )
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/SFMT
  ${CMAKE_SOURCE_DIR}/dep/mersennetwister
  ${CMAKE_SOURCE_DIR}/dep/zlib
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography/Authentication  
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic  
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/game/Accounts
  ${CMAKE_SOURCE_DIR}/src/server/game/Achievements
  ${CMAKE_SOURCE_DIR}/src/server/game/Addons
  ${CMAKE_SOURCE_DIR}/src/server/game/AI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/CoreAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/EventAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/ScriptedAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/SmartScripts
  ${CMAKE_SOURCE_DIR}/src/server/game/AuctionHouse
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlefields
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlefields/Zones  
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds/Zones
  ${CMAKE_SOURCE_DIR}/src/server/game/Calendar
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat/Channels
  ${CMAKE_SOURCE_DIR}/src/server/game/Conditions
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/game/Combat
  ${CMAKE_SOURCE_DIR}/src/server/game/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Corpse
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Creature
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/DynamicObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item/Container
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/GameObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object/Updates
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Pet
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Player
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Transport
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Unit
  ${CMAKE_SOURCE_DIR}/src/server/game/Events
  ${CMAKE_SOURCE_DIR}/src/server/game/Globals
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids/Cells
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids/Notifiers
  ${CMAKE_SOURCE_DIR}/src/server/game/Groups
  ${CMAKE_SOURCE_DIR}/src/server/game/Guilds
  ${CMAKE_SOURCE_DIR}/src/server/game/Handlers
  ${CMAKE_SOURCE_DIR}/src/server/game/Instances
  ${CMAKE_SOURCE_DIR}/src/server/game/Loot
  ${CMAKE_SOURCE_DIR}/src/server/game/Mails
  ${CMAKE_SOURCE_DIR}/src/server/game/Miscellaneous
  ${CMAKE_SOURCE_DIR}/src/server/game/Maps
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/MovementGenerators
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/Waypoints
  ${CMAKE_SOURCE_DIR}/src/server/game/Opcodes
  ${CMAKE_SOURCE_DIR}/src/server/game/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/src/server/game/PrecompiledHeaders
  ${CMAKE_SOURCE_DIR}/src/server/game/Quests
  ${CMAKE_SOURCE_DIR}/src/server/game/Reputation 
  ${CMAKE_SOURCE_DIR}/src/server/game/Scripting
  ${CMAKE_SOURCE_DIR}/src/server/game/Server
  ${CMAKE_SOURCE_DIR}/src/server/game/Server/Packets
  ${CMAKE_SOURCE_DIR}/src/server/game/Server/Protocol
  ${CMAKE_SOURCE_DIR}/src/server/game/Skills
  ${CMAKE_SOURCE_DIR}/src/server/game/Spells
  ${CMAKE_SOURCE_DIR}/src/server/game/Spells/Auras
  ${CMAKE_SOURCE_DIR}/src/server/game/Tickets
  ${CMAKE_SOURCE_DIR}/src/server/game/Tools
  ${CMAKE_SOURCE_DIR}/src/server/game/Warden   
  ${CMAKE_SOURCE_DIR}/src/server/game/Weather
  ${CMAKE_SOURCE_DIR}/src/server/game/World  
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR} 
)

add_library(scripts STATIC ${scripts_STAT_SRCS})

add_dependencies(scripts revision.h)

# Generate precompiled header
if ( USE_SCRIPTPCH )
  if (CMAKE_COMPILER_IS_GNUCXX)
    add_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/ScriptPCH.h)
  elseif(MSVC)
    add_native_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/ScriptPCH)
  endif()
endif()
