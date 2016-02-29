CREATE TABLE `request_mj` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `player_name` varchar(12) NOT NULL,
  `account_id` bigint(10) NOT NULL,
  `msg` text NOT NULL,
  `veilleur_id` bigint(10) DEFAULT NULL,
  `closed` tinyint(1) unsigned zerofill NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;