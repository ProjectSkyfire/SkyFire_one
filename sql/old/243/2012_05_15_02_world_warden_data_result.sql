DROP TABLE IF EXISTS `warden_data_result`;
CREATE TABLE `warden_data_result` (
  `id` int(4) NOT NULL auto_increment,
  `check` int(3) default NULL,
  `data` tinytext,
  `str` tinytext,
  `address` int(8) default NULL,
  `length` int(2) default NULL,
  `result` tinytext,
  `comment` text,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

LOCK TABLES `warden_data_result` WRITE;
INSERT INTO `warden_data_result` VALUES(1003, 243, '', '', 8100062, 2, '7541', 'air jump');
INSERT INTO `warden_data_result` VALUES(1002, 243, '', '', 9208923, 5, 'C0854A3340', 'gravity');
INSERT INTO `warden_data_result` VALUES(1001, 243, '', '', 8979979, 5, 'E04D62503F', 'Hyper speed');
INSERT INTO `warden_data_result` VALUES(1000, 243, '', '', 4840352, 2, '558B', 'lua protection');
UNLOCK TABLES;