#pragma once
#ifdef _MAGICBOX
#include <vector>
#include "MZFileSystem.h"

struct MagicMapNode
{
	short id;
	char mapName[32];
};

class MagicMap
{
private:
	std::vector<MagicMapNode> MagicMapInfo;
public:
	MagicMap();
	~MagicMap();
	static MagicMap* GetInstance();
	bool ParseXML(const char* fileName, MZFileSystem* fileSystem = nullptr);
	bool ParseXML_Map(rapidxml::xml_node<>* element, MagicMapNode& node);
	void Clear();
	const MagicMapNode GetMapIndex(const char* mapName) const
	{
		for (auto& mapInfo : MagicMapInfo)
		{
			if (_stricmp(mapInfo.mapName, mapName) == 0)
				return mapInfo;
		}
		return {};
	}

	bool FindMap(const char* mapName)
	{
		for (auto& mapInfo : MagicMapInfo)
		{
			if (_stricmp(mapInfo.mapName, mapName) == 0)
				return true;
		}
		return false;
	}

	const vector<MagicMapNode> GetMagicMaps() const { return MagicMapInfo; }

}; inline MagicMap* MGetMagicMap() { return MagicMap::GetInstance(); }
#endif