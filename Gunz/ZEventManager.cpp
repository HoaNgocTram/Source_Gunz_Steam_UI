#include "stdafx.h"
#include "ZEventManager.h"
#include "MCommand.h"
#include "MSharedCommandTable.h"

ZEventManager::ZEventManager()
{
	m_dwPlayTimeStartTick = 0;
	m_nAccumulatedPlayTimeMinutes = 0;
}

ZEventManager::~ZEventManager()
{
	Clear();
}

ZEventManager* ZEventManager::GetInstance()
{
	static ZEventManager instance;
	return &instance;
}

void ZEventManager::Init()
{
	m_dwPlayTimeStartTick = GetTickCount();
	m_nAccumulatedPlayTimeMinutes = 0;
	m_vtDailyAttendance.clear();
	m_vtPlayTimeRewards.clear();
}

void ZEventManager::Clear()
{
	m_vtDailyAttendance.clear();
	m_vtPlayTimeRewards.clear();
}

void ZEventManager::SetAttendanceData(const std::vector<ZDailyAttendanceNode>& vtNodes)
{
	m_vtDailyAttendance = vtNodes;
}

// Hàm xử lý gửi yêu cầu nhận quà lên Server
bool ZEventManager::RequestClaimAttendance(int nDay)
{
	for (size_t i = 0; i < m_vtDailyAttendance.size(); ++i)
	{
		if (m_vtDailyAttendance[i].nDay == nDay)
		{
			if (m_vtDailyAttendance[i].nState == ZATTEND_STATE_RECEIVE)
			{
				// Bắn Command trực tiếp bằng macro ZPOSTCMD1 gọn đẹp, không cần qua ZPost.cpp
				ZPOSTCMD1(MC_MATCH_EVENT_CLAIM_ATTENDANCE_REQUEST, MCmdParamInt(nDay));
				return true;
			}
			break;
		}
	}
	return false;
}

// Cập nhật tính phút online (Chuẩn bị sẵn cho Play Time Event sau này)
void ZEventManager::UpdatePlayTime()
{
	if (m_dwPlayTimeStartTick == 0) return;

	DWORD dwCurrentTick = GetTickCount();
	DWORD dwElapsed = 0;

	if (dwCurrentTick >= m_dwPlayTimeStartTick)
		dwElapsed = dwCurrentTick - m_dwPlayTimeStartTick;
	else
		dwElapsed = (0xFFFFFFFF - m_dwPlayTimeStartTick) + dwCurrentTick; // Xử lý khi GetTickCount bị xoay vòng

	m_nAccumulatedPlayTimeMinutes = dwElapsed / 60000;
}