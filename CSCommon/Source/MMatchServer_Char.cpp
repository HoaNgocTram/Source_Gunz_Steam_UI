#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_CharFinalize.h"
#include "MMatchUtil.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchLocale.h"
#include "MMatchObject.h"

#include "MAsyncDBJob_UpdateAccountLastLoginTime.h"
#include "MAsyncDBJob_UpdateCharBRInfo.h"
#include "LadderWars/MAsyncDBJob_LadderWars.h"

void MMatchServer::OnRequestAccountCharInfo(const MUID& uidPlayer, int nCharNum)
{
    // Async DB //////////////////////////////
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MAsyncDBJob_GetAccountCharInfo* pJob=new MAsyncDBJob_GetAccountCharInfo(uidPlayer,pObj->GetAccountInfo()->m_nAID, nCharNum);
	
	pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	// PostAsyncJob(pJob);
}


void MMatchServer::OnRequestSelectChar(const MUID& uidPlayer, const int nCharIndex)
{
	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0)) return;
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return;

	// Async DB //////////////////////////////
	MAsyncDBJob_GetCharInfo* pJob=new MAsyncDBJob_GetCharInfo(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharIndex);
	pJob->SetCharInfo(new MMatchCharInfo);
	pObj->m_DBJobQ.DBJobQ.push_back( pJob );
}


void MMatchServer::OnRequestDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName)
{
	ResponseDeleteChar(uidPlayer, nCharIndex, szCharName);
}

bool MMatchServer::ResponseDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0)) return false;
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return false;

	// 생성조건 통과 - Post AsyncJob
	MAsyncDBJob_DeleteChar* pJob = new MAsyncDBJob_DeleteChar(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharIndex, szCharName);
	pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	// PostAsyncJob(pJob);

	return true;
}


void MMatchServer::OnRequestCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
						 const unsigned int nSex, const unsigned int nHair, const unsigned int nFace, const unsigned int nCostume)
{
	MMatchSex sex = (nSex == 0) ? MMS_MALE : MMS_FEMALE;
	ResponseCreateChar(uidPlayer, nCharIndex, szCharName, MMatchSex(nSex), nHair, nFace, nCostume);
}


bool MMatchServer::ResponseCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
						MMatchSex nSex, const unsigned int nHair, const unsigned int nFace,	const unsigned int nCostume)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if ((pObj->GetAccountInfo()->m_nAID < 0) || (nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT) ||
		(nHair >= MAX_COSTUME_HAIR) || (nCostume >= MAX_COSTUME_TEMPLATE) || (nFace > MAX_COSTUME_FACE))
	{
		int nResult = -1;	// false
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterInt(nResult));			// result
		pNewCmd->AddParameter(new MCommandParameterString(szCharName));		// 만들어진 캐릭터 이름
		RouteToListener(pObj, pNewCmd);
		return false;
	}

	int nResult = -1;	// false

	nResult = ValidateMakingName(szCharName, MIN_CHARNAME, MAX_CHARNAME);
	if (nResult != MOK) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(nResult));			// result
		pNewCmd->AddParameter(new MCommandParameterString(szCharName));		// 만들어진 캐릭터 이름
		RouteToListener(pObj, pNewCmd);

		return false;
	}	

	// 생성조건 통과 - Post AsyncJob
	MAsyncDBJob_CreateChar* pJob = new MAsyncDBJob_CreateChar(uidPlayer, pObj->GetAccountInfo()->m_nAID, 
											szCharName, nCharIndex, nSex, nHair, nFace, nCostume);
	

#ifdef _DEBUG
	// 제가 주석처리했습니다 - bird
	mlog( "Selected character name : %s (", szCharName);

	for ( int i = 0;  i < (int)strlen( szCharName);  i++)
	{
		mlog( "%02X ", szCharName[ i] & 0x00FF);
	}

	mlog( ")  (len = %d)\n", (int)strlen( szCharName));
#endif

	// PostAsyncJob(pJob);

	pObj->m_DBJobQ.DBJobQ.push_back( pJob );

	return true;
}


void MMatchServer::OnCharClear(const MUID& uidPlayer)
{
	//Custom: Clean object
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj)
	{	
		CharFinalize(pObj->GetUID());
		ObjectRemove(pObj->GetUID(), NULL);
	}
}

bool MMatchServer::CharInitialize(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return false;

	pCharInfo->m_nConnTime = GetTickTime();

	m_objectCommandHistory.SetCharacterInfo( uidPlayer, pCharInfo->m_szName, pCharInfo->m_nCID );

	if( !pCharInfo->EquipFromItemList() ) {
		mlog("MMatchServer::CharInitialize - EquipFromItemList Failed(CID - %d)\n", pCharInfo->m_nCID);
		if (!m_MatchDBMgr.ClearAllEquipedItem(pCharInfo->m_nCID)) {
			mlog("MMatchServer::CharInitialize - DB Query(ClearAllEquipedItem) Failed\n");
		}

		pCharInfo->m_EquipedItem.Clear();
	}

	CheckExpiredItems(pObj);
	// Custom: Check invalid items (by sex)
	CheckInvalidItems(pObj);


	// 최대 가질수 있는 무게보다 더 많이 들고 있으면 모든 무기를 해제시킨다.
	int nWeight, nMaxWeight;
	pCharInfo->GetTotalWeight(&nWeight, &nMaxWeight);
	if (nWeight > nMaxWeight) {
		if (!m_MatchDBMgr.ClearAllEquipedItem(pCharInfo->m_nCID)) {
			mlog("MMatchServer::CharInitialize - DB Query(ClearAllEquipedItem) Failed\n");
		}

		pCharInfo->m_EquipedItem.Clear();
	}
	
	// Custom: send friend login notification to all friends.
	MMatchFriendList* pList = &pObj->GetFriendInfo()->m_FriendList;
	for (auto itor = GetObjects()->begin(); itor != GetObjects()->end(); ++itor)
	{
		MMatchObject* playerObj = (*itor).second;
		if (!IsEnabledObject(playerObj)) continue;
		if (!playerObj->GetFriendInfo()) continue;

		if (playerObj->GetFriendInfo()->Find(pObj->GetCharInfo()->m_szName) != NULL)
		{
			char szMsg[255];
			sprintf(szMsg, "Friend '%s' has logged in.", pObj->GetCharInfo()->m_szName);
			Announce(playerObj, szMsg);
		}
	}
	
	if (pCharInfo->m_ClanInfo.IsJoined())
	{
		// Custom: route to everyone in the clan who is not a friend.
		MCommand* pNew = CreateCommand(MC_MATCH_CLAN_MEMBER_CONNECTED, MUID(0,0));
		pNew->AddParameter(new MCommandParameterString((char*)pCharInfo->m_szName));
		pNew->AddParameter(new MCommandParameterInt(pCharInfo->m_ClanInfo.m_nGrade));
		RouteToClanNotFriend(pCharInfo->m_ClanInfo.m_nClanID, uidPlayer, pNew);

		m_ClanMap.AddObject(uidPlayer, pObj);

		// 임시코드... 잘못된 클랜ID 온다면 체크하여 잡기위함...20090224 by kammir
		if(pObj->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
            LOG(LOG_FILE, "[CharInitialize()] %s's ClanID:%d.", pObj->GetAccountName(), pObj->GetCharInfo()->m_ClanInfo.GetClanID());

		// 만약 클랜전 서버일 경우 클랜전 정보도 받아온다. - 엠블렘때문에 그냥 서버도 클랜정보 읽어옴
//		if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
		{
			MMatchClan* pClan = m_ClanMap.GetClan(pCharInfo->m_ClanInfo.m_nClanID);
			if (pClan)
			{
				if (!pClan->IsInitedClanInfoEx())
				{
					pClan->InitClanInfoFromDB();
				}
			}
		}
	}

	return true;
}

void MMatchServer::CheckExpiredItems(MMatchObject* pObj)
{
	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo )
		return;

	if (!pCharInfo->m_ItemList.HasRentItem()) return;

	vector<unsigned long int> vecExpiredItemIDList;
	vector<MUID> vecExpiredItemUIDList;				// 만료된 아이템 UID
	const DWORD dwTick = GetTickTime();

	// 기간 만료 아이템이 있는지 체크하고 있으면 아이템 해제하고 통지한다.
	for (MMatchItemMap::iterator itor = pCharInfo->m_ItemList.begin(); itor != pCharInfo->m_ItemList.end(); ++itor)
	{
		MMatchItem* pCheckItem = (*itor).second;
		if (pCheckItem->IsRentItem())
		{
			// 인스턴스 생성되고나서 지난 시간
			/// unsigned long int nPassTime = MGetTimeDistance(pCheckItem->GetRentItemRegTime(), dwTick);
			// int nPassMinuteTime = nPassTime / (1000 * 60);

			// if ((pCheckItem->GetRentMinutePeriodRemainder()-nPassMinuteTime) <= 0) // 함수 호출로 수정. - by SungE
			if( IsExpiredRentItem(pCheckItem, dwTick) )
			{
				// 장비중이면 벗긴다.
				MMatchCharItemParts nCheckParts = MMCIP_END;
				// if (pCharInfo->m_EquipedItem.IsEquipedItem(pCheckItem, nCheckParts))
				if (pCharInfo->m_EquipedItem.IsEquipedItem(itor->first, nCheckParts))
				{
					ResponseTakeoffItem(pObj->GetUID(), nCheckParts);
				}

				// 아이템 제거
				int nExpiredItemID = pCheckItem->GetDescID();
				if (nExpiredItemID != 0)
				{
					vecExpiredItemUIDList.push_back(pCheckItem->GetUID());
					vecExpiredItemIDList.push_back(nExpiredItemID);
				}
			}
		}
	}

	// 만료된 아이템이 있으면 통지
	if (!vecExpiredItemIDList.empty())
	{
		int nExpiredItemUIDListCount = (int)vecExpiredItemUIDList.size();
		for (int i = 0; i < nExpiredItemUIDListCount; i++)
		{
			// 실제로 여기서 아이템을 지운다.
			RemoveExpiredCharItem(pObj, vecExpiredItemUIDList[i]);
		}

		ResponseExpiredItemIDList(pObj, vecExpiredItemIDList);
	}
}

void MMatchServer::ResponseExpiredItemIDList(MMatchObject* pObj, vector<unsigned long int>& vecExpiredItemIDList)
{
	int nBlobSize = (int)vecExpiredItemIDList.size();
	MCommand* pNewCmd = CreateCommand(MC_MATCH_EXPIRED_RENT_ITEM, MUID(0,0));
	
	void* pExpiredItemIDArray = MMakeBlobArray(sizeof(unsigned long int), nBlobSize);

	for (int i = 0; i < nBlobSize; i++)
	{
		unsigned long int *pItemID = (unsigned long int*)MGetBlobArrayElement(pExpiredItemIDArray, i);
		*pItemID = vecExpiredItemIDList[i];
	}
	pNewCmd->AddParameter(new MCommandParameterBlob(pExpiredItemIDArray, MGetBlobArraySize(pExpiredItemIDArray)));
	MEraseBlobArray(pExpiredItemIDArray);
	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::CheckInvalidItems(MMatchObject* pPlayer)	
{
	MMatchCharInfo*	pCharInfo = pPlayer->GetCharInfo();
	if( NULL == pCharInfo )
		return;

	for (int nPart = 0; nPart < MMCIP_END; ++nPart)
	{
		if (pPlayer->GetCharInfo()->m_EquipedItem.IsEmpty((MMatchCharItemParts)nPart) == false)
		{
			MMatchItem* pItem = pPlayer->GetCharInfo()->m_EquipedItem.GetItem((MMatchCharItemParts)nPart);

			if (pItem->GetDesc())
			{
				if (pItem->GetDesc()->m_nResSex.Ref() != -1 && pItem->GetDesc()->m_nResSex.Ref() != (int)pCharInfo->m_nSex)
				{
					ResponseTakeoffItem(pPlayer->GetUID(), (MMatchCharItemParts)nPart);
				}
			}
		}
	}
}

// 수정되면 true
bool MMatchServer::CorrectEquipmentByLevel(MMatchObject* pPlayer, MMatchCharItemParts nPart, int nLegalItemLevelDiff)	
{
	if (!IsEnabledObject(pPlayer)) return false;

	if (pPlayer->GetCharInfo()->m_EquipedItem.IsEmpty(nPart) == false) {
		MMatchItem* pItem = pPlayer->GetCharInfo()->m_EquipedItem.GetItem(nPart);
		if (pItem->GetDesc())
		{
			if (pItem->GetDesc()->m_nResLevel.Ref() > (pPlayer->GetCharInfo()->m_nLevel+nLegalItemLevelDiff)) {
				ResponseTakeoffItem(pPlayer->GetUID(), nPart);
				return true;
			}
		}
	}
	return false;
}

bool MMatchServer::CharFinalize(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if (MGetServerConfig()->GetServerMode() == MSM_EVENT) 
	{	///< EVENT때 레벨제한 없이 장착한 아이템을 벗긴다
		CorrectEquipmentByLevel(pObj, MMCIP_MELEE);
		CorrectEquipmentByLevel(pObj, MMCIP_PRIMARY);
		CorrectEquipmentByLevel(pObj, MMCIP_SECONDARY);
		CorrectEquipmentByLevel(pObj, MMCIP_CUSTOM1);
		CorrectEquipmentByLevel(pObj, MMCIP_CUSTOM2);
		CorrectEquipmentByLevel(pObj, MMCIP_CUSTOM3);
		CorrectEquipmentByLevel(pObj, MMCIP_COMMUNITY1);
		CorrectEquipmentByLevel(pObj, MMCIP_COMMUNITY2);
		CorrectEquipmentByLevel(pObj, MMCIP_LONGBUFF1);
		CorrectEquipmentByLevel(pObj, MMCIP_LONGBUFF2);
	}

	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
	UpdateCharItemDBCachingData(pObj);	///< Character Item에서 업데이트가 필요한 것들 업데이트

	DWORD nAID = 0;
	if( NULL != pObj->GetAccountInfo() )
		nAID = pObj->GetAccountInfo()->m_nAID;

	MAsyncDBJob_UpdateAccountLastLoginTime* pUpdateAccountLastLoginTimeJob = new MAsyncDBJob_UpdateAccountLastLoginTime(uidPlayer);
	if( NULL != pUpdateAccountLastLoginTimeJob )
	{
		pUpdateAccountLastLoginTimeJob->Input( pObj->GetAccountInfo()->m_nAID );
		// PostAsyncJob( pUpdateAccountLastLoginTimeJob );
		pObj->m_DBJobQ.DBJobQ.push_back( pUpdateAccountLastLoginTimeJob );
	}

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return false;

	// 클랜에 가입되어 있으면 MMatchMap에서 삭제
	if (pCharInfo->m_ClanInfo.IsJoined())
	{
		m_ClanMap.RemoveObject(uidPlayer, pObj);
	}

	// 캐릭끝날때 로그 남긴다
	if (pCharInfo->m_nCID != 0)
	{
		unsigned long int nPlayTime = 0;		// 1 = 1초
		unsigned long int nNowTime = GetTickTime();

		if (pCharInfo->m_nConnTime != 0)
		{
			//Custom: Clean object
			nPlayTime = static_cast<u32>(MGetTimeDistance(pCharInfo->m_nConnTime, nNowTime) / 1000);
		}

		MAsyncDBJob_CharFinalize* pJob = new MAsyncDBJob_CharFinalize(uidPlayer);
		pJob->Input(nAID,
			pCharInfo->m_nCID, 
			nPlayTime, 
			pCharInfo->m_nConnKillCount, 
			pCharInfo->m_nConnDeathCount,
			pCharInfo->m_nConnXP, 
			pCharInfo->m_nXP,
			pCharInfo->m_QuestItemList,
			pCharInfo->m_QMonsterBible,
			pCharInfo->m_DBQuestCachingData.IsRequestUpdateWhenLogout() );
		// PostAsyncJob(pJob);

		pObj->m_DBJobQ.DBJobQ.push_back( pJob );

		pCharInfo->m_DBQuestCachingData.Reset();
	}

	
	for(MMatchCharBattleTimeRewardInfoMap::iterator iter = pCharInfo->GetBRInfoMap().begin();
		iter != pCharInfo->GetBRInfoMap().end(); iter++)
	{
		MMatchCharBRInfo* pInfo = iter->second;
		OnAsyncRequest_UpdateCharBRInfo(uidPlayer, pInfo->GetBRID(), pInfo->GetBRTID(), pInfo->GetRewardCount(), pInfo->GetBattleTime(), pInfo->GetKillCount());
	}
		


	return true;
}

void MMatchServer::OnRequestMySimpleCharInfo(const MUID& uidPlayer)
{
	ResponseMySimpleCharInfo(uidPlayer);
}

void MMatchServer::ResponseMySimpleCharInfo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO, MUID(0,0));

	void* pMyCharInfoArray = MMakeBlobArray(sizeof(MTD_MySimpleCharInfo), 1);
	MTD_MySimpleCharInfo* pMyCharInfo = (MTD_MySimpleCharInfo*)MGetBlobArrayElement(pMyCharInfoArray, 0);

	pMyCharInfo->nXP = pCharInfo->m_nXP;
	pMyCharInfo->nBP = pCharInfo->m_nBP;
	pMyCharInfo->nLC = pCharInfo->m_nLC;
	pMyCharInfo->nLevel = pCharInfo->m_nLevel;

	pNewCmd->AddParameter(new MCommandParameterBlob(pMyCharInfoArray, MGetBlobArraySize(pMyCharInfoArray)));
	MEraseBlobArray(pMyCharInfoArray);

	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::OnRequestCopyToTestServer(const MUID& uidPlayer)
{
#ifndef _DEBUG
	return;
#endif

	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;


	int nResult = MERR_UNKNOWN;

	ResponseCopyToTestServer(uidPlayer, nResult);
}

void MMatchServer::ResponseCopyToTestServer(const MUID& uidPlayer, const int nResult)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_COPY_TO_TESTSERVER, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(nResult));		// result
	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::OnFriendAdd(const MUID& uidPlayer, const char* pszName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	if (pObj->GetFriendInfo() == NULL) return;

	// Custom: Prevent users from adding themselves
	if (!stricmp(pObj->GetName(), pszName))
		return;

	MMatchObject* pTargetObj = GetPlayerByName(pszName);
	if (!IsEnabledObject(pTargetObj)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	const int nTargetCID = pTargetObj->GetCharInfo()->m_nCID;

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchFriendNode* pNode = pObj->GetFriendInfo()->Find(pszName);
	if (pNode) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_ALREADY_EXIST);
		return;
	}
	if (pObj->GetFriendInfo()->m_FriendList.size() > MAX_FRIEND_COUNT) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_TOO_MANY_ADDED);
		return;
	}

	int nCID = pObj->GetCharInfo()->m_nCID;
	int nFriendCID = 0;
	if (m_MatchDBMgr.GetCharCID(pszName, &nFriendCID) == false) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	// 게임 도중에 웹을 통해서 이름을 바꾸면 다를 수 있다.
	if( nTargetCID != nFriendCID )
	{
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (m_MatchDBMgr.FriendAdd(nCID, nFriendCID, 0) == false) {
		mlog("DB Query(FriendAdd) Failed\n");
		return;
	}

	// Custom: Don't use pszName since MLex (in Gunz) always forces it to lowercase.
	pObj->GetFriendInfo()->Add(nFriendCID, 0, pTargetObj->GetName());
	NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_ADD_SUCCEED);
}

void MMatchServer::OnFriendRemove(const MUID& uidPlayer, const char* pszName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;
	if (pObj->GetFriendInfo() == NULL) return;

	MMatchFriendNode* pNode = pObj->GetFriendInfo()->Find(pszName);
	if (pNode == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_NOT_EXIST);
		return;
	}

	const int nNodeCID = pNode->nFriendCID;

	int nCID = pObj->GetCharInfo()->m_nCID;
	int nFriendCID = 0;
	if (m_MatchDBMgr.GetCharCID(pszName, &nFriendCID) == false) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if( nNodeCID != nFriendCID )
	{
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (m_MatchDBMgr.FriendRemove(nCID, nFriendCID) == false) {
		mlog("DB Query(FriendRemove) Failed\n");
		return;
	}

	pObj->GetFriendInfo()->Remove(pszName);
	NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_REMOVE_SUCCEED);
}

void MMatchServer::OnFriendList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	// ASync DB
	if (!pObj->DBFriendListRequested())
	{
		MAsyncDBJob_FriendList* pJob=new MAsyncDBJob_FriendList(uidPlayer, pObj->GetCharInfo()->m_nCID);
		pJob->SetFriendInfo(new MMatchFriendInfo);
		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
		// PostAsyncJob(pJob);
	}
	else if (!pObj->GetFriendInfo())
	{
		// 아직 DB에서 FriendList를 안받아왔으면 그냥 리턴
		return;
	}

	FriendList(uidPlayer);
}

void MMatchServer::FriendList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;
	if (pObj->GetFriendInfo() == NULL) return;

	// Update Friends Status and Description
	pObj->GetFriendInfo()->UpdateDesc();

	MMatchFriendList* pList = &pObj->GetFriendInfo()->m_FriendList;

	void* pListArray = MMakeBlobArray(sizeof(MFRIENDLISTNODE), (int)pList->size());
	int nIndex=0;
	for (MMatchFriendList::iterator i=pList->begin(); i!=pList->end(); i++) {
		MMatchFriendNode* pNode = (*i);
		MFRIENDLISTNODE* pListNode = (MFRIENDLISTNODE*)MGetBlobArrayElement(pListArray, nIndex++);
		pListNode->nState = pNode->nState;
		strcpy(pListNode->szName, pNode->szName);
		strcpy(pListNode->szDescription, pNode->szDescription);
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_FRIENDLIST, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterBlob(pListArray, MGetBlobArraySize(pListArray)));
	MEraseBlobArray(pListArray);
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnFriendMsg(const MUID& uidPlayer, const char* szMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

}

void MMatchServer::OnRequestCharInfoDetail(const MUID& uidChar, const char* szCharName)
{
	ResponseCharInfoDetail(uidChar, szCharName);
}
time_t Hour = time(0);
tm* HourLocal = localtime(&Hour);
#ifdef _HWID
void AutoBanLogs(char* szLogs) {
	FILE* pFile = fopen("AutoBanAccount.txt", "a+");
	fprintf(pFile, "%s", szLogs);
	fclose(pFile);
}
void MMatchServer::OnMatchBanMe(const MUID& uidComm, WarpHackType nType, void* pBlobArray)
{
	MMatchObject* pObj = GetObject(uidComm);
	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0) || (int)nType <= WHT_NONE || nType >= WHT_END)
		return;

	if (pObj->GetAccountInfo()->m_HackingType == MMHT_GAMEGUARD_HACKER)
	{
		return;
	}

	char szType[128];
	bool AutoBanAccount = false;
	char szReport[128];
	char szDate[64] = { 0 };
	char szTime[64] = { 0 };
	sprintf_s(szType, "Unspecified");

	if (nType == WHT_MOUSE)
	{
		sprintf_s(szType, "MouseHack");
		AutoBanAccount = false;
	}
	else if (nType == WHT_KEYBOARD)
	{
		sprintf_s(szType, "KeyHack");
		AutoBanAccount = false;
	}
	else if (nType == WHT_KEYMOUSE64)
	{
		sprintf_s(szType, "KeyMouseHack64");
		AutoBanAccount = false;
	}
	else if (nType == WHT_INTEGRITYTHREAD)
	{
		sprintf_s(szType, "IntegrityDisabled");
		AutoBanAccount = true;
	}
	else if (nType == WHT_MACRO)
	{
		sprintf_s(szType, "Macro");
		AutoBanAccount = false;
	}
	else if (nType == WHT_RAWINPUTHOOK)
	{
		sprintf_s(szType, "RawInput");
		AutoBanAccount = true;
	}
	else if (nType == WHT_TIME_GETTICKCOUNT)
	{
		sprintf_s(szType, "TIME_GETTICKCOUNT");
		AutoBanAccount = true;
	}
	else if (nType == WHT_TIME_TIMEGETTIME)
	{
		sprintf_s(szType, "TIME_TIMEGETTIME");
		AutoBanAccount = true;
	}
	else if (nType == WHT_TIME_QPC)
	{
		sprintf_s(szType, "WHT_TIME_QPC");
		AutoBanAccount = true;
	}
	else if (nType == WHT_D3D_ENDSCENE)
	{
		sprintf_s(szType, "D3D_ENDSCENE");
		AutoBanAccount = true;
	}
	else if (nType == WHT_D3D_DRAWINDEXEDPRIMITIVE)
	{
		sprintf_s(szType, "D3D_DRAWINDEXEDPRIMITIVE");
		AutoBanAccount = true;
	}
	else if (nType == WHT_D3D_RESET)
	{
		sprintf_s(szType, "D3D_RESET");
		AutoBanAccount = true;
	}
	else if (nType == WHT_PICKLESDASKID)
	{
		sprintf_s(szType, "Pickles Dask ID");
		AutoBanAccount = true;
	}
	else if (nType == WHT_MAIETCRC)
	{
		sprintf_s(szType, "Maiet CRC");
		AutoBanAccount = true;
	}
	else if (nType == WHT_MEMCMP)
	{
		sprintf_s(szType, "Memory MP");
		AutoBanAccount = true;
	}
	else if (nType == WHT_CQHACK)
	{
		sprintf_s(szType, "Summon Hack");
		AutoBanAccount = true;
	}
	if (nType == WHT_MEMORY) {
		sprintf_s(szType, "Memory Edited");
		AutoBanAccount = true;
	}
	else if (nType == WHT_UNDERCLOCK)
	{
		sprintf_s(szType, "Underclock");
		AutoBanAccount = true;
	}
	else if (nType == WHT_HOOKS)
	{
		sprintf_s(szType, "Underclock");
		AutoBanAccount = true;
	}
	else if (nType == WHT_CHEATENGINE)
	{
		sprintf_s(szType, "CheatEngine");
		AutoBanAccount = false;
	}
	else if (nType == WHT_PROGRAMS)
	{
		sprintf_s(szType, "Programs");
		AutoBanAccount = true;
	}
	else if (nType == WHT_HITBOX)
	{
		sprintf_s(szType, "Hit Box");
		AutoBanAccount = true;
	}
	else if (nType == WHT_BULLET)
	{
		sprintf_s(szType, "Bullet Count");
		AutoBanAccount = true;
	}
	else if (nType == WHT_RANGE)
	{
		sprintf_s(szType, "Shotgun Range");
		AutoBanAccount = true;
	}
	else if (nType == WHT_HPAP)
	{
		sprintf_s(szType, "HP/AP Mod");
		AutoBanAccount = false;
	}
	else if (nType == WHT_MOUSEMACRO)
	{
		sprintf_s(szType, "Mouse Macro");
		AutoBanAccount = false;
	}
	else if (nType == WHT_INJECT)
	{
		sprintf_s(szType, "Inject DLL");
		AutoBanAccount = true;
	}
	else if (nType == WHT_SPAWN)
	{
		sprintf_s(szType, "Spawnmover");

		AutoBanAccount = true;
	}
	else if (nType == WHT_ESP)
	{
		sprintf_s(szType, "Name ESP");
		AutoBanAccount = false;
	}
	else if (nType == WHT_SUDASH)
	{
		sprintf_s(szType, "Super Dash");
		AutoBanAccount = false;
	}
	else if (nType == WHT_DIEPEER)
	{
		sprintf_s(szType, "OnPeerDead");
		AutoBanAccount = false;
	}
	else if (nType == WHT_RELOADFAST)
	{
		sprintf_s(szType, "Reload Fast");
		AutoBanAccount = false;
	}

	strftime(szDate, 64, "%d/%m/%Y", HourLocal);
	strftime(szTime, 64, "%H:%M %p", HourLocal);

	if (!m_MatchDBMgr.BanHwid(pObj->GetAccountInfo()->m_nAID, pObj->GetAccountInfo()->m_szHWID, nType))
	{
		LOG(LOG_PROG, "[Anti-Cheat] Fail : AID(%u) Reason(%s)", pObj->GetAccountInfo()->m_nAID, szType);
	}
	else
	{
		LOG(LOG_PROG, "[Anti-Cheat] Banned : AID(%u) Reason(%s) [ %s / %s]", pObj->GetAccountInfo()->m_nAID, szType, szDate, szTime);

		if (nType == 2)
		{
			int count = MGetBlobArrayCount(pBlobArray);
			if (MGetBlobArraySize(pBlobArray) == (8 + (count * sizeof(MTD_KeyParams))) && count <= 5)
			{
				for (int i = 0; i < count; i++)
				{
					MTD_KeyParams* kp = (MTD_KeyParams*)MGetBlobArrayElement(pBlobArray, i);
					LOG(LOG_FILE, "vkCode: 0x%02X, scanCode: 0x%02X, flags: 0x%08X, time=%d, dwExtraInfo=0x%08X", kp->vkCode, kp->scanCode, kp->flags, kp->time, kp->extraInfo);
				}
			}
			else
			{
				LOG(LOG_FILE, "Failed blob check for KeyParams");
			}
		}
	}
	if (strlen(pObj->GetCharInfo()->m_szName) > 0)
	{
		sprintf(szReport, "[Easy Anti-Cheat] : AID(%u) Reason(%s) Char(%s) [ %s / %s]", pObj->GetAccountInfo()->m_nAID, szType, pObj->GetCharInfo()->m_szName, szDate, szTime);
		if (AutoBanAccount)
		{
			strcat(szReport, " - User automatically banned.");
		}

		Announce(uidComm, szReport);
		strcat(szReport, "\n");
		AutoBanLogs(szReport);
		if (AutoBanAccount)
		{
			BanAccount(MUID(0, 0), pObj->GetCharInfo()->m_szName, true);
		}
	}
	pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);
}
#endif
void MMatchServer::ResponseCharInfoDetail(const MUID& uidChar, const char* szCharName)
{
	MMatchObject* pObject = GetObject(uidChar);
	if (! IsEnabledObject(pObject)) return;

	MMatchObject* pTarObject = GetPlayerByName(szCharName);
	if (! IsEnabledObject(pTarObject))
	{
		RouteResponseToListener(pObject, MC_MATCH_RESPONSE_RESULT, MERR_NO_TARGET);
		return;
	}


	// Client에 선택한 캐릭터 정보 전송
	MTD_CharInfo_Detail trans_charinfo_detail;
	CopyCharInfoDetailForTrans(&trans_charinfo_detail, pTarObject->GetCharInfo(), pTarObject);
	

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CHARINFO_DETAIL, MUID(0,0));

	void* pCharInfoArray = MMakeBlobArray(sizeof(MTD_CharInfo_Detail), 1);
	MTD_CharInfo_Detail* pTransCharInfoDetail = (MTD_CharInfo_Detail*)MGetBlobArrayElement(pCharInfoArray, 0);
	memcpy(pTransCharInfoDetail, &trans_charinfo_detail, sizeof(MTD_CharInfo_Detail));
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharInfoArray, MGetBlobArraySize(pCharInfoArray)));
	MEraseBlobArray(pCharInfoArray);

	RouteToListener(pObject, pNewCmd);
}
