/*Data update for the table `battleground_template` */
ALTER TABLE `battleground_template` ADD `ScriptName` CHAR(64) NOT NULL DEFAULT '' AFTER `HordeStartO`;
/*Data update for the table `transports` */
ALTER TABLE `transports` ADD `ScriptName` CHAR(64) NOT NULL DEFAULT '' AFTER `period`;
/*Data update for the table `game_weather` */
ALTER TABLE `game_weather` ADD `ScriptName` CHAR(64) NOT NULL DEFAULT '' AFTER `winter_storm_chance`;
