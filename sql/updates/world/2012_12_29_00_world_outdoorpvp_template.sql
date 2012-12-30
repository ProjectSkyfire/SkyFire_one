DROP TABLE IF EXISTS `outdoorpvp_template`;

CREATE TABLE `outdoorpvp_template` (
  `TypeId` TINYINT(2) UNSIGNED NOT NULL,
  `ScriptName` CHAR(64) NOT NULL DEFAULT '',
  `comment` TEXT,
  PRIMARY KEY (`TypeId`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='OutdoorPvP Templates';

LOCK TABLES `outdoorpvp_template` WRITE;
INSERT INTO `outdoorpvp_template` VALUES (1,'outdoorpvp_hp','Hellfire Peninsula'),(2,'outdoorpvp_na','Nagrand'),(3,'outdoorpvp_tf','Terokkar Forest'),(4,'outdoorpvp_zm','Zangarmarsh'),(5,'outdoorpvp_si','Silithus'),(6,'outdoorpvp_ep','Eastern Plaguelands');
UNLOCK TABLES;
