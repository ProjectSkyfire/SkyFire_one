-- Changed type summons/appear commands
DELETE FROM `command` WHERE `name`='goname';
INSERT  INTO `command`(`name`,`security`,`help`) VALUES ('appear',1,'Syntax: .appear [$charactername]\r\n\r\nTeleport to the given character. Either specify the character name or click on the character\'s portrait, e.g. when you are in a group. Character can be offline.');

DELETE FROM `command` WHERE `name`='namego';
INSERT  INTO `command`(`name`,`security`,`help`) VALUES ('summon',1,'Syntax: .summon [$charactername]\r\n\r\nTeleport the given character to you. Character can be offline.');

DELETE FROM `command` WHERE `name`='groupgo';
INSERT  INTO `command`(`name`,`security`,`help`) VALUES ('groupsummon',1,'Syntax: .groupsummon [$charactername]\r\n\r\nTeleport the given character and his group to you. Teleported only online characters but original selected group member can be offline.');

