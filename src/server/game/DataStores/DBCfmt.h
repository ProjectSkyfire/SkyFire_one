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

#ifndef DBCSFRM_H
#define DBCSFRM_H

const char AreaTableEntryfmt[]="iiinixxxxxissssssssssssssssxixxxxxx";
const char AuctionHouseEntryfmt[]="niiixxxxxxxxxxxxxxxxx";
const char AreaTriggerEntryfmt[]="niffffffff";
const char BankBagSlotPricesEntryfmt[]="ni";
const char BattlemasterListEntryfmt[]="niiixxxxxiiiixxssssssssssssssssxx";
const char CharStartOutfitEntryfmt[]="diiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxx";
// 3*12 new item fields in 3.0.x
//const char CharStartOutfitEntryfmt[]="diiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char CharTitlesEntryfmt[]="nxssssssssssssssssxxxxxxxxxxxxxxxxxxi";
const char ChatChannelsEntryfmt[]="iixssssssssssssssssxxxxxxxxxxxxxxxxxx";
                                                            // ChatChannelsEntryfmt, index not used (more compact store)
//const char ChrClassesEntryfmt[]="nxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxix";
const char ChrClassesEntryfmt[]="nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxix";
const char ChrRacesEntryfmt[]="nxixiixxixxxxissssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
const char CinematicSequencesEntryfmt[]="nxxxxxxxxx";
const char CreatureDisplayInfofmt[]="nxxxfxxxxxxxxx";
const char CreatureFamilyfmt[]="nfifiiiissssssssssssssssxx";
const char CreatureSpellDatafmt[]="nxxxxxxxx";
const char DurabilityCostsfmt[]="niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
const char DurabilityQualityfmt[]="nf";
const char EmotesEntryfmt[]="nxxiiix";
const char EmotesTextEntryfmt[]="nxixxxxxxxxxxxxxxxx";
const char FactionEntryfmt[]="niiiiiiiiiiiiiiiiiissssssssssssssssxxxxxxxxxxxxxxxxxx";
const char FactionTemplateEntryfmt[]="niiiiiiiiiiiii";
const char GemPropertiesEntryfmt[]="nixxi";
const char GtCombatRatingsfmt[]="f";
const char GtChanceToMeleeCritBasefmt[]="f";
const char GtChanceToMeleeCritfmt[]="f";
const char GtChanceToSpellCritBasefmt[]="f";
const char GtChanceToSpellCritfmt[]="f";
const char GtOCTRegenHPfmt[]="f";
//const char GtOCTRegenMPfmt[]="f";
const char GtRegenHPPerSptfmt[]="f";
const char GtRegenMPPerSptfmt[]="f";
const char Itemfmt[]="niii";
//const char ItemDisplayTemplateEntryfmt[]="nxxxxxxxxxxixxxxxxxxxxx";
//const char ItemCondExtCostsEntryfmt[]="xiii";
const char ItemExtendedCostEntryfmt[]="niiiiiiiiiiiii";
const char ItemRandomPropertiesfmt[]="nxiiixxxxxxxxxxxxxxxxxxx";
const char ItemRandomSuffixfmt[]="nxxxxxxxxxxxxxxxxxxiiiiii";
const char ItemSetEntryfmt[]="dssssssssssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
const char LockEntryfmt[]="niiiiixxxiiiiixxxiixxxxxxxxxxxxxx";
const char MailTemplateEntryfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char MapEntryfmt[]="nxixssssssssssssssssxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiffiixxi";
const char QuestSortEntryfmt[]="nxxxxxxxxxxxxxxxxx";
const char RandomPropertiesPointsfmt[]="niiiiiiiiiiiiiii";
const char SkillLinefmt[]="nixssssssssssssssssxxxxxxxxxxxxxxxxxxi";
const char SkillLineAbilityfmt[]="niiiixxiiiiixxi";
const char SoundEntriesfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellCastTimefmt[]="nixx";
const char SpellDurationfmt[]="niii";
const char SpellEntryfmt[]="nixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffixiixssssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxxiiii";
const char SpellFocusObjectfmt[]="nxxxxxxxxxxxxxxxxx";
const char SpellItemEnchantmentfmt[]="niiiiiixxxiiissssssssssssssssxiiii";
const char SpellItemEnchantmentConditionfmt[]="nbbbbbxxxxxbbbbbbbbbbiiiiiXXXXX";
const char SpellRadiusfmt[]="nfxf";
const char SpellRangefmt[]="nffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellShapeshiftfmt[]="nxxxxxxxxxxxxxxxxxxiixixxxxxxxxxxxx";
const char StableSlotPricesfmt[] = "ni";
const char SummonPropertiesfmt[] = "niiiii";
const char TalentEntryfmt[]="niiiiiiiixxxxixxixxxi";
const char TalentTabEntryfmt[]="nxxxxxxxxxxxxxxxxxxxiix";
const char TaxiNodesEntryfmt[]="nifffxxxxxxxxxxxxxxxxxii";
const char TaxiPathEntryfmt[]="niii";
const char TaxiPathNodeEntryfmt[]="diiifffiixx";
const char TotemCategoryEntryfmt[]="nxxxxxxxxxxxxxxxxxii";
const char WorldMapAreaEntryfmt[]="xinxffffi";
const char WMOAreaTableEntryfmt[]="niiixxxxxiixxxxxxxxxxxxxxxxx";
const char WorldSafeLocsEntryfmt[]="nifffxxxxxxxxxxxxxxxxx";

#endif

