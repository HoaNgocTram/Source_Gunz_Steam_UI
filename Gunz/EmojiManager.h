#pragma once
#include <vector>
#include <string>
#include <map>

// ============================================================
//  EmojiManager — Multi-set emoji system
//  Each emoji set (default, yahoo, ...) loaded separately
//  Messages carry sender's set ID via \x1F prefix
// ============================================================

class AnimatedGifTexture; // forward declaration

struct EmojiEntry
{
	std::string pattern;     // ":D", "^_^", "O:)", ":v", etc.
	std::string filename;    // "1.png", "diamond.gif"
	bool bNeedSpaceBefore;   // true = needs space (or start of text) before pattern
	int nWidth;              // custom width (0 = use default size)
	int nHeight;             // custom height (0 = use default size)
	bool bAnimated;          // true if this is an animated GIF emoji
};

struct EmojiMatchResult
{
	const EmojiEntry* pEntry;
	int nPatternLen;         // how many chars matched
	int nSetId;              // which set this match came from
};

// One loaded emoji set
struct EmojiSet
{
	std::vector<EmojiEntry> emojis;        // sorted: longest pattern first
	int nLobbySize = 16;
	int nIngameSize = 14;
	std::string basePath;                  // e.g. "Interface/default/emojis/yahoo/"
	std::map<std::string, AnimatedGifTexture*> animatedTextures;

	void DestroyTextures();
};

class EmojiManager
{
public:
	static EmojiManager& GetInstance();

	// --- Loading ---
	bool LoadSet(int nSetId, const char* szFileName);
	void LoadAllSets();   // loads all known sets (1=default,2=noto,3=facebook,4=yahoo)
	void Destroy();

	// --- Query ---
	bool IsLoaded() const;                        // any set loaded?
	bool IsSetLoaded(int nSetId) const;

	// Active set (user's chosen set from config)
	int  GetActiveSet() const { return m_nActiveSet; }
	void SetActiveSet(int n) { m_nActiveSet = n; }

	// --- Emoji matching ---
	EmojiMatchResult FindEmoji(const char* szText, int nPos, int nLen, int nSetId) const;
	EmojiMatchResult FindEmojiW(const wchar_t* szText, int nPos, int nLen, int nSetId) const;

	// Convenience: use active set
	EmojiMatchResult FindEmoji(const char* szText, int nPos, int nLen) const
	{ return FindEmoji(szText, nPos, nLen, m_nActiveSet); }
	EmojiMatchResult FindEmojiW(const wchar_t* szText, int nPos, int nLen) const
	{ return FindEmojiW(szText, nPos, nLen, m_nActiveSet); }

	// --- Sizes ---
	int  GetSize(bool bIngame, int nSetId) const;
	int  GetSize(bool bIngame) const { return GetSize(bIngame, m_nActiveSet); }

	int  GetLobbySize(int nSetId) const;
	int  GetIngameSize(int nSetId) const;
	int  GetLobbySize() const { return GetLobbySize(m_nActiveSet); }
	int  GetIngameSize() const { return GetIngameSize(m_nActiveSet); }

	// Max size across ALL loaded sets (for widget line height)
	int  GetMaxSize(bool bIngame) const;

	void GetDrawSize(const EmojiEntry* pEntry, bool bIngame, int nSetId, int& outW, int& outH) const;
	void GetDrawSize(const EmojiEntry* pEntry, bool bIngame, int& outW, int& outH) const
	{ GetDrawSize(pEntry, bIngame, m_nActiveSet, outW, outH); }

	// --- Line height ---
	int GetLineHeight(const char* szText, int nLen, int nFontHeight, bool bIngame, int nSetId) const;
	int GetLineHeightW(const wchar_t* szText, int nLen, int nFontHeight, bool bIngame, int nSetId) const;
	int GetLineHeight(const char* szText, int nLen, int nFontHeight, bool bIngame) const
	{ return GetLineHeight(szText, nLen, nFontHeight, bIngame, m_nActiveSet); }
	int GetLineHeightW(const wchar_t* szText, int nLen, int nFontHeight, bool bIngame) const
	{ return GetLineHeightW(szText, nLen, nFontHeight, bIngame, m_nActiveSet); }

	// --- Animated textures ---
	AnimatedGifTexture* GetAnimatedTexture(const char* szFilename, int nSetId) const;
	AnimatedGifTexture* GetAnimatedTexture(const char* szFilename) const
	{ return GetAnimatedTexture(szFilename, m_nActiveSet); }

	// --- Emoji set prefix protocol ---
	static const char EMOJI_PREFIX = '\x1F';

	// Parse prefix from text. Returns set ID (0-9), or -1 if no prefix.
	// outSkip = number of chars to skip (0 or 2)
	static int ParsePrefix(const char* szText, int nLen, int& outSkip);
	static int ParsePrefixW(const wchar_t* szText, int nLen, int& outSkip);

	// Scan ENTIRE text for prefix anywhere, blank prefix bytes with spaces
	static int ScanAndStripPrefix(char* szText, int nLen);
	static int ScanAndStripPrefixW(wchar_t* szText, int nLen);

	// Build prefix string for a set ID (returns "" if set 0)
	static std::string MakePrefix(int nSetId);

	// --- Access ---
	const EmojiSet* GetEmojiSet(int nSetId) const;

private:
	EmojiManager() = default;
	~EmojiManager();

	std::map<int, EmojiSet> m_Sets;
	int m_nActiveSet = 0;

	bool LoadAnimatedEmoji(EmojiSet& set, const std::string& filename);
	static bool IsGifFile(const std::string& filename);
	static bool IsWebPFile(const std::string& filename);
	static bool IsAnimatedFile(const std::string& filename);
};

#define ZGetEmojiManager() EmojiManager::GetInstance()

// ============================================================
//  Legacy support — keep weapons/blank for kill feed system
// ============================================================
struct Emoji
{
	char A, B, C;
	char name[20];
	Emoji(char a, char b, char c, char* Name)
	{
		A = a; B = b; C = c;
		memset(name, 0, 20);
		strcpy(name, Name);
	}
	Emoji(char a, char b, char* Name)
	{
		A = a; B = b; C = ' ';
		memset(name, 0, 20);
		strcpy(name, Name);
	}
};

typedef std::vector<Emoji*> Emojis;

extern Emojis m_Weapons;
extern Emojis m_Blank;
