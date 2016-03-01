CREATE TABLE `character_custom` (
  `playerName` varchar(12) NOT NULL,
  `displayId` mediumint(8) unsigned DEFAULT NULL,
  `scale` float unsigned DEFAULT NULL,
  `phaseId` int(10) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
