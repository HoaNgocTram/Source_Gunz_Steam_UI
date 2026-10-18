#ifndef MSHAREDCOMMANDTABLE_H
#define MSHAREDCOMMANDTABLE_H

class MCommandManager;


#define MCOMMAND_VERSION	59 // Custom: Updated CMD version



#define MSCT_MASTER			1
#define MSCT_CLIENT			2
#define MSCT_MATCHSERVER	4
#define MSCT_AGENT			8
#define MSCT_CHECKER		16
#define MSCT_ALL			(MSCT_MASTER+MSCT_CLIENT+MSCT_AGENT)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ÁÖÀÇ : Ä¿¸ÇµEID¸¦ ¼öÁ¤ÇÏ¸E±âÁ¸¿¡ ÀúÀåÇÑ ¸®ÇÃ·¹ÀÌ ÆÄÀÏÀÌ Àç»ýµÇÁE¾ÊÀ» ¼EÀÖ½À´Ï´Ù.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// ¸ðµEÄ¿¹Â´ÏÄÉÀÌÅÍ°¡ °øÀ¯ÇÏ´Â Ä¿¸ÇµE
void MAddSharedCommandTable(MCommandManager* pCommandManager, int nSharedType);


/// ¼­¹EÅ¬¶óÀÌ¾ðÆ® °øÀ¯ »ó¼E
#define MATCH_CYCLE_CHECK_SPEEDHACK	20000


// ±âº» Ä¿¸ÇµE¸®½ºÆ®
#define MC_HELP						0
#define MC_VERSION					1

#define MC_DEBUG_TEST				100

#define MC_LOCAL_INFO				201		///< Local Á¤º¸¸¦ ¾ò´Â´Ù.
#define MC_LOCAL_ECHO				202		///< Local Echo Å×½ºÆ®
#define MC_LOCAL_LOGIN				203		///< Login Ã³¸® Local Command



#define MC_NET_ENUM					301		///< ¿¬°áÇÒ ¼EÀÖ´Â Ä¿¹Â´ÏÄÉÀÌÅÍ Enum
#define MC_NET_CONNECT				302		///< Ä¿¹Â´ÏÄÉÀÌÅÍ¿Í ¿¬°E
#define MC_NET_DISCONNECT			303		///< Ä¿¹Â´ÏÄÉÀÌÅÍ¿Í ¿¬°EÇØÁ¦
#define MC_NET_CLEAR				304		///< Ä¿¹Â´ÏÄÉÀÌÅÍ¿Í ¿¬°EÇØÁ¦ ÈÄ °EÃÀÚ¿EÃ³¸®
#define MC_NET_ONCONNECT			311		///< Ä¿¹Â´ÏÄÉÀÌÅÍ¿Í ¿¬°áµÇ¾úÀ» ¶§
#define MC_NET_ONDISCONNECT			312		///< Ä¿¹Â´ÏÄÉÀÌÅÍ¿Í ¿¬°áÀÌ ²÷°åÀ» ¶§
#define MC_NET_ONERROR				313		///< Ä¿¹Â´ÏÄÉÀÌÅÍ¿Í ¿¬°E¿¡·¯
#define MC_NET_CHECKPING			321		///< Ä¿¹Â´ÏÄÉÀÌÅÍ ¹ÝÀÀ¼Óµµ°Ë»E
#define MC_NET_PING					322		///< Ä¿¹Â´ÏÄÉÀÌÅÍ ¹ÝÀÀ¼Óµµ°Ë»E¿äÃ»
#define MC_NET_PONG					323		///< Ä¿¹Â´ÏÄÉÀÌÅÍ ¹ÝÀÀ¼Óµµ°Ë»EÀÀ´E

#define MC_HSHIELD_PING				324		///< ÇÙ½ÇµE¸Þ¼¼ÁE¿äÃ»(¸ÅÄ¡¼­¹E-> Å¬¶óÀÌ¾ðÆ®)
#define MC_HSHIELD_PONG				325		///< ÇÙ½ÇµE¸Þ¼¼ÁEÀÀ´EÅ¬¶óÀÌ¾ðÆ® -> ¸ÅÄ¡¼­¹E

#define MC_NET_BANPLAYER_FLOODING	326		///< CommandBuilder¿¡¼­ Flooding °ËÃâ½Ã ¸¸µå´Â Command

#define MC_NET_CONNECTTOZONESERVER	331		///< 127.0.0.1:6000 ¿¬°E

#define MC_NET_REQUEST_INFO			341		///< Net Á¤º¸¸¦ ¿äÃ»ÇÑ´Ù.
#define MC_NET_RESPONSE_INFO		342		///< Net Á¤º¸¸¦ ¾ò´Â´Ù.
#define MC_NET_REQUEST_UID			343		///< MUID¸¦ ¿äÃ»
#define MC_NET_RESPONSE_UID			344		///< MUID¸¦ È®º¸ÇØ¼­ µ¹·ÁÁØ´Ù.
#define MC_NET_ECHO					351		///< Net Echo Å×½ºÆ®

#define MC_CLOCK_SYNCHRONIZE		361		///< Å¬·° ½ÌÅ©
#define	MC_SERVER_PING				362     ///Server Ping


// ¸ÅÄ¡¼­¹E°EÃ ¸úÓÉ¾E
#define MC_MATCH_NOTIFY						401		///< ¾Ë¸² ¸Þ½ÃÁE
#define MC_MATCH_ANNOUNCE					402		///< °øÁE
#define MC_MATCH_RESPONSE_RESULT			403		///< request¿¡ ´EÑ °á°ú¸¦ ¾Ë¸²(S -> C)
// °E®ÀÚ ÀEE¸úÓÉ¾E
#define MC_ADMIN_ANNOUNCE					501		///< ÀE¼ Á¢¼ÓÀÚ¿¡°Ô ¹æ¼Û
#define MC_SERVER_ANNOUNCE					503
#define MC_ADMIN_REQUEST_SERVER_INFO		505		///< ¼­¹EÁ¤º¸ ¿äÃ»
#define MC_ADMIN_RESPONSE_SERVER_INFO		506		///< ¼­¹EÁ¤º¸ ÀÀ´E
#define MC_ADMIN_SERVER_HALT				511		///< ¼­¹ö¸¦ Á¾·EÇÑ´Ù
#define MC_ADMIN_TERMINAL					512		///< ÅÍ¹Ì³¯
#define MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE	513	///< °èÁ¤ ±ÇÇÑ º¯°E¿äÃ»
#define MC_ADMIN_RESPONSE_UPDATE_ACCOUNT_UGRADE	514	///< °èÁ¤ ±ÇÇÑ º¯°EÀÀ´E
#define MC_ADMIN_REQUEST_KICK_PLAYER			515		///< Å¸°Ù Á¢¼Ó ²÷±E¿äÃ»
#define MC_ADMIN_RESPONSE_KICK_PLAYER			516		///< Å¸°Ù Á¢¼Ó ²÷±EÀÀ´E
#define MC_ADMIN_REQUEST_MUTE_PLAYER			517		///< Å¸°Ù Ã¤ÆÃ ±ÝÁE¿äÃ»
#define MC_ADMIN_RESPONSE_MUTE_PLAYER			518		///< Å¸°Ù Ã¤ÆÃ ±ÝÁEÀÀ´E
#define MC_ADMIN_REQUEST_BLOCK_PLAYER			519		///< Å¸°Ù Á¢¼Ó ºúÓ° ¿äÃ»
#define MC_ADMIN_RESPONSE_BLOCK_PLAYER			520		///< Å¸°Ù Á¢¼Ó ºúÓ° ÀÀ´E
#define MC_ADMIN_PING_TO_ALL				521		///< Garbage Connection Ã»¼Ò È®ÀÎÀ» À§ÇÑ ¼öµ¿¸úÓÉ
#define MC_ADMIN_REQUEST_SWITCH_LADDER_GAME	522		///< Å¬·£ÀE½ÅÃ» °¡´É¿©ºÎ ¼³Á¤
#define MC_ADMIN_HIDE						531		///< ¿ûÛµÀÚ ¾Èº¸ÀÌ±E
#define MC_ADMIN_RELOAD_CLIENT_HASH			532		///< reload XTrap Hashmap
#define MC_ADMIN_RESET_ALL_HACKING_BLOCK	533		///< ¸ðµEÇØÅ· ºúÓ°À» ÃEÒÇÑ´Ù.
#define MC_ADMIN_RELOAD_GAMBLEITEM			534		///< °×ºE¾ÆÀÌÅÛ Á¤º¸ ´Ù½Ã ÀÐ±E
#define MC_ADMIN_DUMP_GAMBLEITEM_LOG		535		///< ÇöÁ¦ °×ºE¾ÆÀÌÅÛ Á¤º¸¸¦ LogÆÄÀÏ¿¡ Ãâ·ÂÇÑ´Ù.
#define MC_ADMIN_ASSASIN					536		///< GMÀÌ Ä¿¸Ç´õ°¡ µÈ´Ù.
#define MC_ADMIN_KICKALL					537		


// ÀÌº¥Æ® ¸¶½ºÅÍ ¸úÓÉ¾E
#define MC_EVENT_CHANGE_MASTER				601		///< ¹æÀå±ÇÇÑÀ» »¯¾ûÛÂ´Ù
#define MC_EVENT_CHANGE_PASSWORD			602		///< ¹æÀÇ ºñ¹Ð¹øÈ£¸¦ ¹Ù²Û´Ù
#define MC_EVENT_REQUEST_JJANG				611		///< Â¯¸¶Å© ºÎ¿©
#define MC_EVENT_REMOVE_JJANG				612		///< Â¯¸¶Å© È¸¼E
#define MC_EVENT_UPDATE_JJANG				613		///< Â¯¸¶Å© ¾Ë¸²


// ¸ÅÄ¡¼­¹EÅ¬¶óÀÌ¾ðÆ®
#define MC_MATCH_LOGIN							1001	///< ·Î±×ÀÎ
#define MC_MATCH_RESPONSE_LOGIN					1002	///< ·Î±×ÀÎ ÀÀ´E
#define MC_MATCH_LOGIN_NETMARBLE				1003	///< ³Ý¸¶ºúÛ¡¼­ ·Î±×ÀÎ
#define MC_MATCH_LOGIN_NETMARBLE_JP				1004	///< ÀÏº»³Ý¸¶ºúÛ¡¼­ ·Î±×ÀÎ
#define MC_MATCH_LOGIN_FROM_DBAGENT				1005	///< DBAgent·ÎºÎÅÍÀÇ ·Î±×ÀÎ ÀÀ´EÀÏº»³Ý¸¶ºEÀEE
#define MC_MATCH_LOGIN_FROM_DBAGENT_FAILED		1008	///< DBAgent·ÎºÎÅÍÀÇ ·Î±×ÀÎ ½ÇÆÐ(ÀÏº»³Ý¸¶ºEÀEE
#define MC_MATCH_FIND_HACKING					1009	///< ÇØÅ· °ËÃE
#define MC_MATCH_DISCONNMSG						1010	///< Discconst¸Þ½ÃÁE
#define MC_MATCH_LOGIN_NHNUSA					1011	///< ¹Ì±¹ NHN¿¡¼­ ·Î±×ÀÎ.
#define MC_MATCH_LOGIN_GAMEON_JP				1012	///< ÀÏº» gameon¿¡¼­ ·Î±×ÀÎ

#define MC_MATCH_BRIDGEPEER						1006	///< MatchServer¿¡ PeerÁ¤º¸ ¾Ë¸²
#define MC_MATCH_BRIDGEPEER_ACK					1007	///< PeerÁ¤º¸ ¼ö½ÅÈ®ÀÎ

#define MC_MATCH_OBJECT_CACHE					1101	///< ¿ÀºE§Æ® Ä³½Ã

#define MC_MATCH_REQUEST_RECOMMANDED_CHANNEL	1201	///< ÃÖÃÊ ÂE¡ÇÒ Ã¤³Î¿¡´EØ ÃßÃµÀ» ¿äÃ»ÇÑ´Ù.
#define MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL	1202	///< ÃÖÃÊ ÂE¡ÇÒ Ã¤³Î¿¡´EØ ÃßÃµÇÑ´Ù.
#define MC_MATCH_CHANNEL_REQUEST_JOIN			1205	///< Ã¤³Î¿¡ ÂE¡(C -> S)
#define MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME	1206	///< Ã¤³ÎÀÌ¸§À¸·Î Ã¤³Î Á¶ÀÎ ¿äÃ»
#define MC_MATCH_CHANNEL_RESPONSE_JOIN			1207	///< Ã¤³Î Á¶ÀÎ ÀÀ´ES -> C)
#define MC_MATCH_CHANNEL_LEAVE					1208	///< Ã¤³Î Å»ÅE
#define MC_MATCH_CHANNEL_LIST_START				1211	///< Ã¤³Î ¸ñ·Ï ÀEÛ½ÃÀÛ ¿äÃ»
#define MC_MATCH_CHANNEL_LIST_STOP				1212	///< Ã¤³Î ¸ñ·Ï ÀEÛÁßÁE¿äÃ»
#define MC_MATCH_CHANNEL_LIST					1213	///< Ã¤³Î ¸ñ·Ï
#define MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST	1221	///< ÇÃ·¹ÀÌ¾E¸®½ºÆ® ¿äÃ»
#define MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST	1222	///< ÇÃ·¹ÀÌ¾E¸®½ºÆ® ÀÀ´E
#define MC_MATCH_CHANNEL_REQUEST_CHAT			1225	///< Ã¤³Î¿¡ ´E­ ¿äÃ»
#define MC_MATCH_CHANNEL_CHAT					1226	///< Ã¤³Î¿¡¼­ ´E­ ÀÀ´E
#define MC_MATCH_CHANNEL_DUMB_CHAT				1227	///< Ã¤³Î¿¡¼­ ´E­ Â÷´Ü, º¡¾ûÔ® ±â´É - È«±âÁÖ(2009.08.05)

#define MC_MATCH_CHANNEL_REQUEST_RULE			1230	///< Ã¤³Î±ÔÄ¢ ¿äÃ»
#define MC_MATCH_CHANNEL_RESPONSE_RULE			1231	///< Ã¤³Î±ÔÄ¢ ÀÀ´E
#define MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST		1232	///< Ã¤³ÎÀÇ ·Îºñ¿¡ ÀÖ´Â ¸ðµEÇÃ·¹ÀÌ¾E¸®½ºÆ® ¿äÃ»
#define MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST		1233	///< Ã¤³ÎÀÇ ·Îºñ¿¡ ÀÖ´Â ÇÃ·¹ÀÌ¾E¸®½ºÆ® ¿äÃ»


#define MC_MATCH_STAGE_CREATE					1301	///< ½ºÅ×ÀÌÁE»ý¼º
#define MC_MATCH_RESPONSE_STAGE_CREATE			1302	///< ½ºÅ×ÀÌÁE»ý¼º ÀÀ´E
#define MC_MATCH_STAGE_JOIN						1303	///< ½ºÅ×ÀÌÁEÂE¡
#define MC_MATCH_REQUEST_STAGE_JOIN				1304	///< ½ºÅ×ÀÌÁEÂE¡ ¿äÃ»
#define MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN		1305	///< ºñ¹Ð ½ºÅ×ÀÌÁEÂE¡ ¿äÃ»
#define MC_MATCH_RESPONSE_STAGE_JOIN			1306	///< ½ºÅ×ÀÌÁö¿¡ Á¶ÀÎ¿©ºÎ¸¦ ¾Ë¸²
#define MC_MATCH_STAGE_LEAVE					1307	///< ½ºÅ×ÀÌÁEÅ»ÅE
#define MC_MATCH_STAGE_REQUEST_PLAYERLIST		1308	///< ½ºÅ×ÀÌÁöÀÇ À¯Àú¸ñ·Ï ¿äÃ»
#define MC_MATCH_STAGE_FOLLOW					1309	///< ÁöÁ¤ À¯Àú¸¦ µû¶ó¼­ ½ºÅ×ÀÌÁEÂE¡
#define MC_MATCH_RESPONSE_STAGE_FOLLOW			1310	///< ÁöÁ¤ À¯Àú¸¦ µû¶ó¼­ ½ºÅ×ÀÌÁEÂE¡ ÀÀ´E
#define MC_MATCH_REQUEST_STAGE_LIST				1311	///< Å¬¶óÀÌ¾ðÆ®°¡ ½ºÅ×ÀÌÁE¸®½ºÆ® ¿äÃ»
#define MC_MATCH_STAGE_LIST_START				1312	///< ½ºÅ×ÀÌÁE¸ñ·Ï ÀEÛ½ÃÀÛ ¿äÃ»
#define MC_MATCH_STAGE_LIST_STOP				1313	///< ½ºÅ×ÀÌÁE¸ñ·Ï ÀEÛÁßÁE¿äÃ»
#define MC_MATCH_STAGE_LIST						1314	///< ½ºÅ×ÀÌÁE¸ñ·Ï
#define MC_MATCH_STAGE_CHAT						1321	///< ½ºÅ×ÀÌÁE´E­
#define MC_MATCH_STAGE_REQUEST_QUICKJOIN		1322	///< ÄE¶ÀÎ ¿äÃ»
#define MC_MATCH_STAGE_RESPONSE_QUICKJOIN		1323	///< ÄE¶ÀÎ ÀÀ´E
#define MC_MATCH_STAGE_GO						1331	///< ½ºÅ×ÀÌÁE¹øÈ£·Î ÂE¡
#define MC_MATCH_STAGE_REQUIRE_PASSWORD			1332	///< ºñ¹Ð¹æÀÌ¶EÆÐ½º¿öµå°¡ ÇÊ¿äÇÏ´Ù.(S -> C)


#define MC_MATCH_STAGE_REQUEST_ENTERBATTLE		1401	///< ½ºÅ×ÀÌÁEÀEEÂE¡ÇÑ´Ù°E¿äÃ»
#define MC_MATCH_STAGE_ENTERBATTLE				1402	///< ½ºÅ×ÀÌÁEÀEõÂE¡
#define MC_MATCH_STAGE_LEAVEBATTLE_TO_SERVER	1403	///< ½ºÅ×ÀÌÁEÀEõÅ»ÅEC -> S)
#define MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT	1404	///< ½ºÅ×ÀÌÁEÀEõÅ»ÅES -> C)

#define MC_MATCH_REQUEST_STAGESETTING			1411	///< ½ºÅ×ÀÌÁöÀÇ ¼³Á¤À» ¿äÃ»
#define MC_MATCH_RESPONSE_STAGESETTING			1412	///< ½ºÅ×ÀÌÁöÀÇ ¼³Á¤À» ¾Ë¸²
#define MC_MATCH_STAGESETTING					1413	///< ½ºÅ×ÀÌÁE¼³Á¤
#define MC_MATCH_STAGE_MAP						1414	///< ½ºÅ×ÀÌÁE¸Ê ¼³Á¤
#define MC_MATCH_STAGE_REQUEST_FORCED_ENTRY		1415	///< ½ºÅ×ÀÌÁö¿¡¼­ °ÔÀÓ¿¡ ³­ÀÔ ¿äÃ»
#define MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY	1416	///< ½ºÅ×ÀÌÁE°ÔÀÓ ³­ÀÔ ÀÀ´E
#define MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE	1417	///< ½ºÅ×ÀÌÁE¸±·¹ÀÌ¸Ê ¿ä¼Ò(Å¸ÀÔ, È¸ÂE ¾÷µ¥ÀÌÆ®
#define MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE	1418	///< ½ºÅ×ÀÌÁE¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¾÷µ¥ÀÌÆ®

#define MC_MATCH_STAGE_MASTER					1421	///< ½ºÅ×ÀÌÁE¹æÀE¾Ë¸²
#define MC_MATCH_STAGE_PLAYER_STATE				1422	///< ½ºÅ×ÀÌÁE·¹µEµûÜÇ ÇÃ·¹ÀÌ¾E»óÅÂ Á¤º¸
#define MC_MATCH_STAGE_TEAM						1423	///< ½ºÅ×ÀÌÁEÆÀ
#define MC_MATCH_STAGE_START					1431	///< ½ºÅ×ÀÌÁE½ÃÀÛ(Ä«¿ûâ®´Ù¿E
#define MC_MATCH_STAGE_LAUNCH					1432	///< ½ºÅ×ÀÌÁE½ÇÇE
#define MC_MATCH_STAGE_RELAY_LAUNCH				1433	///< ¸±·¹ÀÌµÈ ½ºÅ×ÀÌÁE½ÇÇE

#define MC_MATCH_LOADING_COMPLETE				1441	///< ·ÎµùÀÌ ³¡³µÀ½
#define MC_MATCH_STAGE_FINISH_GAME				1442	///< ½ºÅ×ÀÌÁE°ÔÀÓ Á¾·E

#define MC_MATCH_REQUEST_GAME_INFO				1451	///< °ÔÀÓ¾È Á¤º¸¸¦ ´Þ¶E¿äÃ»
#define MC_MATCH_RESPONSE_GAME_INFO				1452	///< °ÔÀÓ¾È Á¤º¸¸¦ ¾Ë·ÁÁØ´Ù
#define MC_MATCH_RESPONSE_GAME_INFO_V2			1453	// Custom: Larger Game Info Packet

#define MC_MATCH_REQUEST_PEERLIST				1461	///< ÂE©ÁßÀÎ À¯ÀúµéÀÇ PeerÁ¤º¸¸¦ ¿äÃ»
#define MC_MATCH_RESPONSE_PEERLIST				1462	///< ÂE©ÁßÀÎ À¯ÀúµéÀÇ PeerÁ¤º¸¸¦ ¾Ë¸²

#define MC_MATCH_REQUEST_PEER_RELAY				1471	///< ¸ÞÄ¡¼­¹ö¿¡ ÇØ´çPeer¿¡ ´EÑ Áß°è¿äÃ»
#define MC_MATCH_RESPONSE_PEER_RELAY			1472	///< ÇØ´çPeer¿¡ ´EÑ Áß°èÇã°¡ ÅE¸

// °ÔÀÓ °EÃ
#define MC_MATCH_GAME_ROUNDSTATE				1501	///< ¶ó¿ûÑåÀÇ »óÅÂ¸¦ µ¿±âÈ­
#define MC_MATCH_ROUND_FINISHINFO				1502	///< ¶ó¿ûÑEÁ¾·á½Ã Á¤º¸ ¾÷µ¥ÀÌÆ® (C <- S)
#define MC_MATCH_GAME_KILL						1511	///< Ä³¸¯ÅÍ »ç¸Á (C -> S)
#define MC_MATCH_GAME_DEAD						1512	///< Ä³¸¯ÅÍ Á×¾ú´Ù (S -> C)
#define MC_MATCH_GAME_LEVEL_UP					1513	///< °ÔÀÓÁß ·¹º§¾E
#define MC_MATCH_GAME_LEVEL_DOWN				1514	///< °ÔÀÓÁß ·¹º§´Ù¿E
#define MC_MATCH_GAME_REQUEST_SPAWN				1515	///< Ä³¸¯ÅÍ ºÎÈ°¿äÃ» (C -> S)
#define MC_MATCH_GAME_RESPONSE_SPAWN			1516	///< Ä³¸¯ÅÍ ºÎÈ°¿äÃ» (C -> S)
#define MC_MATCH_GAME_TEAMBONUS					1517	///< ÆÀ º¸³Ê½º(S -> C)
#define MC_MATCH_GAME_CHAT						1518	// Custom: in-game chat server sided.

#define MC_MATCH_GAME_REQUEST_TIMESYNC			1521	///< Match¼­¹ö¿¡ ½Ã°£µ¿±âÈ­ ¿äÃ»
#define MC_MATCH_GAME_RESPONSE_TIMESYNC			1522	///< ½Ã°£µ¿±âÈ­
#define MC_MATCH_GAME_REPORT_TIMESYNC			1523	///< ½Ã°£µ¿±E°Ë»E

#define MC_MATCH_REQUEST_SUICIDE				1531	///< ÀÚ»E¿äÃ»
#define MC_MATCH_RESPONSE_SUICIDE				1532	///< ÀÚ»EÀÀ´E
#define MC_MATCH_RESPONSE_SUICIDE_RESERVE		1533	///< ÀÚ»E¿¹¾E
#define MC_MATCH_NOTIFY_THROW_TRAPITEM			1535	///< Æ®·¦¾ÆÀÌÅÛ ÅõÃ´ ¾Ë¸²(C -> S)
#define MC_MATCH_NOTIFY_ACTIVATED_TRAPITEM		1536	///< Æ®·¦¾ÆÀÌÅÛ ¹ßµ¿ ¾Ë¸²(C -> S)
#define MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST	1437	///< ³­ÀÔÇÑ À¯Àú¿¡°Ô ÇöÀE¹ßµ¿ ÁßÀÎ Æ®·¦¾ÆÀÌÅÛ ¸ñ·Ï ¾Ë·ÁÁÖ±ES -> C)
#define MC_MATCH_REQUEST_OBTAIN_WORLDITEM		1541	///< ¾ÆÀÌÅÛ ¸Ôµµ·Ï ¿äÃ»
#define MC_MATCH_OBTAIN_WORLDITEM				1542	///< ¾ÆÀÌÅÛ ¸ÔÀ½ ºEÎµåÄ³½ºÆ®(S -> C)
#define MC_MATCH_SPAWN_WORLDITEM				1543	///< ¾ÆÀÌÅÛÀÌ ½ºÆùµÊ
#define MC_MATCH_REQUEST_SPAWN_WORLDITEM		1544	///< ¾ÆÀÌÅÛÀ» ¶³±¸µµ·Ï ¿äÃ»(C -> S)
#define MC_MATCH_REMOVE_WORLDITEM				1545	///< ¾ÆÀÌÅÛ ¾ø¾ûÝES -> C)
#define MC_MATCH_ASSIGN_COMMANDER				1551	///< ¾Ï»EE´EåÀ¸·Î ÀÓ¸E
#define MC_MATCH_RESET_TEAM_MEMBERS				1552	///< ÆÀ¸â¹E´Ù½Ã ¼¼ÆÃ
#define MC_MATCH_SET_OBSERVER					1553	///< °­Á¦·Î ¿ÉÀú¹ö¸ðµå·Î ÀE¯


// µ¿ÀÇ°EÃ
#define MC_MATCH_REQUEST_PROPOSAL				1561	///< ´Ù¸¥ »ç¶÷ÀÇ µ¿ÀÇ ¿äÃ»
#define MC_MATCH_RESPONSE_PROPOSAL				1562	///< µ¿ÀÇ ¿äÃ» ÀÀ´E
#define MC_MATCH_ASK_AGREEMENT					1563	///< ´Ù¸¥ »ç¶÷¿¡°Ô µ¿ÀÇ ÁúÀÇ(S -> C)
#define MC_MATCH_REPLY_AGREEMENT				1564	///< µ¿ÀÇ ÀÀ´EC -> S)

// ·¡´E°EÃ (Å¬·£ÀE
#define MC_MATCH_LADDER_REQUEST_CHALLENGE		1571	///< ·¡´EµµÀE¿äÃ»(C -> S)
#define MC_MATCH_LADDER_RESPONSE_CHALLENGE		1572	///< ·¡´EµµÀEÀÀ´E
#define MC_MATCH_LADDER_SEARCH_RIVAL			1574	///< ·¡´E°Ë»öÁß		
#define MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE	1575	///< ·¡´EµµÀEÃEÒ¿äÃ»
#define MC_MATCH_LADDER_CANCEL_CHALLENGE		1576	///< ·¡´EµµÀEÃEÒ
#define MC_MATCH_LADDER_PREPARE					1578	///< ·¡´E½ÃÀÛÁØºE
#define MC_MATCH_LADDER_LAUNCH					1579	///< ·¡´E·±Ä¡ (S -> C)

// ÀÇ»ç¼ÒÅE°EÃ
#define MC_MATCH_USER_WHISPER					1601	///< ±Ó¼Ó¸»
#define MC_MATCH_USER_WHERE						1602	///< À§Ä¡È®ÀÎ
#define MC_MATCH_USER_OPTION					1605	///< °¢Á¾¿É¼Ç(±Ó¸»°ÅºÎ,ÃÊ´EÅºÎ,Ä£±¸°ÅºÎ µûÑE
#define MC_MATCH_CHATROOM_CREATE				1651	///< Ã¤ÆÃ·E°³¼³
#define MC_MATCH_CHATROOM_JOIN					1652	///< Ã¤ÆÃ·EÂE¡
#define MC_MATCH_CHATROOM_LEAVE					1653	///< Ã¤ÆÃ·EÅ»ÅE
#define MC_MATCH_CHATROOM_INVITE				1661	///< Ã¤ÆÃ·EÃÊ´E
#define MC_MATCH_CHATROOM_CHAT					1662	///< Ã¤ÆÃ
#define MC_MATCH_CHATROOM_SELECT_WRITE			1665	///< Ã¤ÆÃ·E¼±ÅÃ

// Ä³¸¯ÅÍ °EÃ
#define MC_MATCH_REQUEST_ACCOUNT_CHARLIST		1701	///< °èÁ¤ÀÇ ÀE¼ Ä³¸¯ÅÍ ¸®½ºÆ®¸¦ ¿äÃ»
#define MC_MATCH_RESPONSE_ACCOUNT_CHARLIST		1702	///< °èÁ¤ÀÇ ÀE¼ Ä³¸¯ÅÍ ¸®½ºÆ®¸¦ ÀÀ´E
#define MC_MATCH_REQUEST_SELECT_CHAR			1703	///< Ä³¸¯ÅÍ ¼±ÅÃ ¿äÃ»
#define MC_MATCH_RESPONSE_SELECT_CHAR			1704	///< Ä³¸¯ÅÍ ¼±ÅÃ ÀÀ´E
#define MC_MATCH_REQUEST_MYCHARINFO				1705	///< ³» Ä³¸¯ÅÍ Á¤º¸ ¿äÃ»
#define MC_MATCH_RESPONSE_MYCHARINFO			1706	///< ³» Ä³¸¯ÅÍ Á¤º¸ ÀÀ´E
#define MC_MATCH_REQUEST_CREATE_CHAR			1711	///< ³» Ä³¸¯ÅÍ »ý¼º ¿äÃ»
#define MC_MATCH_RESPONSE_CREATE_CHAR			1712	///< ³» Ä³¸¯ÅÍ »ý¼º ÀÀ´E
#define MC_MATCH_REQUEST_DELETE_CHAR			1713	///< ³» Ä³¸¯ÅÍ »èÁ¦ ¿äÃ»
#define MC_MATCH_RESPONSE_DELETE_CHAR			1714	///< ³» Ä³¸¯ÅÍ »èÁ¦ ÀÀ´E
#define MC_MATCH_REQUEST_COPY_TO_TESTSERVER		1715	///< Ä³¸¯ÅÍ Å×½ºÆ®¼­¹ö·Î ÀEÛ ¿äÃ» - ¹Ì±¸ÇE
#define MC_MATCH_RESPONSE_COPY_TO_TESTSERVER	1716	///< Ä³¸¯ÅÍ Å×½ºÆ®¼­¹ö·Î ÀEÛ ÀÀ´E- ¹Ì±¸ÇE
#define MC_MATCH_REQUEST_CHARINFO_DETAIL		1717	///< ´Ù¸¥ ÇÃ·¹ÀÌ¾EÁ¤º¸º¸±EµûÜÇ ÀÚ¼¼ÇÑ ÇÃ·¹ÀÌ¾EÁ¤º¸ ¿äÃ»
#define MC_MATCH_RESPONSE_CHARINFO_DETAIL		1718	///< ´Ù¸¥ ÇÃ·¹ÀÌ¾EÁ¤º¸º¸±EµûÜÇ ÀÚ¼¼ÇÑ ÇÃ·¹ÀÌ¾EÁ¤º¸ ÀÀ´E
#define MC_MATCH_REQUEST_ACCOUNT_CHARINFO		1719	///< °èÁ¤ÀÇ ÇÑ Ä³¸¯ÅÍ Á¤º¸ ¿äÃ»
#define MC_MATCH_RESPONSE_ACCOUNT_CHARINFO		1720	///< °èÁ¤ÀÇ ÇÑ Ä³¸¯ÅÍ Á¤º¸ ÀÀ´E
#define MC_MATCH_REQUEST_CREATE_ACCOUNT			1721	/// Custom: Creating Account from in game
#define MC_MATCH_RESPONSE_CREATE_ACCOUNT		1722	/// Custom: Respond to account creation request
// ¾ÆÀÌÅÛ °EÃ
#define MC_MATCH_REQUEST_SIMPLE_CHARINFO		1801	///< Ä³¸¯ÅÍÀÇ °£´ÜÇÑ Á¤º¸ ¿äÃ»	
#define MC_MATCH_RESPONSE_SIMPLE_CHARINFO		1802	///< Ä³¸¯ÅÍÀÇ °£´ÜÇÑ Á¤º¸ ÀÀ´E
#define MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO		1803	///< ³» Ä³¸¯ÅÍ XP, BPµEÁ¤º¸ ¿äÃ»
#define MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO	1804	///< ³» Ä³¸¯ÅÍ XP, BPµEÁ¤º¸ ÀÀ´E

#define MC_MATCH_REQUEST_BUY_ITEM				1811	///< ¾ÆÀÌÅÛ ±¸¸Å ¿äÃ»
#define MC_MATCH_RESPONSE_BUY_ITEM				1812	///< ¾ÆÀÌÅÛ ±¸¸Å ÀÀ´E
#define MC_MATCH_REQUEST_SELL_ITEM				1813	///< ¾ÆÀÌÅÛ ÆÇ¸Å ¿äÃ»
#define MC_MATCH_RESPONSE_SELL_ITEM				1814	///< ¾ÆÀÌÅÛ ÆÇ¸Å ÀÀ´E
#define MC_MATCH_REQUEST_SHOP_ITEMLIST			1815	///< ¼¥¿¡¼­ ÆÇ¸ÅÇÏ´Â ¾ÆÀÌÅÛ ¸®½ºÆ® ¿äÃ»
#define MC_MATCH_RESPONSE_SHOP_ITEMLIST			1816	///< ¼¥¿¡¼­ ÆÇ¸ÅÇÏ´Â ¾ÆÀÌÅÛ ¸®½ºÆ® ÀÀ´E

#define MC_MATCH_REQUEST_CHARACTER_ITEMLIST		1821	///< ³» ¾ÆÀÌÅÛ ¸®½ºÆ® ÁÖ½Ã¿À
#define MC_MATCH_RESPONSE_CHARACTER_ITEMLIST	1822	///< ¾ÆÀÌÅÛ ¸®½ºÆ® ¿©ƒ…¿À
#define MC_MATCH_REQUEST_EQUIP_ITEM				1823	///< ¾ÆÀÌÅÛ ÀåºE¿äÃ»
#define MC_MATCH_RESPONSE_EQUIP_ITEM			1824	///< ¾ÆÀÌÅÛ ÀåºEÀÀ´E
#define MC_MATCH_REQUEST_TAKEOFF_ITEM			1825	///< ¾ÆÀÌÅÛ ÇØÁ¦ ¿äÃ»
#define MC_MATCH_RESPONSE_TAKEOFF_ITEM			1826	///< ¾ÆÀÌÅÛ ÇØÁ¦ ÀÀ´E
#define MC_RESPONSE_GAMBLEITEMLIST				1827	///< DBÀÇ °×ºE¾ÆÀÌÅÛ Á¤º¸ ÀEÛ.
#define MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK	1828	///< Ä³¸¯ÅÍÀÇ Look¸¸µE¾÷µ¥ÀÌÆ®ÇÒ¶§ »ç¿EÑ´Ù.
#define MC_MATCH_REQUEST_CHARACTER_ITEMLIST_FORCE	1829	///< ³» ¾ÆÀÌÅÛ ¸®½ºÆ® ÁÖ½Ã¿À : Áßº¹ ¿äÃ»µµ ¹«½ÃÇÏÁE¾Ê´Â ¹öÀEÁÖÀÇÇØ¼­ »ç¿EÒ°Í)

#define MC_MATCH_REQUEST_ACCOUNT_ITEMLIST			1831	///< ³» Ã¢°E¾ÆÀÌÅÛ ¸®½ºÆ® ÁÖ½Ã¿À
#define MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST			1832	///< Ã¢°E¾ÆÀÌÅÛ ¸®½ºÆ® ¿©ƒ…¿À
#define MC_MATCH_REQUEST_BRING_ACCOUNTITEM			1833	///< Ã¢°E¾ÆÀÌÅÛÀ» ³» Ä³¸¯ÅÍ·Î °¡Á®¿À±E
#define MC_MATCH_RESPONSE_BRING_ACCOUNTITEM			1834	///< Ã¢°E¾ÆÀÌÅÛ °¡Á®¿À±EÀÀ´E
#define MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM		1835	///< ³» Ä³½¬¾ÆÀÌÅÛ Ã¢°úÓÎ ¿Å±â±E¿äÃ»
#define MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM	1836	///< ³» Ä³½¬¾ÆÀÌÅÛ Ã¢°úÓÎ ¿Å±â±EÀÀ´E
#define MC_MATCH_EXPIRED_RENT_ITEM					1837	///< ¾ÆÀÌÅÛÀÌ ±â°£ ¸¸·áµÇ¾ú½À´Ï´Ù.(S -> C)

#define MC_MATCH_REQUEST_GAMBLE					1841	///< »Ì±E(C -> S)
#define MC_MATCH_RESPONSE_GAMBLE				1842	///< »Ì±E°á°E(S -> C)

#define MC_MATCH_REWARD_BATTLE_TIME				1843	///< ¹èÆ² ½Ã°£ º¸»E°øÁES->C)

// ¾ÆÀÌÅÛ °EÃ(¼Ò¸ð¼º)
#define MC_MATCH_REQUEST_USE_SPENDABLE_NORMAL_ITEM		1850	///< ÀÏ¹Ý ¼Ò¸ð¼º ¾ÆÀÌÅÛ »ç¿E¿äÃ»(C -> S)
#define MC_MATCH_REQUEST_USE_SPENDABLE_BUFF_ITEM		1860	///< ¹öÇÁ ¼Ò¸ð¼º ¾ÆÀÌÅÛ »ç¿E¿äÃ»(C -> S)
#define MC_MATCH_RESPONSE_USE_SPENDABLE_BUFF_ITEM		1861	///< ¹öÇÁ ¼Ò¸ð¼º ¾ÆÀÌÅÛ »ç¿E°á°ES -> C)
#define MC_MATCH_SPENDABLE_BUFF_ITEM_STATUS				1865	///< ¹öÇÁ ¼Ò¸ð¼º ¾ÆÀÌÅÛ »óÅÂ(S -> C)
#define MC_MATCH_REQUEST_USE_SPENDABLE_COMMUNITY_ITEM	1870	///< Ä¿¹Â´ÏÆ¼ ¼Ò¸ð¼º ¾ÆÀÌÅÛ »ç¿E¿äÃ»(C -> S)

// Ä£±¸ °EÃ
#define MC_MATCH_FRIEND_ADD						1901	///< Ä£±¸ Ãß°¡
#define MC_MATCH_FRIEND_REMOVE					1902	///< Ä£±¸ »èÁ¦
#define MC_MATCH_FRIEND_LIST					1903	///< Ä£±¸ ¸ñ·Ï ¿äÃ»
#define MC_MATCH_RESPONSE_FRIENDLIST			1904	///< Ä£±¸ ¸ñ·Ï ÀEÛ
#define MC_MATCH_FRIEND_MSG						1905	///< Ä£±¸ Ã¤ÆÃ


// Å¬·£°EÃ
#define MC_MATCH_CLAN_REQUEST_CREATE_CLAN			2000	///< Å¬·£ »ý¼º ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CREATE_CLAN			2001	///< Å¬·£ »ý¼º ÀÀ´ES -> C)
#define MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT			2002	///< ¹ß±âÀÎ¿¡°Ô Å¬·£ »ý¼º µ¿ÀÇ ÁúÀÇ(S -> C)
#define MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT		2003	///< ¹ß±âÀÎÀÌ µ¿ÀÇ¸¦ ÀÀ´EC -> S)
#define MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN	2004	///< ¹ß±âÀÎÀÌ µ¿ÀÇÇÑ Å¬·£ »ý¼º ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN	2005	///< ¹ß±âÀÎÀÌ µ¿ÀÇÇÑ Å¬·£ »ý¼º ÀÀ´ES -> C)
#define MC_MATCH_CLAN_REQUEST_CLOSE_CLAN			2006	///< ¸¶½ºÅÍ°¡ Å¬·£ Æó¼E¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN			2007	///< ¸¶½ºÅÍÀÇ Å¬·£ Æó¼EÀÀ´ES -> C)
#define MC_MATCH_CLAN_REQUEST_JOIN_CLAN				2008	///< ¿ûÛµÀÚ°¡ °¡ÀÔÀÚÀÇ °¡ÀÔÀ» ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_JOIN_CLAN			2009	///< ¿ûÛµÀÚÀÇ °¡ÀÔÀÚ °¡ÀÔÀÇ ÀÀ´ES -> C)
#define MC_MATCH_CLAN_ASK_JOIN_AGREEMENT			2010	///< °¡ÀÔÀÚ¿¡°Ô °¡ÀÔ µ¿ÀÇ ÁúÀÇ(S -> C)
#define MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT			2011	///< °¡ÀÔÀÚÀÇ °¡ÀÔ µ¿ÀÇ ÀÀ´EC -> S)
#define MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN		2012	///< °¡ÀÔÀÚ°¡ µ¿ÀÇÇÑ Å¬·£ °¡ÀÔ ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN		2013	///< °¡ÀÔÀÚ°¡ µ¿ÀÇÇÑ Å¬·£ °¡ÀÔ ÀÀ´ES -> C)
#define MC_MATCH_CLAN_REQUEST_LEAVE_CLAN			2014	///< Å¬·£¿øÀÌ Å»Åð¸¦ ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN			2015	///< Å¬·£¿øÀÇ Å»ÅðÀÇ ÀÀ´ES -> C)
#define MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO			2016	///< Ä³¸¯ÅÍÀÇ Å¬·£Á¤º¸°¡ ¾÷µ¥ÀÌÆ®µÊ(S -> C)
#define MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE	2017	///< ¸â¹öÀÇ ±ÇÇÑ º¯°E¿äÃ»(C -> S)
#define MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE	2018	///< ¸â¹öÀÇ ±ÇÇÑ º¯°EÀÀ´ES -> C)
#define MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER	2019	///< ¸â¹ö¸¦ Å»ÅE¿äÃ»(C -> S)
#define MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER	2020	///< ¸â¹ö¸¦ Å»ÅEÀÀ´ES -> C)
#define MC_MATCH_CLAN_REQUEST_MSG					2021	///< Å¬·£ Ã¤ÆÃ ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_MSG							2022	///< Å¬·£ Ã¤ÆÃ(S -> C)
#define MC_MATCH_CLAN_REQUEST_MEMBER_LIST			2023	///< Å¬·£ ¸â¹E¸®½ºÆ® ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_MEMBER_LIST			2024	///< Å¬·£ ¸â¹E¸®½ºÆ® ÀÀ´ES -> C)
#define MC_MATCH_CLAN_REQUEST_CLAN_INFO				2025	///< Å¬·£ Á¤º¸ ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CLAN_INFO			2026	///< Å¬·£ Á¤º¸ ÀÀ´ES -> C)
#define MC_MATCH_CLAN_STANDBY_CLAN_LIST				2027	///< Å¬·£ÀE´EâÁßÀÎ Å¬·£ ¸®½ºÆ® ÀÀ´ES -> C)
#define MC_MATCH_CLAN_MEMBER_CONNECTED				2028	///< Å¬·£¿øÀÌ Á¢¼ÓÇÔ(S -> C)
#define MC_MATCH_FRIEND_CONNECTED					2029	///< Friend >
#define MC_MATCH_CLAN_REQUEST_EMBLEMURL				2051	///< Å¬·£¸¶Å© URL ¿äÃ»(C -> S)
#define MC_MATCH_CLAN_RESPONSE_EMBLEMURL			2052	///< Å¬·£¸¶Å© URL ÀÀ´ES -> C)
#define MC_MATCH_CLAN_LOCAL_EMBLEMREADY				2055	///< Å¬·£¸¶Å© ´Ù¿ûÓÎµE¿Ï·EC -> C)
#define MC_MATCH_CLAN_ACCOUNCE_DELETE				2056	///< Å¬·£ Æó¼E¿äÃ» Á¢¼EÁ¤º¸ °øÁE(S -> C)


// ÅõÇ¥°EÃ
#define MC_MATCH_CALLVOTE							2100	///< ÅõÇ¥¸¦ Á¦¾ÈÇÑ´Ù. (C -> S)
#define MC_MATCH_NOTIFY_CALLVOTE					2101	///< ÅõÇ¥Á¦¾ÈÀ» ¾Ë¸°´Ù. (S -> C)
#define MC_MATCH_NOTIFY_VOTERESULT					2102	///< ÅõÇ¥°á°ú¸¦ ¾Ë¸°´Ù. (S -> C)
#define MC_MATCH_VOTE_YES							2105	///< ÅõÇ¥ (C -> S)
#define MC_MATCH_VOTE_NO							2106	///< ÅõÇ¥ (C -> S)
#define MC_MATCH_VOTE_RESPONSE						2107	///< ÅõÇ¥ ¿¹¿Ü.(S -> C)
#define MC_MATCH_VOTE_STOP							2108	///< ÅõÇ¥ ÃEÒ. (S -> C)

// ¹æ¼Û°EÃ
#define MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES			2200	///< Å¬·£ÀÌ °è¼Ó ¿¬½ÂÁßÀÔ´Ï´Ù.(S -> C)
#define MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES		2201	///< Å¬·£ÀÇ ¿¬½ÂÀ» ÀúÁöÇÏ¿´½À´Ï´Ù.(S -> C)
#define MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES			2202	///< xx´ÔÀÌ xxÃ¤³ÎÀÇ xx¹æ¿¡¼­ x¿¬½ÂÁßÀÔ´Ï´Ù.(S -> C)
#define MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES		2203	///< xx´ÔÀÌ xx´ÔÀÇ xx¿¬½ÂÀ» ÀúÁöÇß½À´Ï´Ù. (S -> C)


// Duel Tournament °EÃ.. Added by È«±âÁÖ
#define MC_MATCH_DUELTOURNAMENT_REQUEST_JOINGAME				2300	///< µà¾EÅä³Ê¸ÕÆ®ÀÇ ÂE¡ ½ÅÃ»(C -> S)
#define MC_MATCH_DUELTOURNAMENT_RESPONSE_JOINGAME				2301	///< µà¾EÅä³Ê¸ÕÆ®ÀÇ ÂE¡ ½ÅÃ»¿¡ ´EÑ ÀÀ´ES -> C)
#define MC_MATCH_DUELTOURNAMENT_REQUEST_CANCELGAME				2302	///< µà¾EÅä³Ê¸ÕÆ®ÀÇ ÂE¡ ½ÅÃ» ÃEÒ(C -> S)
#define MC_MATCH_DUELTOURNAMENT_CHAR_INFO						2303	///< µà¾EÅä³Ê¸ÕÆ®ÀÇ ÄÉ¸¯ÅÍ Á¤º¸(S -> C)
#define MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH					2304	///< µà¾EÅä³Ê¸ÕÆ® ½ÃÀÛ ÀE
#define MC_MATCH_DUELTOURNAMENT_LAUNCH_MATCH					2305	///< µà¾EÅä³Ê¸ÕÆ® ½ÃÀÛ! (S - > C)
#define MC_MATCH_DUELTOURNAMENT_CANCEL_MATCH					2306	///< µà¾EÅä³Ê¸ÕÆ® ÃEÒ! (S - > C)
#define MC_MATCH_DUELTOURNAMENT_CHAR_INFO_PREVIOUS				2307	///< µà¾EÅä³Ê¸ÕÆ® Ä³¸¯ÅÍ Áö³­ÁÖ Á¤º¸(S -> C)
#define MC_MATCH_DUELTOURNAMENT_NOT_SERVICE_TIME				2308	///< ÇöÀEµà¾óÅä³Ê¸ÕÆ® ¼­ºñ½º ½Ã°£ÀÌ ¾Æ´Õ´Ï´Ù.(¿ÀÇÂ½Ã°£ %s%d½Ã00ºÐ ~ %s%d½Ã59ºÐ) (S - > C)

#define MC_MATCH_DUELTOURNAMENT_REQUEST_SIDERANKING_INFO		2400	///< ÄÉ¸¯ÅÍ ·©Å·ÀÇ ±ÙÃ³ ·©Å· Á¤º¸ ¿äÃ»(C -> S)
#define MC_MATCH_DUELTOURNAMENT_RESPONSE_SIDERANKING_INFO		2401	///< ÄÉ¸¯ÅÍ ·©Å·ÀÇ ±ÙÃ³ ·©Å· Á¤º¸ ÀÀ´ES -> C)
#define MC_MATCH_DUELTOURNAMENT_GROUPRANKING_INFO				2402	///< µà¾EÅä³Ê¸ÕÆ®ÀÇ ±×·E·©Å· Á¤º¸(S -> C)

#define MC_MATCH_DUELTOURNAMENT_GAME_INFO						2500	///< µà¾EÅä³Ê¸ÕÆ® °ÔÀÓ ³»¿¡¼­ÀÇ Á¤º¸
#define MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO			2501	///< µà¾EÅä³Ê¸ÕÆ® °ÔÀÓ ³»¿¡¼­ÀÇ ¶ó¿ûÑE½ÂÆÐ °á°EÁ¤º¸
#define MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO			2502	///< µà¾EÅä³Ê¸ÕÆ® °ÔÀÓ ³»¿¡¼­ÀÇ ¸ÅÄ¡ ½ÂÆÐ °á°EÁ¤º¸
#define MC_MATCH_DUELTOURNAMENT_GAME_PLAYER_STATUS				2503	///< µà¾EÅä³Ê¸ÕÆ® °ÔÀÓ ³»¿¡¼­ÀÇ ¶ó¿ûÑå°¡ Á¾·áµÉ ¶§, ÇØ´E°ÔÀÌ¸ÓÀÇ »óÅÂ¸¦ ¼­¹ö·Î º¸³¿(C -> S)
#define MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO		2504	///< µà¾EÅä³Ê¸ÕÆ® °ÔÀÓ ³»¿¡¼­ÀÇ ´ÙÀ½ ¸ÅÄ¡¾÷ÀÇ ÇÃ·¹ÀÌ¾EÁ¤º¸

// °ÔÀÓ ·E°EÃ
// ¹ö¼­Ä¿ ¸ðµE
#define MC_MATCH_ASSIGN_BERSERKER					3001	///< ¹ö¼­Ä¿ ÀÓ¸ES -> C)


#define MC_MATCH_DUEL_QUEUEINFO						3100	///< ´Eâ¿­ ¼ø¹E¾Ë¸²(S -> C)
#define MC_MATCH_DUEL_QUEUEINFO_V2					3101	// Custom: Duel Queue Info V2


// ¸ÅÄ¡ ¿¡ÀÌÁ¯Æ® °EÃ ¸úÓÉ¾E
#define MC_MATCH_REGISTERAGENT				5001	///< ¿¡ÀÌÁ¯Æ® µûÓÏ
#define MC_MATCH_UNREGISTERAGENT			5002	///< ¿¡ÀÌÁ¯Æ® ÇØÁE
#define MC_MATCH_AGENT_REQUEST_LIVECHECK	5011	///< ¿¡ÀÌÁ¯Æ® È®ÀÎ¿äÃ»
#define MC_MATCH_AGENT_RESPONSE_LIVECHECK	5012	///< ¿¡ÀÌÁ¯Æ® È®ÀÎÀÀ´E
#define MC_AGENT_ERROR						5013	///< ¿¡ÀÌÀE® °EÃ ¿¡·¯

#define MC_AGENT_CONNECT					5021	///< ¸ÅÄ¡¼­¹ö¿¡ ¿¡ÀÌÁ¯Æ® ¼­¹EµûÓÏ
#define MC_AGENT_DISCONNECT					5022	///< ¸ÅÄ¡¼­¹ö¿¡ ¿¡ÀÌÁ¯Æ® ¼­¹EµûÓÏÇØÁE
#define MC_AGENT_LOCAL_LOGIN				5023	///< Å¬¶óÀÌ¾ðÆ® Á¢¼ÓÃ³¸®
#define MC_AGENT_RESPONSE_LOGIN				5024	///< Å¬¶óÀÌ¾ðÆ®¿¡ ·Î±×ÀÎ ÅEE
//#define MC_AGENT_MATCH_VALIDATE				5031	///< ¸ÅÄ¡¼­¹ö¿ÍÀÇ Á¢¼ÓÈ®ÀÎ
#define MC_AGENT_STAGE_RESERVE				5051	///< ¿¡ÀÌÀE®¿¡ ½ºÅ×ÀÌÁE¹èÁ¤
#define MC_AGENT_STAGE_RELEASE				5052	///< ¿¡ÀÌÀE®¿¡ ½ºÅ×ÀÌÁEÇØÁE
#define MC_AGENT_STAGE_READY				5053	///< ¿¡ÀÌÀE®¿¡ ½ºÅ×ÀÌÁEÁØºñ¿Ï·E
#define MC_AGENT_LOCATETO_CLIENT			5061	///< Å¬¶óÀÌ¾ðÆ®¿¡°Ô Agent ¾Ë¸²
#define MC_AGENT_RELAY_PEER					5062	///< ¿¡ÀÌÀE®¿¡ Áß°èÁö½Ã
#define MC_AGENT_PEER_READY					5063	///< ¸ÞÄ¡¼­¹ö¿¡ ÇÇ¾ûÝØºE¾Ë¸²
#define MC_AGENT_PEER_BINDTCP				5071	///< Å¬¶óÀÌ¾ðÆ® Bind TCP
#define MC_AGENT_PEER_BINDUDP				5072	///< Å¬¶óÀÌ¾ðÆ® Bind UDP
#define MC_AGENT_PEER_UNBIND				5073	///< Å¬¶óÀÌ¾ðÆ® Unbind
#define MC_GUNZ_SEND_COUNTRYCODE			5074    ///< Banderas
#define MC_PEER_KILLSTREAK					5075    ///< KillerStreak
#define MC_AGENT_TUNNELING_TCP				5081	///< TCP ÅÍ³Î¸µ
#define MC_AGENT_TUNNELING_UDP				5082	///< UDP ÅÍ³Î¸µ
#define MC_AGENT_ALLOW_TUNNELING_TCP		5083	///< TCP ÅÍ³Î¸µ Çã¿E
#define MC_AGENT_ALLOW_TUNNELING_UDP		5084	///< UDP ÅÍ³Î¸µ Çã¿E
#define MC_AGENT_DEBUGPING					5101	///< µð¹ö±EE
#define MC_AGENT_DEBUGTEST					5102	///< µð¹ö±EE

#define MC_TEST_BIRDTEST1					60001	///< ¹öµEÅ×½ºÆ®1
#define MC_TEST_PEERTEST_PING				60002	///< Target Peer Test
#define MC_TEST_PEERTEST_PONG				60003	///< Target Peer Test



/// Ä¿¸ÇµE¼¼ºÎ ¿É¼Ç

// MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LISTÀÇ ¼¼ºÎ¿É¼Ç
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NORMAL		0		
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN		1
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN		2


// Äù½ºÆ® °EÃ ¸úÓÉ¾E¼­¹ÙÀÌ¹E¸ðµE Äù½ºÆ® ¸ðµE°øÅE
#define MC_QUEST_NPC_SPAWN					6000	///< NPC ½ºÆES -> C)
#define MC_QUEST_ENTRUST_NPC_CONTROL		6001	///< NPCÀÇ Á¶Á¾À» ÇÇ¾ûÛ¡°Ô À§ÀÓ(S -> C)
#define MC_QUEST_CHECKSUM_NPCINFO			6002	///< NPCÁ¤º¸µéÀÇ Ã¼Å©¼¶À» ¼­¹ö¿¡°Ô º¸³½´Ù(C -> S)
#define MC_QUEST_REQUEST_NPC_DEAD			6003	///< NPC Á×À½ ¿äÃ»(C -> S)
#define MC_QUEST_NPC_DEAD					6004	///< NPC Á×À½(S -> C)
#define MC_QUEST_REFRESH_PLAYER_STATUS		6005	///< Á×¾úÀ¸¸E´Ù½Ã ÅÂ¾ûÏª°E ¸ðµE»óÅÂ¸¦ ÃÖ»óÀ¸·Î º¹±Í(S -> C)
#define MC_QUEST_NPC_ALL_CLEAR				6006	///< NPC ¸ðµÎ »èÁ¦(S -> C)
#define MC_MATCH_QUEST_REQUEST_DEAD			6007	///< ÇÃ·¹ÀÌ¾EÁ×¾ú´Ù°E¿äÃ»(C -> S) Äù½ºÆ®¸ðµå¿¡¼­´Â MC_MATCH_GAME_KILL´EÅ º¸³½´Ù.
#define MC_MATCH_QUEST_PLAYER_DEAD			6008	///< ÇÃ·¹ÀÌ¾EÁ×À½(S -> C)
#define MC_QUEST_OBTAIN_QUESTITEM			6009	///< Äù½ºÆ® ¾ÆÀÌÅÛ È¹µES -> C)
#define MC_QUEST_STAGE_MAPSET				6010	///< ½ºÅ×ÀÌÁE¸Ê¼Â º¯°ES <-> C)
#define MC_QUEST_OBTAIN_ZITEM				6011	///< ÀÏ¹Ý ¾ÆÀÌÅÛ È¹µES -> C)
#define MC_QUEST_PING						6012	///< ¼­¹E>Å¬¶EÇÎ (S -> C)
#define MC_QUEST_PONG						6013	///< Å¬¶E>¼­¹EÇÎ (C -> S)
#define MC_QUEST_NPCLIST					6014	///< ÇöÁ¦ Äù½ºÆ®¿¡¼­ »ç¿EÒ NPCÀÇ Á¤º¸.(S->C) 
///<  Å¬¶ó´Â ¼­¹öÀÇ Á¤º¸¸¦ °¡Áö°ENPC¸¦ ÃÊ±âÈ­ ÇÑ´Ù.


#define MC_QUEST_PEER_NPC_BASICINFO			6040	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â NPC ±âº»Á¤º¸
#define MC_QUEST_PEER_NPC_HPINFO			6041	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â NPC Ã¼·ÂÁ¤º¸
#define MC_QUEST_PEER_NPC_ATTACK_MELEE		6042	///< ±ÙÁ¢ °ø°Ý
#define MC_QUEST_PEER_NPC_ATTACK_RANGE		6043	///< ¿ø°Å¸® °ø°Ý
#define MC_QUEST_PEER_NPC_SKILL_START		6044	///< ½ºÅ³ Ä³½ºÆÃ ½ÃÀÛ
#define MC_QUEST_PEER_NPC_SKILL_EXECUTE		6045	///< ½ºÅ³ Ä³½ºÆÃ¼º°EÈ¿°E½ÃÀÛ
#define MC_QUEST_PEER_NPC_DEAD				6046	///< Á×Àº ±× ½ÃÁ¡¿¡ ÇÇ¾ûÑé¿¡°Ô º¸³½´Ù. 
#define MC_QUEST_PEER_NPC_BOSS_HPAP			6047	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â NPC º¸½º Ã¼·Â ¾Æ¸Ó

// Äù½ºÆ® °ÔÀÓ ÁøÇE°EÃ(Äù½ºÆ® ¸ðµE
#define MC_QUEST_GAME_INFO					6051	///< °ÔÀÓ Á¤º¸. ·ÎµùÇÒ ¸®¼Ò½º µûÜ» ¾Ë¸²(S -> C)
#define MC_QUEST_COMBAT_STATE				6052	///< Äù½ºÆ® °ÔÀÓ³» »óÅÂ Á¤º¸(S -> C)
#define MC_QUEST_SECTOR_START				6053	///< ¸Ê¼½ÅÍ °ÔÀÓ ½ÃÀÛ(S -> C)
#define MC_QUEST_COMPLETED					6054	///< ÇØ´EÄù½ºÆ® Å¬¸®¾ES -> C)
#define MC_QUEST_FAILED						6055	///< Äù½ºÆ® ½ÇÆÐ(S -> C)

#define MC_QUEST_REQUEST_MOVETO_PORTAL		6057	///< Æ÷Å»·Î ÀÌµ¿ÇÔ(C -> S)
#define MC_QUEST_MOVETO_PORTAL				6058	///< Æ÷Å»·Î ÀÌµ¿(S -> C)
#define MC_QUEST_READYTO_NEWSECTOR			6059	///< »õ·Î¿E¼½ÅÍ ÁØºE¿Ï·ES <-> C)

#define MC_GAME_START_FAIL					6060	///< Äù½ºÆ® ½ÃÀÛÀ» ½ÇÆÐ.(S -> C)
#define MC_QUEST_STAGE_GAME_INFO			6061	///< ´EâÁß ½ºÅ×ÀÌÁEÁ¤º¸ ¾Ë¸²(QL, ½Ã³ª¸®¿ÀID). (S -> C)
#define MC_QUEST_SECTOR_BONUS				6062	///< ¼½ÅÍ º¸³Ê½º(S -> C)

// Äù½ºÆ® °ÔÀÓ ÁøÇE°EÃ(¼­¹ÙÀÌ¹E¸ðµE
#define MC_QUEST_ROUND_START				6100	///< »õ·Î¿E¶ó¿ûÑE½ÃÀÛ(S -> C)
#define MC_QUEST_REQUEST_QL					6101	///< ÇöÀEÄù½ºÆ® ½ºÅ×ÀÌÁöÀÇ QLÁ¤º¸¸¦ ¿äÃ».(C -> S)
#define MC_QUEST_RESPONSE_QL				6102	///< ÇöÀEÄù½ºÆ® ½ºÅ×ÀÌÁöÀÇ QLÁ¤º¸¸¦ ¾Ë¸².(S -> C)
#define MC_QUEST_SURVIVAL_RESULT			6103	///< ¼­¹ÙÀÌ¹E°á°EÅE¸(S -> C)
#define MC_SURVIVAL_RANKINGLIST				6104	///< ¼­¹ÙÀÌ¹E»óÀ§ ·©Å· ¸ñ·Ï ÀEÛ(S -> C)
#define MC_SURVIVAL_PRIVATERANKING			6105	///< À¯ÀúÀÇ ¼­¹ÙÀÌ¹E·©Å· Á¤º¸ ÀEÛ(S -> C)


// Äù½ºÆ® Å×½ºÆ®¿E
#define MC_QUEST_TEST_REQUEST_NPC_SPAWN		6901	///< NPC ½ºÆEºÎÅ¹(C -> S)
#define MC_QUEST_TEST_REQUEST_CLEAR_NPC		6902	///< NPC Clear ºÎÅ¹(C -> S)
#define MC_QUEST_TEST_REQUEST_SECTOR_CLEAR	6903	///< ¼½ÅÍ Clear(C -> S)	
#define MC_QUEST_TEST_REQUEST_FINISH		6904	///< Äù½ºÆ® Complete(C -> S)

/// ½Å±ÔÄù½ºÆ®¿E(Ã§¸°ÁEÄù½ºÆ®)
#define MC_NEWQUEST_NPC_SPAWN				6301	///< NPC ½ºÆE(S -> C)
#define MC_NEWQUEST_REQUEST_NPC_SPAWN		6304	///< NPC ½ºÆE(C -> S)
#define MC_NEWQUEST_REQUEST_NPC_DEAD		6302	///< NPC Á×À½ ¿äÃ»(C -> S)
#define MC_NEWQUEST_NPC_DEAD				6303	///< NPC Á×À½(S -> C)
#define MC_NEWQUEST_MOVE_TO_NEXT_SECTOR		6341	///< ´ÙÀ½ ¼½ÅÍ·Î ÀÌµ¿ (S -> C)
#define MC_NEWQUEST_PEER_NPC_BASICINFO		6351	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â NPC ±âº»Á¤º¸
#define MC_NEWQUEST_PEER_NPC_ACTION_EXECUTE	6404	///< NPC ¾×¼Ç ½ÃÀÛ
#define MC_NEWQUEST_PEER_NPC_ATTACK_MELEE	6405	///< NPC ±ÙÁ¢°ø°Ý	//todok del ÀÌ°Ç ¾×¼Ç ½ÇÇàÅEö¸¦ ¹ÞÀ¸¸EpeerµéÀÌ °¢ÀÚ Ã³¸®ÇÏ°Ô µÆ´Ù
#define MC_NEWQUEST_NEW_CONTROL				6406
#define MC_NEWQUEST_OBTAIN_ZITEM			6407	///< ÀÏ¹Ý ¾ÆÀÌÅÛ È¹µES -> C)


// MatchServer Schedule°EÃ.
#define MC_MATCH_SCHEDULE_ANNOUNCE_MAKE				7001	///< ½ºÄÉÁE°øÁE»ý¼º.
#define MC_MATCH_SCHEDULE_ANNOUNCE_SEND				7002	///< ½ºÄÉÁE°øÁEÀEÛ.
#define MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN	7003	///< Å¬·£¼­¹EÅ¬·£ÀEºñÈ°¼ºÈ­.
#define MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON		7004	///< Å¬·£¼­¹EÅ¬·£ÀEÈ°¼ºÈ­.
#define MC_MATCH_SCHEDULE_STOP_SERVER				7005	///< ¼­¹EÁ¾·E


// x-trap
#define MC_REQUEST_XTRAP_HASHVALUE					8001	///< ÀÌÀEúÀÇ È£È¯À¸·Î Á¸Àç¸¸(»ç¿EÏÁE¾Ê´Â´Ù.)
#define MC_RESPONSE_XTRAP_HASHVALUE					8002	///< ÀÌÀEúÀÇ È£È¯À¸·Î Á¸Àç¸¸(»ç¿EÏÁE¾Ê´Â´Ù.)
#define MC_REQUEST_XTRAP_SEEDKEY					8003	///< ¾ÏÈ£Å° ¿äÃ»(S -> C)
#define MC_RESPONSE_XTRAP_SEEDKEY					8004	///< ¾ÏÈ£Å° ÀÀ´EC -> S)
#define MC_REQUEST_XTRAP_DETECTCRACK				8005	///< ÀÌÀEúÀÇ È£È¯À¸·Î Á¸Àç¸¸(»ç¿EÏÁE¾Ê´Â´Ù.)

// gameguard
#define MC_REQUEST_GAMEGUARD_AUTH					9001
#define MC_RESPONSE_GAMEGUARD_AUTH					9002
#define MC_REQUEST_FIRST_GAMEGUARD_AUTH				9003
#define MC_RESPONSE_FIRST_GAMEGUARD_AUTH			9004

// ÀÚ¼E
#define MC_REQUEST_GIVE_ONESELF_UP					9101	///< ÀÚ¼öÇÏ´Ù.


//// ÇÇ¾ûáõÇÇ¾E°EÃ ¸úÓÉ¾E/////////////////////////////////////////////////////////////
#define MC_PEER_PING				10001	///< peerµE»çÀÌÀÇ network latency(ping)À» ÃøÁ¤ÇÏ±âÀ§ÇÑ command
#define MC_PEER_PONG				10002	///< ping¿¡ ´EÑ ÀÀ´E
#define MC_PEER_UDPTEST				10005	///< Peer to Peer UDP ÀEÛÀÌ °¡´ÉÇÑÁE°Ë»çÇÑ´Ù.
#define MC_PEER_UDPTEST_REPLY		10006	///< Peer to Peer UDP ÀEÛÀÌ °¡´ÉÇÑÁE°Ë»çÇÑ´Ù.

#define MC_UDP_PING					10007	///< Client¿Í AgentµE»çÀÌÀÇ pingÀ» ÃøÁ¤ÇÏ±âÀ§ÇÑ command
#define MC_UDP_PONG					10008	///< ping¿¡ ´EÑ ÀÀ´E

#define MC_PEER_OPENED				10011	///< °³º° Peer ÅEÅ °³ÅEË¸²
#define MC_PEER_BASICINFO			10012	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â Ä³¸¯ÅÍ ±âº»Á¤º¸ ÅõÇ¥ÆÇÁ¤ÀÇ ±Ù°Å°¡ µÈ´Ù
#define MC_PEER_HPINFO				10013	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â Ä³¸¯ÅÍ Ã¼·ÂÁ¤º¸ ÅõÇ¥ ÆÇÁ¤ÀÇ ±Ù°Å°¡ µÈ´Ù
#define MC_PEER_HPAPINFO			10014	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â Ä³¸¯ÅÍ Ã¼·Â/¾Æ¸Ó Á¤º¸
#define MC_PEER_DUELTOURNAMENT_HPAPINFO		10016	///< Á¤±âÀûÀ¸·Î ¾÷µ¥ÀÌÆ®µÇ´Â µà¾óÅä³Ê¸ÕÆ® ÀEEÄ³¸¯ÅÍ Ã¼·Â/¾Æ¸Ó Á¤º¸

#define MC_PEER_CHANGECHARACTER		10015
#define MC_PEER_MOVE				10021   ///< ¿¡ ³ªÁß¿¡ MC_OBJECT_MOVE·Î ¹Ù²ûÚûÚßÇÒµE.
#define MC_PEER_CHANGE_WEAPON		10022   
#define MC_PEER_CHANGE_PARTS		10023
#define MC_PEER_ATTACK				10031
#define MC_PEER_DAMAGE				10032
#define MC_PEER_RELOAD				10033
#define MC_PEER_SHOT				10034
#define MC_PEER_SHOT_SP				10035
#define MC_PEER_SKILL				10036
#define MC_PEER_SHOT_MELEE			10037	///< ±ÙÁ¢°ø°Ý
#define MC_PEER_DIE					10041
#define MC_PEER_SPAWN				10042
#define MC_PEER_DASH				10045
#define MC_PEER_SPMOTION			10046
#define MC_PEER_SLASH_CHARGING		10047
#define MC_PEER_SLASH_CHARGED		10048
#define MC_PEER_SLASH				10049
#define MC_PEER_CHAT				10052
#define MC_PEER_CHAT_ICON			10053	///< Ã¤ÆÃ ½ÃÀÛ/³¡. ¸Ó¸®¿¡ ¾ÆÀÌÄÜÀ» º¸¿©ÁÖ°Å³ª °¨Ãá´Ù.
#define MC_PEER_REACTION			10054	///< ÀÌÆåÆ®¸¦ À§ÇØ ¾Ë·ÁÁØ´Ù 
#define MC_PEER_ENCHANT_DAMAGE		10055	///< ÀÎÃ¦Æ® µ¥¹ÌÁö¸¦ ¸Â¾Ò´Ù
#define MC_PEER_BUFF_INFO			10056	///< ¹öÇÁ ÀÌÆåÆ®¸¦ ÀEÞ(¼ô¹öÇÁÀÇ °æ¿E ¹ßµ¿½Ã »ó´Eµ ÅEö¸¦ ¹ÞÀ¸¹Ç·Î µû·Î º¸³»ÁÖÁE¾Ê¾Æµµ µÈ´Ù)
											///< ±×·¯³ª ³­ÀÔÀÚ¿¡°Ô´Â ÀÚ½ÅÀÇ ¹ßµ¿ ÁßÀÎ ÀÌÆåÆ®¸¦ ¾Ë·ÁÁÖ¾ûÚß ÇÏ¹Ç·Î ±×¶§ »ç¿EÑ´Ù)

#define MC_PEER_VAMPIRE				10066
#define MC_PEER_SNIFER				10067

#define MC_ADMIN_FREEZE				10068
#define MC_ADMIN_SPAWN_RESPONSE		10069
#define MC_ADMIN_SUMMON				10070
#define MC_ADMIN_SLAP				10071
#define MC_ADMIN_SPAWN				10072	
#define MC_ADMIN_PKICK				10073
#define MC_ADMIN_NAT				10074
#define MC_VIP_ANNOUNCE				10077

#define MC_MATCH_FLAG_EFFECT		10080
#define MC_MATCH_FLAG_CAP			10081
#define MC_MATCH_REQUEST_FLAG_CAP	10082
#define MC_MATCH_FLAG_STATE			10083

#define MC_ADMIN_GOTO				10084
#define MC_ADMIN_SILENCE			10085

#define MC_MATCH_REQUEST_INFECT		10086
#define MC_MATCH_INFECT				10087
#define MC_MATCH_LASTSURVIVOR		10088

#define MC_MATCH_GUNGAME_WEAPONDATA 10089

#define MC_GUNZ_ANTILEAD_OLD		10093
#define MC_ADMIN_TEAM				10094
#define MC_MATCH_REQUEST_ROLL		10095
#define MC_MATCH_RESPONSE_ROLL		10096
#define MC_REQUEST_RESOURCE_CRC32				11001
#define MC_RESPONSE_RESOURCE_CRC32				12002


// Äù½ºÆ® ¾ÆÀÌÅÛ °EÃ ºÎºÐ.
#define MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST		21000	///< ³ªÀÇ Äù½ºÆ® ¾ÆÀÌÅÛ ¸ñ·ÏÀ» ¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST		21001	///< ³ªÀÇ Äù½ºÆ® ¾ÆÀÌÅÛ ¸ñ·ÏÀ» ÀEÛ.(S -> C)
#define MC_MATCH_REQUEST_BUY_QUEST_ITEM				21002	///< Äù½ºÆ® ¾ÆÀÌÅÛ ±¸ÀÔ ¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_BUY_QUEST_ITEM			21003	///< Äù½ºÆ® ¾ÆÀÌÅÛ ±¸ÀÔ ¿Ï·E(S -> C)
#define MC_MATCH_REQUEST_SELL_QUEST_ITEM			21004	///< ³ªÀÇ Äù½ºÆ® ¾ÆÀÌÅÛ ÆÇ¸Å ¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_SELL_QUEST_ITEM			21005	///< ³ªÀÇ Äù½ºÆ® ¾ÆÀÌÅÛ ÆÇ¸Å ¿Ï·E(S -> C)
#define MC_MATCH_USER_REWARD_QUEST					21006	///< Äù½ºÆ®°¡ ¿Ï·áµÈÈÄ º¸»óµÈ Äù½ºÆ® ¾ÆÀÌÅÛ Á¤º¸.
#define MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM		21007	///< Èñ»ý ¾ÆÀÌÅÛÀ» ½½·Ô¿¡ ³õ±E¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM		21008	///< Èñ»ý ¾ÆÀÌÅÛÀ» ½½·Ô¿¡ ³õ±EÀÀ´E(S -> C)
#define MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM	21009	///< ½½·Ô¿¡ ¿Ã·Á³õÀº Èñ»ý ¾ÆÀÌÅÛÀ» È¸¼E¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM	21010	///< ½º·Ô¿¡ ¿Ã·Á³õÀº Èñ»ý ¾ÆÀÌÅÛÀ» ÈÖ¼EÀÀ´E(S -> C)
#define MC_MATCH_REQUEST_SLOT_INFO					21011	///< Èñ»ý ¾ÆÀÌÅÛ ½½·Ô Á¤º¸¸¦ ¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_SLOT_INFO					21012	///< Èñ»ý ¾ÆÀÌÅÛ ½½·Ô Á¤º¸¸¦ ¾Ë¸².(S -> C)
#define MC_MATCH_NEW_MONSTER_INFO					21013	///< ¸ó½ºÅÍ µµ°¨¿¡ Ãß°¡µÉ »õ·Î¿E¸ó½ºÅÍ Á¤º¸.
#define MC_MATCH_REQUEST_MONSTER_BIBLE_INFO			21014	///< ¸ó½ºÅÍ µµ°¨ÀÇ ³»¿E¿äÃ».(C -> S)
#define MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO		21015	///< ¸ó½ºÅÍ µµ°¨ÀÇ ³»¿E¾Ë¸².(S -> C)


/// Keeper¿Í Keeper Manager¿Í ÅEÅÇÏ´Âµ¥ ÇÊ¿äÇÑ ¸úÓÉ. 2005.06.03 Ä¿¸ÇµEÁ¤¸® ÇÊ¿E
#define MC_KEEPER_MANAGER_CONNECT				31001	///< keeper Manager¿¡¼­ keeper·Î Á¢¼Ó ¿äÃ».
#define MC_CHECK_KEEPER_MANAGER_PING			31002	///< Keeper¿Í Keeper Manager°¡ Á¤»óÀûÀ¸·Î ¿¬°áµÇ¾EÀÖ´ÂÁE°Ë»E
#define MC_REQUEST_MATCHSERVER_STATUS			31004	///< MatchServerÀÇ Á¤º¸¸¦ ¿äÃ».(Keeper -> MatchServer)
#define MC_RESPONSE_MATCHSERVER_STATUS			31005	///< Keeper·Î MatchServerÀÇ Á¤º¸¸¦ ÀEÛÇØ ÁÜ.(MatchServer -> Keeper)
// 2005.06.01 ~
#define MC_RESPONSE_KEEPER_MANAGER_CONNECT		31006	///< Keeper managerÀÇ Á¢¼ÓÀÌ Á¤»óÀûÀ¸·Î ÀÌ·ç¾EÁE» ¾Ë¸².( Keeper -> Keeper Manager )
#define MC_REQUEST_CONNECTION_STATE				31007	///< ÇöÁ¦ ¼­¹öÀÇ »óÅÂ.(Keeper Manager -> Keeper)
#define MC_RESPONSE_CONNECTION_STATE			31008	///< ¼­¹ö»óÅÂ ÀÀ´E(Keeper -> Keeper Manager)
#define MC_REQUEST_SERVER_HEARBEAT				31009	///< ¼­¹ö°¡ µ¿ÀÛÇÏ°EÀÖ´ÂÁEÀÀ´E¿äÃ».(Keeper -> MatchServer)
#define MC_RESPONSE_SERVER_HEARHEAT				31010	///< ¼­¹ö°¡ µ¿ÀÛÇÏ°EÀÖÀ»À» ÀÀ´E(MatchServer -> Keeper)
#define MC_REQUEST_KEEPER_CONNECT_MATCHSERVER	31011	///< Keeper°¡ MatchServer·Î Á¢¼ÓÇÏµµ·Ï ¿äÃ»ÇÔ(Keeper Manager -> Keeper -> MatchServer )
#define MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER	31012	///< Keeper°¡ MatchServer¿¡ Á¢¼ÓÇß´Ù´Â ÀÀ´E(MatchServer -> Keeper -> Keeper Manager )
#define MC_REQUEST_REFRESH_SERVER				31013	///< Keeper ManagerÀÇ ¼­¹E¸®½ºÆ® ¼öµ¿ °»½ÅÀ» À§ÇØ¼­ Keeper·Î MatchServerÀÇ HeartbaetÃ¤Å© ¿äÃ».(Keeper Manager -> Keeepr)
#define MC_REQUEST_LAST_JOB_STATE				31014	///< ¸¶Áö¸· ½ÇÇàµÈ ÀÏÀÇ °á°ú¸¦ ¿äÃ».(Keeper -> Keeper Manager )
#define MC_RESPONSE_LAST_JOB_STATE				31015	///< ÀÛ¾E°á°E¿äÃ» ÀÀ´E(Keeper -> Keeper Manager)
#define MC_REQUEST_CONFIG_STATE					31016	///< Keeper¿¡ ÀúÀåµÈ ¼³Á¤ Á¤º¸ ¿äÃ».(Keeper Manager -> Keeper)
#define MC_RESPONSE_CONFIG_STATE				31017	///< Keeper¿¡ ÀúÀåµÈ ¼³Á¤ Á¤º¸ ¿äÃ» ÀÀ´E(Keeper -> Keeper Manager)
#define MC_REQUEST_SET_ONE_CONFIG				31018	///< ConfigÇÏ³ª ¼³Á¤ ¿äÃ».(Keeper Manager -> Keeper)
#define MC_RESPONSE_SET_ONE_CONFIG				31019	///< ConfigÇÏ³ª ¼³Á¤ ¿äÃ» ÀÀ´E(Keeper -> Keeper Manager)
#define MC_REQUEST_KEEPERMGR_ANNOUNCE			31020	///< Keeper Manager¿¡¼­ °¢ ¼­¹ö·Î °øÁö¸¦ º¸³»µµ·Ï Keeper¿¡ ¿äÃ»ÇÔ.(Keeper Manager -> Keeper)
#define MC_REQUEST_KEEPER_ANNOUNCE				31021	///< Keeper¿¡¼­ MatchServer°¡ °øÁö»çÇ×À» ¾Ë¸®µµ·Ï ¿äÃ»ÇÔ.(Keeper -> MatchServer)
#define MC_REQUEST_RESET_PATCH					31022	///< ÇöÁ¦+±ûÝEÁøÇàÇÑ ÆÐÄ¡ ÀÛ¾÷»óÅÂ¸¦ ÃÊ±âÈ­ÇÔ.
#define MC_REQUEST_DISCONNECT_SERVER			31023	///< Keeper¿Í MatchServer¿ÍÀÇ ¿¬°EÁ¾·E
#define MC_REQUEST_REBOOT_WINDOWS				31024	///< À©µµ¿EÀçºÎÆÃ.(Keeper Manager -> Keeper)
#define MC_REQUEST_ANNOUNCE_STOP_SERVER			31025	///< °øÁö»çÇ×°EÇÔ°Ô ¼­¹ö¸¦ Á¾·E½ÃÅ´ ¿äÃ».
#define MC_RESPONSE_ANNOUNCE_STOP_SERVER		31026	///< °øÁö»çÇ×°EÇÔ°Ô ¼­¹ö¸¦ Á¾·E½ÃÅ´ ¿äÃ» ÀÀ´E
#define MC_REQUEST_SERVER_AGENT_STATE			31027	///< ÇöÁ¦ ¼­¹ö¿Í ¿¡ÀÌÀE®ÀÇ ½ÇÇE»óÅÂ ¿äÃ».
#define MC_RESPONSE_SERVER_AGENT_STATE			31028	///< ÇöÁ¦ ¼­¹ö¿Í ¿¡ÀÌÀE®ÀÇ ½ÇÇE»óÅÂ ¿äÃ» ÀÀ´E
#define MC_REQUEST_WRITE_CLIENT_CRC				31029	///< Å¬¶óÀÌ¾ðÆ®ÀÇ CRC CheckSumÀ» ¼­¹EiniÆÄÀÏ¿¡ ÀúÀE¿äÃ».(KeeperManager -> Keeper)
#define MC_RESPONSE_WRITE_CLIENT_CRC			31030	///< Å¬¶óÀÌ¾ðÆ®ÀÇ CRC CheckSumÀ» ¼­¹EiniÆÄÀÏ¿¡ ÀúÀE¿äÃ» ÀÀ´E(Keeper -> Keeper Manager)
#define MC_REQUEST_KEEPER_RELOAD_SERVER_CONFIG	31031	///< Keeper¿Í ¿¬°áµÇ¾ûÜÖ´Â ¼­¹ö°¡ ¼³Á¤ ÆÄÀÏÀ» ´Ù½Ã ·ÎµåÇÏµµ·Ï ¿äÃ»ÇÔ.(KeeperManager -> Keeper)
#define MC_REQUEST_RELOAD_CONFIG				31032	///< ¼­¹öÀÇ ¼³Á¤ ÆÄÀÏÀ» ´Ù½Ã ·Î´ÙÇÏµµ·Ï ¿äÃ».(Keeper -> server)
#define MC_REQUEST_KEEPER_ADD_HASHMAP			31033	
#define MC_RESPONSE_KEEPER_ADD_HASHMAP			31034
#define MC_REQUEST_ADD_HASHMAP					31035
#define MC_RESPONSE_ADD_HASHMAP					31036

// MatchServer ÆÐÄ¡ °EÃ.
#define MC_REQUEST_DOWNLOAD_SERVER_PATCH_FILE	32024	///< Keeper°¡ FTP¼­¹ö¿¡¼­ PatchÆÄÀÏÀ» ´Ù¿ûÓÎµE½ÃÅ´.(Keeper Manager -> Keeper )
#define MC_REQUEST_START_SERVER					32026	///< MatchServer½ÇÇE¿äÃ».(Keeper Manager -> Keeper)
#define MC_REQUEST_STOP_SERVER					32028	///< ¼­¹EÁ¤ÁE¿äÃ».(Keeper -> Keeper Manager)
#define MC_REQUEST_PREPARE_SERVER_PATCH			32030	///< ÆÐÄ¡ ÁØºEÀÛ¾E(¾ÐÃEÇ®±E ÀÓ½Ã ÆÄÀÏ Á¦°Å.)
#define MC_REQUEST_SERVER_PATCH					32032	///< ÆÐÄ¡ ÆÄÀÏÀ» º¹»çÇÔ.(Keeper Manager -> Keeper )
// MatchAgent ÆÐÄ¡ °EÃ.
#define MC_REQUEST_STOP_AGENT_SERVER			33034	///< Agent serverÁ¤ÁE¿äÃ».(Keeper Manager -> Keeper )
#define MC_REQUEST_START_AGENT_SERVER			33036	///< Agent server½ÃÀÛ ¿äÃ».(Keeper Manager -> Keeper)
#define MC_REQUEST_DOWNLOAD_AGENT_PATCH_FILE	33038	///< request agent patch file download.(Keeper Manager -> Keeper)
#define MC_REQUEST_PREPARE_AGENT_PATCH			33040	///< reqeust prepare agent patch.(Keeper Manager -> Keeper)
#define MC_REQUEST_AGENT_PATCH					33042	///< request agent patch.(Keeper Manager -> Keeper)
#define MC_REQUEST_SERVER_STATUS				33043	///< server¿Í agentÀÇ Á¤º¸¸¦ Keeper·Î ¿äÃ»ÇÔ.(Keeper Manager -> keeper)
#define MC_RESPONSE_SERVER_STATUS				33044	///< server¿Í agetnÀÇ Á¤º¸ ¿äÃ» ÀÀ´E(Keeper -> Keeper Manager)
// Keeper manager schedule.
#define MC_REQUEST_KEEPER_MANAGER_SCHEDULE		34001	///< request keeper manager schedule.(Keeper Manager -> Keeper -> MatchServer)
#define MC_RESPONSE_KEEPER_MANAGER_SCHEDULE		34002	///< Keeper ManagerÀÇ ½ºÄÉÁEûÓÏ ¿äÃ» ÀÀ´E
#define MC_REQUEST_START_SERVER_SCHEDULE		34003	///< ´Ù½Ã½ÃÀÛ ½ºÄÉÁÙ¿¡¼­ ¼­¹ö¸¦ ½ÃÀÛÇÏ´Â ½ÃÄÉÁÙ. ½ÃÀÛÇÒ¶§±ûÝE½ºÄÉÁÙÀÌ Á¦°ÅµÇÁE¾ÊÀ½.
///


/// Locator°EÃ.
#define MC_REQUEST_SERVER_LIST_INFO				40001	/// Á¢¼Ó°¡´ÉÇÑ ¼­¹öÀÇ ¸®½ºÆ® Á¤º¸ ¿äÃ».(Client -> Locator)
#define MC_RESPONSE_SERVER_LIST_INFO			40002	/// Á¢¼Ó°¡´ÉÇÑ ¼­¹E¸®½ºÆ® Á¤º¸ ÀÀ´E(Locator -> Client)
#define MC_RESPONSE_BLOCK_COUNTRY_CODE_IP		40003	/// Á¢¼ÓºÒ°¡ ±¹°¡ ÄÚµåÀÇ IPÀÀ´E(Locator -> Client)


// filter.
#define MC_RESPONSE_BLOCK_COUNTRYCODE			50001	/// ºúÓ°µÈ ±¹°¡ÄÚµåÀÇ IP°¡ Á¢¼Ó½Ã ÅE¸¿E
#define MC_LOCAL_UPDATE_USE_COUNTRY_FILTER		50002
#define MC_LOCAL_GET_DB_IP_TO_COUNTRY			50003
#define MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE		50004
#define MC_LOCAL_GET_DB_CUSTOM_IP				50005
#define MC_LOCAL_UPDATE_IP_TO_COUNTRY			50006
#define MC_LOCAL_UPDATE_BLOCK_COUTRYCODE		50007
#define MC_LOCAL_UPDATE_CUSTOM_IP				50008
#define MC_LOCAL_UPDATE_ACCEPT_INVALID_IP		50009
#define MC_ADMIN_GIVE_COINSCASH					50010
#define MC_ADMIN_GIVE_COINSMEDALS				50011
#define MC_ADMIN_SENDITEM					    50012
#define MC_STAFF_CHAT							50013
#define MC_GUNZ_ANTI_LEAD_N						50014

#define MC_GUNZ_LASTDMG							50023
#define MC_GUNZ_DMGGIVEN						50024
#define MC_KILL_ALL                             50025
#define MC_ADMIN_TELEPORT                       50026
#define MC_TELEPORT_ALL							50027
#define MC_ADMIN_FREEZE_ALL						50028
#define MC_REPORT_USER							50029
#define MC_ADMIN_STAGE_JOIN						50030

////////// Spy Mode //////////
#define MC_SPY_STAGE_BAN_MAP_LIST					50032
#define MC_SPY_STAGE_ACTIVATE_MAP					50033
#define MC_SPY_STAGE_REQUEST_START					50034
#define MC_SPY_GAME_INFO							50035
#define MC_SPY_GAME_RESULT							50036
#define MC_SPY_GAME_SCORE							50037
#define MC_SPY_STAGE_REQUEST_BAN_MAP_LIST			50038
#define MC_MATCH_FREEZESPY							50039
#define MC_MATCH_UNFREEZESPY					    50040
#define MC_MATCH_SPY_ROUNDRESULT			    	50041


//////////////////LadderWars/////////////////////////////
#define MC_MATCH_LADDERWARS_CHARINFO			50052
#define MC_MATCH_JOIN_LADDERWARS				50053
#define MC_MATCH_LADDERWARS_COUNTER			    50054
#define MC_MATCH_LADDERWARS_SIDERANK			50055
#define MC_MATCH_LADDERWARS_INVITED			    50056
#define MC_MATCH_LADDERWARS_FRIENDINVITE		50057
#define MC_MATCH_LADDERWARS_FRIENDACCEPT		50058
#define MC_MATCH_LADDERWARS_FRIENDLEAVE		    50059
#define MC_MATCH_LADDERWARS_VOTE				50085
#define MC_MATCH_LADDERWARS_VOTE_UPDATE		    50086
#define MC_MATCH_LADDERWARS_RANDOM_MAPS		    50087
#define MC_PEER_LADDERWARS				        10118

////////////////////////////////////////////////////////
#define MC_ADMIN_WALL							50060
#define MC_UGRADEID_SEND                        50061
#define MC_ADMIN_GIVE_COINSMEDALS_RESPONSE		50065

//////////////////ChatVoice/////////////////////////////
#define MC_MATCH_SEND_VOICE_CHAT               50067
#define MC_MATCH_RECEIVE_VOICE_CHAT            50068
////////////////////////////////////////////////////////
#define MC_FIND_WEAPON						   50069 //Kill Feed
#define MC_PEER_SET_SWORD_COLOR                50070
#define MC_MATCH_RESPONSE_SPEC                 50071
#define MC_PEER_SPEC                           50072
#define MC_MATCH_REQUEST_SPEC                  50073
#define MC_PEER_MOVE_DELTA                     50074

/////////////////////////no lead//////////////////////////
#define MC_PEER_NOLEADBLOB			           50075
#define MC_PEER_NOLEAD				           50076

///////////////////autobanned////////////////////////////
#define MC_MATCH_BANME				            10091
#define MC_ADMIN_SBTEST				            10092
#define MC_MATCH_BANACCOUNT                     10106

///////////////////Information Player////////////////////////////
#define MC_MATCH_INFORMATION					10100
#define MC_MATCH_INFORMATION_RECEIVE			10095
#define MC_MATCH_INFORMATION_HPAP				10096
#define MC_MATCH_INFORMATION_HPAP_GO			10097
#define MC_MATCH_INFORMATION_CLOTHES			10098
#define MC_MATCH_INFORMATION_CLOTHES_GO			10099

/////////////////////////Skill Map//////////////////////////////
#define MC_MATCH_UPDATE_SKILLMAP_BESTTIME	    10101
#define MC_MATCH_REQUEST_SKILLMAP_BESTTIME	    10102
#define MC_MATCH_RESPONSE_SKILLMAP_BESTTIME	    10103
#define MC_SKILLMAP_OBTAIN_ITEM				    10105

///////////////////Comandos Staff//////////////////////////////

#define MC_ALL_CMD								10107
#define MC_ADMIN_CRASH							10108
#define MC_MATCH_UPDATE_CLIENT_SETTINGS         10109
#define MC_PEER_UPDATECHAR                      10110

///////////////////Magic Box//////////////////////////////
#define MC_DROPGUNGAME_RESPONSE_ENTERGAME		10111
#define MC_DROPGUNGAME_REQUEST_ITEM				10112
#define MC_DROPGUNGAME_RESPONSE_ITEM			10113
#define MC_DROPGUNGAME_RESPONSE_WORLDITEMS		10114

/////////////////MapTime/////////////////////////////////
//Custom: Lightmaps Time Code
#define MC_MATCH_STAGE_MAPTOD					10115

/////////////////Clan War Inite//////////////////////////
#define MC_INVITE_CLANWAR						10116

/////////////////Chat Voice Icon/////////////////////////
#define MC_PEER_CHATVOICE_ICON			            10117

/////////////////HP/AP/DMG////////////////////////////////
#define MC_PEER_DAMAGE_TAB						10119
#define MC_PEER_HPAP_TAB						10120

//////////////////SPMOTIOLASTREPLY////////////////////////
#define MC_MATCH_SPMOTION						10121
#define MC_MATCH_SPMOTION_REPLY					10122

/////////////////////////LADDERJOIN///////////////////////
#define MC_MATCH_STAGE_REQUEST_REJOIN			10123

///////////////////////ROCKETGUIDE////////////////////////
#define MC_REQUEST_GUIDEDMISSLE					10124
#define MC_RESPONSE_GUIDEDMISSLE				10125

///////////////////////PORTALGUNZ/////////////////////////
#define MC_MATCH_REQUEST_SPAWN_GAMEPORTAL		10126
#define MC_REQUEST_MOVETO_GAMEPORTAL		    10127
#define MC_RESPONSE_MOVETO_GAMEPORTAL		    10128

///////////////////////FLOATDMGTEXT///////////////////////
#define MC_PEER_DAMAGECOUNTER					10129

///////////////////////NEW TAB SHOP///////////////////////
#define MC_MATCH_BUY_PACKAGE					10130 // Điền ID Command
#define MC_MATCH_RESPONSE_BUY_PACKAGE			10131

///////////////////////EVENT TAB/////////////////////////
#define MC_MATCH_EVENT_CLAIM_ATTENDANCE_REQUEST    10132 //
#define MC_MATCH_EVENT_CLAIM_ATTENDANCE_RESPONSE   10133 //
#define MC_MATCH_EVENT_ATTENDANCE_INFO_REQUEST     10134
#define MC_MATCH_EVENT_ATTENDANCE_INFO_RESPONSE    10135


#endif