ALTER TABLE `character_battleground_data`
ROW_FORMAT=DEFAULT,
CHANGE `instance_id` `instanceId` INT(10) UNSIGNED NOT NULL COMMENT 'Instance Identifier';
