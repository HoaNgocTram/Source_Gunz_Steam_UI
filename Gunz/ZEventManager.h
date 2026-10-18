#pragma once
#ifndef _ZEVENTMANAGER_H
#define _ZEVENTMANAGER_H

#include <vector>
#include <string>
#include "MTypes.h"

// Loại phần thưởng
enum ZEVENT_REWARD_TYPE {
	ZEVENT_REWARD_BOUNTY = 0,
	ZEVENT_REWARD_ZCOIN,
	ZEVENT_REWARD_ITEM
};

// Trạng thái của từng ngày điểm danh
enum ZEVENT_ATTENDANCE_STATE {
	ZATTEND_STATE_LOCKED = 0,    // Chưa đến ngày / Khóa
	ZATTEND_STATE_RECEIVE,      // Được nhận (Nút sáng màu xanh như Day 1 trong ảnh)
	ZATTEND_STATE_RECEIVED      // Đã nhận rồi
};

// Cấu trúc dữ liệu 1 ô điểm danh ngày (Day 1, Day 2...)
struct ZDailyAttendanceNode {
	int nDay;                   // Ngày thứ 1, 2, 3...
	std::string strRewardName;  // Tên hiển thị: "Bounty x10000", "Outlaw Package"...
	ZEVENT_REWARD_TYPE nType;   // Loại quà: Bounty, Z-Coin, hay Item
	int nRewardID;              // ItemID hoặc số lượng Bounty/Coin
	int nCount;                 // Số lượng item
	ZEVENT_ATTENDANCE_STATE nState; // Trạng thái hiện tại
};

// Cấu trúc dữ liệu cho Event thời gian chơi (Dành cho tính năng sau)
struct ZPlayTimeRewardNode {
	int nRequiredMinutes;       // Số phút cần chơi (30p, 60p, 120p...)
	std::string strRewardName;
	int nItemID;
	bool bReceived;
};

class ZEventManager {
private:
	std::vector<ZDailyAttendanceNode> m_vtDailyAttendance;
	std::vector<ZPlayTimeRewardNode>  m_vtPlayTimeRewards;

	DWORD m_dwPlayTimeStartTick;
	int   m_nAccumulatedPlayTimeMinutes; // Tổng phút đã chơi trong ngày

public:
	ZEventManager();
	~ZEventManager();

	static ZEventManager* GetInstance();

	void Init();
	void Clear();

	// --- Quản lý điểm danh ---
	void SetAttendanceData(const std::vector<ZDailyAttendanceNode>& vtNodes);
	std::vector<ZDailyAttendanceNode>& GetAttendanceData() { return m_vtDailyAttendance; }
	bool RequestClaimAttendance(int nDay);

	// --- Quản lý Play Time (Tính năng sau này) ---
	void UpdatePlayTime();
	int GetAccumulatedPlayTimeMinutes() { return m_nAccumulatedPlayTimeMinutes; }
};

#define ZGetEventManager() ZEventManager::GetInstance()

#endif