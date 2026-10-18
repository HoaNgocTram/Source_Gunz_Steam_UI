#pragma once

class MNewQuestNpcObject;
struct MUID;

typedef map<MUID, MNewQuestNpcObject*>	MapNpc;
typedef MapNpc::iterator				ItorNpc;

typedef vector<MUID>					vecNPCUid;
typedef vecNPCUid::iterator				ItorNPCUid;

class MNewQuestNpcObject
{
private:
	MUID m_uid;
	std::string m_nActorType;
	MUID m_uidController;
	int m_nDropItemId;
	bool m_bBoss;
	int m_nIndex;
	int m_nCustomIndex;

public:
	MNewQuestNpcObject(MUID& uid) : m_uid(uid), m_uidController(0, 0), m_nDropItemId(0), m_nIndex(0), m_nCustomIndex(0), m_bBoss(false) { m_nActorType = ""; }
	~MNewQuestNpcObject() {}

	MUID GetUID() { return m_uid; }

	// 이 npc를 조종할 플레이어
	void AssignControl(MUID& uidPlayer) { m_uidController=uidPlayer; }
	void ReleaseControl() { m_uidController.SetInvalid(); }
	MUID& GetController() { return m_uidController; }

	int GetDropItemId()			{ return m_nDropItemId; }
	void SetDropItemId(int id)	{ m_nDropItemId = id; }
	bool HasDropItem()			{ return m_nDropItemId != 0; }
	std::string GetActorType() { return m_nActorType; }
	void SetActorType(std::string type) { m_nActorType = type; }
	bool IsBoss() { return m_bBoss; }
	void SetBoss(bool isboss) { m_bBoss = isboss; }
	int GetIndex() { return m_nIndex; }
	void SetIndex(int val) { m_nIndex = val; }
	int GetCustomIndex() { return m_nCustomIndex; }
	void SetCustomIndex(int val) { m_nCustomIndex = val; }
};

class MNewQuestNpcManager
{
private:
	typedef map<MUID, MNewQuestNpcObject*>	MapNpc;
	typedef MapNpc::iterator				ItorNpc;

	typedef vector<MUID>					vecNPCUid;
	typedef vecNPCUid::iterator				ItorNPCUid;

public:
	MapNpc m_mapNpc;

	void AddNpcObject(MUID uidNpc, MUID uidController, int nCustomSpawnIndex, int nSpawnIndex, bool bBoss, std::string actorType, int nDropItemId);
	void DeleteNpcObject(MUID uidNpc);

	int GetNumNpc() { return (int)m_mapNpc.size(); }
	MNewQuestNpcObject* GetNpc(MUID uid);
	void SetController(MUID uidNpc, MUID uidChar);
	vecNPCUid GetNPCByController(MUID uidChar);
};