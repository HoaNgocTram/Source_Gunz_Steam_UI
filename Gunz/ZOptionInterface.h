#pragma once

#include "RTypes.h"

class ZOptionInterface
{

	int		mOldScreenWidth;
	int		mOldScreenHeight;

	RPIXELFORMAT	mnOldBpp;

	// Custom: Window mode
	bool		mbOldScreenWindowed;

	bool	mbTimer;
	DWORD	mTimerTime;


public:
	ZOptionInterface(void);
	virtual ~ZOptionInterface(void);

	void Update(void);

	void InitInterfaceOption(void);
	bool SaveInterfaceOption(void);

	void OptimizationVideoOption();

	// ∏ÆªÁ¿Ã¡Ó ∞¸∑√
	void ScaleOptionChildren(MWidget* pParent, float scale);
	void Resize(int w, int h);		/// ∏ÆªÁ¿Ã¡Ó∏¶ ≈Î«ÿ «ÿªÛµµø° ¿˚¿¿«œ¥¬ ¿Œ≈Õ∆‰¿ÃΩ∫∏¶ ±∏«ˆ«—¥Ÿ.
	bool ResizeWidgetRecursive( MWidget* pWidget/*, int w, int h*/);
	bool ResizeWidget(const char* szName, int w, int h);
	void AdjustMultipliedWidgetsManually();
	void ResizeDefaultFont( int newScreenHeight );

	void ShowResizeConfirmDialog( bool Resized );
	bool IsDiffScreenResolution();
	bool TestScreenResolution();
	void GetOldScreenResolution();
	bool SetTimer( bool b = false,  float time = 0.0f );

	// ≥◊∆Æøˆ≈© ∆˜∆Æ∫Ø∞Ê
	void ShowNetworkPortConfirmDialog();
	bool IsDiffNetworkPort();

	// Custom: Windowed Mode widget
	void ShowWindowConfirmDialog();
	bool IsDiffWindow();

	// æ◊º«≈∞ ¡ﬂ∫π »Æ¿Œ
	void OnActionKeySet(ZActionKey* pActionKey, int key);

protected:
	// Luu s?n con tr? d? d˘ng luÙn, khÙng c?n FindWidget n?a
	bool m_bWidgetsCached = false; // C? ki?m tra d„ luu cache chua

	MComboBox* m_pResCombo = nullptr;
	MComboBox* m_pCrosshairCombo = nullptr;
	MComboBox* m_pOutlineModeCombo = nullptr;
	MComboBox* m_pOutlineColorCombo = nullptr;
	MComboBox* m_pSwordTrailCombo = nullptr;
	MComboBox* m_BGMVolumeSlider = nullptr;
	MComboBox* m_EffectVolumeSlider = nullptr;
	MComboBox* m_VoiceVolumeSlider = nullptr;
	MComboBox* m_VideoGamma = nullptr;
	MComboBox* m_CharTexLevel = nullptr;
	MComboBox* m_MapTexLevel = nullptr;
	MComboBox* m_EffectLevel = nullptr;
	MComboBox* m_TextureFormat = nullptr;
	MComboBox* m_MovingPictureResolution = nullptr;
	MComboBox* m_MovingPictureFileSize = nullptr;
	MComboBox* m_LanguageSelectComboBox = nullptr;
	MComboBox* m_Reflection = nullptr;
	MComboBox* m_LightMap = nullptr;
	MComboBox* m_DynamicLight = nullptr;
	MComboBox* m_Shader = nullptr;
	MComboBox* m_WindowedMode = nullptr;
	MComboBox* m_Archetype = nullptr;
	MComboBox* m_WhiteMap = nullptr;
	MComboBox* m_Stencil = nullptr;
	MComboBox* m_ShotEffects = nullptr;
	MComboBox* m_BulletMarks = nullptr;
	MComboBox* m_SparkShot = nullptr;
	MComboBox* m_HitMaker = nullptr;
	MComboBox* m_KillCam = nullptr;
	MComboBox* m_Blood = nullptr;
	MComboBox* m_ScoreNew = nullptr;
	MComboBox* m_BulletBar = nullptr;
	MComboBox* m_NPCBar = nullptr;
	MComboBox* m_DynamicLoad = nullptr;
	MComboBox* m_NoFog = nullptr;
	MComboBox* m_CodeChat = nullptr;
	MComboBox* m_ExtraInfo = nullptr;
	MComboBox* m_NewBar = nullptr;
	MComboBox* m_RespawnEffect = nullptr;
	MComboBox* m_Visible = nullptr;
	MComboBox* m_LoginInstantSelect = nullptr;
	MComboBox* m_TrailColorCheck = nullptr;
	MComboBox* m_NoSmoke = nullptr;
	MComboBox* m_3DShotGun = nullptr;
	MComboBox* m_FloatDmgMode = nullptr;
	MComboBox* m_EmojiSetCombo = nullptr;
	MComboBox* m_KillFeed = nullptr;
	MComboBox* m_KillStreak = nullptr;
	MComboBox* m_LowPoly = nullptr;
	MComboBox* m_NewChatEnable = nullptr;
	MComboBox* m_FirstPerson = nullptr;
	MComboBox* m_PingColors = nullptr;
	MComboBox* m_NoHud = nullptr;
	MComboBox* m_PostureChar = nullptr;
	MComboBox* m_LatencyGraphic = nullptr;
	MComboBox* m_ChatBackGroundNormal = nullptr;
	MComboBox* m_DiscordOption = nullptr;
	MComboBox* m_VoiceChatToggle = nullptr;
	MComboBox* m_MultiSampling = nullptr;
	MComboBox* m_WeaponRotation = nullptr;
	MComboBox* m_FrameLimit_Display = nullptr;
	MComboBox* m_HPAPScoreboard = nullptr;
	MComboBox* m_ChatFilter = nullptr;
	MComboBox* m_BGMMute = nullptr;
	MComboBox* m_EffectMute = nullptr;
	MComboBox* m_Effect3D = nullptr;
	MComboBox* m_8BitSound = nullptr;
	MComboBox* m_16BitSound = nullptr;
	MComboBox* m_InverseSound = nullptr;
	MComboBox* m_HWMixing = nullptr;
	MComboBox* m_HitSound = nullptr;
	MComboBox* m_NarrationSound = nullptr;
	MComboBox* m_PlayInBackground = nullptr;
	MComboBox* m_InvertMouse = nullptr;
	MComboBox* m_InvertJoystick = nullptr;
	MComboBox* m_NetworkPort1 = nullptr;
	MComboBox* m_NetworkPort2 = nullptr;
	MComboBox* m_MouseSensitivityEdit = nullptr;
	MComboBox* m_BoostOption = nullptr;
	MComboBox* m_NormalChatOption = nullptr;
	MComboBox* m_TeamChatOption = nullptr;
	MComboBox* m_ClanChatOption = nullptr;
	MComboBox* m_WhisperOption = nullptr;
	MComboBox* m_InviteOption = nullptr;
	MComboBox* m_CrossHairPreviewCanvas = nullptr;
	MComboBox* m_CameraDistance = nullptr;
	MComboBox* m_ChatBackGroundOpacity = nullptr;
	MComboBox* m_CrossSize = nullptr;
	MComboBox* m_FrameLimit_PerSecond = nullptr;
	MComboBox* m_MouseSensitivitySlider = nullptr;
	MComboBox* m_JoystickSensitivitySlider = nullptr;

	void CacheWidgets();
};



///////////////////// ¿Ã«œ interface listener

// ø…º«
DECLARE_LISTENER(ZGetOptionGammaSliderChangeListener)
DECLARE_LISTENER(ZGetLoadDefaultKeySettingListener)
DECLARE_LISTENER(ZSetOptimizationListener)
DECLARE_LISTENER(ZGetSaveOptionButtonListener)
DECLARE_LISTENER(ZGetCancelOptionButtonListener)
DECLARE_LISTENER(ZGetGammaOption)
DECLARE_LISTENER(ZGetCancelResizeConfirmListener)
DECLARE_LISTENER(ZGetRequestResizeListener)
DECLARE_LISTENER(ZGetNetworkPortChangeRestartListener)
DECLARE_LISTENER(ZGetNetworkPortChangeCancelListener)
DECLARE_LISTENER(ZGetWindowChangeRestartListener)
DECLARE_LISTENER(ZGetWindowChangeCancelListener)
DECLARE_LISTENER(ZGetViewConfirmCancelListener)
DECLARE_LISTENER(ZGetViewConfrimAcceptListener)
DECLARE_LISTENER(ZGet8BitSoundListener)
DECLARE_LISTENER(ZGet16BitSoundListener)
DECLARE_LISTENER(ZGetBGMVolumeSizeSliderListener)
DECLARE_LISTENER(ZGetEffectVolumeSizeSliderListener)

// Custom: Volumen VoiceChat.
DECLARE_LISTENER(ZGetVoiceVolumeSizeSliderListener)

DECLARE_LISTENER(ZGetMouseSensitivitySliderListener)
DECLARE_LISTENER(ZGetMouseSensitivityEditListener)

DECLARE_LISTENER(ZGetOutlineModeComboBoxListener)
DECLARE_LISTENER(ZGetOutlineColorComboBoxListener)