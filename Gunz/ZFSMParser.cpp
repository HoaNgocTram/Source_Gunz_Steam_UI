#include "stdafx.h"
#include "ZFSMParser.h"
#include "ZFSMTransition.h"
#include "ZFSMFunctionServer.h"
#include "ZFSMState.h"
#include "ZFSM.h"
#include "ZActorActionManager.h"
#include "ZActorAction.h"

ZFSMParser::ZFSMParser()
{
	m_pActionMgr = NULL;
}

void ZFSMParser::ParseMethodStatement(ZFSMMethodCall* pMc, const char* szStatement)
{
	if (!pMc) { _ASSERT(0); return; }
	pMc->strMethodName = "";

	for (int k=0; k<MAX_FSMMETHOD_ARG; ++k)
		pMc->nArg[k] = FSMMETHOD_ARG_NONE;
	
	enum { READ_METHOD, READ_ARG };

	int nReadMode = READ_METHOD;
	int iArg = 0;

	char szArg[32] = "";

	const char* szBegin = szStatement;
	const char* sz = szStatement;
	
	while (1)
	{
		// 메소드를 읽는 과정에서 ':'가 나오면 메소드 이름을 저장
		if (sz[0]==':' && nReadMode==READ_METHOD)
		{
			pMc->strMethodName.assign(szBegin, sz);
			++sz;	// ':'는 건너뜀
			szBegin = sz;
			nReadMode = READ_ARG;
		}

		// 인자를 읽는 과정에서 ';'가 나오면 인자를 저장
		else if (sz[0]==';' && nReadMode==READ_ARG)
		{
			if (iArg >= MAX_FSMMETHOD_ARG)
			{
				_ASSERT(0);
				return;
			}

			strncpy(szArg, szBegin, sz - szBegin);
			pMc->nArg[iArg] = atoi(szArg);
			++iArg;
			++sz;
			szBegin = sz;
		}

		// 문자열 끝이 나오면 현재 과정에 따라 처리
		else if (sz[0]==0)
		{
			switch (nReadMode)
			{
				// 이 경우는 메소드에 인자가 없이 끝난 경우
			case READ_METHOD:
				pMc->strMethodName.assign(szStatement);
				return;
				// 인자 나열이 끝난 경우
			case READ_ARG:
				strncpy(szArg, szBegin, sz - szBegin);
				pMc->nArg[iArg] = atoi(szArg);
				return;
			}
		}

		// 메소드 이름 읽는 도중 공백문자를 만난 경우 (메소드 이름 앞의 공백문자만 제대로 처리됨)
		else if (sz[0]=='\t' || sz[0]==' ')
		{
			if (nReadMode == READ_METHOD)
				++szBegin;
		}

		++sz;
	}
}

void ZFSMParser::DevideMultipleMethodStatement(vector<string>& out, const char* szStatement)
{
	out.clear();
	if (!szStatement) return;

	char szSingleStatement[128] = "";
	ZFSMMethodCall mc;

	const char* szBegin = szStatement;
	const char* sz = szStatement;

	while (1)
	{
		if (sz[0]==',')
		{
			strncpy(szSingleStatement, szBegin, sz - szBegin);
			out.push_back(szSingleStatement);

			++sz;	// ','를 건너뜀
			szBegin = sz;
		}

		else if (sz[0]==0)
		{
			int len = sz - szBegin;
			strncpy(szSingleStatement, szBegin, len);
			szSingleStatement[len] = 0;
			out.push_back(szSingleStatement);

		}

		if (sz[0]==0)
			return;

		++sz;
	}
}

bool ZFSMParser::ParseFunction(ZFSMFunctionCall* out_pFuncCall, const char* sz)
{
	if (!out_pFuncCall) return false;
	if (!sz) return false;

	ZFSMMethodCall mc;
	ParseMethodStatement(&mc, sz);

	int numArg = 0;
	for (int i=0; i<MAX_FSMMETHOD_ARG; ++i)
		if (mc.nArg[i] != FSMMETHOD_ARG_NONE) ++numArg;

	memset(out_pFuncCall, 0, sizeof(ZFSMFunctionCall));

	int funcId = ZFSMFunctionServer::FSMFUNC_INVALID;

	const char* szFuncName = mc.strMethodName.c_str();
		 if (!stricmp(szFuncName, ""))					{ funcId = ZFSMFunctionServer::FSMFUNC_DONOTHING;			if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "dice"))				{ funcId = ZFSMFunctionServer::FSMFUNC_DICE;				if(numArg!=0) return false; }
	
	else if (!stricmp(szFuncName, "findTarget"))		{ funcId = ZFSMFunctionServer::FSMFUNC_FIND_TARGET;			if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "setTargetLastAttacker"))		{ funcId = ZFSMFunctionServer::FSMFUNC_SET_TARGET_LAST_ATTACKER;	if(numArg!=0) return false; }

	else if (!stricmp(szFuncName, "rotateToTarget"))	{ funcId = ZFSMFunctionServer::FSMFUNC_ROTATE_TO_TARGET;	if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "faceToTarget"))		{ funcId = ZFSMFunctionServer::FSMFUNC_FACE_TO_TARGET;		if(numArg!=0) return false; }
	
	else if (!stricmp(szFuncName, "reduceGroggy"))		{ funcId = ZFSMFunctionServer::FSMFUNC_REDUCE_GROGGY;		if(numArg!=1) return false; }
	
	else if (!stricmp(szFuncName, "buildWaypointsToTarget"))	{ funcId = ZFSMFunctionServer::FSMFUNC_BUILD_WAYPOINTS_TO_TARGET;	if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "runWaypoints"))		{ funcId = ZFSMFunctionServer::FSMFUNC_RUN_WAYPOINTS;		if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "findTargetInHeight")) {funcId = ZFSMFunctionServer::FSMFUNC_FIND_TARGET_IN_HEIGHT; if(numArg!=1) return false; }
	else if (!stricmp(szFuncName, "runAlongTargetOrbital")) {funcId = ZFSMFunctionServer::FSMFUNC_RUN_ALONG_TARGET_ORBITAL; if(numArg!=1) return false; }
	else if (!stricmp(szFuncName, "speedAccel")) {funcId = ZFSMFunctionServer::FSMFUNC_SPEED_ACCEL; if(numArg!=1) return false; }
	else if (!stricmp(szFuncName, "turnOrbitalDirection")) {funcId = ZFSMFunctionServer::FSMFUNC_TURN_ORBITAL_DIRECTION; if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "clearWaypoints")) {funcId = ZFSMFunctionServer::FSMFUNC_CLEAR_WAYPOINTS; if(numArg!=0) return false; }
	else if (!stricmp(szFuncName, "runWaypointsAlongRoute")) { funcId = ZFSMFunctionServer::FSMFUNC_RUN_WAYPOINTS_ALONG_ROUTE; if (numArg != 0) return false; }
	else if (!stricmp(szFuncName, "faceToLastestAttacker")) { funcId = ZFSMFunctionServer::FSMFUNC_FACE_TO_LATEST_ATTACKER; if (numArg != 0) return false; }
	else if (!stricmp(szFuncName, "findTargetInDist")) { funcId = ZFSMFunctionServer::FSMFUNC_FIND_TARGET_IN_DIST; if (numArg != 1) return false; }
	if (funcId == ZFSMFunctionServer::FSMFUNC_INVALID)
	{
		mlog("Unknown FSM function name : \'%s\'\n", szFuncName);
		return false;
	}

	out_pFuncCall->id = funcId;
	for (int i=0; i<MAX_FSMMETHOD_ARG; ++i)
		out_pFuncCall->nArg[i] = mc.nArg[i];

	return true;
}

bool ZFSMParser::ParseFunctionsStatement(vector<ZFSMFunctionCall>& out, const char* szStatement)
{
	vector<string> v;
	DevideMultipleMethodStatement(v, szStatement);

	int n = (int)v.size();
	for (int i=0; i<n; ++i)
	{
		ZFSMFunctionCall funcCall;
		if (false == ParseFunction(&funcCall, v[i].c_str()))
		{
			// 중간에 뭔가 잘못되면 지금까지 읽은 걸 다 삭제한다
			out.clear();
			return false;
		}
		out.push_back(funcCall);
	}

	return true;
}

bool ZFSMParser::ParseCondition( ZFSMCondition* out_pCond, const char* sz )
{
	if (!out_pCond) return false;

	ZFSMMethodCall mc;
	ParseMethodStatement(&mc, sz);

	int numArg = 0;
	for (int i=0; i<MAX_FSMMETHOD_ARG; ++i)
		if (mc.nArg[i] != FSMMETHOD_ARG_NONE) ++numArg;

	E_FSMCOND_ID condId = FSMCOND_INVALID;

	const char* szCondName = mc.strMethodName.c_str();
		 if (!stricmp(szCondName, "default"))					{ condId = FSMCOND_DEFAULT;					if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "Dice"))						{ condId = FSMCOND_DICE;					if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "hpEqual"))					{ condId = FSMCOND_HP_EQUAL;				if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "HpLess"))					{ condId = FSMCOND_HP_LESS;					if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "HpGreater"))					{ condId = FSMCOND_HP_GREATER;				if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "GroggyGreater"))				{ condId = FSMCOND_GROGGY_GREATER;			if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "VelocityZLess"))				{ condId = FSMCOND_VELOCITY_Z_LESS;			if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "VelocityMagnitudeLess"))		{ condId = FSMCOND_VELOCITY_MAGNITUDE_LESS;	if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "EndAction"))					{ condId = FSMCOND_END_ACTION;				if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "IsLanding"))					{ condId = FSMCOND_IS_LANDING;				if(numArg!=0) return false; }
	
	else if (!stricmp(szCondName, "HasTarget"))					{ condId = FSMCOND_HAS_TARGET;				if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "HasNoTarget"))				{ condId = FSMCOND_HAS_NO_TARGET;			if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "DistTarget"))				{ condId = FSMCOND_DIST_TARGET_IN;			if(numArg!=2) return false; }
	else if (!stricmp(szCondName, "LookAtTarget"))				{ condId = FSMCOND_LOOK_AT_TARGET;			if(numArg!=1) return false; }

	else if (!stricmp(szCondName, "IsEmptySpace"))				{ condId = FSMCOND_IS_EMPTY_SPACE;			if(numArg!=2) return false; }
	
	else if (!stricmp(szCondName, "TimeElapsedSinceEntered"))	{ condId = FSMCOND_TIME_ELAPSED_SINCE_ENTERED;	if(numArg!=1) return false; }
	else if (!stricmp(szCondName, "EndLightningDamage"))		{ condId = FSMCOND_END_LIGHTNINGDAMAGE;		if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "CanSeeTarget"))		{ condId = FSMCOND_CAN_SEE_TARGET;		if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "AngleTargetHeight"))		{ condId = FSMCOND_ANGLE_TARGET_HEIGHT;		if(numArg!=2) return false; }
	else if (!stricmp(szCondName, "FailedBuildWaypoints"))		{ condId = FSMCOND_FAILED_BUILD_WAYPOINTS;		if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "CannotSeeTarget"))		{ condId = FSMCOND_CANNOT_SEE_TARGET;		if(numArg!=0) return false; }
	else if (!stricmp(szCondName, "TargetHeightHigher"))	{ condId = FSMCOND_TARGET_HEIGHT_HIGHER; if(numArg!=1) return false;}

	else if (!_stricmp(szCondName, "SummonLess"))	{ condId = FSMCOND_SUMMONLESS; if(numArg!=1) return false;}
	//todok distTarget:x;y 를 파싱할 때 y가 x보다 큰 경우 경고를 내야 할까?

	if (condId == FSMCOND_INVALID)
	{
		mlog("Unknown FSM condition name : \'%s\'\n", szCondName);
		return false;
	}

	out_pCond->Init(condId, mc.nArg[0], mc.nArg[1], mc.nArg[2], mc.nArg[3]);
	return true;
}

ZFSMTransition* ZFSMParser::ParseTransition(const char* szCondition, const char* szNextState)
{
	if (!szCondition || szCondition[0]==0) return NULL;
	if (!szNextState || szNextState[0]==0) return NULL;

	vector<string> v;
	DevideMultipleMethodStatement(v, szCondition);
	
	if (v.empty()) return NULL;

	ZFSMTransition* pTrans = new ZFSMTransition;
	int n = (int)v.size();
	for (int i=0; i<n; ++i)
	{
		ZFSMCondition* pCond = new ZFSMCondition;
		if (ParseCondition(pCond, v[i].c_str()))
			pTrans->AddCondition(pCond);
		else
		{
			delete pCond;
			delete pTrans;	// 뭔가 잘못되면 전부 삭제하고 중지
			return NULL;
		}
	}

	pTrans->SetNextStateName(szNextState);
	return pTrans;
}

bool ZFSMParser::ReadXml(MZFileSystem* pFileSystem, const char* szFileName, vector<ZFSM*>& out, ZActorActionManager* pActionMgr)
{
	m_pActionMgr = pActionMgr;

	MXmlDocument xmlIniData;
	xmlIniData.Create();

	char *buffer;
	MZFile mzf;

	if(pFileSystem) {
		if(!mzf.Open(szFileName,pFileSystem))  {
			if(!mzf.Open(szFileName))  {
				xmlIniData.Destroy();
				return false;
			}
		}
	} 
	else  {

		if(!mzf.Open(szFileName)) {

			xmlIniData.Destroy();
			return false;
		}
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer,mzf.GetLength());

	bool bResult = ReadXmlFromStr(out, buffer);

	delete[] buffer;
	mzf.Close();

	return bResult;
}

bool ZFSMParser::ReadXmlFromStr(vector<ZFSM*>& out, const char* szXmlContent)
{
	if (!szXmlContent) return NULL;

	MXmlDocument xmlDocument;
	xmlDocument.Create();

	if (!xmlDocument.LoadFromMemory((char*)szXmlContent))
	{
		xmlDocument.Destroy();
		return NULL;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlDocument.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	bool bResult = true;;

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "FSM"))
		{
			ZFSM* pFSM = ReadXmlNode_FSM(chrElement);
			if (pFSM)
				out.push_back(pFSM);
			else 
			{
				for (int i=0; i<(int)out.size(); ++i) 
					delete out[i];
				out.clear();
				bResult = false;
				break;
			}
		}
	}

	xmlDocument.Destroy();
	return bResult;
}

ZFSM* ZFSMParser::ReadXmlNode_FSM(MXmlElement& elem)
{
	MXmlElement childElem;
	char szTagName[256];

	string strFsmName, strEntryStateName;
	elem.GetAttribute(&strFsmName, "name");
	elem.GetAttribute(&strEntryStateName, "entrystate");

	if (strFsmName.empty())
	{
		mlog("error : FSM name attribute is empty.\n");
		return NULL;
	}

	if (strEntryStateName.empty())
	{
		mlog("error : FSM entrystate attribute is empty.\n");
		return NULL;
	}

	ZFSM* pFsm = new ZFSM;
	pFsm->SetName(strFsmName.c_str());
	pFsm->SetEntryStateName(strEntryStateName.c_str());

	int iCount = elem.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		childElem = elem.GetChildNode(i);
		childElem.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "STATE"))
		{
			ZFSMState* pState = ReadXmlNode_STATE(childElem);
			if (pState)
			{
				pFsm->AddState(pState);
			}
			else
			{
				delete pFsm;
				return NULL;
			}
		}
	}

	// built-in 스테이트 추가
	ZFSMState* pBuildinState;
	pBuildinState = ZFSMState::CreateBuiltInState_Die(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_WaitDelete(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_BlastedSoar(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_BlastedFall(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_BlastedDown(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_BlastedStandup(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_BlastedThrust(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_BlastedThrustDown(m_pActionMgr);
	pFsm->AddState(pBuildinState);
	pBuildinState = ZFSMState::CreateBuiltInState_LightningStun(m_pActionMgr);
	pFsm->AddState(pBuildinState);

	// 전이가능한 스테이트를 포인터로 연결
	pFsm->PrepareTransNextState();
	return pFsm;
}

ZFSMState* ZFSMParser::ReadXmlNode_STATE(MXmlElement& elem)
{
	MXmlElement childElem;
	char szTagName[256];

	string strName, strFunc, strEnterFunc, strExitFunc, strAction;
	int nCooltime;
	elem.GetAttribute(&strName, "name");
	elem.GetAttribute(&strFunc, "func");
	elem.GetAttribute(&strEnterFunc, "enterfunc");
	elem.GetAttribute(&strExitFunc, "exitfunc");
	elem.GetAttribute(&strAction, "action");
	elem.GetAttribute(&nCooltime, "cooltime", 0);

	if (strName.empty())
	{
		mlog("error : FSM STATE name attribute is empty.\n");
		return NULL;
	}

	ZFSMState* pState = new ZFSMState;
	pState->SetName(strName.c_str());
	pState->SetCooltime(nCooltime);

	vector<ZFSMFunctionCall> vecFuncCall;
	if (!ParseFunctionsStatement(vecFuncCall, strFunc.c_str()))
	{
		delete pState;
		return NULL;
	}
	pState->SetFunctionCalls(vecFuncCall);

	vector<ZFSMFunctionCall> vecEnterFuncCall;
	if (!ParseFunctionsStatement(vecEnterFuncCall, strEnterFunc.c_str()))
	{
		delete pState;
		return NULL;
	}
	pState->SetEnterFunctionCalls(vecEnterFuncCall);

	vector<ZFSMFunctionCall> vecExitFuncCall;
	if (!ParseFunctionsStatement(vecExitFuncCall, strExitFunc.c_str()))
	{
		delete pState;
		return NULL;
	}
	pState->SetExitFunctionCalls(vecExitFuncCall);


	// action 찾아서 연결하기
	if (!m_pActionMgr)
	{
		_ASSERT(0);
		delete pState;
		return NULL;
	}

	ZActorAction* pAction = m_pActionMgr->GetAction(strAction.c_str());
	pState->SetAction(pAction);	// pAction == NULL도 가능
	

	int iCount = elem.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		childElem = elem.GetChildNode(i);
		childElem.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "TRANS"))
		{
			ZFSMTransition* pTrans = ReadXmlNode_TRANS(childElem);
			if (pTrans)
			{
				pState->AddTransition(pTrans);
			}
			else
			{
				delete pState;
				return NULL;
			}
		}
	}


	// dice 전처리
	pState->MakeDiceFuncHasMaxValue();
	pState->MakeDiceConditionHasRange();

	return pState;
}

ZFSMTransition* ZFSMParser::ReadXmlNode_TRANS(MXmlElement& elem)
{
	string strCond, strNext;
	elem.GetAttribute(&strCond, "cond");
	elem.GetAttribute(&strNext, "next");

	ZFSMTransition* pTrans = ParseTransition(strCond.c_str(), strNext.c_str());
	return pTrans;
}