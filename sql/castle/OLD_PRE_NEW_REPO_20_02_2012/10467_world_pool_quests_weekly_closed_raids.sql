-- Entferne Weekly Raid quests in Inis, welche bei uns noch nicht offen sind - Malygos, Flame Leviathan, Razorscale, Ignis, XT-002, Lord Marrowgar
DELETE FROM `pool_quest` WHERE `pool_entry` = 5678 AND `entry` IN (24584, 24585, 24586, 24587, 24588, 24590);