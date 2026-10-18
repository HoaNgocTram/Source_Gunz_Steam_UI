#pragma once
#ifdef _PAINTMODE
#include <vector>
#include "MZFileSystem.h"

struct PaintMapNode
{
	short id;
	char mapName[32];
};

class PaintMap
{
private:
	std::vector<PaintMapNode> PaintMapInfo;
public:
	PaintMap();
	~PaintMap();
	static PaintMap* GetInstance();
	bool ParseXML(const char* fileName, MZFileSystem* fileSystem = nullptr);
	bool ParseXML_Map(rapidxml::xml_node<>* element, PaintMapNode& node);
	void Clear();
	const PaintMapNode GetMapIndex(const char* mapName) const
	{
		for (auto& mapInfo : PaintMapInfo)
		{
			if (_stricmp(mapInfo.mapName, mapName) == 0)
				return mapInfo;
		}
		return {};
	}

	bool FindMap(const char* mapName)
	{
		for (auto& mapInfo : PaintMapInfo)
		{
			if (_stricmp(mapInfo.mapName, mapName) == 0)
				return true;
		}
		return false;
	}

	const vector<PaintMapNode> GetPaintMaps() const { return PaintMapInfo; }

}; inline PaintMap* MGetPaintMap() { return PaintMap::GetInstance(); }
#endif