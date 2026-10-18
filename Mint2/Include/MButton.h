#ifndef MBUTTON_H
#define MBUTTON_H

#include "MWidget.h"
#include "MLookNFeel.h"

class MMsgBox;
class MButtonGroup;

/// ¹öÆ° Å¸ÀÔ
enum MButtonType{
	MBT_NORMAL = 0,		///< ÀÏ¹Ý ¹öÆ°
	MBT_PUSH,			///< Çª½¬ ¹öÆ°
	MBT_PUSH2,
};

/// µðÆúÆ® Å° µî·Ï
enum MButtonKeyAssigned{
	MBKA_NONE = 0,		///< ¾Æ¹«Å°µµ µî·ÏµÇ¾î ÀÖÁö ¾ÊÀº ¹öÆ°
	MBKA_ENTER,			///< Enter Å° ´­¸®¸é ½ÇÇàµÇ´Â ¹öÆ°
	MBKA_ESC,			///< ESC Å° ´­¸®¸é ½ÇÇàµÇ´Â ¹öÆ°
};

class MButton;
/// ButtonÀÇ Draw ÄÚµå°¡ ÀÖ´Â Å¬·¡½º, ÀÌ Å¬·¡½º¸¦ »ó¼Ó¹Þ¾Æ¼­ Ä¿½ºÅÒ·èÀ» °¡Áú ¼ö ÀÖ´Ù.
class MButtonLook{
protected:
	bool	m_bWireLook;
protected:
	virtual void OnDownDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnUpDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnOverDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnDisableDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnCheckBoxDraw(MButton* pButton, MDrawContext* pDC, bool bPushed){};
public:
	virtual void OnDrawText(MButton* pButton, MRECT& r, MDrawContext* pDC);
	virtual void OnDraw(MButton* pButton, MDrawContext* pDC);
	virtual MRECT GetClientRect(MButton* pButton, MRECT& r);
	void SetWireLook(bool b) {	m_bWireLook = b;	}
	bool GetWireLook() const{	return m_bWireLook; }
public:
	MButtonLook():m_bWireLook(false){}
};


/// ¹öÆ° Å¬·¡½º
class MButton : public MWidget{
protected:
	bool		m_bMouseOver;
	bool		m_bLButtonDown;
	bool		m_bRButtonDown;
	MCOLOR		m_TextColor;
	bool		m_bShowText;
	MAlignmentMode	m_AlignmentMode;
	MButtonType		m_Type;
	bool		m_bChecked;				///< MBT_PUSHÀÎ °æ¿ì
	bool		m_bComboDropped;
	bool		m_bStretch;

	bool		m_bScaleOnHover;  // Cờ bật/tắt hiệu ứng phóng to
	bool		m_bIsScaled;      // Nút có đang bị phóng to hay không?
	MRECT		m_rcOriginal;     // Lưu lại tọa độ, kích thước gốc để khôi phục
	
	MButtonGroup	*m_pButtonGroup;	///< ¼ÓÇØÀÖ´Â ±×·ì
	int				m_nIndexInGroup;	///< °°Àº ±×·ì³»¿¡¼­ÀÇ index

public:
	bool		m_bEnableEnter;			///< Æ÷Ä¿½º ÀÖÀ»¶§ EnterÅ°·Î ¹öÆ° ´©¸£±â È°¼ºÈ­ ( Default : true )
	bool		m_bHighlight;			///< Highlight(&) Áö¿ø
	MBitmap*	m_pIcon;				///< ¾ÆÀÌÄÜ
	MButtonKeyAssigned	m_nKeyAssigned;	///< Key Assigned
	MMsgBox*	m_pMsgBox;				///< ¸Þ¼¼Áö ¹Ú½º¸¦ ÅëÇÑ È®ÀÎ ¸Þ¼¼Áö
	string		m_strIDLConfirmText;	///< xml¿¡ Á¤ÀÇµÈ È®ÀÎ ¸Þ½ÃÁö

	MPOINT		m_ClickPos;
	MPOINT		m_LDragStartClickPos;
	int			m_LDragVariationX;
	int			m_LDragVariationY;

private:
	//void DrawButton(MDrawContext* pDC);
protected:
	//virtual void OnDraw(MDrawContext* pDC);
	/// ¸¶¿ì½º°¡ ¹öÆ° ¾È¿¡ µé¾î¿Ã¶§ ¹ß»ýÇÏ´Â ÀÌº¥Æ® ÇÚµé·¯
	virtual void OnMouseIn(void);
	/// ¸¶¿ì½º°¡ ¹öÆ° ¹ÛÀ¸·Î ºüÁ® ³ª°¥¶§ ¹ß»ýÇÏ´Â ÀÌº¥Æ® ÇÚµé·¯
	virtual void OnMouseOut(void);
	/// ¹öÆ°ÀÌ ´­¸±¶§ ¹ß»ýÇÏ´Â ÀÌº¥Æ® ÇÚµé·¯
	virtual void OnButtonDown(void);
	/// ¹öÆ°ÀÌ ¶ç¾îÁú¶§ ÀÖÀ»¶§ ¹ß»ýÇÏ´Â ÀÌº¥Æ® ÇÚµé·¯
	virtual void OnButtonUp(void);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	/// Å¬¸¯µÇ¾îÁ³À»¶§
	virtual void OnButtonClick(void);	

	virtual bool OnShow(void);
	virtual void OnHide(void);
public:
	void SetScaleOnHover(bool bScale) { m_bScaleOnHover = bScale; }
	bool GetScaleOnHover() { return m_bScaleOnHover; }

	MButton(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MButton(void);
	/// ¹öÆ° ÅØ½ºÆ® ÄÃ·¯ ÁöÁ¤
	void SetTextColor(MCOLOR color);
	/// ¹öÆ° ÅØ½ºÆ® ÄÃ·¯ ¾ò±â
	MCOLOR GetTextColor(void);
	/// ¹öÆ°¿¡ ÅØ½ºÆ® º¸¿©ÁÖ±â
	void ShowText(bool bShow=true);
	virtual bool DefaultCommand(void);

	/// ¹öÆ°³» ÅØ½ºÆ® Á¤·Ä ¾ò±â
	MAlignmentMode GetAlignment(void);
	/// ¹öÆ°³» ÅØ½ºÆ® Á¤·Ä ÁöÁ¤
	MAlignmentMode SetAlignment(MAlignmentMode am);

	/// ¹öÆ° Å¸ÀÔ ÁöÁ¤
	void SetType(MButtonType t);
	/// ¹öÆ° Å¸ÀÔ ¾ò±â
	MButtonType GetType(void);

	/// Push ¹öÆ°ÀÎ °æ¿ì Ã¼Å©
	void SetCheck(bool bCheck);
	/// Push ¹öÆ°ÀÎ °æ¿ì Ã¼Å©°ª ¾ò±â
	bool GetCheck(void);

	/// ¹öÆ°ÀÌ ´­·ÁÁ® ÀÖ´Â°¡? ( ¸¶¿ì½º·Î ´©¸£´Â »óÅÂ¸¦ ¾ò¾î³¾ ¼ö ÀÖ´Ù. )
	bool IsButtonDown(void);	// Async Button Down ?
	/// ¸¶¿ì½º°¡ ¹öÆ° À§¿¡ ¿Ã·ÁÁ® ÀÖ´Â°¡?
	bool IsMouseOver(void);

	/// Confirm Message Box
	void SetConfirmMessageBox(const char* szMessage);			/// µðÆúÆ® È®ÀÎ ¸Þ½ÃÁö
	void SetAlterableConfirmMessage(const char* szMessage);		/// Æ¯¼öÇÑ °æ¿ì¿¡ È®ÀÎ ¸Þ½ÃÁö¸¦ ÇÑµ¿¾È ¹Ù²ãÄ¡±â ÇÒ ¼ö ÀÖ´Ù
	void RestoreIDLConfirmMessage();							/// È®ÀÎ ¸Þ½ÃÁö¸¦ ¹Ù²ãÄ¡±âÇÑ ÂÊ¿¡¼­ Ã¥ÀÓÁö°í º¹¿øÇØÁà¾ß ÇÑ´Ù~

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);

	int GetLDragVariationX() {
		return m_LDragVariationX;
	}

	int GetLDragVariationY() {
		return m_LDragVariationY;
	}

	DECLARE_LOOK(MButtonLook)
	DECLARE_LOOK_CLIENT()

	void SetComboDropped(bool b) {
		m_bComboDropped = b;
	}
	bool GetComboDropped() const{
		return m_bComboDropped;
	}

	void SetStretch( bool b) {
		m_bStretch = b;
	}

	bool GetStretch( ) const {
		return m_bStretch;
	}

	void SetButtonGroup(MButtonGroup *pGroup);

	int GetIndexInGroup() { 
		return m_nIndexInGroup; 
	}

	virtual void MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight);

#define MINT_BUTTON	"Button"
	virtual const char* GetClassName(void){ return MINT_BUTTON; }
};

#define MBTN_CLK_MSG		"click"	///< ¹öÆ°ÀÌ Å¬¸¯µÉ¶§ Listener¿¡°Ô ¹ß»ýµÇ´Â ¸Þ¼¼Áö
#define MBTN_RCLK_MSG		"rclick"///< R ¹öÆ°ÀÌ Å¬¸¯µÉ¶§ Listener¿¡°Ô ¹ß»ýµÇ´Â ¸Þ¼¼Áö
#define MBTN_DN_MSG			"down"	///< ¹öÆ°ÀÌ ´­¸±¶§
#define MBTN_UP_MSG			"up"	///< ¹öÆ°ÀÌ ¶ç¾îÁú¶§
#define MBTN_RDN_MSG		"rdown"	///< R¹öÆ°ÀÌ ´­¸±¶§
#define MBTN_RUP_MSG		"rup"	///< R¹öÆ°ÀÌ ¶ç¾îÁú¶§
#define MBTN_IN_MSG			"in"	///< ¹öÆ° À§¿¡ Ä¿¼­°¡ ¿Ã·Á Áú¶§
#define MBTN_OUT_MSG		"out"	///< ¹öÆ° À§¿¡ Ä¿¼­°¡ ºüÁú¶§ ¿Ã·Á Áú¶§


// ¹öÆ° ±×·ìÀ» ±¸ÇöÇÏ´Âµ¥ ÇÊ¿äÇÑ Å¬·¡½º
class MButtonGroup {
	friend MButton;
public:
	MButtonGroup();
	~MButtonGroup();

protected:
	int		m_nCount;
	MButton *m_pPrevious;
};

#endif