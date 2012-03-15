DROP TABLE IF EXISTS `channels`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `channels` (
  `m_name` TEXT NOT NULL,
  `m_team` INT(10) UNSIGNED NOT NULL,
  `m_announce` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `m_moderate` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `m_public` TINYINT(1) UNSIGNED NOT NULL DEFAULT '1',
  `m_password` TEXT,
  `BannedList` LONGTEXT,
  PRIMARY KEY  (`m_name`(10),`m_team`)
) ENGINE=INNODB DEFAULT CHARSET = utf8 ROW_FORMAT=DYNAMIC COMMENT='Channel System';
/*!40101 SET character_set_client = @saved_cs_client */;
--
-- Dumping data for table `channels`
--

LOCK TABLES `channels` WRITE;
/*!40000 ALTER TABLE `channels` DISABLE KEYS */;
/*!40000 ALTER TABLE `channels` ENABLE KEYS */;
UNLOCK TABLES;