#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "Msg.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MAsyncDBJob_InsertConnLog.h"
#include "RTypes.h"
#include "MMatchUtil.h"
#include <winbase.h>
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchStatus.h"
#include "MMatchLocale.h"
#include "md5wrapper.h" // Custom: MD5 hashing

bool MMatchServer::CheckOnLoginPre(const MUID& CommUID, int nCmdVersion, bool& outbFreeIP, string& strCountryCode3)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return false;

	// ÇÁ·ÎÅäÄÝ ¹öÀü Ã¼Å©
	if (nCmdVersion != MCOMMAND_VERSION)
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_COMMAND_INVALID_VERSION);
		Post(pCmd);	
		return false;
	}

	// free login ip¸¦ °Ë»çÇÏ±âÀü¿¡ debug¼­¹ö¿Í debug ip¸¦ °Ë»çÇÑ´Ù.
	// ¼­¹ö°¡ debugÅ¸ÀÔÀÎÁö °Ë»ç.
	if( MGetServerConfig()->IsDebugServer() && MGetServerConfig()->IsDebugLoginIPList(pCommObj->GetIPString()) )
	{
		outbFreeIP = true;
		return true;
	}

	// ÃÖ´ëÀÎ¿ø Ã¼Å©
	bool bFreeLoginIP = false;
	if (MGetServerConfig()->CheckFreeLoginIPList(pCommObj->GetIPString()) == true) {
		bFreeLoginIP = true;
		outbFreeIP = true;
		return true;
	} else {
		outbFreeIP = false;

		if ((int)m_Objects.size() >= MGetServerConfig()->GetMaxUser())
		{
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_FULL_PLAYERS);
			Post(pCmd);	
			return false;
		}
	}

	// Á¢¼ÓÀ» ¸·¾Æ³õÀº Áö¿ªÀÇ IPÀÎ°¡
	if( CheckIsValidIP(CommUID, pCommObj->GetIPString(), strCountryCode3, MGetServerConfig()->IsUseFilter()) )
		IncreaseNonBlockCount();
	else
	{
		IncreaseBlockCount();

		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_BLOCK_IP);
		Post(pCmd);	
		return false;
	}

	return true;
}
#ifdef _HWID
void MMatchServer::OnMatchLogin(MUID CommUID, const char* szUserID, const char* szPassword, int nCommandVersion, unsigned long nChecksumPack, char* szEncryptMd5Value, char* szHwid)
{
#else
void MMatchServer::OnMatchLogin(MUID CommUID, const char* szUserID, const char* szPassword, int nCommandVersion, unsigned long nChecksumPack, char* szEncryptMd5Value)
{
#endif
	//MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);

	int nMapID = 0;
	unsigned int nAID = 0;

	char szDBPassword[32] = "";
	string strCountryCode3;

	bool bFreeLoginIP = false;

	if (!CheckOnLoginPre(CommUID, nCommandVersion, bFreeLoginIP, strCountryCode3)) return;
	if (!m_MatchDBMgr.GetLoginInfo(szUserID, &nAID, szDBPassword))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);	
		LOG(LOG_PROG, "Wrong UserID\n");
		//SAFE_DELETE_ARRAY(szDBPassword);
		return;
	}

	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj)
	{
		if (!m_MatchDBMgr.UpdateLastConnDate(szUserID, pCommObj->GetIPString()))
		{	
			//mlog("DB Query(OnMatchLogin > UpdateLastConnDate) Failed");
		}

	}
	//if (strcmp(szDBPassword, szPassword) !=0 )
	//{
	////	mlog("Failed Login Based on Hash\n");
	//	MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
	//	Post(pCmd);	
	//	//SAFE_DELETE_ARRAY(szDBPassword);
	//	return;
	//}
	if (strcmp(szDBPassword, szPassword))
	{
		LOG(LOG_PROG, "Wrong Password\n");
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);
		return;
	}

	MMatchAccountInfo accountInfo;
	if (!m_MatchDBMgr.GetAccountInfo(nAID, &accountInfo, MGetServerConfig()->GetServerID()))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_GETACCOUNTINFO);
		Post(pCmd);	
	}

	MMatchAccountPenaltyInfo accountpenaltyInfo;
	if( !m_MatchDBMgr.GetAccountPenaltyInfo(nAID, &accountpenaltyInfo) ) 
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_GETACCOUNTINFO);
		Post(pCmd);	
	}

#ifndef _DEBUG
	MMatchObject* pCopyObj = GetPlayerByAID(accountInfo.m_nAID);
 	if (pCopyObj != NULL) 
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pCopyObj->GetUID(), MERR_MULTIPLE_LOGIN);
		Post(pCmd);	
		Disconnect(pCopyObj->GetUID());
		return; // Custom: Duplicated Login Add Disconnect
	}
#endif

	if ((accountInfo.m_nUGrade == MMUG_BLOCKED) || (accountInfo.m_nUGrade == MMUG_PENALTY))
	{
		LOG(LOG_PROG, "Accound banned : UserID(%s)", accountInfo.m_szUserID);
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);	
		//SAFE_DELETE_ARRAY(szDBPassword);
		return;
	}
#ifdef _HWID
	if (strlen(szHwid) < 8) // or 14
	{
		LOG(LOG_PROG, "Hwid error : AID(%u). [bad size: %d - content: %s]\n \n", accountInfo.m_nAID, strlen(szHwid), szHwid);
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_AUTHENTICATION);
		Post(pCmd);
		return;
	}

	if (!m_MatchDBMgr.UpdateHwid(accountInfo.m_nAID, szHwid))
	{
		mlog("DB Query(OnMatchLogin > UpdateHWID) Failed");
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_AUTHENTICATION);
		Post(pCmd);
		return;
	}

	strcpy_s(accountInfo.m_szHWID, szHwid);

	if (m_MatchDBMgr.IsBannedHwid(szHwid))
	{
		LOG(LOG_PROG, "Hwid banned : AID(%u)", accountInfo.m_nAID);
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);
		return;
	}

//#ifdef _DEBUG

	if (MGetServerConfig()->IsUseMD5())
	{
		unsigned char szMD5Value[MAX_MD5LENGH] = { 0, };
		pCommObj->GetCrypter()->Decrypt(szEncryptMd5Value, MAX_MD5LENGH, (MPacketCrypterKey*)pCommObj->GetCrypter()->GetKey());
		memcpy(szMD5Value, szEncryptMd5Value, MAX_MD5LENGH);

		/*if ((memcmp(m_szMD5Value, szMD5Value, MAX_MD5LENGH)) != 0)
		{
			LOG(LOG_PROG, "MD5 error : AID(%u).\n \n", accountInfo.m_nAID);
			return;
		}*/
		if ((memcmp(m_szMD5Value, szMD5Value, MAX_MD5LENGH)) != 0)
		{
			LOG(LOG_PROG, "MD5 error : AID(%u).\n \n", accountInfo.m_nAID);
			// Gửi phản hồi lỗi về Client trước khi return
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_AUTHENTICATION);
			Post(pCmd);
			return;
		}
	}
//#endif

#endif
	AddObjectOnMatchLogin(CommUID, &accountInfo, &accountpenaltyInfo, bFreeLoginIP, strCountryCode3, nChecksumPack);
	//SAFE_DELETE_ARRAY(szDBPassword);
}

void MMatchServer::OnMatchLoginFromNetmarbleJP(const MUID& CommUID, const char* szLoginID, const char* szLoginPW, int nCmdVersion, unsigned long nChecksumPack)
{
	bool bFreeLoginIP = false;
	string strCountryCode3;

	// ÇÁ·ÎÅäÄÝ, ÃÖ´ëÀÎ¿ø Ã¼Å©
	if (!CheckOnLoginPre(CommUID, nCmdVersion, bFreeLoginIP, strCountryCode3)) return;

	// DBAgent¿¡ ¸ÕÀú º¸³»°í ÀÀ´äÀ» ¹ÞÀ¸¸é ·Î±×ÀÎ ÇÁ·Î¼¼½º¸¦ ÁøÇàÇÑ´Ù.
	if (!MGetLocale()->PostLoginInfoToDBAgent(CommUID, szLoginID, szLoginPW, bFreeLoginIP, nChecksumPack, GetClientCount()))
	{
		mlog( "Server user full(DB agent error).\n" );
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_FULL_PLAYERS);
		Post(pCmd);
		return;
	}
}

void MMatchServer::OnMatchLoginFromDBAgent(const MUID& CommUID, const char* szLoginID, const char* szName, int nSex, bool bFreeLoginIP, unsigned long nChecksumPack)
{
//#ifndef LOCALE_NHNUSA
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return;

	string strCountryCode3;
	CheckIsValidIP( CommUID, pCommObj->GetIPString(), strCountryCode3, false );

	const char* pUserID = szLoginID;
	char szPassword[16] = "";			// ÆÐ½º¿öµå´Â ¾ø´Ù
	char szCertificate[16] = "";
	const char* pName = szName;
	int nAge = 20;

	bool bCheckPremiumIP = MGetServerConfig()->CheckPremiumIP();
	const char* szIP = pCommObj->GetIPString();
	DWORD dwIP = pCommObj->GetIP();

	// Async DB
	MAsyncDBJob_GetLoginInfo* pNewJob = new MAsyncDBJob_GetLoginInfo(CommUID);
	pNewJob->Input(new MMatchAccountInfo,
					new MMatchAccountPenaltyInfo,
					pUserID, 
					szPassword, 
					szCertificate, 
					pName, 
					nAge, 
					nSex, 
					bFreeLoginIP, 
					nChecksumPack,
					bCheckPremiumIP,
					szIP,
					dwIP,
					strCountryCode3);
	PostAsyncJob(pNewJob);
//#endif
}

void MMatchServer::OnMatchLoginFailedFromDBAgent(const MUID& CommUID, int nResult)
{
//#ifndef LOCALE_NHNUSA
	// ÇÁ·ÎÅäÄÝ ¹öÀü Ã¼Å©
	MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, nResult);
	Post(pCmd);	
//#endif
}

MCommand* MMatchServer::CreateCmdMatchResponseLoginOK(const MUID& uidComm, 
													  MUID& uidPlayer, 
													  const char* szUserID, 
													  MMatchUserGradeID nUGradeID, 
													  MMatchPremiumGradeID nPGradeID,
	                                                  int nCountryFlag,
													  int nCash, int nEvent,
													  const unsigned char* pbyGuidReqMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_LOGIN, uidComm);
	pCmd->AddParameter(new MCommandParameterInt(MOK));
	pCmd->AddParameter(new MCommandParameterString(MGetServerConfig()->GetServerName()));
	pCmd->AddParameter(new MCommandParameterChar((char)MGetServerConfig()->GetServerMode()));
	pCmd->AddParameter(new MCommandParameterString(szUserID));
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)nUGradeID));
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)nPGradeID));
	pCmd->AddParameter(new MCommandParameterInt(nCountryFlag));
	pCmd->AddParameter(new MCommandParameterInt(nCash));
	pCmd->AddParameter(new MCommandParameterInt(nEvent));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterBool((bool)MGetServerConfig()->IsEnabledSurvivalMode()));
	pCmd->AddParameter(new MCommandParameterBool((bool)MGetServerConfig()->IsEnabledDuelTournament()));
//	pCmd->AddParameter(new MCommandParameterString(szRandomValue));

//	void* pBlob1 = MMakeBlobArray(sizeof(unsigned char), 64);
//	unsigned char *pCmdBlock1 = (unsigned char*)MGetBlobArrayElement(pBlob1, 0);
//	CopyMemory(pCmdBlock1, szRandomValue, 64);

//	pCmd->AddParameter(new MCommandParameterBlob(pBlob1, MGetBlobArraySize(pBlob1)));
//	MEraseBlobArray(pBlob1);
	
	void* pBlob = MMakeBlobArray(sizeof(unsigned char), SIZEOF_GUIDREQMSG);
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, pbyGuidReqMsg, SIZEOF_GUIDREQMSG);

	pCmd->AddParameter(new MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);

	return pCmd;
}

MCommand* MMatchServer::CreateCmdMatchResponseLoginFailed(const MUID& uidComm, const int nResult)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_LOGIN, uidComm);
	pCmd->AddParameter(new MCommandParameterInt(nResult));						// 0. Result
	pCmd->AddParameter(new MCommandParameterString(MGetServerConfig()->GetServerName()));	// 1. ServerName
	pCmd->AddParameter(new MCommandParameterChar((char)MGetServerConfig()->GetServerMode())); // 2. ServerMode
	pCmd->AddParameter(new MCommandParameterString(""));						// 3. AccountID (Để trống khi fail)
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)MMUG_FREE));	// 4. UGradeID
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)MMPG_FREE));	// 5. PGradeID
	pCmd->AddParameter(new MCommandParameterInt(0));							// 6. Country (ANH THIẾU CÁI NÀY)
	pCmd->AddParameter(new MCommandParameterInt(0));							// 7. Cash (ANH THIẾU CÁI NÀY)
	pCmd->AddParameter(new MCommandParameterInt(0));							// 8. Event (ANH THIẾU CÁI NÀY)
	pCmd->AddParameter(new MCommandParameterUID(MUID(0, 0)));					// 9. uidPlayer
	pCmd->AddParameter(new MCommandParameterBool((bool)MGetServerConfig()->IsEnabledSurvivalMode()));	// 10. Survival
	pCmd->AddParameter(new MCommandParameterBool((bool)MGetServerConfig()->IsEnabledDuelTournament())); // 11. Duel

	// 12. Blob (EncryptMsg) - Luôn phải có cho đủ bộ 13
	unsigned char tmp = 0;
	void* pBlob = MMakeBlobArray(sizeof(unsigned char), 1);
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
	if (pCmdBlock) *pCmdBlock = tmp;

	pCmd->AddParameter(new MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);

	return pCmd;
}


bool MMatchServer::AddObjectOnMatchLogin(const MUID& uidComm, 
										const MMatchAccountInfo* pSrcAccountInfo,
										const MMatchAccountPenaltyInfo* pSrcAccountPenaltyInfo,
										bool bFreeLoginIP, string strCountryCode3, unsigned long nChecksumPack)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(uidComm);
	if (pCommObj == NULL) return false;

	MUID AllocUID = uidComm;
	int nErrCode = ObjectAdd(uidComm);
	if(nErrCode!=MOK) {
		LOG(LOG_DEBUG, MErrStr(nErrCode) );
	}

	MMatchObject* pObj = GetObject(AllocUID);
	if (pObj == NULL) {
		// Notify Message ÇÊ¿ä -> ·Î±×ÀÎ °ü·Ã - ÇØ°á(Login Fail ¸Þ¼¼Áö ÀÌ¿ë)
		// Disconnect(uidComm);
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(AllocUID, MERR_FAILED_LOGIN_RETRY);
		Post(pCmd);	
		return false;
	}

	pObj->AddCommListener(uidComm);
	pObj->SetObjectType(MOT_PC);

	memcpy(pObj->GetAccountInfo(), pSrcAccountInfo, sizeof(MMatchAccountInfo));
	memcpy(pObj->GetAccountPenaltyInfo(), pSrcAccountPenaltyInfo, sizeof(MMatchAccountPenaltyInfo));
		
	pObj->SetFreeLoginIP(bFreeLoginIP);
	pObj->SetCountryCode3( strCountryCode3 );
	pObj->UpdateTickLastPacketRecved();
	pObj->UpdateLastHShieldMsgRecved();

	if (pCommObj != NULL)
	{
		pObj->SetPeerAddr(pCommObj->GetIP(), pCommObj->GetIPString(), pCommObj->GetPort());
	}


	SetClientClockSynchronize(uidComm);

	// ÇÁ¸®¹Ì¾ö IP¸¦ Ã¼Å©ÇÑ´Ù.
	if (MGetServerConfig()->CheckPremiumIP())
	{
		if (pCommObj)
		{
			bool bIsPremiumIP = false;
			bool bExistPremiumIPCache = false;
			
			bExistPremiumIPCache = MPremiumIPCache()->CheckPremiumIP(pCommObj->GetIP(), bIsPremiumIP);

			// ¸¸¾à Ä³½¬¿¡ ¾øÀ¸¸é Á÷Á¢ DB¿¡¼­ Ã£µµ·Ï ÇÑ´Ù.
			if (!bExistPremiumIPCache)
			{
				if (m_MatchDBMgr.CheckPremiumIP(pCommObj->GetIPString(), bIsPremiumIP))
				{
					// °á°ú¸¦ Ä³½¬¿¡ ÀúÀå
					MPremiumIPCache()->AddIP(pCommObj->GetIP(), bIsPremiumIP);
				}
				else
				{
					MPremiumIPCache()->OnDBFailed();
				}

			}

			if (bIsPremiumIP) pObj->GetAccountInfo()->m_nPGrade = MMPG_PREMIUM_IP;
		}		
	}

	if (!PreCheckAddObj(uidComm))
	{
		// º¸¾È °ü·Ã ÃÊ±âÈ­ ¼­¹ö ¼³Á¤¿¡ ¹®Á¦°¡ »ý°å´Ù°í ·Î±×ÀÎ ½ÇÆÐ¸¦ ¸®ÅÏÇÑ´Ù. //
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(uidComm, MERR_FAILED_AUTHENTICATION);
		Post(pCmd);	
		return false;
	}

	pObj->m_nInviteWar = pObj->GetAccountInfo()->m_nInviteWar;

	MCommand* pCmd = CreateCmdMatchResponseLoginOK(uidComm, 
												   AllocUID, 
												   pObj->GetAccountInfo()->m_szUserID,
												   pObj->GetAccountInfo()->m_nUGrade,
                                                   pObj->GetAccountInfo()->m_nPGrade,
		                                           pObj->GetAccountInfo()->m_nCountryFlag,
												   pObj->GetAccountInfo()->m_nCash,
		                                           pObj->GetAccountInfo()->m_nEvent,
//												   pObj->GetAntiHackInfo()->m_szRandomValue,
												   pObj->GetHShieldInfo()->m_pbyGuidReqMsg);	                                           
	                                               Post(pCmd);	

	// Á¢¼Ó ·Î±×¸¦ ³²±ä´Ù.
	//m_MatchDBMgr.InsertConnLog(pObj->GetAccountInfo()->m_nAID, pObj->GetIPString(), pObj->GetCountryCode3() );

	// Á¢¼Ó ·Î±×
	MAsyncDBJob_InsertConnLog* pNewJob = new MAsyncDBJob_InsertConnLog(uidComm);
	pNewJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetIPString(), pObj->GetCountryCode3() );
	PostAsyncJob(pNewJob);

	// Client DataFile ChecksumÀ» °Ë»çÇÑ´Ù.
	// 2006.2.20 dubble. filelist checksumÀ¸·Î º¯°æ
	unsigned long nChecksum = nChecksumPack ^ uidComm.High ^ uidComm.Low;
	if( MGetServerConfig()->IsUseFileCrc() && !MMatchAntiHack::CheckClientFileListCRC(nChecksum, pObj->GetUID()) && 
		!MGetServerConfig()->IsDebugLoginIPList(pObj->GetIPString()) )
	{
		LOG(LOG_PROG, "Invalid filelist crc (%u) , UserID(%s)\n ", nChecksum, pObj->GetAccountInfo()->m_szUserID);
//		pObj->SetBadFileCRCDisconnectWaitInfo();
		pObj->DisconnectHacker( MMHT_BADFILECRC);
	}
	/*
	if (nChecksum != GetItemFileChecksum()) {
		LOG(LOG_PROG, "Invalid ZItemChecksum(%u) , UserID(%s) ", nChecksum, pObj->GetAccountInfo()->m_szUserID);
		Disconnect(uidComm);
		return false;
	}
	*/

	pObj->LoginCompleted();

	return true;
}
