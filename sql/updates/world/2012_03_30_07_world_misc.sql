-- For Love Eternal --

REPLACE INTO `creature_ai_scripts` (`id`, `creature_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_type`, `action1_param1`, `action1_param2` ,`action1_param3`, `action2_type`, `action2_param1`, `action2_param2`, `action2_param3`, `action3_type`, `action3_param1`, `action3_param2`, `action3_param3`, `comment`) VALUES

('364401','3644','17','254','100','1','3843','0','0','0','23','1','0','0','17','73','0','0','5','18','0','0','Cerellean Whiteclaw - Stands and Cries - Phase Initial Set for Quest 963'),
('364402','3644','1','253','100','1','4000','4000','4000','4000','5','1','0','0','1','-803','0','0','23','1','0','0','Cerellean Whiteclaw - Dialog 1 for Quest 963'),
('364403','3644','1','251','100','1','9000','9000','9000','9000','5','1','0','0','1','-804','0','0','23','1','0','0','Certellean Whiteclaw - Dialog 2 for Quest 963'),
('364404','3644','1','247','100','1','5000','5000','5000','5000','5','1','0','0','1','-805','0','0','23','1','0','0','Certellean Whiteclaw - Dialog 3 for Quest 963'),
('364405','3644','1','223','100','1','2000','2000','2000','2000','5','5','0','0','1','-806','0','0','23','1','0','0','Certellean Whiteclaw - Dialog 4 for Quest 963'),
('364406','3644','1','191','100','1','15000','15000','15000','15000','5','5','0','0','1','-807','0','0','23','1','0','0','Certellean Whiteclaw - Dialog 5 for Quest 963'),
('364407','3644','1','239','100','1','15000','15000','15000','15000','17','73','8','0','23','1','0','0','0','0','0','0','Cerellean Whiteclaw - Kneels Down for Quest 963'),
('364408','3644','1','127','100','1','15000','15000','15000','15000','5','18','0','0','22','0','0','0','0','0','0','0','Cerellean Whiteclaw - Cries Closing for Quest 963'),
('384301','3843','1','0','100','0','10000','10000','0','0','5','1','0','0','1','-808','0','0','0','0','0','0','Anaya Dawnrunner - Dialog 1 for Quest 963'),
('384302','3843','1','0','100','0','25000','25000','0','0','5','1','0','0','1','-809','0','0','0','0','0','0','Anaya Dawnrunner - Dialog 2 for Quest 963'),
('384303','3843','1','0','100','0','30000','30000','0','0','5','1','0','0','1','-810','0','0','0','0','0','0','Anaya Dawnrunner - Dialog 3 for Quest 963'),
('384304','3843','1','0','100','0','40000','40000','0','0','5','1','0','0','1','-811','0','0','0','0','0','0','Anaya Dawnrunner - Dialog 4 for Quest 963'),
('384305','3843','1','0','100','0','44000','44000','0','0','1','-812','0','0','0','0','0','0','0','0','0','0','Anaya Dawnrunner - Closing Emote for Quest 963');

REPLACE INTO `creature_ai_texts`(`entry`, `content_default`, `sound`, `type`, `language`, `comment`) VALUES

('-803','Anaya...? Do my eyes deceive me? Is it really you?','0','0','0','3644 for Quest 963'),
('-804','That fates should be so cruel as to permit us only this after a thousand years apart...','0','0','0','3644 for Quest 963'),
('-805','Do you hate me\, my love? That I was forced to destroy your living form\, that your spirit be released from unhappy bondage.','0','0','0','3644 for Quest 963'),
('-806','No! Anaya... Anaya! Don\'t leave me! Please...','0','0','0','3644 for Quest 963'),
('-807','How\, my love? How will I find the strength to face the ages of the world without you by my side...','0','0','0','3644 for Quest 963'),
('-808','The ages have been cruel to you and I\, my love\, but be assured\, it is\, and at long last we are reunited.','0','0','0','3843 for Quest 963'),
('-809','Let it not trouble your heart\, beloved. You have freed me from slavery\, and for that I love you all the more.','0','0','0','3843 for Quest 963'),
('-810','Sadly\, even this must be cut short... The ties that bind me to this world weaken\, and pull me away...','0','0','0','3843 for Quest 963'),
('-811','Farewell\, Cerellean\, until we are joined once again...','0','0','0','3843 for Quest 963'),
('-812','Anaya\'s soft voice trails away into the mists\, \"Know that I love you always...\"','0','2','0','3843 for Quest 963');

UPDATE creature_template SET  AIName='EventAI' WHERE entry IN (3843,3644);
