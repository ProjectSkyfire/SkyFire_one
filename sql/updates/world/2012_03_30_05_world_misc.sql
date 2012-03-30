-- Deeprun Rat Roundup

REPLACE INTO `creature_ai_scripts`(`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES 
(77780607,13016,8,2,100,1,21050,-1,80000,80000,36,13017,0,0,22,1,0,0,0,0,0,0,'Deeprun Rat - q6661 - morph as Enthralled Deeprun Rat - set phase 1'),
(77780608,13016,0,1,100,1,79000,79000,79000,79000,37,0,0,0,22,0,0,0,0,0,0,0,'Deeprun Rat - q6661 - morph back - back phase 0'),
(77770609,13016,1,1,100,1,79000,79000,79000,79000,37,0,0,0,22,0,0,0,0,0,0,0,'Deeprun Rat - q6661 - morph back - back phase 0 also when not in combat - case player leave zone'),(77770610,13016,0,1,100,1,1,1,10000,10000,20,0,0,0,11,42269,0,0,0,0,0,0,'Deeprun Rat - q6661 - stop move stop melee - ping Monty 7yards'),
(77770611,13016,0,1,100,0,1000,1000,0,0,11,21051,0,0,0,0,0,0,0,0,0,0,'Deeprun Rat - q6661 - aura Melodious Rapture Visual'),
(77770612,13016,0,2,100,0,0,0,0,0,24,0,0,0,0,0,0,0,0,0,0,0,'Deeprun Rat - q6661 - evade'),
(77770613,12997,8,0,100,1,42269,-1,10000,10000,1,-7150,0,0,11,42272,0,0,11,21052,0,0,'Monty - q6661 - Monty say - ping & bashes rats'),
(77770614,13016,8,1,100,1,42272,-1,10000,10000,33,13017,1,0,37,0,0,0,0,0,0,0,'Deeprun Rat - q6661 - on Monty ping give KC & die'),
(77770615,12997,4,0,100,0,0,0,0,0,24,0,0,0,0,0,0,0,0,0,0,0,'Monty - q6661 - ignore aggro');

REPLACE INTO `creature_ai_texts` VALUES
(-7150,'Ahh... Just what I needed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,0,0,'20885');

UPDATE creature_template SET  AIName='EventAI' WHERE entry IN (13016,12997);