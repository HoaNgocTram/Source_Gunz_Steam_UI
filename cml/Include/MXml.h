#ifndef MXML_H
#define MXML_H
#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <comutil.h>
#include <stdio.h>
#include <string>
using namespace std;

//#define _MSXML2


#ifdef _MSXML2
	#import "msxml4.dll" named_guids no_implementation
//	using namespace MSXML2;

	typedef MSXML2::IXMLDOMDocumentPtr				MXmlDomDocPtr;
	typedef MSXML2::IXMLDOMNodePtr					MXmlDomNodePtr;
	typedef MSXML2::IXMLDOMNodeListPtr				MXmlDomNodeListPtr;
	typedef MSXML2::IXMLDOMElementPtr				MXmlDomElementPtr;
	typedef MSXML2::IXMLDOMProcessingInstructionPtr MXmlDomPIPtr;
	typedef MSXML2::IXMLDOMNamedNodeMapPtr			MXmlDomNamedNodeMapPtr;
	typedef MSXML2::IXMLDOMTextPtr					MXmlDomTextPtr;
	typedef MSXML2::IXMLDOMParseErrorPtr			MXmlDomParseErrorPtr;
#else
	#import "msxml3.dll" named_guids no_implementation rename_namespace("MSXML")

	typedef MSXML::IXMLDOMDocumentPtr				MXmlDomDocPtr;
	typedef MSXML::IXMLDOMNodePtr					MXmlDomNodePtr;
	typedef MSXML::IXMLDOMNodeListPtr				MXmlDomNodeListPtr;
	typedef MSXML::IXMLDOMElementPtr				MXmlDomElementPtr;
	typedef MSXML::IXMLDOMProcessingInstructionPtr	MXmlDomPIPtr;
	typedef MSXML::IXMLDOMNamedNodeMapPtr			MXmlDomNamedNodeMapPtr;
	typedef MSXML::IXMLDOMTextPtr					MXmlDomTextPtr;
	typedef MSXML::IXMLDOMParseErrorPtr				MXmlDomParseErrorPtr;

//	using namespace MSXML;
#endif

class MXmlDocument;

/// IXMLDOMNode ·¡ÇÎ Å¬·¡½º
class MXmlNode
{
private:

protected:
	MXmlDomNodePtr		m_pDomNode;			///< IXMLDOMNode ½º¸¶Æ® Æ÷ÀÎÅÍ
public:
	/// Default constructor.
	MXmlNode() { m_pDomNode = NULL; }
	MXmlNode(MXmlDomNodePtr a_pDomNode) { m_pDomNode = a_pDomNode; }
	/// Default destructor.
	virtual ~MXmlNode() { m_pDomNode = NULL; }

	/// IXMLDOMNodeÀÎÅÍÆäÀÌ½º·Î ¹ÝÈ¯
	MXmlDomNodePtr	GetXmlDomNodePtr() { return m_pDomNode; }
	/// IXMLDOMNode¸¦ ¼³Á¤
	void			SetXmlDomNodePtr(MXmlDomNodePtr pNode) { m_pDomNode = pNode; }

	/// IXMLDOMNode°¡ NULLÀÎÁö ¿©ºÎ
	bool IsEmpty() { if (m_pDomNode == NULL) return true; else return false; }
	/// NodeÀÇ ÀÌ¸§À» ¹ÝÈ¯.
	/// @param sOutStr			[out] ¹ÝÈ¯°ª
	void GetNodeName(char* sOutStr);
	/// NodeÀÇ Text¸¦ ¹ÝÈ¯.
	/// @param sOutStr			[out] ¹ÝÈ¯°ª
	void GetText(char* sOutStr, int nMaxCharNum = -1);
	/// NodeÀÇ Text¸¦ ¼³Á¤.
	/// @param sOutStr			[in] ¼³Á¤ÇÒ Text
	void SetText(const char* sText);
	
	/// Child NodeÀÇ °¹¼ö¸¦ ¹ÝÈ¯.
	int	GetChildNodeCount();
	/// NodeÀÇ Å¸ÀÔÀ» ¹ÝÈ¯.
	DOMNodeType GetNodeType();
	/// Child Node°¡ ÀÖ³ª?
	bool HasChildNodes();

	void NextSibling();
	void PreviousSibling();

	bool AppendChild(MXmlNode node);

	/// ÇØ´çÀÌ¸§À» °¡Áø Child Node¸¦ Ã£´Â´Ù.
	/// ¾øÀ¸¸é NULL·Î ¼³Á¤
	/// @param sNodeName		[in] Ã£À» NodeÀÇ ÀÌ¸§
	bool FindChildNode(const char* sNodeName, MXmlNode* pOutNode);

	/// ºÎ¸ð Node¸¦ ¹ÝÈ¯. ¾øÀ¸¸é NULL·Î ¹ÝÈ¯ÇÑ´Ù.
	MXmlNode GetParent() { if (m_pDomNode) return MXmlNode(m_pDomNode->parentNode); else return MXmlNode(); }
	/// ÀÎµ¦½º·Î Child Node¸¦ ¹ÝÈ¯
	/// @param iIndex			[in] ÀÎµ¦½º
	MXmlNode GetChildNode(int iIndex);

	/// ÆÐÅÏ½ÄÀ» ÀÌ¿ëÇÏ¿© ÇØ´ç ³ëµå¸¦ Ã£´Â´Ù. °¡Àå ¸ÕÀú Ã£´Â Ã¹³ëµå¸¸ ¹ÝÈ¯
	/// @param sQueryStr		[in] ÆÐÅÏ½Ä
	MXmlNode SelectSingleNode(TCHAR* sQueryStr);
	/// ÆÐÅÏ½Ä¿¡ ¸Â´Â ¿©·¯°³ÀÇ ³ëµå¸¦ Ã£´Â´Ù.
	/// @param sQueryStr		[in] ÆÐÅÏ½Ä
	MXmlDomNodeListPtr	SelectNodes(TCHAR* sQueryStr);

	MXmlNode& operator= (MXmlNode aNode) { m_pDomNode = aNode.GetXmlDomNodePtr(); return *this; }
};

/// IXMLDOMElement ·¡ÇÎ Å¬·¡½º
class MXmlElement: public MXmlNode
{
private:

protected:

public:
	/// Default constructor.
	MXmlElement() { }
	MXmlElement(MXmlDomElementPtr a_pDomElement)	{ m_pDomNode = a_pDomElement; }
	MXmlElement(MXmlDomNodePtr a_pDomNode)			{ m_pDomNode = a_pDomNode; }
	MXmlElement(MXmlNode aNode)						{ m_pDomNode = aNode.GetXmlDomNodePtr(); }
	/// Default destructor.
	virtual ~MXmlElement() { }
	/// ÅÂ±× ÀÌ¸§À» ¹ÝÈ¯ÇÑ´Ù.
	/// @param sOutStr			[out] ÅÂ±× ÀÌ¸§
	void GetTagName(char* sOutStr) { MXmlNode::GetNodeName(sOutStr); }
	
	/// ÇØ´çÅÂ±×·Î µÑ·¯½ÎÀÎ Contents¸¦ ¹ÝÈ¯
	/// @param sOutStr			[out] ¹ÝÈ¯°ª
	void GetContents(char* sOutStr) { MXmlNode::GetText(sOutStr); }
	void GetContents(int* ipOutValue);
	void GetContents(unsigned int* ipOutValue);
	void GetContents(bool* bpOutValue);
	void GetContents(float* fpOutValue);
	void GetContents(string* pstrValue);

	/// Contents¸¦ ¼³Á¤
	void SetContents(const char* sStr) { MXmlNode::SetText(sStr); }
	void SetContents(int iValue);
	void SetContents(bool bValue);
	void SetContents(float fValue);

	bool GetChildContents(char* sOutStr, const char* sChildTagName, int nMaxCharNum = -1);
	bool GetChildContents(int* iOutValue, const char* sChildTagName);
	bool GetChildContents(unsigned long* iOutValue, const char* sChildTagName);
	bool GetChildContents(float* fOutValue, const char* sChildTagName);
	bool GetChildContents(bool* bOutValue, const char* sChildTagName);

	/// ¼Ó¼º°ªÀ» ¹ÝÈ¯ - ¼±Çü°Ë»öÀÌ¶ó ½Ã°£Àº ¿À·¡ °É¸°´Ù.
	/// @param sOutText			[out] ¹ÝÈ¯µÉ ¼Ó¼º°ª
	/// @param sAttrName		[in] ¼Ó¼º ÀÌ¸§
	bool GetAttribute(char* sOutText, const char* sAttrName, char* sDefaultText = "");
	bool GetAttribute(int* ipOutValue, const char* sAttrName, int nDefaultValue = 0);
	bool GetAttribute(bool* bOutValue, const char* sAttrName, bool bDefaultValue = false);
	bool GetAttribute(float* fpOutValue, const char* sAttrName, float fDefaultValue = 0.0f);
	bool GetAttribute(string* pstrOutValue, const char* sAttrName, char* sDefaultValue = "");
	/// ¼Ó¼ºÀ» Ãß°¡ÇÑ´Ù.
	/// @param sAttrName		[in] ¼Ó¼º ÀÌ¸§
	/// @param sAttrText		[in] ¼Ó¼º°ª
	bool AddAttribute(const char* sAttrName, const char* sAttrText);
	bool AddAttribute(const char* sAttrName, int iAttrValue);
	bool AddAttribute(const char* sAttrName, bool bAttrValue);
	/// ¼Ó¼ºÀ» Àç¼³Á¤. ÇØ´ç¼Ó¼ºÀÌ Á¸ÀçÇÏÁö ¾ÊÀ¸¸é Ãß°¡ÇÑ´Ù.
	/// @param sAttrName		[in] ¼Ó¼º ÀÌ¸§
	/// @param sAttrText		[in] ¼Ó¼º°ª
	bool SetAttribute(const char* sAttrName, char* sAttrText);
	bool SetAttribute(const char* sAttrName, int iAttrValue);
	/// ¼Ó¼ºÀ» Áö¿î´Ù.
	bool RemoveAttribute(const char* sAttrName);

	int GetAttributeCount();
	/// ¼Ó¼º°ª ¹ÝÈ¯ - À§ÀÇ GetAttributeÇÔ¼öº¸´Ù ºü¸£´Ù.
	void GetAttribute(int index, char* szoutAttrName, char* szoutAttrValue);

	/// ÀÚ½Ä Element¸¦ Ãß°¡ÇÑ´Ù.
	/// @param sTagName			[in] ÅÂ±× ÀÌ¸§
	/// @param sTagText			[in] ÅÂ±× Contents
	bool AppendChild(const char* sTagName, const char* sTagText = NULL);
	bool AppendChild(MXmlElement aChildElement);

	/// ÀÚ½Ä Element¸¦ Ãß°¡ÇÑ´Ù.
	/// @param sTagName			[in] ÅÂ±× ÀÌ¸§
	MXmlElement	CreateChildElement(const char* sTagName);

	/// ÅØ½ºÆ®¸¦ Ãß°¡ÇÑ´Ù.
	bool AppendText(const char* sText);

	MXmlElement& operator= (MXmlElement aElement) { m_pDomNode = aElement.GetXmlDomNodePtr(); return *this; }
	MXmlElement& operator= (MXmlNode aNode) { m_pDomNode = aNode.GetXmlDomNodePtr(); return *this; }
};

/// XML Document Å¬·¡½º.
class MXmlDocument
{
private:
	bool							m_bInitialized;		///< ÃÊ±âÈ­µÇ¾ú´ÂÀÇ ¿©ºÎ
	MXmlDomDocPtr*					m_ppDom;			///< IXMLDOMDocument
protected:

public:
	/// Default constructor.
	MXmlDocument();
	/// Default destructor.
	virtual ~MXmlDocument();

	/// ÃÊ±âÈ­. ÀÌ ÇÔ¼ö´Â ÀÌ Å¬·¡½º¸¦ »ç¿ëÇÏ±â Àü¿¡ ²À ¼öÇàµÇ¾î¾ß ÇÑ´Ù
	bool				Create(void);
	/// ¸¶¹«¸®.
	bool				Destroy(void);

	/// XML ÆÄÀÏÀ» ÀÐ´Â´Ù.
	bool				LoadFromFile(const char* m_sFileName);
	/// XML ¸Þ¸ð¸®¹öÆÛ·ÎºÎÅÍ ÀÐ´Â´Ù.
	bool				LoadFromMemory(const char* szBuffer, LANGID lanid = LANG_KOREAN);

	/// XML ÆÄÀÏ·Î ÀúÀå.
	bool				SaveToFile(const char* m_sFileName);

	/// processing instruction node¸¦ »ý¼º.
	bool				CreateProcessingInstruction( const char* szHeader = "version=\"1.0\"");
	/// Node¸¦ »èÁ¦. ¸¸¾à Child Node°¡ ÀÖÀ¸¸é ÇÔ²² »èÁ¦ÇÑ´Ù.
	bool				Delete(MXmlNode* pNode);

	MXmlElement			CreateElement(const char* sName);

	bool				AppendChild(MXmlNode node);

	/// XML DOM tree ÃÖ»óÀ§ Node¸¦ ¹ÝÈ¯.
	MXmlDomDocPtr		GetDocument()	{ return (*m_ppDom); }
	/// ÃÖ»óÀ§ Element¸¦ ¹ÝÈ¯.
	MXmlElement			GetDocumentElement()	{ return MXmlElement((*m_ppDom)->documentElement); }

	/// ÅÂ±× ÀÌ¸§À¸·Î Element¸¦ Ã£´Â´Ù. °¡Àå ¸ÕÀú Ã£´Â Ã¹³ëµå¸¸ ¹ÝÈ¯.
	MXmlNode			FindElement(TCHAR* sTagName);

	bool				IsInitialized() { return m_bInitialized; }
};

// Utils
#ifdef _UTF8
static inline std::string BSTRToUTF8(BSTR b)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, b, -1, NULL, 0, NULL, NULL);
	std::string out;
	out.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, b, -1, &out[0], len, NULL, NULL);
	return out;
}
#define _BSTRToAscii(s) BSTRToUTF8((BSTR)s).c_str()
#else
#define _BSTRToAscii(s) (const char*)(_bstr_t)(s)
#endif
BSTR _AsciiToBSTR(const char* ascii);

#endif