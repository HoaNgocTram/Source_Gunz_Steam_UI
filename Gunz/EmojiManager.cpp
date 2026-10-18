#include "stdafx.h"
#include "EmojiManager.h"
#include "AnimatedGifTexture.h"
#include "MXml.h"
#include "MZFileSystem.h"
#include "MDebug.h"
#include "ZApplication.h"
#include "RealSpace2.h"
#include <algorithm>

static void emojilog(const char* fmt, ...) {
	static FILE* fp = nullptr;
	if (!fp) {
		fp = fopen("emojis.log", "w");
		if (!fp) return;
	}
	va_list ap;
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
	fflush(fp);
}

// ============================================================
//  EmojiSet
// ============================================================

void EmojiSet::DestroyTextures()
{
	for (auto& pair : animatedTextures)
	{
		if (pair.second)
		{
			pair.second->Destroy();
			delete pair.second;
		}
	}
	animatedTextures.clear();
	emojis.clear();
}

// ============================================================
//  EmojiManager — Singleton
// ============================================================

EmojiManager& EmojiManager::GetInstance()
{
	static EmojiManager instance;
	return instance;
}

EmojiManager::~EmojiManager()
{
	Destroy();
}

void EmojiManager::Destroy()
{
	for (auto& pair : m_Sets)
		pair.second.DestroyTextures();
	m_Sets.clear();
}

bool EmojiManager::IsGifFile(const std::string& filename)
{
	if (filename.size() < 4) return false;
	std::string ext = filename.substr(filename.size() - 4);
	return (stricmp(ext.c_str(), ".gif") == 0);
}

bool EmojiManager::IsWebPFile(const std::string& filename)
{
	size_t len = filename.size();
	if (len < 5) return false;
	std::string ext = filename.substr(len - 5);
	return (stricmp(ext.c_str(), ".webp") == 0);
}

bool EmojiManager::IsAnimatedFile(const std::string& filename)
{
	return IsGifFile(filename) || IsWebPFile(filename);
}

bool EmojiManager::LoadAnimatedEmoji(EmojiSet& set, const std::string& filename)
{
	if (set.animatedTextures.find(filename) != set.animatedTextures.end())
		return true;

	std::string fullPath = set.basePath + filename;

	MZFile mzFile;
	if (!mzFile.Open(fullPath.c_str(), ZApplication::GetFileSystem()))
	{
		if (!mzFile.Open(fullPath.c_str(), (MZFileSystem*)NULL))
		{
			emojilog("EmojiManager: Cannot open animated file %s\n", fullPath.c_str());
			return false;
		}
	}

	unsigned long fileSize = mzFile.GetLength();
	uint8_t* fileData = new uint8_t[fileSize];
	mzFile.Read(fileData, fileSize);

	AnimatedGifTexture* pAnimTex = new AnimatedGifTexture();
	if (!pAnimTex->CreateFromMemory(fileData, (int)fileSize, RGetDevice()))
	{
		emojilog("EmojiManager: Failed to create animated texture for %s\n", filename.c_str());
		delete pAnimTex;
		delete[] fileData;
		return false;
	}

	delete[] fileData;
	set.animatedTextures[filename] = pAnimTex;

	emojilog("EmojiManager: Loaded animated emoji '%s' (%d frames)\n",
		filename.c_str(), pAnimTex->GetFrameCount());
	return true;
}

// ============================================================
//  Loading
// ============================================================

bool EmojiManager::LoadSet(int nSetId, const char* szFileName)
{
	// If set already loaded, destroy old data first
	auto it = m_Sets.find(nSetId);
	if (it != m_Sets.end())
		it->second.DestroyTextures();

	EmojiSet& set = m_Sets[nSetId];

	MXmlDocument xmlDoc;
	xmlDoc.Create();

	MZFile mzFile;
	if (!mzFile.Open(szFileName, (MZFileSystem*)NULL))
	{
		if (!mzFile.Open(szFileName, ZApplication::GetFileSystem()))
		{
			emojilog("EmojiManager: Cannot open %s\n", szFileName);
			xmlDoc.Destroy();
			m_Sets.erase(nSetId);
			return false;
		}
	}

	char* buffer = new char[mzFile.GetLength() + 1];
	buffer[mzFile.GetLength()] = 0;
	mzFile.Read(buffer, mzFile.GetLength());

	if (!xmlDoc.LoadFromMemory(buffer))
	{
		emojilog("EmojiManager: Failed to parse %s\n", szFileName);
		xmlDoc.Destroy();
		delete[] buffer;
		m_Sets.erase(nSetId);
		return false;
	}

	MXmlElement rootElement = xmlDoc.GetDocumentElement();

	// Read global attributes
	char szTemp[64];
	if (rootElement.GetAttribute(szTemp, "lobby_size"))
		set.nLobbySize = atoi(szTemp);
	if (rootElement.GetAttribute(szTemp, "ingame_size"))
		set.nIngameSize = atoi(szTemp);

	// Extract base path
	set.basePath = szFileName;
	size_t lastSlash = set.basePath.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		set.basePath = set.basePath.substr(0, lastSlash + 1);
	else
		set.basePath = "";

	int nStaticCount = 0;
	int nAnimatedCount = 0;

	int nCount = rootElement.GetChildNodeCount();
	for (int i = 0; i < nCount; i++)
	{
		MXmlElement elem = rootElement.GetChildNode(i);

		char szTag[64];
		elem.GetTagName(szTag);
		if (szTag[0] == '#') continue;
		if (stricmp(szTag, "EMOJI") != 0) continue;

		EmojiEntry entry;
		char szPattern[64] = "";
		char szFile[64] = "";
		char szSpace[16] = "true";
		char szWidth[16] = "0";
		char szHeight[16] = "0";

		elem.GetAttribute(szPattern, "pattern");
		elem.GetAttribute(szFile, "file");
		elem.GetAttribute(szSpace, "space");
		elem.GetAttribute(szWidth, "w");
		elem.GetAttribute(szHeight, "h");

		if (szPattern[0] == 0 || szFile[0] == 0) continue;

		entry.pattern = szPattern;
		entry.filename = szFile;
		entry.bNeedSpaceBefore = (stricmp(szSpace, "false") != 0);
		entry.nWidth = atoi(szWidth);
		entry.nHeight = atoi(szHeight);
		entry.bAnimated = IsAnimatedFile(entry.filename);

		if (entry.bAnimated)
		{
			if (LoadAnimatedEmoji(set, entry.filename))
				nAnimatedCount++;
			else
				entry.bAnimated = false;
		}
		else
		{
			nStaticCount++;
		}

		set.emojis.push_back(entry);
	}

	// Sort: longer patterns first
	std::sort(set.emojis.begin(), set.emojis.end(), [](const EmojiEntry& a, const EmojiEntry& b) {
		return a.pattern.size() > b.pattern.size();
		});

	xmlDoc.Destroy();
	delete[] buffer;

	emojilog("EmojiManager: Set %d loaded %d emojis (%d static, %d animated) from %s\n",
		nSetId, (int)set.emojis.size(), nStaticCount, nAnimatedCount, szFileName);

	return true;
}

void EmojiManager::LoadAllSets()
{
	struct SetInfo { int id; const char* path; };
	static const SetInfo sets[] = {
		{ 1, "Interface/default/emojis/default/emojis.xml"  },
		{ 2, "Interface/default/emojis/noto/emojis.xml"      },
		{ 3, "Interface/default/emojis/facebook/emojis.xml" },
		{ 4, "Interface/default/emojis/yahoo/emojis.xml"    },
	};

	for (const auto& s : sets)
	{
		// Try to load, don't fail if a set is missing
		LoadSet(s.id, s.path);
	}

	emojilog("EmojiManager: LoadAllSets complete, %d sets loaded\n", (int)m_Sets.size());
}

// ============================================================
//  Query
// ============================================================

bool EmojiManager::IsLoaded() const
{
	return !m_Sets.empty();
}

bool EmojiManager::IsSetLoaded(int nSetId) const
{
	return m_Sets.find(nSetId) != m_Sets.end();
}

const EmojiSet* EmojiManager::GetEmojiSet(int nSetId) const
{
	auto it = m_Sets.find(nSetId);
	if (it != m_Sets.end())
		return &it->second;
	return nullptr;
}

// ============================================================
//  Sizes
// ============================================================

int EmojiManager::GetSize(bool bIngame, int nSetId) const
{
	const EmojiSet* pSet = GetEmojiSet(nSetId);
	if (!pSet)
	{
		// Fallback: try any loaded set
		if (!m_Sets.empty())
			pSet = &m_Sets.begin()->second;
		else
			return bIngame ? 14 : 16;
	}
	return bIngame ? pSet->nIngameSize : pSet->nLobbySize;
}

int EmojiManager::GetLobbySize(int nSetId) const
{
	const EmojiSet* pSet = GetEmojiSet(nSetId);
	return pSet ? pSet->nLobbySize : 16;
}

int EmojiManager::GetIngameSize(int nSetId) const
{
	const EmojiSet* pSet = GetEmojiSet(nSetId);
	return pSet ? pSet->nIngameSize : 14;
}

int EmojiManager::GetMaxSize(bool bIngame) const
{
	int maxSize = 0;
	for (const auto& pair : m_Sets)
	{
		int sz = bIngame ? pair.second.nIngameSize : pair.second.nLobbySize;
		if (sz > maxSize) maxSize = sz;
	}
	return maxSize > 0 ? maxSize : (bIngame ? 14 : 16);
}

void EmojiManager::GetDrawSize(const EmojiEntry* pEntry, bool bIngame, int nSetId, int& outW, int& outH) const
{
	int baseSize = GetSize(bIngame, nSetId);

	if (pEntry->nWidth > 0 && pEntry->nHeight > 0)
	{
		outW = pEntry->nWidth;
		outH = pEntry->nHeight;
		return;
	}

	// For animated GIFs, auto-calculate from actual texture dimensions
	if (pEntry->bAnimated)
	{
		AnimatedGifTexture* pTex = GetAnimatedTexture(pEntry->filename.c_str(), nSetId);
		if (pTex && pTex->GetWidth() > 0 && pTex->GetHeight() > 0)
		{
			int srcW = pTex->GetWidth();
			int srcH = pTex->GetHeight();
			if (srcW > srcH)
			{
				outH = baseSize;
				outW = (int)((float)baseSize * srcW / srcH);
			}
			else if (srcH > srcW)
			{
				outW = baseSize;
				outH = (int)((float)baseSize * srcH / srcW);
			}
			else
			{
				outW = outH = baseSize;
			}
			return;
		}
	}

	outW = pEntry->nWidth > 0 ? pEntry->nWidth : baseSize;
	outH = pEntry->nHeight > 0 ? pEntry->nHeight : baseSize;
}

// ============================================================
//  Line height
// ============================================================

int EmojiManager::GetLineHeight(const char* szText, int nLen, int nFontHeight, bool bIngame, int nSetId) const
{
	const EmojiSet* pSet = GetEmojiSet(nSetId);
	if (!pSet) return nFontHeight;

	int maxH = nFontHeight;
	int defSize = GetSize(bIngame, nSetId);

	for (int i = 0; i < nLen; i++)
	{
		// Skip prefix chars
		if (szText[i] == EMOJI_PREFIX && i + 1 < nLen && szText[i + 1] >= '0' && szText[i + 1] <= '9')
		{
			i++; // skip digit too
			continue;
		}

		EmojiMatchResult match = FindEmoji(szText, i, nLen, nSetId);
		if (match.pEntry)
		{
			int w, h;
			GetDrawSize(match.pEntry, bIngame, nSetId, w, h);
			if (h > maxH) maxH = h;
			i += match.nPatternLen - 1;
		}
	}
	return maxH;
}

int EmojiManager::GetLineHeightW(const wchar_t* szText, int nLen, int nFontHeight, bool bIngame, int nSetId) const
{
	const EmojiSet* pSet = GetEmojiSet(nSetId);
	if (!pSet) return nFontHeight;

	int maxH = nFontHeight;

	for (int i = 0; i < nLen; i++)
	{
		if (szText[i] == (wchar_t)EMOJI_PREFIX && i + 1 < nLen && szText[i + 1] >= L'0' && szText[i + 1] <= L'9')
		{
			i++;
			continue;
		}

		EmojiMatchResult match = FindEmojiW(szText, i, nLen, nSetId);
		if (match.pEntry)
		{
			int w, h;
			GetDrawSize(match.pEntry, bIngame, nSetId, w, h);
			if (h > maxH) maxH = h;
			i += match.nPatternLen - 1;
		}
	}
	return maxH;
}

// ============================================================
//  Animated textures
// ============================================================

AnimatedGifTexture* EmojiManager::GetAnimatedTexture(const char* szFilename, int nSetId) const
{
	const EmojiSet* pSet = GetEmojiSet(nSetId);
	if (!pSet) return nullptr;

	auto it = pSet->animatedTextures.find(szFilename);
	if (it != pSet->animatedTextures.end())
		return it->second;
	return nullptr;
}

// ============================================================
//  Emoji matching
// ============================================================

EmojiMatchResult EmojiManager::FindEmoji(const char* szText, int nPos, int nLen, int nSetId) const
{
	EmojiMatchResult result = { nullptr, 0, nSetId };

	const EmojiSet* pSet = GetEmojiSet(nSetId);
	if (!pSet) return result;

	for (const auto& entry : pSet->emojis)
	{
		int patLen = (int)entry.pattern.size();
		if (nPos + patLen > nLen) continue;

		if (entry.bNeedSpaceBefore && nPos > 0 && szText[nPos - 1] != ' ')
			continue;

		bool bMatch = true;
		for (int j = 0; j < patLen; j++)
		{
			if (tolower((unsigned char)szText[nPos + j]) != tolower((unsigned char)entry.pattern[j]))
			{
				bMatch = false;
				break;
			}
		}

		if (bMatch)
		{
			result.pEntry = &entry;
			result.nPatternLen = patLen;
			return result;
		}
	}

	return result;
}

EmojiMatchResult EmojiManager::FindEmojiW(const wchar_t* szText, int nPos, int nLen, int nSetId) const
{
	EmojiMatchResult result = { nullptr, 0, nSetId };

	const EmojiSet* pSet = GetEmojiSet(nSetId);
	if (!pSet) return result;

	for (const auto& entry : pSet->emojis)
	{
		int patLen = (int)entry.pattern.size();
		if (nPos + patLen > nLen) continue;

		if (entry.bNeedSpaceBefore && nPos > 0 && szText[nPos - 1] != L' ')
			continue;

		bool bMatch = true;
		for (int j = 0; j < patLen; j++)
		{
			if (towlower(szText[nPos + j]) != towlower((wchar_t)entry.pattern[j]))
			{
				bMatch = false;
				break;
			}
		}

		if (bMatch)
		{
			result.pEntry = &entry;
			result.nPatternLen = patLen;
			return result;
		}
	}

	return result;
}

// ============================================================
//  Prefix protocol
// ============================================================

int EmojiManager::ParsePrefix(const char* szText, int nLen, int& outSkip)
{
	outSkip = 0;
	if (nLen >= 2 && szText[0] == EMOJI_PREFIX && szText[1] >= '0' && szText[1] <= '9')
	{
		outSkip = 2;
		return szText[1] - '0';
	}
	return -1;
}

int EmojiManager::ParsePrefixW(const wchar_t* szText, int nLen, int& outSkip)
{
	outSkip = 0;
	if (nLen >= 2 && szText[0] == (wchar_t)EMOJI_PREFIX && szText[1] >= L'0' && szText[1] <= L'9')
	{
		outSkip = 2;
		return szText[1] - L'0';
	}
	return -1;
}

std::string EmojiManager::MakePrefix(int nSetId)
{
	if (nSetId <= 0 || nSetId > 9) return "";
	char buf[3] = { EMOJI_PREFIX, (char)('0' + nSetId), '\0' };
	return std::string(buf);
}

int EmojiManager::ScanAndStripPrefix(char* szText, int nLen)
{
	for (int i = 0; i < nLen - 1; i++)
	{
		if ((unsigned char)szText[i] == (unsigned char)EMOJI_PREFIX && szText[i + 1] >= '0' && szText[i + 1] <= '9')
		{
			int nSet = szText[i + 1] - '0';
			// Remove 2 bytes by shifting rest of string left
			memmove(szText + i, szText + i + 2, nLen - i - 2);
			szText[nLen - 2] = 0;
			return nSet;
		}
	}
	return -1;
}

int EmojiManager::ScanAndStripPrefixW(wchar_t* szText, int nLen)
{
	for (int i = 0; i < nLen - 1; i++)
	{
		if (szText[i] == (wchar_t)EMOJI_PREFIX && szText[i + 1] >= L'0' && szText[i + 1] <= L'9')
		{
			int nSet = szText[i + 1] - L'0';
			wmemmove(szText + i, szText + i + 2, nLen - i - 2);
			szText[nLen - 2] = 0;
			return nSet;
		}
	}
	return -1;
}