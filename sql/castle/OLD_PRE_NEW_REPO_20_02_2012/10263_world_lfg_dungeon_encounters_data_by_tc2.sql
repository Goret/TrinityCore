DELETE FROM `lfg_dungeon_encounters` WHERE `achievementId` IN (1068,1069,1070,1071,1072,1073,1074,1075,1076,1077,1078,1079,1080,1081,1082,1091,1092,1093,1094,1095,1096,1097,1231,1232,1233,1234,1235,1236,1237,1238,1239,1240,1241,1242,1504,1505,1506,1507,1508,1509,1510,1511,1512,1513,1514,1515,4026,4027,4715,4716,4720,4721,4726,4727);
INSERT INTO `lfg_dungeon_encounters` (`achievementId`,`dungeonId`) VALUES
(1092,258),
(1091,258),
(1094,258),
(1095,258),
(1096,258),
(1093,258),
(1097,258),
(1071,259),
(1069,259),
(1068,259),
(1072,259),
(1070,259),
(1074,259),
(1075,259),
(1076,259),
(1077,259),
(1078,259),
(1079,259),
(1080,259),
(1081,259),
(1082,259),
(1073,259),
(1504,262),
(1505,262),
(1515,262),
(1506,262),
(1507,262),
(1508,262),
(1509,262),
(1510,262),
(1511,262),
(1512,262),
(1513,262),
(1514,262),
(4027,262),
(4716,262),
(4721,262),
(4727,262),
(1242,261),
(1231,261),
(1241,261),
(1232,261),
(1233,261),
(1234,261),
(1235,261),
(1236,261),
(1237,261),
(1238,261),
(1239,261),
(1240,261),
(4026,261),
(4715,261),
(4720,261),
(4726,261);

-- Set Repeatable flag
UPDATE `quest_template` SET `SpecialFlags`=`SpecialFlags`|1 WHERE `entry` IN (24791,24789,24896,24895,24894,24889,24893,24892,24891,24890,24923);
-- Set LFD and repeatable flag
UPDATE `quest_template` SET `SpecialFlags`=`SpecialFlags`|8|1 WHERE `entry` IN (24790,24788,24922,24888,24887,24886,24881,24885,24884,24883,24882);