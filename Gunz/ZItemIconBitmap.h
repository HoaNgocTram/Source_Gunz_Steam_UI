#pragma once

#include <string>
#include <map>

class MBitmap;
class MBitmapR2; // Forward declaration
struct MMatchItemDesc;

MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc);
const char* GetItemIconBitmap_Potion(MMatchItemDesc* pDesc);
const char* GetItemIconBitmap_Trap(MMatchItemDesc* pDesc);
MBitmap* GetItemThumbnailBitmap(MMatchItemDesc* pDesc);

// Struct lưu thông tin vị trí Icon trong Atlas
struct ZItemIconInfo {
    std::string strSource; // Tên file atlas (vd: itemicon_male_s_00.png)
    int nX;                // Tọa độ X trong atlas
    int nY;                // Tọa độ Y trong atlas
    int nWidth;            // Chiều rộng (W)
    int nHeight;           // Chiều cao (H)

    ZItemIconInfo() : nX(0), nY(0), nWidth(0), nHeight(0) {}
};

// Manager đọc XML và cache vị trí
class ZItemIconMgr {
private:
    std::map<int, ZItemIconInfo>      m_IconMap;
    std::map<std::string, MBitmapR2*> m_mapAtlasBitmaps;

public:
    ZItemIconMgr() {}
    ~ZItemIconMgr(); // Destructor tự động dọn dẹp bộ nhớ

    bool ReadXML(const char* szFileName);
    bool GetIconInfo(int nItemID, ZItemIconInfo& outInfo);
    MBitmap* GetAtlasBitmap(const char* szSourceFile);

    // Hàm giải phóng bộ nhớ Atlas
    void CleanUpAtlas();
};