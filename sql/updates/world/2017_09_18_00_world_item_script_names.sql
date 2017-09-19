/*Data for the table `item_script_names` */
DROP TABLE IF EXISTS `item_script_names`;

CREATE TABLE `item_script_names` (
  `Id` INT(10) UNSIGNED NOT NULL,
  `ScriptName` VARCHAR(64) NOT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;


INSERT  INTO `item_script_names`(`Id`,`ScriptName`) VALUES (24538,'item_only_for_flight'),(34475,'item_only_for_flight'),(34489,'item_only_for_flight'),(31742,'item_nether_wraith_beacon'),(39878,'item_mysterious_egg'),(44717,'item_disgusting_jar'),(33098,'item_petrov_cluster_bombs'),(5397,'item_defias_gunpowder'),(31088,'item_tainted_core');

