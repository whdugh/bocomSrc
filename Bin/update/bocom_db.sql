drop database if exists bocom_db;
create database bocom_db;

SET NAMES UTF8;
SET FOREIGN_KEY_CHECKS=0;

use bocom_db;

CREATE TABLE `CHAN_INFO` (
  `CHAN_ID` int(11) NOT NULL,
  `CHAN_PLACE` varchar(64) default NULL,
  `CHAN_KIND` int(11) default NULL,
  `CHAN_FORMAT` int(11) default NULL,
  `CHAN_EVENTCAPTURETIME` int(11) default NULL,
  `CHAN_TRAFFICSTATTIME` int(11) default NULL,
  `CHAN_EVENTDETECTDELAY` int(11) default NULL,
  `CHAN_BRIGHTNESS` int(11) default NULL,
  `CHAN_CONTRAST` int(11) default NULL,
  `CHAN_SATURATION` int(11) default NULL,
  `CHAN_HUE` int(11) default NULL,
  `CHAN_CAP_TYPE` int(11) default NULL,
  `CHAN_CAP_BEGINTIME` int(11) default NULL,
  `CHAN_CAP_ENDTIME` int(11) default NULL,
  `CHAN_YUV_HOST` varchar(16) default NULL,
  `CHAN_YUV_PORT` int(11) default NULL,
  `CHAN_SRC_FILE` varchar(64) default NULL,
  `CHAN_DETECT_KIND` int(11) default NULL,
  `CHAN_DETECT_TIME` int(11) default NULL,
  `CHAN_EVENT_CAPTURE` tinyint(4) default NULL,
  `CHAN_RAW_CAPTURE` tinyint(4) default NULL,
  `CHAN_RUN` int(11) default NULL,
  `CHAN_RMSHADE` tinyint(4) default NULL,
  `CHAN_RMTINGLE` tinyint(4) default NULL,
  `CHAN_SENSITIVE` tinyint(4) default NULL,
  `CAMERA_ID` int(11) default NULL,
  `CHAN_DIRECTION` int(11) default NULL,
  `CHAN_SHOWTIME` int(11) default NULL,
  `CAMERA_TYPE` int(11) default NULL,
  `PANNEL_ID` int(11) default NULL,
  `CAMERA_STATE` int(11) default NULL,
  `CANTON_ID` char(6) default NULL,
  `SYN_HOST` varchar(16) default NULL,
  `SYN_PORT` int(11) default NULL,
  `SYN_MAC` varchar(64) default NULL,
  `CHAN_PRESET` int(11) default NULL,
  `MONITOR_ID` int(11) default NULL,
  `VIDEO_ID` int(11) default NULL,
  `USERNAME` varchar(16) default NULL,
  `PASSWD` varchar(16) default NULL,
  `YUV_FORMAT` int(11) default NULL,
  `VIDEO_BEGINTIME` datetime default NULL,
  `VIDEO_ENDTIME` datetime default NULL,
  `WORKMODE` int(11) default NULL,
  `CAMERAIP` varchar(16) default NULL,
  `CAM_MULTI_IP` varchar(16) default NULL,
  `CAMERASELF_ID` int(11) default 0,
  PRIMARY KEY  (`CHAN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


CREATE TABLE `NUMBER_PLATE_INFO` (
  `ID` bigint(20) NOT NULL auto_increment,
  `NUMBER` varchar(20) default NULL,
  `COLOR` int(11) default NULL,
  `CREDIT` int(11) default NULL,
  `CHANNEL` int(11) default NULL,
  `ROAD` int(11) default NULL,
  `TIME` datetime default NULL,
  `MITIME` int(11) default NULL,
  `SMALLPICSIZE` int(11) default NULL,
  `SMALLPICWIDTH` int(11) default NULL,
  `SMALLPICHEIGHT` int(11) default NULL,
  `PICSIZE` int(11) default NULL,
  `PICWIDTH` int(11) default NULL,
  `PICHEIGHT` int(11) default NULL,
  `POSLEFT` int(11) default NULL,
  `POSTOP` int(11) default NULL,
  `POSRIGHT` int(11) default NULL,
  `POSBOTTOM` int(11) default NULL,
  `PICPATH` varchar(255) default NULL,
  `SMALLPICPATH` varchar(255) default NULL,
  `STATUS` tinyint(4) default NULL COMMENT '状态',
  `CARCOLOR` int(11) default NULL COMMENT '车身颜色',
  `TYPE` int(11) default NULL COMMENT '类型',
  `FACTORY` int(11) default NULL COMMENT '厂家',
  `SPEED` double default NULL,
  `PIC_ID` bigint(20) default NULL,
  `CARCOLORWEIGHT` int(11) default NULL,
  `CARCOLORSECOND` int(11) default NULL,
  `CARCOLORWEIGHTSECOND` int(11) default NULL,
  `CARNUMBER_TYPE` int(11) default NULL,
  `CONTOUR_WIDTH` int(11) default NULL,
  `PECCANCY_KIND` int(11) default NULL,
  `TYPE_DETAIL` int(11) default NULL,
  `DIRECTION` int(11) default NULL,
  `VIDEOPATH` varchar(255) default NULL,
  `VIDEOSAVE` tinyint(4) default NULL,
  `TIMESECOND` datetime default NULL,
  `MITIMESECOND` int(11) default NULL,
  `LATITUDE` int(11) default NULL,
  `LONGITUDE` int(11) default NULL, 
  `FLAG` int(11) default 0,
	`PIC_FLAG` int(11) default 0,
	`VIDEO_FLAG` int(11) default 0, 
	`REDLIGHT_TIME` int(11) default 0, 
	`PLACE` varchar(255) default NULL, 
	`CrossingNumber` varchar(32) default NULL, 
	`REDLIGHT_BEGIN_TIME` datetime default NULL,
  `REDLIGHT_BEGIN_MITIME` int(11) default 0,
	`REDLIGHT_END_TIME` datetime default NULL,
  `REDLIGHT_END_MITIME` int(11) default 0, 
  `LIMIT_SPEED` int(11) default 0,
	`OVER_SPEED` int(11) default 0,
  PRIMARY KEY  (`ID`),
  KEY `TIME_INDEX` (`TIME`),
  KEY `PATH_INDEX` (`PICPATH`), 
  KEY `VIDEO_INDEX` (`VIDEOPATH`),
  KEY `STATUS_INDEX` (`STATUS`,`TIME`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


CREATE TABLE `SPECIAL_CARD_INFO` (
  `ID` int(11) NOT NULL auto_increment,
  `DEPARTMENT` varchar(60) character set utf8 default NULL,
  `BEHAVIOR_KIND` int(11) default NULL,
  `BEGIN_TIME` datetime default NULL,
  `END_TIME` datetime default NULL,
  `NUMBER` varchar(20) character set utf8 default NULL,
  `CARNUMBER_TYPE` int(11) default NULL,
  `CAR_TYPE` int(11) default NULL,
  `COLOR` int(11) default NULL,
  `KIND` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


CREATE TABLE `SYSTEM_EVENT_INFO` (
  `ID` bigint(20) NOT NULL auto_increment,
  `TIME` datetime default NULL,
  `EVENT` varchar(64) default NULL,
  `LEVEL` int(11) default NULL,
  `STATUS` tinyint(4) default NULL,
  `CODE` int(11) default NULL,
  PRIMARY KEY  (`ID`),
  KEY `TIME_INDEX` (`TIME`),
  KEY `STATUS_INDEX` (`STATUS`,`TIME`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


CREATE TABLE `TIME_INFO` (
  `ID` int(11) NOT NULL auto_increment,
  `TIME` datetime default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


CREATE TABLE `TRAFFIC_EVENT_INFO` (
  `ID` bigint(20) NOT NULL auto_increment,
  `CHANNEL` int(11) default NULL,
  `ROAD` int(11) default NULL,
  `KIND` int(11) default NULL,
  `BEGIN_TIME` datetime default NULL,
  `BEGIN_MITIME` int(11) default NULL,
  `END_TIME` datetime default NULL,
  `END_MITIME` int(11) default NULL,
  `PICSIZE` int(11) default NULL,
  `PICWIDTH` int(11) default NULL,
  `PICHEIGHT` int(11) default NULL,
  `POSX` int(11) default NULL,
  `POSY` int(11) default NULL,
  `TEXT` varchar(16) default NULL,
  `STATUS` tinyint(4) default NULL,
  `PICPATH` varchar(256) default NULL,
  `BEGIN_VIDEO_TIME` datetime default NULL,
  `BEGIN_VIDEO_MITIME` int(11) default NULL,
  `END_VIDEO_TIME` datetime default NULL,
  `END_VIDEO_MITIME` int(11) default NULL,
  `VIDEOPATH` varchar(256) default NULL,
  `COLOR` int(11) default NULL,
  `TYPE` int(11) default NULL,
  `SPEED` double default NULL,
  `VIDEOSAVE` tinyint(4) default NULL,
  `PIC_ID` bigint(20) default NULL,
  `VIDEO_ID` bigint(20) default NULL,
  `COLORWEIGHT` int(11) default NULL,
  `COLORSECOND` int(11) default NULL,
  `COLORWEIGHTSECOND` int(11) default NULL,
  `COLORTHIRD` int(11) default NULL,
  `COLORWEIGHTTHIRD` int(11) default NULL,
  `EVENT_ID` bigint(20) default NULL,
  `PLATE_ID` bigint(20) default NULL,
  `DIRECTION` int(11) default NULL,
  `TIMESECOND` datetime default NULL,
  `MITIMESECOND` int(11) default NULL, 
  PRIMARY KEY  (`ID`),
  KEY `TIME_INDEX` (`BEGIN_TIME`),
  KEY `PATH_INDEX` (`PICPATH`), 
  KEY `VIDEO_INDEX` (`VIDEOPATH`),
  KEY `STATUS_INDEX` (`STATUS`,`BEGIN_TIME`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


CREATE TABLE `TRAFFIC_STATISTIC_INFO` (
  `S_ID` bigint(20) NOT NULL auto_increment,
  `ID` bigint(20) NOT NULL COMMENT '序号',
  `TIME` datetime default NULL COMMENT '发生时间',
  `CHANNEL` int(11) default NULL COMMENT '发生通道',
  `ROAD` int(11) default NULL COMMENT '车道',
  `STATTIMELEN` int(11) default NULL COMMENT '结束时间',
  `KIND` int(11) default NULL COMMENT '统计类型',
  `VALUE` double default NULL COMMENT '类型值',
  `STATUS` tinyint(4) default NULL COMMENT '交通流量的图片或视频文件ID',
  `TYPE` int(11) default NULL COMMENT '车牌还是事件的流量统计',
  PRIMARY KEY  (`S_ID`),
  KEY `TIME_BEGIN_INDEX` (`TIME`),
  KEY `STATUS_INDEX` (`STATUS`,`TIME`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


CREATE TABLE `USER_INFO` (
  `USER_NAME` varchar(10) NOT NULL,
  `USER_PW` varchar(16) NOT NULL,
  `USER_RIGHT1` int(11) default '0',
  `USER_RIGHT2` int(11) default NULL,
  `ADD_USER` tinyint(4) default '1',
  `DEL_USER` tinyint(4) default '1',
  `MODIFY_USER` tinyint(4) default '1',
  `LIST_USER` tinyint(4) default '1',
  `ADD_CHANNEL` tinyint(4) default '1',
  `DEL_CHANNEL` tinyint(4) default '1',
  `MODIFY_CHANNEL` tinyint(4) default '1',
  `SEARCH_EVENT` tinyint(4) default '1',
  `SEARCH_ALARM` tinyint(4) default '1',
  `SEARCH_CARD` tinyint(4) default '1',
  `SEARCH_RECORD` tinyint(4) default '1',
  `SEARCH_LOG` tinyint(4) default '1',
  `CHANNEL_PARA` tinyint(4) default '1',
  `ADJUST_CHANNEL_PARA` tinyint(4) default '1',
  `SAVE_CHANNEL_PARA` tinyint(4) default '1',
  `GET_ROADWAY` tinyint(4) default '1',
  `SAVE_ROADWAY` tinyint(4) default '1',
  `DELETE_ROADWAY` tinyint(4) default '1',
  `GET_SYSINFO` tinyint(4) default '1',
  `BACKUPDB` tinyint(4) default '1',
  `DEL_EVENT` tinyint(4) default '1',
  `DEL_ALARM` tinyint(4) default '1',
  `DEL_RECORD` tinyint(4) default '1',
  `DEL_CARD` tinyint(4) default '1',
  `DEL_LOG` tinyint(4) default '1',
  PRIMARY KEY  (`USER_NAME`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `PIC_INFO` (
  `ID` bigint(20) NOT NULL auto_increment,
  `PIC_ID` bigint(20) default NULL,
  `VIDEO_ID` bigint(20) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `VIDEO_FILE_INFO` (
  `ID` bigint(20) NOT NULL auto_increment,
  `CHANNEL` int(11) default NULL,
  `BEGIN_TIME` datetime default NULL,
  `BEGIN_MITIME` int(11) default NULL,
  `END_TIME` datetime default NULL,
  `END_MITIME` int(11) default NULL,
  `PATH` varchar(256) default NULL,
  `VIDEO_TYPE` int(11) default NULL,
  `STATUS` tinyint(4) default NULL,
  PRIMARY KEY  (`ID`),
  KEY `TIME_INDEX` (`BEGIN_TIME`), 
  KEY `VIDEO_INDEX` (`PATH`),
  KEY `STATUS_INDEX` (`STATUS`,`BEGIN_TIME`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `DEVICE_INFO` (
  `ID` int(11) NOT NULL auto_increment,
  `DETECTOR_TYPE` varchar(64) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `HISTORY_VIDEO_INFO` (
  `ID` bigint(20) NOT NULL auto_increment,
  `DEVICE_ID` int(11) default NULL,
  `VIDEO_NAME` varchar(255) default NULL,
  `VIDEO_FORMAT` int(11) default NULL,
  `VIDEO_BEGIN_TIME` datetime default NULL,
  `VIDEO_BEGIN_MITIME` int(11) default NULL,
  `VIDEO_END_TIME` datetime default NULL,
  `VIDEO_END_MITIME` int(11) default NULL,
  `DETECT_STATUS` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `BASE_PLATE_INFO` (
  `P_ID` bigint(20) NOT NULL auto_increment,
  `NUMBER` varchar(20) default NULL,
  `COLOR` int(11) default NULL,
  `CARNUMBER_TYPE` int(11) default NULL,
  `CONTOUR_WIDTH` int(11) default NULL,
  `PECCANCY_KIND` int(11) default NULL,
  PRIMARY KEY  (`P_ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8; 

CREATE TABLE `NUMBER_PLATE_INFO_RECV`(
  `ID` bigint(20) NOT NULL auto_increment,
  `NUMBER` varchar(20) default NULL,
  `COLOR` int(11) default NULL,
  `CREDIT` int(11) default NULL,
  `CHANNEL` int(11) default NULL,
  `ROAD` int(11) default NULL,
  `TIME` datetime default NULL,
  `MITIME` int(11) default NULL,
  `SMALLPICSIZE` int(11) default NULL,
  `SMALLPICWIDTH` int(11) default NULL,
  `SMALLPICHEIGHT` int(11) default NULL,
  `PICSIZE` int(11) default NULL,
  `PICWIDTH` int(11) default NULL,
  `PICHEIGHT` int(11) default NULL,
  `POSLEFT` int(11) default NULL,
  `POSTOP` int(11) default NULL,
  `POSRIGHT` int(11) default NULL,
  `POSBOTTOM` int(11) default NULL,
  `PICPATH` varchar(255) default NULL,
  `SMALLPICPATH` varchar(255) default NULL,
  `STATUS` tinyint(4) default NULL COMMENT '状态',
  `CARCOLOR` int(11) default NULL COMMENT '车身颜色',
  `TYPE` int(11) default NULL COMMENT '类型',
  `FACTORY` int(11) default NULL COMMENT '厂家',
  `SPEED` double default NULL,
  `PIC_ID` bigint(20) default NULL,
  `CARCOLORWEIGHT` int(11) default NULL,
  `CARCOLORSECOND` int(11) default NULL,
  `CARCOLORWEIGHTSECOND` int(11) default NULL,
  `CARNUMBER_TYPE` int(11) default NULL,
  `CONTOUR_WIDTH` int(11) default NULL,
  `PECCANCY_KIND` int(11) default NULL,
  `TYPE_DETAIL` int(11) default NULL,
  `DIRECTION` int(11) default NULL,
  `VIDEOPATH` varchar(255) default NULL,
  `VIDEOSAVE` tinyint(4) default NULL,
  `TIMESECOND` datetime default NULL,
  `MITIMESECOND` int(11) default NULL,
  `PLACE` varchar(255) default NULL,   
  `CrossingNumber` varchar(32) default NULL, 
  PRIMARY KEY  (`ID`),
  KEY `TIME_INDEX` (`TIME`),
  KEY `PATH_INDEX` (`PICPATH`),
  KEY `STATUS_INDEX` (`STATUS`,`TIME`)) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
                                                                                             
CREATE TABLE `DEVICE_STATUS`(
	`JXDM` varchar(15) default NULL,
	`JCSJ` datetime default NULL,
	`ZTLX` varchar(15) default NULL,
	`ZTMS` int(11) default NULL,
	`YCXX` varchar(100) default NULL) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO `USER_INFO` VALUES ('admin', 'admin', '1', null, '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1');
INSERT INTO `USER_INFO` VALUES ('123', '123', '0', null, '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1');

INSERT INTO `PIC_INFO` VALUES ('1', '1','1');
INSERT INTO `DEVICE_INFO` VALUES ('1', 'MVS-1000-LVS-121Y');
