#include "StdAfx.h"
#include "ZOptionInterface.h"
#include "MSlider.h"
#include "ZConfiguration.h"
#include "ZActionKey.h"
#include "FileInfo.h"
#include "ZCanvas.h"
#include "ZInput.h"
#include "ZRoomListBox.h"
#include "ZShopEquipListbox.h"
#include "ZShopEquipInterface.h"
#include "RFrameWork.h"

#include "RGMain.h"
#include "FunctionalListener.h"

#include "MDrawContext.h"


#include "ZCamera.h"
#include "EmojiManager.h"


#define DEFAULT_SLIDER_MAX			10000

#define DEFAULT_GAMMA_SLIDER_MIN	50
#define DEFAULT_GAMMA_SLIDER_MAX	800

#define LISTBOX_CELL_ARRANGE(pList, feldsize, flsize)   { pList->GetField(1)->nTabSize += (int)( (float)feldsize * flsize) - (int)( (float)feldsize);  }  // ±×¸²Ä­ Å©±â´Â °úÝ¤, ±×ºÎºÐ¸¸Å­ 2¹ø¿­¿¡ ´õÇÔ

static	map< int, D3DDISPLAYMODE> gDisplayMode;
#define DEFAULT_REFRESHRATE			0

template< class F, class S>
class value_equals
{
private:
	S second;
public:
	value_equals(const S& s) : second(s) {}
	bool operator() (pair<const F, S> elem)
	{ return elem.second == second; }
};

bool operator == ( D3DDISPLAYMODE lhs, D3DDISPLAYMODE rhs )
{
	return( lhs.Width == rhs.Width && lhs.Height == rhs.Height && lhs.Format == rhs.Format );
}

// Custom: Support 16:9 aspect ratios
// Custom: Deprecate 640x480
static int widths[]={ 800,1024,1280,1280,1366,1600, 1600, 1920, 2560 };
static int heights[]={ 600,768,720,960,768,1200, 900, 1080, 1440};

//static int widths[]={ 640,800,1024,1280,1600,1280,1440, 1650, 1920, 2560};
//static int heights[]={ 480,600,768,960,1200,800,900, 1050, 1200, 1600};

void ZOptionInterface::CacheWidgets()
{
	if (m_bWidgetsCached) return; // Nếu đã có rồi thì không làm lại nữa

	MIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pRes) return;

	// Chỉ tìm 1 lần duy nhất bằng tên (String Search) ở đây thôi
	m_pResCombo = (MComboBox*)pRes->FindWidget("ScreenResolution");
	m_pCrosshairCombo = (MComboBox*)pRes->FindWidget("CrossHairComboBox");
	m_pOutlineModeCombo = (MComboBox*)pRes->FindWidget("OutlineMode");
	m_pOutlineColorCombo = (MComboBox*)pRes->FindWidget("OutlineColor");
	m_pSwordTrailCombo = (MComboBox*)pRes->FindWidget("SwordTrail");
	m_MouseSensitivitySlider = (MComboBox*)pRes->FindWidget("MouseSensitivitySlider");
	m_JoystickSensitivitySlider = (MComboBox*)pRes->FindWidget("JoystickSensitivitySlider");
	m_BGMVolumeSlider = (MComboBox*)pRes->FindWidget("BGMVolumeSlider");
	m_EffectVolumeSlider = (MComboBox*)pRes->FindWidget("EffectVolumeSlider");
	m_VoiceVolumeSlider = (MComboBox*)pRes->FindWidget("VoiceVolumeSlider");
	m_VideoGamma = (MComboBox*)pRes->FindWidget("VideoGamma");
	m_CharTexLevel = (MComboBox*)pRes->FindWidget("CharTexLevel");
	m_MapTexLevel = (MComboBox*)pRes->FindWidget("MapTexLevel");
	m_EffectLevel = (MComboBox*)pRes->FindWidget("EffectLevel");
	m_TextureFormat = (MComboBox*)pRes->FindWidget("TextureFormat");
	m_MovingPictureResolution = (MComboBox*)pRes->FindWidget("MovingPictureResolution");
	m_MovingPictureFileSize = (MComboBox*)pRes->FindWidget("MovingPictureFileSize");
	m_LanguageSelectComboBox = (MComboBox*)pRes->FindWidget("LanguageSelectComboBox");
	m_Reflection = (MComboBox*)pRes->FindWidget("Reflection");
	m_LightMap = (MComboBox*)pRes->FindWidget("LightMap");
	m_DynamicLight = (MComboBox*)pRes->FindWidget("DynamicLight");
	m_Shader = (MComboBox*)pRes->FindWidget("Shader");
	m_WindowedMode = (MComboBox*)pRes->FindWidget("WindowedMode");
	m_Archetype = (MComboBox*)pRes->FindWidget("Archetype");
	m_WhiteMap = (MComboBox*)pRes->FindWidget("WhiteMap");
	m_Stencil = (MComboBox*)pRes->FindWidget("Stencil");
	m_ShotEffects = (MComboBox*)pRes->FindWidget("ShotEffects");
	m_BulletMarks = (MComboBox*)pRes->FindWidget("BulletMarks");
	m_SparkShot = (MComboBox*)pRes->FindWidget("SparkShot");
	m_HitMaker = (MComboBox*)pRes->FindWidget("HitMaker");
	m_KillCam = (MComboBox*)pRes->FindWidget("KillCam");
	m_Blood = (MComboBox*)pRes->FindWidget("Blood");
	m_ScoreNew = (MComboBox*)pRes->FindWidget("ScoreNew");
	m_BulletBar = (MComboBox*)pRes->FindWidget("BulletBar");
	m_NPCBar = (MComboBox*)pRes->FindWidget("NPCBar");
	m_DynamicLoad = (MComboBox*)pRes->FindWidget("DynamicLoad");
	m_NoFog = (MComboBox*)pRes->FindWidget("NoFog");
	m_CodeChat = (MComboBox*)pRes->FindWidget("CodeChat");
	m_ExtraInfo = (MComboBox*)pRes->FindWidget("ExtraInfo");
	m_NewBar = (MComboBox*)pRes->FindWidget("NewBar");
	m_RespawnEffect = (MComboBox*)pRes->FindWidget("RespawnEffect");
	m_Visible = (MComboBox*)pRes->FindWidget("Visible");
	m_LoginInstantSelect = (MComboBox*)pRes->FindWidget("LoginInstantSelect");
	m_TrailColorCheck = (MComboBox*)pRes->FindWidget("TrailColorCheck");
	m_NoSmoke = (MComboBox*)pRes->FindWidget("NoSmoke");
	m_3DShotGun = (MComboBox*)pRes->FindWidget("3DShotGun");
	m_FloatDmgMode = (MComboBox*)pRes->FindWidget("FloatDmgMode");
	m_EmojiSetCombo = (MComboBox*)pRes->FindWidget("EmojiSetCombo");
	m_KillFeed = (MComboBox*)pRes->FindWidget("KillFeed");
	m_KillStreak = (MComboBox*)pRes->FindWidget("KillStreak");
	m_LowPoly = (MComboBox*)pRes->FindWidget("LowPoly");
	m_NewChatEnable = (MComboBox*)pRes->FindWidget("NewChatEnable");
	m_FirstPerson = (MComboBox*)pRes->FindWidget("FirstPerson");
	m_PingColors = (MComboBox*)pRes->FindWidget("PingColors");
	m_NoHud = (MComboBox*)pRes->FindWidget("NoHud");
	m_PostureChar = (MComboBox*)pRes->FindWidget("PostureChar");
	m_LatencyGraphic = (MComboBox*)pRes->FindWidget("LatencyGraphic");
	m_ChatBackGroundNormal = (MComboBox*)pRes->FindWidget("ChatBackGroundNormal");
	m_DiscordOption = (MComboBox*)pRes->FindWidget("DiscordOption");
	m_VoiceChatToggle = (MComboBox*)pRes->FindWidget("VoiceChatToggle");
	m_MultiSampling = (MComboBox*)pRes->FindWidget("MultiSampling");
	m_WeaponRotation = (MComboBox*)pRes->FindWidget("WeaponRotation");
	m_FrameLimit_Display = (MComboBox*)pRes->FindWidget("FrameLimit_Display");
	m_HPAPScoreboard = (MComboBox*)pRes->FindWidget("HPAPScoreboard");
	m_ChatFilter = (MComboBox*)pRes->FindWidget("ChatFilter");
	m_BGMMute = (MComboBox*)pRes->FindWidget("BGMMute");
	m_EffectMute = (MComboBox*)pRes->FindWidget("EffectMute");
	m_Effect3D = (MComboBox*)pRes->FindWidget("Effect3D");
	m_8BitSound = (MComboBox*)pRes->FindWidget("8BitSound");
	m_16BitSound = (MComboBox*)pRes->FindWidget("16BitSound");
	m_InverseSound = (MComboBox*)pRes->FindWidget("InverseSound");
	m_HWMixing = (MComboBox*)pRes->FindWidget("HWMixing");
	m_HitSound = (MComboBox*)pRes->FindWidget("HitSound");
	m_NarrationSound = (MComboBox*)pRes->FindWidget("NarrationSound");
	m_PlayInBackground = (MComboBox*)pRes->FindWidget("PlayInBackground");
	m_InvertMouse = (MComboBox*)pRes->FindWidget("InvertMouse");
	m_InvertJoystick = (MComboBox*)pRes->FindWidget("InvertJoystick");
	m_NetworkPort1 = (MComboBox*)pRes->FindWidget("NetworkPort1");
	m_NetworkPort2 = (MComboBox*)pRes->FindWidget("NetworkPort2");
	m_BoostOption = (MComboBox*)pRes->FindWidget("BoostOption");
	m_NormalChatOption = (MComboBox*)pRes->FindWidget("NormalChatOption");
	m_TeamChatOption = (MComboBox*)pRes->FindWidget("TeamChatOption");
	m_ClanChatOption = (MComboBox*)pRes->FindWidget("ClanChatOption");
	m_WhisperOption = (MComboBox*)pRes->FindWidget("WhisperOption");
	m_InviteOption = (MComboBox*)pRes->FindWidget("InviteOption");
	m_CrossHairPreviewCanvas = (MComboBox*)pRes->FindWidget("CrossHairPreviewCanvas");
	m_CameraDistance = (MComboBox*)pRes->FindWidget("CameraDistance");
	m_ChatBackGroundOpacity = (MComboBox*)pRes->FindWidget("ChatBackGroundOpacity");
	m_CrossSize = (MComboBox*)pRes->FindWidget("CrossSize");
	m_FrameLimit_PerSecond = (MComboBox*)pRes->FindWidget("FrameLimit_PerSecond");

	m_bWidgetsCached = true;
	mlog("ZOptionInterface: Widgets Cached Successfully.\n");
}

ZOptionInterface::ZOptionInterface(void)
{
	mbTimer = false;

//	mOldScreenWidth = 0;
//	mOldScreenHeight = 0;
	mnOldBpp = D3DFMT_A8R8G8B8;
	mTimerTime = 0;

	mOldScreenWidth = 800;
	mOldScreenHeight = 600;

	// Custom: Window mode widget
	//mbOldScreenWindowed = RIsFullScreen();
}

ZOptionInterface::~ZOptionInterface(void)
{
	gDisplayMode.clear();
}
static int i = 0;
void ZOptionInterface::InitInterfaceOption(void)
{
	CacheWidgets(); //Luôn gọi hàm cache trước

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	mlog("start InitInterface option\n");
	/*
	// Mouse Sensitivity Min/Max (Z_MOUSE_SENSITIVITY_MIN ~ Z_MOUSE_SENSITIVITY_MAX)
	BEGIN_WIDGETLIST("MouseSensitivitySlider", pResource, MSlider*, pWidget);
	pWidget->SetMinMax(Z_MOUSE_SENSITIVITY_MIN, Z_MOUSE_SENSITIVITY_MAX);
	pWidget->SetValue(Z_MOUSE_SENSITIVITY);
	END_WIDGETLIST();
	*/

	// Close any open combobox dropdowns to prevent stuck state
	{
		const char* comboNames[] = {
			"WindowedMode", "ScreenResolution", "CharTexLevel", "MapTexLevel",
			"EffectLevel", "TextureFormat", "AntiAliasing",
			"SwordTrail", "TrailColorBox", "EmojiSetBox", "MovingPictureResolution",
			NULL
		};
		for (int i = 0; comboNames[i] != NULL; i++)
		{
			MComboBox* pCB = (MComboBox*)pResource->FindWidget(comboNames[i]);
			if (pCB)
				pCB->CloseComboBoxList();
		}
	}

	MSlider* pWidget = (MSlider*)pResource->FindWidget("MouseSensitivitySlider");
	if (pWidget)
	{
		pWidget->SetMinMax(MOUSE_SENSITIVITY_MIN, MOUSE_SENSITIVITY_MAX);
		pWidget->SetValue(ZGetConfiguration()->GetMouseSensitivityInInt());
	}

#ifdef _TRAILCOLOR
	MComboBox* pComboBoxTrail = (MComboBox*)pResource->FindWidget("TrailColorBox");
	if (pComboBoxTrail)
	{
		pComboBoxTrail->SetSelIndex(Z_ETC_TRAILBOXCOLOR);

		//GunzState state = ZApplication::GetGameInterface()->GetState();
		//if (state == GUNZ_GAME)
		//	pComboBoxTrail->Enable(false);
		//else
		//	pComboBoxTrail->Enable(true);

	}
#endif
	/*MSlider* pWidget = (MSlider*)pResource->FindWidget("MouseSensitivitySlider");
	if(pWidget)
	{
		pWidget->SetMinMax(MOUSE_SENSITIVITY_MIN, MOUSE_SENSITIVITY_MAX);
		pWidget->SetValue( ZGetConfiguration()->GetMouseSensitivityInInt());
	}*/
	pWidget = (MSlider*)pResource->FindWidget("JoystickSensitivitySlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(ZGetConfiguration()->GetJoystick()->fSensitivity * DEFAULT_SLIDER_MAX);
	}

	pWidget = (MSlider*)pResource->FindWidget("BGMVolumeSlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(Z_AUDIO_BGM_VOLUME*DEFAULT_SLIDER_MAX);
	}
	pWidget = (MSlider*)pResource->FindWidget("EffectVolumeSlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(Z_AUDIO_EFFECT_VOLUME*DEFAULT_SLIDER_MAX);
	}
	// Custom: Volumen VoiceChat.
	pWidget = (MSlider*)pResource->FindWidget("VoiceVolumeSlider");
	if (pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(Z_ETC_VOICE_VOLUME * DEFAULT_SLIDER_MAX);
	}
	pWidget = (MSlider*)pResource->FindWidget("VideoGamma");
	if(pWidget)
	{
		pWidget->SetMinMax(DEFAULT_GAMMA_SLIDER_MIN, DEFAULT_GAMMA_SLIDER_MAX);
		pWidget->SetValue(Z_VIDEO_GAMMA_VALUE*DEFAULT_GAMMA_SLIDER_MAX);
		pWidget->SetValue(Z_VIDEO_GAMMA_VALUE);
	}


	// Outline Mode ComboBox
	{
		MComboBox* pOutlineMode = (MComboBox*)pResource->FindWidget("OutlineMode");
		if (pOutlineMode)
		{
			pOutlineMode->RemoveAll();
			pOutlineMode->Add("Disable");
			pOutlineMode->Add("All");
			pOutlineMode->Add("Not Me");
			pOutlineMode->Add("Not My Team");
			int nMode = ZGetConfiguration()->GetVideo()->nOutlineMode;
			if (nMode < 0 || nMode > 4) nMode = 0;
			pOutlineMode->SetSelIndex(nMode);
		}
	}

	// Outline Color ComboBox
	{
		MComboBox* pOutlineColor = (MComboBox*)pResource->FindWidget("OutlineColor");
		if (pOutlineColor)
		{
			pOutlineColor->RemoveAll();
			pOutlineColor->Add("Black");
			pOutlineColor->Add("White");
			pOutlineColor->Add("Yellow");
			pOutlineColor->Add("Red");
			pOutlineColor->Add("Green");
			pOutlineColor->Add("Blue");
			pOutlineColor->Add("Orange");
			pOutlineColor->Add("Purple");
			pOutlineColor->Add("Pink");
			pOutlineColor->Add("Ocean");
			pOutlineColor->Add("Cyan");
			pOutlineColor->Add("Gold");

			static const DWORD s_OutlineColors[] = {
				0x00000000, // Black
				0x00FFFFFF, // White
				0x00FFFF00, // Yellow
				0x00FF0000, // Red
				0x0000FF00, // Green
				0x000000FF, // Blue
				0x00FF8000, // Orange
				0x00800080, // Purple
				0x00FF69B4, // Pink
				0x0000CED1, // Ocean
				0x0000FFFF, // Cyan
				0x00FFD700, // Gold
			};

			DWORD dwSaved = ZGetConfiguration()->GetVideo()->dwOutlineColor;
			int nSel = 0;
			int nColorCount = sizeof(s_OutlineColors) / sizeof(DWORD);
			for (int i = 0; i < nColorCount; i++) {
				if (s_OutlineColors[i] == dwSaved) { nSel = i; break; }
			}
			pOutlineColor->SetSelIndex(nSel);
		}
	}

	// Action Key
	for(int i=0; i<ZACTION_COUNT; i++){
		char szItemName[256];
		sprintf(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);

		BEGIN_WIDGETLIST(szItemName, pResource, ZActionKey*, pWidget);
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey);
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKeyAlt);
		END_WIDGETLIST();
	}

	//	ComboBox
	{
		MComboBox *pWidget = (MComboBox*)pResource->FindWidget("ScreenResolution");
		// Dùng static để lưu trạng thái, biến này sẽ không bị reset khi hàm kết thúc
		static bool bResolutionLoaded = false;

		if (pWidget)
		{
			// CHỈ CHẠY QUÉT DRIVER KHI DANH SÁCH TRỐNG HOẶC CHƯA TỪNG NẠP
			if (pWidget->GetCount() == 0 || !bResolutionLoaded)
			{
				pWidget->RemoveAll();
				gDisplayMode.clear();

				int dmIndex = 0;
				char szBuf[256];
				D3DDISPLAYMODE ddm;

				// Khai báo kiểu D3DFORMAT rõ ràng để tránh lỗi Incompatible type
				D3DFORMAT formats[2] = { D3DFMT_X8R8G8B8, D3DFMT_R5G6B5 };

				for (int i = 0; i < 2; i++)
				{
					// Kiểm tra hàm RGetAdapterModeCount của đại ca (nếu thiếu tham số thì thêm D3DADAPTER_DEFAULT)
					int nDM = RGetAdapterModeCount(formats[i]);

					for (int idm = 0; idm < nDM; ++idm)
					{
						if (REnumAdapterMode(D3DADAPTER_DEFAULT, formats[i], idm, &ddm))
						{
							// 1. Chặn độ phân giải thấp
							if (ddm.Width < 800 || ddm.Height < 600) continue;

							// 2. Chặn tỷ lệ 16:10 (Logic của đại ca)
							float fAspect = (float)ddm.Width / (float)ddm.Height;
							if (fAspect > 1.59f && fAspect < 1.61f) continue;

							ddm.RefreshRate = DEFAULT_REFRESHRATE;

							// 3. Kiểm tra trùng lặp (Dùng iterator chuẩn cũ để không lỗi Identifier)
							bool bDuplicate = false;
							for (map<int, D3DDISPLAYMODE>::iterator it = gDisplayMode.begin(); it != gDisplayMode.end(); ++it)
							{
								if (it->second.Width == ddm.Width &&
									it->second.Height == ddm.Height &&
									it->second.Format == ddm.Format)
								{
									bDuplicate = true;
									break;
								}
							}

							if (!bDuplicate)
							{
								gDisplayMode.insert(map<int, D3DDISPLAYMODE>::value_type(dmIndex++, ddm));
								sprintf(szBuf, "%d x %d %dbpp", ddm.Width, ddm.Height, (ddm.Format == D3DFMT_X8R8G8B8) ? 32 : 16);
								pWidget->Add(szBuf);
							}
						}
					}
				}

				// Trường hợp không tìm thấy mode nào (Fix lỗi màn hình đen)
				if (gDisplayMode.size() == 0)
				{
					int widths[] = { 800, 1024, 1280, 1280, 1366, 1600, 1600, 1920, 2560 };
					int heights[] = { 600, 768, 720, 960, 768, 1200, 900, 1080, 1440 };
					for (int i = 0; i < 10; ++i)
					{
						ddm.Width = widths[i / 2];
						ddm.Height = heights[i / 2];
						ddm.RefreshRate = DEFAULT_REFRESHRATE;
						ddm.Format = ((i % 2 == 1) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5);

						gDisplayMode.insert(map<int, D3DDISPLAYMODE>::value_type(i, ddm));
						sprintf(szBuf, "%dx%d  %d bpp", ddm.Width, ddm.Height, (i % 2 == 1) ? 32 : 16);
						pWidget->Add(szBuf);
					}
				}

				bResolutionLoaded = true; // Đánh dấu đã nạp xong
				mlog("ZOption: Resolution list cached successfully.\n");
			}

			// --- PHẦN LUÔN CHẠY ĐỂ CẬP NHẬT LỰA CHỌN (KHÔNG GÂY LAG) ---
			D3DDISPLAYMODE currentDDM;
			currentDDM.Width = RGetScreenWidth();
			currentDDM.Height = RGetScreenHeight();
			currentDDM.RefreshRate = DEFAULT_REFRESHRATE;
			currentDDM.Format = RGetPixelFormat();

			map<int, D3DDISPLAYMODE>::iterator iter = find_if(gDisplayMode.begin(), gDisplayMode.end(), value_equals<int, D3DDISPLAYMODE>(currentDDM));

			if (iter != gDisplayMode.end())
				pWidget->SetSelIndex(iter->first);
			else
				pWidget->SetSelIndex(0);
		}

		pWidget = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nCharTexLevel);
		}

		pWidget = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nMapTexLevel);
		}

		pWidget = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nEffectLevel);
		}

		pWidget = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nTextureFormat);
		}

		// µ¿¿µ»EÄ¸ÃÄ 20081017... by kam
		pWidget = (MComboBox*)pResource->FindWidget("MovingPictureResolution");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetMovingPicture()->iResolution); // µ¿¿µ»EÄ¸ÃÄ ÇØ»óµµ ¼¼ÆÃ
		}
		// µ¿¿µ»EÄ¸ÃÄ 20081028... by kam
		pWidget = (MComboBox*)pResource->FindWidget("MovingPictureFileSize");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetMovingPicture()->iFileSize); // µ¿¿µ»EÄ¸ÃÄ ÆÄÀÏÅ©±E¿E?¦Ç?
		}

		// ¾ð¾E¼±ÅÃ
		pWidget = (MComboBox*)pResource->FindWidget("LanguageSelectComboBox");
		if(pWidget)	
		{
			pWidget->RemoveAll();
			size_t size = ZGetConfiguration()->GetLocale()->vecSelectableLanguage.size();

			for (unsigned int i=0; i<size; ++i) {
				pWidget->Add( ZGetConfiguration()->GetLocale()->vecSelectableLanguage[i].strLanguageName.c_str());
			}

			pWidget->SetSelIndex( ZGetConfiguration()->GetSelectedLanguageIndex());

			GunzState state = ZApplication::GetGameInterface()->GetState();
			if (state == GUNZ_GAME || state == GUNZ_STAGE)
				pWidget->Enable(false);
			else
				pWidget->Enable(true);
		}
	}

	//	Button
	{
		MButton* pWidget = (MButton*)pResource->FindWidget("Reflection");
		if( pWidget )
		{
			pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bReflection);
		}

		pWidget = (MButton*)pResource->FindWidget("LightMap");
		if( pWidget )
		{
			//if(ZGetConfiguration()->GetVideo()->bTerrible && !RIsHardwareTNL() )
			{
				pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bLightMap);

				if(ZGetGame()) {
					ZGetGame()->GetWorld()->GetBsp()->LightMapOnOff(ZGetConfiguration()->GetVideo()->bLightMap);
				}
				else {
					RBspObject::SetDrawLightMap(ZGetConfiguration()->GetVideo()->bLightMap);
				}
			}
		}

		pWidget = (MButton*)pResource->FindWidget("DynamicLight");
		if( pWidget )
		{
			pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bDynamicLight);
		}

		pWidget = (MButton*)pResource->FindWidget("Shader");
		if( pWidget )
		{
			//if( !RShaderMgr::shader_enabled )
			if(!RIsSupportVS())
			{
				pWidget->SetCheck(false);
				pWidget->Enable(false);
			}
			else
			{
				pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bShader);
			}
		}

		// Custom: Windowed mode widget
		MComboBox* pNewWidget = (MComboBox*)pResource->FindWidget("WindowedMode");
		if( pNewWidget )
		{
			pNewWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nScreenType );
		}

		pWidget = (MButton*)pResource->FindWidget("Archetype");
		if( pWidget )
		{
			pWidget->SetCheck( ZGetConfiguration()->GetVideo()->bArchetype );
		}

		pWidget = (MButton*)pResource->FindWidget("WhiteMap");
		if( pWidget )
		{
			pWidget->SetCheck( ZGetConfiguration()->GetVideo()->bWhitemap );
			SetWhiteMap(ZGetConfiguration()->GetVideo()->bWhitemap);
		}

		// Custom: 24bit depth buffer option
		pWidget = (MButton*)pResource->FindWidget("Stencil");
		if (pWidget)
		{
			pWidget->SetCheck(Z_VIDEO_STENCILBUFFER);
		}

		pWidget = (MButton*)pResource->FindWidget("ShotEffects");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bShotEffects);
		}

		pWidget = (MButton*)pResource->FindWidget("BulletMarks");
		if (pWidget)
		{
			pWidget->SetCheck(Z_VIDEO_BULLET_MARKS);
		}
		pWidget = (MButton*)pResource->FindWidget("SparkShot");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_SPARK);
		}
		pWidget = (MButton*)pResource->FindWidget("HitMaker");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_HITMAKER);
		}
		pWidget = (MButton*)pResource->FindWidget("KillCam");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_KILLCAM);
		}
		pWidget = (MButton*)pResource->FindWidget("Blood");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_BLOOD);
		}
		pWidget = (MButton*)pResource->FindWidget("ScoreNew");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_SCORE);
		}
#ifdef _BULLETBAR
		pWidget = (MButton*)pResource->FindWidget("BulletBar");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_BULLETBAR);
		}
#endif

#ifdef _NPCBAR
		pWidget = (MButton*)pResource->FindWidget("NPCBar");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_NPCBAR);
		}
#endif

		/*pWidget = (MButton*)pResource->FindWidget("NewRenderer");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetEtc()->bNewRenderer);
		}*/

		/*pWidget = (MButton*)pResource->FindWidget("D3D9EX");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_D3D9E);
		}*/

#ifdef _DYNAMIC
		pWidget = (MButton*)pResource->FindWidget("DynamicLoad");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_DYNAMIC);
		}
#endif

		pWidget = (MButton*)pResource->FindWidget("NoFog");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_NOFOG);
		}
		pWidget = (MButton*)pResource->FindWidget("CodeChat");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_CODECHAT);
		}
		pWidget = (MButton*)pResource->FindWidget("ExtraInfo");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_EXTRA_SHOW);
		}
		pWidget = (MButton*)pResource->FindWidget("NewBar");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_NEWBAR);
		}
		pWidget = (MButton*)pResource->FindWidget("RespawnEffect");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_RT);
		}

		pWidget = (MButton*)pResource->FindWidget("Visible");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_VISIBLE);
		}
		pWidget = (MButton*)pResource->FindWidget("LoginInstantSelect");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetEtc()->bInstantLogin);
		}
#ifdef _TRAILCOLOR
		pWidget = (MButton*)pResource->FindWidget("TrailColorCheck");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetEtc()->bTrailCheck);

			GunzState state = ZApplication::GetGameInterface()->GetState();
			if (state == GUNZ_GAME)
				pWidget->Enable(false);
			else
				pWidget->Enable(true);

		}
#endif
		pWidget = (MButton*)pResource->FindWidget("NoSmoke");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_NOSMOKE);
		}
		pWidget = (MButton*)pResource->FindWidget("3DShotGun");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_3DSHOTGUN);
		}
#ifdef _FLOATDMG_MODE
		{
			MComboBox* pCombo = (MComboBox*)pResource->FindWidget("FloatDmgMode");
			if (pCombo)
			{
				pCombo->SetSelIndex(Z_FLOATDMG_MODE);
			}
		}
#endif
		{
			MComboBox* pCombo = (MComboBox*)pResource->FindWidget("EmojiSetCombo");
			if (pCombo)
			{
				pCombo->SetSelIndex(Z_ETC_EMOJISET);
			}
		}

#ifdef _KILLFEED
		pWidget = (MButton*)pResource->FindWidget("KillFeed");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_KILLFEED);
		}
#endif

		pWidget = (MButton*)pResource->FindWidget("KillStreak");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_KILL);
		}

		pWidget = (MButton*)pResource->FindWidget("LowPoly");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_LOWPOLY);
		}

		pWidget = (MButton*)pResource->FindWidget("NewChatEnable");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_CHATNEW);
		}

		pWidget = (MButton*)pResource->FindWidget("FirstPerson");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_PERSON);
		}
		pWidget = (MButton*)pResource->FindWidget("PingColors");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_COLORS);
		}

		pWidget = (MButton*)pResource->FindWidget("NoHud");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_HUD);
		}

		pWidget = (MButton*)pResource->FindWidget("PostureChar");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_POSTURECHAR);
		}

		pWidget = (MButton*)pResource->FindWidget("LatencyGraphic");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_LATENCY);
		}

#ifdef _CHATBACKGROUND
		pWidget = (MButton*)pResource->FindWidget("ChatBackGroundNormal");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_CHATBACKGROUND);
		}
#endif

		pWidget = (MButton*)pResource->FindWidget("DiscordOption");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_DISCORD);
		}

#ifdef _VOICE_CHAT
		pWidget = (MButton*)pResource->FindWidget("VoiceChatToggle");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ECT_VOICE);
		}
#endif
		// Custom: Rewrote SwordTrail ComboBox
		MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("SwordTrail");
		if( pComboBox )
		{
			pComboBox->SetSelIndex( ZGetConfiguration()->GetEtc()->nSwordTrail );
			RSetTrackSize(ZGetConfiguration()->GetEtc()->nSwordTrail);
		}
		pComboBox = (MComboBox*)pResource->FindWidget("MultiSampling");
		if (pComboBox)
		{
			pComboBox->SetSelIndex(Z_VIDEO_MULTISAMPLING);
		}
		pWidget = (MButton*)pResource->FindWidget("WeaponRotation");
		if( pWidget )
		{
			pWidget->SetCheck( ZGetConfiguration()->GetEtc()->bWeaponRotation );
		}

		pWidget = (MButton*)pResource->FindWidget("FrameLimit_Display");
		if( pWidget )
		{
			pWidget->SetCheck( ZGetConfiguration()->GetEtc()->bShowFPS );
		}

		pWidget = (MButton*)pResource->FindWidget("HPAPScoreboard");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetEtc()->bHPAPScoreboard);
		}
		pWidget = (MButton*)pResource->FindWidget("ChatFilter");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetEtc()->bChatFilter);
		}

		pWidget = (MButton*)pResource->FindWidget("BGMMute");
		if(pWidget)
		{
			pWidget->SetCheck( !ZGetConfiguration()->GetAudio()->bBGMMute );
		}

		pWidget = (MButton*)pResource->FindWidget("EffectMute");
		if(pWidget)
		{
			pWidget->SetCheck( !ZGetConfiguration()->GetAudio()->bEffectMute );
		}

		pWidget	= (MButton*)pResource->FindWidget("Effect3D");
		if(pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetAudio()->b3DSound);
		}

		pWidget = (MButton*)pResource->FindWidget("8BitSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_8BITSOUND);
			pWidget = (MButton*)pResource->FindWidget("16BitSound");
			if(pWidget) pWidget->SetCheck(!Z_AUDIO_8BITSOUND);
		}
		pWidget = (MButton*)pResource->FindWidget("InverseSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_INVERSE);
		}
		pWidget = (MButton*)pResource->FindWidget("HWMixing");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_HWMIXING);
		}
		pWidget = (MButton*)pResource->FindWidget("HitSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_HITSOUND);
		}
		pWidget = (MButton*)pResource->FindWidget("NarrationSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_NARRATIONSOUND);
		}
		pWidget = (MButton*)pResource->FindWidget("PlayInBackground");
		if (pWidget)
			pWidget->SetCheck(Z_AUDIO_PLAY_IN_BACKGROUND);
		pWidget = (MButton*)pResource->FindWidget("InvertMouse");
		if(pWidget)
		{
			pWidget->SetCheck(Z_MOUSE_INVERT);
		}
		pWidget = (MButton*)pResource->FindWidget("InvertJoystick");
		if(pWidget)
		{
			pWidget->SetCheck(Z_JOYSTICK_INVERT);
		}
		pWidget = (MButton*)pResource->FindWidget("HPAPScoreboard");
		/*if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_HPAP_SCOREBOARD);
		}*/
		{
			//MComboBox* pWidget = (MComboBox*)pResource->FindWidget("ChatBackgroundColorOption");
			//if (pWidget)
			//{
			//	pWidget->SetSelIndex(Z_CHAT_BACKGROUND_COLOR - 1);
			//}
		}
	}

	// Etc
	{	
		MEdit* pEdit = (MEdit*)pResource->FindWidget("NetworkPort1");
		if (pEdit)
		{
			char szBuf[64];
			sprintf(szBuf, "%d", Z_ETC_NETWORKPORT1);
			pEdit->SetText(szBuf);
		}

		pEdit = (MEdit*)pResource->FindWidget("NetworkPort2");
		if (pEdit)
		{
			char szBuf[64];
			sprintf(szBuf, "%d", Z_ETC_NETWORKPORT2);
			pEdit->SetText(szBuf);
		}

		pEdit = (MEdit*)pResource->FindWidget("MouseSensitivityEdit");
		if (pEdit)
		{
			char szBuf[1024];
			sprintf(szBuf, "%d", ZGetConfiguration()->GetMouseSensitivityInInt());
			pEdit->SetText(szBuf);
			pEdit->SetMaxLength(16);
		}

		MButton* pBtnBoost = (MButton*)pResource->FindWidget("BoostOption");
		if (pBtnBoost)
		{
			pBtnBoost->SetCheck(Z_ETC_BOOST);
		}

		MButton* pBtnNormalChat = (MButton*)pResource->FindWidget("NormalChatOption");
		if (pBtnNormalChat)
		{
			pBtnNormalChat->SetCheck( Z_ETC_REJECT_NORMALCHAT);
		}

		MButton* pBtnTeamChat = (MButton*)pResource->FindWidget("TeamChatOption");
		if (pBtnTeamChat)
		{
			pBtnTeamChat->SetCheck(Z_ETC_REJECT_TEAMCHAT);
		}

		MButton* pBtnClanChat = (MButton*)pResource->FindWidget("ClanChatOption");
		if (pBtnClanChat)
		{
			pBtnClanChat->SetCheck(Z_ETC_REJECT_CLANCHAT);
		}

		MButton* pBtnWhisper = (MButton*)pResource->FindWidget("WhisperOption");
		if (pBtnWhisper)
		{
			pBtnWhisper->SetCheck(Z_ETC_REJECT_WHISPER);
		}

		MButton* pBtnInvite = (MButton*)pResource->FindWidget("InviteOption");
		if (pBtnInvite)
		{
			pBtnInvite->SetCheck(Z_ETC_REJECT_INVITE);
		}

		MComboBox *pComboBox = (MComboBox*)pResource->FindWidget("CrossHairComboBox");
		if(pComboBox)
		{
			pComboBox->RemoveAll();

			// ±âº» Å©·Î½º ?EEÇÏµåÄÚµùÀ¸·Î ÀÔ·Â

			for (int i = 0; i < ZCSP_CUSTOM; i++)
			{
				char szText[256];
				sprintf(szText, "%s %d", ZMsg(MSG_WORD_TYPE), i+1);
				pComboBox->Add(szText);
			}
			char szCustomFile[256];
			sprintf(szCustomFile, "%s%s%s", PATH_CUSTOM_CROSSHAIR, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_TAILER);
			if (IsExist(szCustomFile)) pComboBox->Add("Custom");

			if (Z_ETC_CROSSHAIR >= pComboBox->GetCount())	// »ç¿EÚÁöÁ¤ÀÌ¿´´Âµ?»ç¿EÚÁöÁ¤À?¾ø¾ûÝ³À» °æ¿E
			{
				Z_ETC_CROSSHAIR = 0;
			}
			pComboBox->SetSelIndex(Z_ETC_CROSSHAIR);			
		}

		ZCanvas* pCrossHairPreview = (ZCanvas*)pResource->FindWidget("CrossHairPreviewCanvas");
		if (pCrossHairPreview)
		{
			pCrossHairPreview->SetOnDrawCallback(ZCrossHair::OnDrawOptionCrossHairPreview);
		}
#ifdef _CAMERADISTANCE
		pEdit = (MEdit*)pResource->FindWidget("CameraDistance");
		if (pEdit)
		{
			char szBuf[64];
			sprintf(szBuf, "%i", Z_ETC_CAMERA);
			pEdit->SetText(szBuf);
			pEdit->SetMaxLength(3);
			ZGetCamera()->RSetCameraDistance(Z_ETC_CAMERA);
		}
#endif
#ifdef _CHATBACKGROUND	
		pEdit = (MEdit*)pResource->FindWidget("ChatBackGroundOpacity");
		if (pEdit)
		{
			if (Z_ETC_CHATOPACITY < 1 || Z_ETC_CHATOPACITY > 100)
				Z_ETC_CHATOPACITY = 0;

			char szBuf[64];
			sprintf(szBuf, "%d", Z_ETC_CHATOPACITY);
			pEdit->SetText(szBuf);
		}
#endif

#ifdef _CROSS_SIZE_BOX
		pEdit = (MEdit*)pResource->FindWidget("CrossSize");
		if (pEdit)
		{
			if (Z_ETC_CROSS_SIZE < 50.0f || Z_ETC_CROSS_SIZE > 250.0f)
				Z_ETC_CROSS_SIZE = 50.0f;

			char szBuf[64];
			sprintf(szBuf, "%d", Z_ETC_CROSS_SIZE);
			pEdit->SetText(szBuf);
		}
#endif

#ifdef _CROSS_TRASP
		pEdit = (MEdit*)pResource->FindWidget("CrossTraspanrent");
		if (pEdit)
		{
			if (Z_ETC_CROSSTRASP < 1.f || Z_ETC_CROSSTRASP > 100.f)
				Z_ETC_CROSSTRASP = 1.f;

			char szBuf[64];
			sprintf(szBuf, "%d", Z_ETC_CROSSTRASP);
			pEdit->SetText(szBuf);
		}
#endif

		// Custom: FrameLimit_PerSecond is now an editable box
		pEdit = (MEdit*)pResource->FindWidget("FrameLimit_PerSecond");
		if (pEdit)
		{
			if (Z_ETC_FRAMELIMIT_PERSECOND < 60 || Z_ETC_FRAMELIMIT_PERSECOND > 1000)
				Z_ETC_FRAMELIMIT_PERSECOND = 0;

			char szBuf[64];
			sprintf(szBuf, "%d", Z_ETC_FRAMELIMIT_PERSECOND);
			pEdit->SetText(szBuf);
			RSetFrameLimitPerSeceond(Z_ETC_FRAMELIMIT_PERSECOND);

			//SetWidgetToolTipText("FrameLimit_PerSecond", "Acceptable values: 0 or 60~1000. Recommended: 250", MAM_RIGHT | MAM_TOP);
		}
#ifdef _INPUTFPS
		pComboBox = (MComboBox*)pResource->FindWidget("InputRate");
		if (pComboBox) {
			if (Z_ETC_INPUT >= pComboBox->GetCount())
			{
				Z_ETC_INPUT = 0;
			}
			pComboBox->SetSelIndex(Z_ETC_INPUT);
			RSetUpdateLimitPerSecond(Z_ETC_INPUT);
		}

#endif

	}

	//Macro
	{
		static char stemp_str[ZCONFIG_MACRO_MAX][80] = {
			"MacroF1",
			"MacroF2",
			"MacroF3",
			"MacroF4",
			"MacroF5",
			"MacroF6",
			"MacroF7",
			"MacroF8",
		};

		ZCONFIG_MACRO* pMacro = ZGetConfiguration()->GetMacro();

		if(pMacro) {
			MEdit* pEdit = NULL;
			for(int i=0;i<ZCONFIG_MACRO_MAX;i++) {

				pEdit = (MEdit*) pResource->FindWidget(stemp_str[i]);

				if (pEdit) {
					pEdit->SetText( pMacro->GetString(i) );
				}
			}
		}
	}

	{
		auto&& Cfg = *ZGetConfiguration();
		std::pair<const char*, bool> Buttons[] = {
			{"CamFixOption",              Cfg.GetCamFix()}, //Custom: FOV Camera Add By Desperate
			{ "ChatFontBoldOption",       Cfg.GetChat()->BoldFont },
			{ "ChatTimeStampOption",      Cfg.GetChat()->TimeStamp },
			{ "UnlockedDirOption",        Cfg.GetUnlockedDir()},
			//{ "ChatEnableCursorOption",   Cfg.GetChat()->EnableCursor },
			{"FastWeaponCycleOption",     Cfg.FastWeaponCycle},
			{ "", false },
		};

		for (auto&& ButtonInfo : Buttons)
		{
			auto ButtonWidget = ZFindWidgetAs<MButton>(ButtonInfo.first);
			if (ButtonWidget)
				ButtonWidget->SetCheck(ButtonInfo.second);
		}


		char StringFontSize[64];
		char StringFOV[64];
		sprintf_safe(StringFOV, "%.2f", Cfg.GetFOV()); //Custom: FOV Camera Add By Desperate
		if(Cfg.GetChat()->FontSize >= 20)
			sprintf_safe(StringFontSize, "%d", 20);
		else if(Cfg.GetChat()->FontSize <= 10)
			sprintf_safe(StringFontSize, "%d", 10);
		else
			sprintf_safe(StringFontSize, "%d", Cfg.GetChat()->FontSize);

		char StringBackgroundColor[64];
		sprintf_safe(StringBackgroundColor, "%08X", Cfg.GetChat()->BackgroundColor);

#ifdef _TRAILCOLOR	                                  
		char StringTrailColor[64];
		sprintf_safe(StringTrailColor, "%08X", Cfg.GetEtc()->bTrailColors);
#endif

		std::pair<const char*, const char*> Edits[] = {
			{ "FOVOption",                 StringFOV}, //Custom: FOV Camera Add By Desperate
			{ "ChatFontOption",            Cfg.GetChat()->Font.c_str() },
			{ "ChatFontSizeOption",         StringFontSize },
			{ "ChatBackgroundColorOption",	StringBackgroundColor},
			{ "TrailColor",	                StringTrailColor},
			{ "", false },
		};

		for (auto&& EditInfo : Edits)
		{
			auto EditWidget = ZFindWidgetAs<MEdit>(EditInfo.first);
			if (EditWidget)
				EditWidget->SetText(EditInfo.second);
		}
	}
	mlog("end of InitInterface option ok\n");
}

bool ZOptionInterface::SaveInterfaceOption(void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MDrawContext* pDC = Mint::GetInstance()->GetDrawContext();

#ifdef _TRAILCOLOR
	int bTrailColors = 0;
	MComboBox* pComboBoxTrail = (MComboBox*)pResource->FindWidget("TrailColorBox");
	if (pComboBoxTrail)
	{
		bTrailColors = pComboBoxTrail->GetSelIndex();
		if (Z_ETC_TRAILBOXCOLOR != bTrailColors)
		{
			Z_ETC_TRAILBOXCOLOR = bTrailColors;
		}
	}
#endif
	{ // ½½¶óÀÌ´E
		MSlider* pWidget = (MSlider*)pResource->FindWidget("MouseSensitivitySlider");
		Z_MOUSE_SENSITIVITY = (float) ((MSlider*)pWidget)->GetValue() / (float)MOUSE_SENSITIVITY_MAX;

		pWidget = (MSlider*)pResource->FindWidget("JoystickSensitivitySlider");
		Z_JOYSTICK_SENSITIVITY = (float) ((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;

		pWidget = (MSlider*)pResource->FindWidget("BGMVolumeSlider");
		if(pWidget)
		{
			Z_AUDIO_BGM_VOLUME = (float) ((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;
			ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME) ;
		}
		pWidget = (MSlider*)pResource->FindWidget("EffectVolumeSlider");
		if(pWidget)
		{
			Z_AUDIO_EFFECT_VOLUME = (float) ((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;

			ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
		}

		// Custom: Volumen VoiceChat.
		pWidget = (MSlider*)pResource->FindWidget("VoiceVolumeSlider");
		if (pWidget)
		{
			Z_ETC_VOICE_VOLUME = (float)((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;
			//ZGetSoundEngine()->SetEffectVolume(Z_ETC_VOICE_VOLUME);
		}

	}

	int i=0;

	//for(i=0; i<ZACTION_COUNT; i++) {
	//	ZGetInput()->UnregisterActionKey(i);
	//}

	// ¸ðµÎ Å¬¸®¾ûäÄ ÀçµûÓÏ
	ZGetInput()->ClearActionKey();

	for(i=0; i<ZACTION_COUNT; i++){
		char szItemName[256];
		sprintf(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);
		ZActionKey* pWidget = (ZActionKey*)pResource->FindWidget(szItemName);
		if(pWidget==NULL) continue;
		int nKey = 0;
		pWidget->GetActionKey(&nKey);
		//		Mint::GetInstance()->UnregisterActionKey(i);
		//		Mint::GetInstance()->RegisterActionKey(i, nKey);	// Å° µûÓÏ

//		ZGetInput()->UnregisterActionKey(i);
		ZGetInput()->RegisterActionKey(i,nKey);

		ZVIRTUALKEY altKey;
		pWidget->GetActionAltKey(&altKey);
		if(altKey!=-1)
			ZGetInput()->RegisterActionKey(i,altKey);

		// ZConfigurationÀ¸·Î ¿É¼Ç ÀúÀE
		ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey = nKey;
		ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKeyAlt = altKey;
	}

	// »EÅ°¸ÊÈÄ ¸ðµEÅ°»óÅÂ¸¦ release·Î ¼ÂÆÃ
	ZGetInput()->OffActionKeys();

	/*
	int nCnt[ZACTION_COUNT];

	for(i=0; i<ZACTION_COUNT; i++)
	{
		nCnt[i] = ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey;
	}
	*/

	{
		Z_VIDEO_WIDTH = RGetScreenWidth();
		Z_VIDEO_HEIGHT	= RGetScreenHeight();
		Z_VIDEO_FULLSCREEN	= RGetScreenType();
		Z_VIDEO_BPP	= RGetPixelFormat()==D3DFMT_X8R8G8B8 ? 32:16 ;

		MComboBox*	pWidget = (MComboBox*)pResource->FindWidget("CharTexLevel");

		int TexLevel = 0;
		DWORD flag = 0;
		int EffectLevel = 0;
		int nTextureFormat = 0;

		if(pWidget)	{

			TexLevel = pWidget->GetSelIndex();

			if( ZGetConfiguration()->GetVideo()->bTerrible ){
				ZGetConfiguration()->GetVideo()->nCharTexLevel = TexLevel;
				if( TexLevel == 2 )
					SetObjectTextureLevel(TexLevel+2);
				else
					SetObjectTextureLevel(TexLevel);

				flag |= RTextureType_Object;
			}
			else if( ZGetConfiguration()->GetVideo()->nCharTexLevel != TexLevel ) {
				ZGetConfiguration()->GetVideo()->nCharTexLevel = TexLevel;
				SetObjectTextureLevel(TexLevel);
				flag |= RTextureType_Object;
			}
		}

		pWidget = (MComboBox*)pResource->FindWidget("MapTexLevel");

		if(pWidget)	{

			TexLevel = pWidget->GetSelIndex();

			if( ZGetConfiguration()->GetVideo()->bTerrible ){
				ZGetConfiguration()->GetVideo()->nCharTexLevel = TexLevel;
				if( TexLevel == 2 )
					SetObjectTextureLevel(TexLevel+2);
				else
					SetObjectTextureLevel(TexLevel);

				flag |= RTextureType_Object;
			}
			if( ZGetConfiguration()->GetVideo()->nMapTexLevel != TexLevel ) {
				ZGetConfiguration()->GetVideo()->nMapTexLevel = TexLevel;

				SetMapTextureLevel(TexLevel);
				flag |= RTextureType_Map;
			}
		}
		// Custom: Archetype brought to scope
		{
			MButton* pWidget2 = (MButton*)pResource->FindWidget("Archetype");
			if( pWidget2 )
			{
				bool bTickedBefore = Z_VIDEO_ARCHETYPE;
				Z_VIDEO_ARCHETYPE = pWidget2->GetCheck();

				if (/*!bTickedBefore &&*/ Z_VIDEO_ARCHETYPE)
					SetMapTextureLevel(8);
				else
					SetMapTextureLevel(ZGetConfiguration()->GetVideo()->nMapTexLevel);

				if (bTickedBefore != Z_VIDEO_ARCHETYPE)
				{
					if (!(flag & RTextureType_Map))
						flag |= RTextureType_Map;
				}
			}
		}

		// Custom: Whitemaps
		{
			MButton* pWidget2 = (MButton*)pResource->FindWidget("WhiteMap");
			if( pWidget2 )
			{
				Z_VIDEO_WHITEMAP = pWidget2->GetCheck();
				if (Z_VIDEO_WHITEMAP)
					SetWhiteMap(true);
				else
					SetWhiteMap(false);
			}
		}

		pWidget = (MComboBox*)pResource->FindWidget("EffectLevel");

		if(pWidget)	{

			EffectLevel = pWidget->GetSelIndex();

			if( ZGetConfiguration()->GetVideo()->nEffectLevel != EffectLevel ) {
				ZGetConfiguration()->GetVideo()->nEffectLevel = EffectLevel;
				SetEffectLevel(EffectLevel);
				//				flag |= RTextureType_Map;
			}
		}

		pWidget = (MComboBox*)pResource->FindWidget("TextureFormat");

		if(pWidget)	{

			nTextureFormat = pWidget->GetSelIndex();

			if( ZGetConfiguration()->GetVideo()->nTextureFormat != nTextureFormat ) {
				ZGetConfiguration()->GetVideo()->nTextureFormat = nTextureFormat;
				SetTextureFormat(nTextureFormat);
				flag = RTextureType_All;
			}
		}
		if(flag) 
		{
			RChangeBaseTextureLevel(flag);
		}
	}
	{ // µ¿¿µ»EÄ¸ÃÄ 20081017... by kam
		MComboBox*	pWidget = (MComboBox*)pResource->FindWidget("MovingPictureResolution");
		int iResolution = 0;
		if(pWidget)	{
			iResolution = pWidget->GetSelIndex();
			ZGetConfiguration()->GetMovingPicture()->iResolution = iResolution;	// configuration¿¡ ¼¼ÆÃÇÑ´Ù.
			SetBandiCaptureConfig(iResolution);									// ¹ÝµðÄ¸Ã³ µ¿¿µ»EÇØ»óµµ¿¡ Àû¿EÑ´?
		}
		pWidget = (MComboBox*)pResource->FindWidget("MovingPictureFileSize");
		int iFileSize = 0;
		if(pWidget)	{
			iFileSize = pWidget->GetSelIndex();
			ZGetConfiguration()->GetMovingPicture()->iFileSize = iFileSize;	// configuration¿¡ ¼¼ÆÃÇÑ´Ù.
			SetBandiCaptureFileSize(iFileSize);								// ¹ÝµðÄ¸Ã³ µ¿¿µ»EÆÄÀÏÅ©±E¼¼ÆÃ¿¡ Àû¿EÑ´?
		}
	}
	{	// ¾ð¾ûØ±ÅÃ
#ifdef _MULTILANGUAGE
		MComboBox*	pWidget = (MComboBox*)pResource->FindWidget("LanguageSelectComboBox");
		if(pWidget)	{
			ZGetConfiguration()->SetSelectedLanguageIndex( pWidget->GetSelIndex());
		}
#endif //_MULTILANGUAGE
	}
	{
		MButton* pWidget = (MButton*)pResource->FindWidget("Reflection");
		if(pWidget)
		{
			Z_VIDEO_REFLECTION = pWidget->GetCheck();
		}

		pWidget = (MButton*)pResource->FindWidget("LightMap");
		if(pWidget)
		{
			Z_VIDEO_LIGHTMAP = pWidget->GetCheck();

			if(ZGetGame()) {
				ZGetGame()->GetWorld()->GetBsp()->LightMapOnOff(Z_VIDEO_LIGHTMAP);
			}
			else {
				RBspObject::SetDrawLightMap(Z_VIDEO_LIGHTMAP);
			}
		}

		pWidget = (MButton*)pResource->FindWidget("DynamicLight");
		if(pWidget)
		{
			Z_VIDEO_DYNAMICLIGHT = pWidget->GetCheck();
		}

		pWidget = (MButton*)pResource->FindWidget("Shader");
		if(pWidget)
		{
			Z_VIDEO_SHADER = pWidget->GetCheck();

			if( Z_VIDEO_SHADER )
			{
				RGetShaderMgr()->SetEnable();
			}
			else
			{
				RGetShaderMgr()->SetDisable();
			}
			//*/
		}

		// Custom: Windowed mode widget
		bool bResetDevice = false;
		MComboBox* pNewWidget = (MComboBox*)pResource->FindWidget("WindowedMode");
		if (pNewWidget)
		{
			int nNewScreenType = pNewWidget->GetSelIndex();

			// Kiểm tra xem có khác với giá trị hiện tại trong Config không
			if (nNewScreenType != ZGetConfiguration()->GetVideo()->nScreenType)
			{
				// Cập nhật ngay vào Config
				ZGetConfiguration()->GetVideo()->nScreenType = nNewScreenType;
				bResetDevice = true;
			}
		}

		MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("SwordTrail");
		if( pComboBox )
		{
			ZGetConfiguration()->GetEtc()->nSwordTrail = pComboBox->GetSelIndex();
			RSetTrackSize(ZGetConfiguration()->GetEtc()->nSwordTrail);
		}
		pWidget = (MButton*)pResource->FindWidget("WeaponRotation");
		if( pWidget )
		{
			ZGetConfiguration()->GetEtc()->bWeaponRotation = pWidget->GetCheck();
		}


		// Custom: 24bit depth buffer
		pWidget = (MButton*)pResource->FindWidget("Stencil");
		if (pWidget)
		{
			bool bStencil = pWidget->GetCheck();
			if (bStencil != Z_VIDEO_STENCILBUFFER)
			{
				Z_VIDEO_STENCILBUFFER = pWidget->GetCheck();
				bResetDevice = true;
			}
		}

		pWidget = (MButton*)pResource->FindWidget("ShotEffects");
		if (pWidget)
		{
			bool bOldVal = pWidget->GetCheck();
			if (bOldVal != ZGetConfiguration()->GetVideo()->bShotEffects)
			{
				ZGetConfiguration()->GetVideo()->bShotEffects = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("BulletMarks");
		if (pWidget)
		{
			bool bBulletMarks = pWidget->GetCheck();
			if (bBulletMarks != Z_VIDEO_BULLET_MARKS)
			{
				Z_VIDEO_BULLET_MARKS = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("SparkShot");
		if (pWidget)
		{
			bool bSparkShot = pWidget->GetCheck();
			if (bSparkShot != Z_ETC_SPARK)
			{
				Z_ETC_SPARK = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("NewBar");
		if (pWidget)
		{
			bool bNewBar = pWidget->GetCheck();
			if (bNewBar != Z_ETC_NEWBAR)
			{
				Z_ETC_NEWBAR = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("HitMaker");
		if (pWidget)
		{
			bool bHitMaker = pWidget->GetCheck();
			if (bHitMaker != Z_ECT_HITMAKER)
			{
				Z_ECT_HITMAKER = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("KillCam");
		if (pWidget)
		{
			bool bKillCam = pWidget->GetCheck();
			if (bKillCam != Z_ECT_KILLCAM)
			{
				Z_ECT_KILLCAM = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("Blood");
		if (pWidget)
		{
			bool bBlood = pWidget->GetCheck();
			if (bBlood != Z_ECT_BLOOD)
			{
				Z_ECT_BLOOD = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("ScoreNew");
		if (pWidget)
		{
			bool bScore = pWidget->GetCheck();
			if (bScore != Z_ECT_SCORE)
			{
				Z_ECT_SCORE = pWidget->GetCheck();
			}
		}
#ifdef _BULLETBAR
		pWidget = (MButton*)pResource->FindWidget("BulletBar");
		if (pWidget)
		{
			bool bBulletBar = pWidget->GetCheck();
			if (bBulletBar != Z_ECT_BULLETBAR)
			{
				Z_ECT_BULLETBAR = pWidget->GetCheck();
			}
		}
#endif

		pWidget = (MButton*)pResource->FindWidget("LoginInstantSelect");
		if (pWidget)
		{
			Z_ETC_INSTANT_LOGIN = pWidget->GetCheck();
		}

#ifdef _TRAILCOLOR
		pWidget = (MButton*)pResource->FindWidget("TrailColorCheck");
		if (pWidget)
		{
			Z_ETC_TRAILCHECK = pWidget->GetCheck();
		}
#endif

#ifdef _NPCBAR
		pWidget = (MButton*)pResource->FindWidget("NPCBar");
		if (pWidget)
		{
			bool bNPCbar = pWidget->GetCheck();
			if (bNPCbar != Z_ECT_NPCBAR)
			{
				Z_ECT_NPCBAR = pWidget->GetCheck();
			}
		}
#endif

		/*pWidget = (MButton*)pResource->FindWidget("NewRenderer");
		if (pWidget)
		{
			ZGetConfiguration()->GetEtc()->bNewRenderer = pWidget->GetCheck();
		}*/

#ifdef _DYNAMIC
		pWidget = (MButton*)pResource->FindWidget("DynamicLoad");
		if (pWidget)
		{
			bool bDynamicLoad = pWidget->GetCheck();
			if (bDynamicLoad != Z_ETC_DYNAMIC)
			{
				Z_ETC_DYNAMIC = pWidget->GetCheck();
			}
		}
#endif

		/*pWidget = (MButton*)pResource->FindWidget("D3D9EX");
		if (pWidget)
		{
			bool isEnabledD3D9Ex = pWidget->GetCheck();
			if (isEnabledD3D9Ex != Z_ETC_D3D9E)
			{
				OSVERSIONINFOEX os;
				os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
				GetVersionEx((OSVERSIONINFO*)&os);
				if (os.dwMajorVersion < 6)
					pWidget->Enable(false);
				Z_ETC_D3D9E = pWidget->GetCheck();
				ZApplication::GetGameInterface()->ShowMessage("Please restart the client for the changes to take effect.");
				pWidget->SetCheck(ZGetConfiguration()->GetEtc()->enableD3D9Ex);
			}
		}*/

		pWidget = (MButton*)pResource->FindWidget("NoFog");
		if (pWidget)
		{
			bool bNoFog = pWidget->GetCheck();
			if (bNoFog != Z_ECT_NOFOG)
			{
				Z_ECT_NOFOG = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("CodeChat");
		if (pWidget)
		{
			bool bCodeChat = pWidget->GetCheck();
			if (bCodeChat != Z_ECT_CODECHAT)
			{
				Z_ECT_CODECHAT = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("ExtraInfo");
		if (pWidget)
		{
			bool bExtraInfo = pWidget->GetCheck();
			if (bExtraInfo != Z_ECT_EXTRA_SHOW)
			{
				Z_ECT_EXTRA_SHOW = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("RespawnEffect");
		if (pWidget)
		{
			bool bRT = pWidget->GetCheck();
			if (bRT != Z_ECT_RT)
			{
				Z_ECT_RT = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("Visible");
		if (pWidget)
		{
			bool bVisible = pWidget->GetCheck();
			if (bVisible != Z_ECT_VISIBLE)
			{
				Z_ECT_VISIBLE = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("NoSmoke");
		if (pWidget)
		{
			bool bNoSmoke = pWidget->GetCheck();
			if (bNoSmoke != Z_ECT_NOSMOKE)
			{
				Z_ECT_NOSMOKE = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("3DShotGun");
		if (pWidget)
		{
			bool b3dShotgun = pWidget->GetCheck();
			if (b3dShotgun != Z_ECT_3DSHOTGUN)
			{
				Z_ECT_3DSHOTGUN = pWidget->GetCheck();
			}
		}
		pWidget = (MButton*)pResource->FindWidget("FirstPerson");
		if (pWidget)
		{
			bool bFirstPerson = pWidget->GetCheck();
			if (bFirstPerson != Z_ETC_PERSON)
			{
				Z_ETC_PERSON = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("PingColors");
		if (pWidget)
		{
			bool bPingColors = pWidget->GetCheck();
			if (bPingColors != Z_ETC_COLORS)
			{
				Z_ETC_COLORS = pWidget->GetCheck();
			}
		}
#ifdef _FLOATDMG_MODE
		{
			MComboBox* pCombo = (MComboBox*)pResource->FindWidget("FloatDmgMode");
			if (pCombo)
			{
				int nMode = pCombo->GetSelIndex();
				if (nMode != Z_FLOATDMG_MODE)
				{
					ZGetConfiguration()->GetEtc()->nFloatDmgMode = nMode;
				}
			}
		}
#endif
		{
			MComboBox* pCombo = (MComboBox*)pResource->FindWidget("EmojiSetCombo");
			if (pCombo)
			{
				int nSet = pCombo->GetSelIndex();
				if (nSet != Z_ETC_EMOJISET)
				{
					ZGetConfiguration()->GetEtc()->nEmojiSet = nSet;
					ZGetEmojiManager().SetActiveSet(nSet);
				}
			}
		}

#ifdef _KILLFEED
		pWidget = (MButton*)pResource->FindWidget("KillFeed");
		if (pWidget)
		{
			bool bKillFeed = pWidget->GetCheck();
			if (bKillFeed != Z_ETC_KILLFEED)
			{
				Z_ETC_KILLFEED = pWidget->GetCheck();
		}
	}
#endif
		pWidget = (MButton*)pResource->FindWidget("NewChatEnable");
		if (pWidget)
		{
			bool bNewChat = pWidget->GetCheck();
			if (bNewChat != Z_ETC_CHATNEW)
			{
				Z_ETC_CHATNEW = pWidget->GetCheck();
			}
		}
#ifdef _CHATBACKGROUND
		pWidget = (MButton*)pResource->FindWidget("ChatBackGroundNormal");
		if (pWidget)
		{
			bool bChatBackground = pWidget->GetCheck();
			if (bChatBackground != Z_ETC_CHATBACKGROUND)
			{
				Z_ETC_CHATBACKGROUND = pWidget->GetCheck();
			}
		}
#endif

		pWidget = (MButton*)pResource->FindWidget("DiscordOption");
		if (pWidget)
		{
			bool bDiscord = pWidget->GetCheck();
			if (bDiscord != Z_ETC_DISCORD)
			{
				ZApplication::GetGameInterface()->ShowMessage("Discord: Restart the game to make it work.");
				Z_ETC_DISCORD = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("KillStreak");
		if (pWidget)
		{
			bool bKill = pWidget->GetCheck();
			if (bKill != Z_ECT_KILL)
			{
				Z_ECT_KILL = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("LowPoly");
		if (pWidget)
		{
			bool bLowPoly = pWidget->GetCheck();
			if (bLowPoly != Z_ECT_LOWPOLY)
			{
				Z_ECT_LOWPOLY = pWidget->GetCheck();
			}
		}

#ifdef _VOICE_CHAT
		pWidget = (MButton*)pResource->FindWidget("VoiceChatToggle");
		if (pWidget)
		{
			bool bVoiceChat = pWidget->GetCheck();
			if (bVoiceChat != Z_ECT_VOICE)
			{
				Z_ECT_VOICE = pWidget->GetCheck();
			}
		}
#endif

		pWidget = (MButton*)pResource->FindWidget("LatencyGraphic");
		if (pWidget)
		{
			bool bLatencyGraphic = pWidget->GetCheck();
			if (bLatencyGraphic != Z_ETC_LATENCY)
			{
				Z_ETC_LATENCY = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("NoHud");
		if (pWidget)
		{
			bool bNoHud = pWidget->GetCheck();
			if (bNoHud != Z_ETC_HUD)
			{
				Z_ETC_HUD = pWidget->GetCheck();
			}
		}

		pWidget = (MButton*)pResource->FindWidget("PostureChar");
		if (pWidget)
		{
			bool bPosture = pWidget->GetCheck();
			if (bPosture != Z_ETC_POSTURECHAR)
			{
				Z_ETC_POSTURECHAR = pWidget->GetCheck();
			}
		}

		int nSamplingType = 0;
		pComboBox = (MComboBox*)pResource->FindWidget("MultiSampling");
		if (pComboBox)
		{
			nSamplingType = pComboBox->GetSelIndex();
			if (Z_VIDEO_MULTISAMPLING != nSamplingType)
			{
				Z_VIDEO_MULTISAMPLING = pComboBox->GetSelIndex();
				bResetDevice = true;
			}
		}

		//{
		//	pComboBox = (MComboBox*)pResource->FindWidget("ChatBackgroundColorOption");
		//	if (pWidget)
		//	{
		//		Z_CHAT_BACKGROUND_COLOR = pComboBox->GetSelIndex() + 1;
		//	}

		//	GetRGMain().GetChat().SetBackgroundColor();
		//}
		
		if (bResetDevice)
		{
			// Đảm bảo g_nScreenWidth lấy từ giá trị mới nhất mà người dùng vừa chọn trên UI
			// (Z_VIDEO_WIDTH thường là macro trỏ đến Config, hãy chắc chắn UI đã cập nhật nó)
			g_nScreenWidth = ZGetConfiguration()->GetVideo()->nWidth;
			g_nScreenHeight = ZGetConfiguration()->GetVideo()->nHeight;

			RMODEPARAMS Params = {
				g_nScreenWidth,
				g_nScreenHeight,
				ZGetConfiguration()->GetVideo()->nScreenType, // Bây giờ giá trị này đã mới
				RGetPixelFormat()
			};

			RAdvancedGraphics::bDepthBuffering = Z_VIDEO_STENCILBUFFER;
			RAdvancedGraphics::nMultiSampling = Z_VIDEO_MULTISAMPLING;

			RResetDevice(&Params);

			// QUAN TRỌNG: Sau khi Reset xong phải Save ngay để file XML được cập nhật
			ZGetConfiguration()->Save();
		}

		pWidget = (MButton*)pResource->FindWidget("FrameLimit_Display");
		if( pWidget )
		{
			ZGetConfiguration()->GetEtc()->bShowFPS = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("HPAPScoreboard");
		if (pWidget)
		{
			ZGetConfiguration()->GetEtc()->bHPAPScoreboard = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("ChatFilter");
		if (pWidget)
		{
			ZGetConfiguration()->GetEtc()->bChatFilter = pWidget->GetCheck();
		}
		// Custom: End widgets

		pWidget	= (MButton*)pResource->FindWidget("BGMMute");
		if( pWidget )
		{
			Z_AUDIO_BGM_MUTE	= !(pWidget->GetCheck());

			ZGetSoundEngine()->SetMusicMute( Z_AUDIO_BGM_MUTE );
		}
		pWidget	= (MButton*)pResource->FindWidget("EffectMute");
		if(pWidget)
		{
			Z_AUDIO_EFFECT_MUTE = !(pWidget->GetCheck());
			ZGetSoundEngine()->SetEffectMute( Z_AUDIO_EFFECT_MUTE );

		}
		// Custom: 3D shotgun sounds
		pWidget	= (MButton*)pResource->FindWidget("Effect3D");
		if(pWidget)
		{
			Z_AUDIO_3D_SOUND = pWidget->GetCheck();		
		
		}
		pWidget = (MButton*)pResource->FindWidget("8BitSound");
		if(pWidget)
		{
			Z_AUDIO_8BITSOUND = pWidget->GetCheck();
#ifdef _BIRDSOUND

#else
			ZGetSoundEngine()->SetSamplingBits(Z_AUDIO_8BITSOUND);
#endif
		}
		pWidget = (MButton*)pResource->FindWidget("InverseSound");
		if(pWidget)
		{
			Z_AUDIO_INVERSE = pWidget->GetCheck();
#ifdef _BIRDSOUND

#else
			ZGetSoundEngine()->SetInverseSound( Z_AUDIO_INVERSE );
#endif
		}
		pWidget = (MButton*)pResource->FindWidget("HWMixing");
		if(pWidget)
		{
			Z_AUDIO_HWMIXING = pWidget->GetCheck();
#ifdef _BIRDSOUND

#else
			ZGetSoundEngine()->Reset(g_hWnd, Z_AUDIO_HWMIXING);
#endif
		}
		pWidget = (MButton*)pResource->FindWidget("HitSound");
		if(pWidget)
		{
			Z_AUDIO_HITSOUND = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("NarrationSound");
		if(pWidget)
		{
			Z_AUDIO_NARRATIONSOUND = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("PlayInBackground");
		if (pWidget)
			Z_AUDIO_PLAY_IN_BACKGROUND = pWidget->GetCheck();
			extern bool g_bPlayMusicInBackground;
			g_bPlayMusicInBackground = Z_AUDIO_PLAY_IN_BACKGROUND;
		pWidget = (MButton*)pResource->FindWidget("InvertMouse");
		if(pWidget)
		{
			Z_MOUSE_INVERT = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("InvertJoystick");
		if(pWidget)
		{
			Z_JOYSTICK_INVERT = pWidget->GetCheck();
		}
		/*pWidget = (MButton*)pResource->FindWidget("HPAPScoreboard");
		if (pWidget)
		{
			pWidget->SetCheck(Z_ETC_HPAP_SCOREBOARD);
		}*/
		//MComboBox* MCombobox = (MComboBox*)pResource->FindWidget("ChatBackgroundColorOption");
		//if (MCombobox)
		//{
		//	Z_CHAT_BACKGROUND_COLOR = MCombobox->GetSelIndex() + 1;
		//}

		//GetRGMain().GetChat().SetBackgroundColor();
	}
	{	// Etc
		MEdit* pEdit = (MEdit*)pResource->FindWidget("NetworkPort1");
		if (pEdit)
		{
			int nPreviousPort = Z_ETC_NETWORKPORT1;
			Z_ETC_NETWORKPORT1 = atoi(pEdit->GetText());
		}

		pEdit = (MEdit*)pResource->FindWidget("NetworkPort2");
		if (pEdit)
		{
			int nPreviousPort = Z_ETC_NETWORKPORT2;
			Z_ETC_NETWORKPORT2 = atoi(pEdit->GetText());
		}

		MButton* pBoost = (MButton*)pResource->FindWidget("BoostOption");
		if(pBoost)
		{
			if (Z_ETC_BOOST != pBoost->GetCheck()) {
				Z_ETC_BOOST = pBoost->GetCheck();
				if (Z_ETC_BOOST)
					ZGetGameClient()->PriorityBoost(true);
				else
					ZGetGameClient()->PriorityBoost(false);
			}
		}

		MButton* pNormalChat = (MButton*)pResource->FindWidget("NormalChatOption");
		if(pNormalChat)
		{
			if (Z_ETC_REJECT_NORMALCHAT != pNormalChat->GetCheck()) {
				Z_ETC_REJECT_NORMALCHAT = pNormalChat->GetCheck();
				if (Z_ETC_REJECT_NORMALCHAT)
					ZGetGameClient()->SetRejectNormalChat(true);
				else
					ZGetGameClient()->SetRejectNormalChat(false);
			}
		}

		MButton* pTeamChat = (MButton*)pResource->FindWidget("TeamChatOption");
		if(pTeamChat)
		{
			if (Z_ETC_REJECT_TEAMCHAT != pTeamChat->GetCheck()) {
				Z_ETC_REJECT_TEAMCHAT = pTeamChat->GetCheck();
				if (Z_ETC_REJECT_TEAMCHAT)
					ZGetGameClient()->SetRejectTeamChat(true);
				else
					ZGetGameClient()->SetRejectTeamChat(false);
			}
		}

		MButton* pClanChat = (MButton*)pResource->FindWidget("ClanChatOption");
		if(pClanChat)
		{
			if (Z_ETC_REJECT_CLANCHAT != pClanChat->GetCheck()) {
				Z_ETC_REJECT_CLANCHAT = pClanChat->GetCheck();
				if (Z_ETC_REJECT_CLANCHAT)
					ZGetGameClient()->SetRejectClanChat(true);
				else
					ZGetGameClient()->SetRejectClanChat(false);
			}
		}

		MButton* pWhisper = (MButton*)pResource->FindWidget("WhisperOption");
		if(pWhisper)
		{
			if (Z_ETC_REJECT_WHISPER != pWhisper->GetCheck()) {
				Z_ETC_REJECT_WHISPER = pWhisper->GetCheck();
				if (Z_ETC_REJECT_WHISPER)
					ZGetGameClient()->SetRejectWhisper(true);
				else
					ZGetGameClient()->SetRejectWhisper(false);
				ZPostUserOption();
			}
		}

		MButton* pInvite = (MButton*)pResource->FindWidget("InviteOption");
		if(pInvite)
		{
			if (Z_ETC_REJECT_INVITE != pInvite->GetCheck()) {
				Z_ETC_REJECT_INVITE = pInvite->GetCheck();
				if (Z_ETC_REJECT_INVITE)
					ZGetGameClient()->SetRejectInvite(true);
				else
					ZGetGameClient()->SetRejectInvite(false);
				ZPostUserOption();
			}
		}

		MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("CrossHairComboBox");
		if (pComboBox)
		{
			Z_ETC_CROSSHAIR = pComboBox->GetSelIndex();
		}
#ifdef _CAMERADISTANCE
		pEdit = (MEdit*)pResource->FindWidget("CameraDistance");
		if (pEdit)
		{
			int nCameraDistance = Z_ETC_CAMERA;
			Z_ETC_CAMERA = atoi(pEdit->GetText());
			ZGetCamera()->RSetCameraDistance(atoi(pEdit->GetText()));
			ZGetCamera()->UpdateDist();
		}
#endif
#ifdef _CHATBACKGROUND
		pEdit = (MEdit*)pResource->FindWidget("ChatBackGroundOpacity");
		if (pEdit)
		{
			int nOpacity = atoi(pEdit->GetText());

			if (nOpacity < 1 || nOpacity > 100)
				nOpacity = 0;

			Z_ETC_CHATOPACITY = atoi(pEdit->GetText());
		}
#endif

#ifdef _CROSS_SIZE_BOX
		pEdit = (MEdit*)pResource->FindWidget("CrossSize");
		if (pEdit)
		{
			float nCross_Size = atoi(pEdit->GetText());

			if (nCross_Size < 50.0f || nCross_Size > 250.0f)
				nCross_Size = 50.0f;

			Z_ETC_CROSS_SIZE = atoi(pEdit->GetText());
		}
#endif

#ifdef _CROSS_TRASP
		pEdit = (MEdit*)pResource->FindWidget("CrossTraspanrent");
		if (pEdit)
		{
			int nCrossTrasp = atoi(pEdit->GetText());

			if (nCrossTrasp < 1.f || nCrossTrasp > 100.f)
				nCrossTrasp = 1.f;

			Z_ETC_CROSSTRASP = atoi(pEdit->GetText());
		}
#endif

		// Custom: FrameLimit_PerSecond is now an editable box
		pEdit = (MEdit*)pResource->FindWidget("FrameLimit_PerSecond");
		if (pEdit)
		{
			int nFrameLimit = atoi(pEdit->GetText());
			
			if (nFrameLimit < 60 || nFrameLimit > 1000)
				nFrameLimit = 0;

			Z_ETC_FRAMELIMIT_PERSECOND = atoi(pEdit->GetText());
			RSetFrameLimitPerSeceond(Z_ETC_FRAMELIMIT_PERSECOND);
		}
#ifdef _INPUTFPS
		pComboBox = (MComboBox*)pResource->FindWidget("InputRate");
		if (pComboBox)
		{
			Z_ETC_INPUT = pComboBox->GetSelIndex();
			RSetUpdateLimitPerSecond(Z_ETC_INPUT);
		}
#endif

		// Save Outline Mode
		pComboBox = (MComboBox*)pResource->FindWidget("OutlineMode");
		if (pComboBox)
			ZGetConfiguration()->GetVideo()->nOutlineMode = pComboBox->GetSelIndex();

		// Save Outline Color
		pComboBox = (MComboBox*)pResource->FindWidget("OutlineColor");
		if (pComboBox)
		{
			static const DWORD s_OutlineColors[] = {
				0x00000000, 0x00FFFFFF, 0x00FFFF00, 0x00FF0000,
				0x0000FF00, 0x000000FF, 0x00FF8000, 0x00800080,
				0x00FF69B4, 0x0000CED1, 0x0000FFFF, 0x00FFD700,
			};
			int nSel = pComboBox->GetSelIndex();
			if (nSel >= 0 && nSel < 12)
				ZGetConfiguration()->GetVideo()->dwOutlineColor = s_OutlineColors[nSel];
		}

		//pComboBox = (MComboBox*)pResource->FindWidget("WaterQuality");
		//if (pComboBox)
		//{
		//	int i = pComboBox->GetSelIndex();
		//	ZGetGame()->GetWorld()->GetWaters()->OnInvalidate();
		//	if (Z_ETC_WATERQUALITY != i)
		//	{
		//		Z_ETC_WATERQUALITY = i;
		//	}
		//	RSetWaterQuality(i);
		//	ZGetGame()->GetWorld()->GetWaters()->OnRestore();
		//}

		/*
		pComboBox = (MComboBox*)pResource->FindWidget("FrameLimit_PerSecond");
		if (pComboBox)
		{
			Z_ETC_FRAMELIMIT_PERSECOND = pComboBox->GetSelIndex();
			RSetFrameLimitPerSeceond(Z_ETC_FRAMELIMIT_PERSECOND);
		}
		*/
	}

	{
		auto&& Cfg = *ZGetConfiguration();

		const auto OldFont = Cfg.GetChat()->Font;
		const auto OldBoldFont = Cfg.GetChat()->BoldFont;
		const auto OldFontSize = Cfg.GetChat()->FontSize;


		std::pair<const char*, bool*> Buttons[] = {
			{ "CamFixOption",            &Cfg.bCamFix }, //Custom: FOV Camera Add By Desperate
			{ "ChatFontBoldOption",      &Cfg.GetChat()->BoldFont },
			{ "ChatTimeStampOption",     &Cfg.GetChat()->TimeStamp },
			{ "UnlockedDirOption",       &Cfg.UnlockedDir },
			//{ "ChatEnableCursorOption",  &Cfg.GetChat()->EnableCursor },
			{ "FastWeaponCycleOption",   &Cfg.FastWeaponCycle },
			{"",false},
		};

		for (auto&& ButtonInfo : Buttons)
		{
			auto ButtonWidget = ZFindWidgetAs<MButton>(ButtonInfo.first);
			if (ButtonWidget)
				*ButtonInfo.second = ButtonWidget->GetCheck();
		}

		using SetEditFunction = void(*)(const char*);
		std::pair<const char*, SetEditFunction> Edits[] = {
			{ "FOVOption",      [](const char* Value) { //Custom: FOV Camera Add By Desperate
				ZGetConfiguration()->FOV = atof(Value); } },
			{ "ChatFontOption", [](const char* Value) {
				ZGetConfiguration()->GetChat()->Font = Value; } },
			{ "ChatFontSizeOption", [](const char* Value) {
				ZGetConfiguration()->GetChat()->FontSize = StringToInt<int>(Value).value_or(15); } },
			{ "ChatBackgroundColorOption", [](const char* Value) {
				ZGetConfiguration()->GetChat()->BackgroundColor =
					StringToInt<u32, 16>(Value).value_or(0x80000000); } },
			{ "",false },
#ifdef _TRAILCOLOR
			{ "TrailColor", [](const char* Value) 
				{
				ZGetConfiguration()->GetEtc()->bTrailColors = StringToInt<u32, 16>(Value).value_or(0xFFFFFFFF);			
				} 
				},
			{ 
				"",false 
			},
#endif
		};

		for (auto&& EditInfo : Edits)
		{
			auto EditWidget = ZFindWidgetAs<MEdit>(EditInfo.first);
			if (EditWidget)
				EditInfo.second(EditWidget->GetText());
		}

		const auto& CurFont = Cfg.GetChat()->Font;
		const auto CurBoldFont = Cfg.GetChat()->BoldFont;
		auto CurFontSize = Cfg.GetChat()->FontSize;
		if (!iequals(OldFont, CurFont) || OldBoldFont != CurBoldFont)
		{
			GetRGMain().GetChat().SetFont(CurFont, CurBoldFont);
		}
		if (OldFontSize != CurFontSize)
		{
			if (CurFontSize >= 20)
				CurFontSize = 20;
			else if (CurFontSize <= 10)
				CurFontSize = 10;

			GetRGMain().GetChat().SetFontSize(CurFontSize);
		}
		GetRGMain().GetChat().SetBackgroundColor(Cfg.GetChat()->BackgroundColor);
		SetFOV(ToRadian(Cfg.FOV)); //Custom: FOV Camera Add By Desperate
	}
	{
		// Macro

		static char stemp_str[ZCONFIG_MACRO_MAX][80] = {
			"MacroF1",
			"MacroF2",
			"MacroF3",
			"MacroF4",
			"MacroF5",
			"MacroF6",
			"MacroF7",
			"MacroF8",
		};

		ZCONFIG_MACRO* pMacro = ZGetConfiguration()->GetMacro();

		if(pMacro) {

			MEdit* pEdit = NULL;

			for(int i=0;i<ZCONFIG_MACRO_MAX;i++) {

				pEdit = (MEdit*) pResource->FindWidget(stemp_str[i]);

				if (pEdit) {

					pMacro->SetString(i, (char*)pEdit->GetText());
				}
			}
		}

	}

	// °¨¸¶°ª ÀúÀE
	MSlider* pSlider = (MSlider*)pResource->FindWidget("VideoGamma");
	if (pSlider != NULL) 
	{
		Z_VIDEO_GAMMA_VALUE = pSlider->GetValue();
	}

	ZGetConfiguration()->Save( Z_LOCALE_XML_HEADER);

	return true;
}



void ZOptionInterface::ShowResizeConfirmDialog( bool Resized )
{
	if( Resized )
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("ViewConfirm");
		if(pWidget!= 0)
			pWidget->Show( true, true );
	}
	else
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("ResizeConfirm");
		if(pWidget!= 0)
			pWidget->Show( true, true );
	}
}

bool ZOptionInterface::SetTimer( bool b, float time /* = 0.f  */ )
{
	static DWORD DeadTime = 0;

	if( !b )
	{
		mbTimer = b;
		return false;
	}

	if( !mbTimer )
	{
		mTimerTime	= timeGetTime();
		mbTimer		= true;
		DeadTime		= time*1000;
	}

	if(( timeGetTime() - mTimerTime ) > DeadTime )
	{
		DeadTime = 0;
		mbTimer	= false;
		return true;
	}
	else
	{
		char szBuf[128];
		sprintf(szBuf, "%d", min(max( (10 - (int)(( timeGetTime() - mTimerTime ) * 0.001)),0),10));

		char szText[ 128];
		ZTransMsg( szText, MSG_BACKTOTHEPREV, 1, szBuf);

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MLabel* Countdown = (MLabel*)pResource->FindWidget( "ViewConfirm_CountDown" );
		if ( Countdown)
			Countdown->SetText( szText);
	}
	return false;
}

void ZOptionInterface::ShowNetworkPortConfirmDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("NetworkPortConfirm");
	if(pWidget!= 0) pWidget->Show( true, true );
}

bool ZOptionInterface::IsDiffNetworkPort()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	int nCurrPort = ntohs( ZGetGameClient()->GetSafeUDP()->GetLocalPort() );

	int nNewPort1, nNewPort2;

	MEdit* pEdit1 = (MEdit*)pResource->FindWidget("NetworkPort1");
	if ( pEdit1)
		nNewPort1 = atoi( pEdit1->GetText());
	else
		return false;


	MEdit* pEdit2 = (MEdit*)pResource->FindWidget("NetworkPort2");
	if ( pEdit2)
		nNewPort2 = atoi( pEdit2->GetText());
	else
		return false;


	if ( nNewPort1 > nNewPort2)
	{
		char szStr[ 25];
		itoa( Z_ETC_NETWORKPORT1, szStr, 10);
		pEdit1->SetText( szStr);
		itoa( Z_ETC_NETWORKPORT2, szStr, 10);
		pEdit2->SetText( szStr);

		return false;
	}


	if ( ( nNewPort1 != Z_ETC_NETWORKPORT1) || ( nNewPort2 != Z_ETC_NETWORKPORT2))
		return true;

	return false;
}

// Custom: Windowed mode widget
void ZOptionInterface::ShowWindowConfirmDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("WindowConfirm");

	if(pWidget!= 0) pWidget->Show( true, true );
}

bool ZOptionInterface::IsDiffWindow()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	int nScreenType = 0;

	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("WindowedMode");
	if (pComboBox)
		nScreenType = pComboBox->GetSelIndex();
	else
		return false;

	if (nScreenType != Z_VIDEO_FULLSCREEN)
	{
		Z_VIDEO_FULLSCREEN = nScreenType;
		return true;
		ZGetGameInterface()->ReserveResetApp(true);
	}

	return false;
}

void ZOptionInterface::OptimizationVideoOption()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pButton = 0;
	MComboBox* pCombo = 0;
	MLabel* pLabel = 0;

	ZGetConfiguration()->SetForceOptimization( true );

	if(!RIsHardwareTNL())
	{	
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(0); // 16 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(false);
		pButton = (MButton*)pResource->FindWidget("LightMap");
		if(pButton!=0) pButton->SetCheck(false);		
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(false);

		pCombo	= (MComboBox*)pResource->FindWidget("ScreenResolution");
		if( pCombo != 0)
		{
			D3DDISPLAYMODE ddm;
			// Custom: Deprecate 640x480
			ddm.Width = 800;//640;
			ddm.Height = 600;//480;
			ddm.Format = D3DFMT_R5G6B5;
			ddm.RefreshRate = DEFAULT_REFRESHRATE;
			map<int, D3DDISPLAYMODE>::iterator iter_ = find_if( gDisplayMode.begin(), gDisplayMode.end(), value_equals<int, D3DDISPLAYMODE>(ddm));
			if( iter_ != gDisplayMode.end() )
			{
				int n = iter_->first;
				pCombo->SetSelIndex( n );
			}
		}

		ZGetConfiguration()->GetVideo()->bTerrible = true;
		return;
	}	

	ZGetConfiguration()->GetVideo()->bTerrible = false;

	int nVMem = RGetApproxVMem() /1024 /1024;
	if( nVMem < 512 )
	{		
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 16 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(false);
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(false);		
	}
	else if( nVMem < 1024 )
	{
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // º¸ÅE
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // ³ª»Ý
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // º¸ÅE
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 16 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(false);
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(true);
	}
	else // nVMem > 64
	{
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(0); // º¸ÅE
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(0); // º¸ÅE
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(0); // ÁÁÀ½
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 32 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(true);		
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(true);
	}

	//pLabel = (MLabel*)pResource->FindWidget("Lightmap Label");
	//if(pLabel!=0) pLabel->SetTextColor(MCOLOR(64,64,64));
	pButton = (MButton*)pResource->FindWidget("LightMap");
	if(pButton!=0) {
		pButton->SetCheck(true);		
		//pButton->Enable(false);
	}

	if(RIsSupportVS())
	{
		pButton = (MButton*)pResource->FindWidget("Shader");
		if(pButton!=0) pButton->SetCheck(true);
	}
	else
	{		
		pButton = (MButton*)pResource->FindWidget("Shader");
		if(pButton!=0) pButton->SetCheck(false);
	}
}

bool ZOptionInterface::ResizeWidgetRecursive( MWidget* pWidget/*, int w, int h*/)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	//MWidget* pWidget = pResource->FindWidget(szName);
	if(pWidget==NULL) return false;
	int n = pWidget->GetChildCount();
	for( int i = 0; i < n; ++i)
	{
		MWidget* pChildWidget = pWidget->GetChild(i);
		ResizeWidgetRecursive( pChildWidget/*, w, h */);
	}

	// idl¿¡¼­ ÀÐ¾ûØ­ ÀûÀýÇÑ °ªÀ» °¡Áö°EÀÖ´Â À§Á¬ÀÌ¶ó¸E±×°ªÀ¸·Î ¸®»çÀÌÁûãÑ´Ù
	if(pWidget->GetIDLRect().w>0 && pWidget->GetIDLRect().h>0)	
	{
		// idl ¿¡¼­´Â 800x600 ±âÁØÀ¸·Î ±â¼úµÇ¾ûÜÖ´Ù
		const float tempWidth = ((float)RGetScreenWidth()) / 800;
		const float tempHeight = ((float)RGetScreenHeight()) / 600;

		MRECT r=pWidget->GetIDLRect();
		r.x*=tempWidth;
		r.w*=tempWidth;
		r.y*=tempHeight;
		r.h*=tempHeight;
		pWidget->SetBounds(r);
	}
	else 
	{
		const float tempWidth = ((float)RGetScreenWidth()) / mOldScreenWidth;
		const float tempHeight = ((float)RGetScreenHeight()) / mOldScreenHeight;
		MPOINT p = pWidget->GetPosition();
		p.Scale( tempWidth, tempHeight );
		pWidget->SetPosition( p );
		MRECT r=pWidget->GetRect();
		pWidget->SetSize( r.w*tempWidth, r.h*tempHeight );
	}
	return true;
}

void ZOptionInterface::AdjustMultipliedWidgetsManually()
{
	// ±âÁ¸ÇÔ¼E-> bool ZOptionInterface::ResizeWidget(const char* szName, int w, int h) ¿¡¼­ °¡Á®¿Â ÄÚµå´Ù
	// ÇÏµåÄÚµùµéÀÌ´Ù......

	int w = RGetScreenWidth();
	int h = RGetScreenHeight();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget;
	
	pWidget = pResource->FindWidget("Login");
	if (pWidget) {
		// Resize frame
		pWidget->SetSize( w, h);

		// Resize background image
		pWidget = pResource->FindWidget( "Login_BackgrdImg");
		if ( pWidget)
			pWidget->SetSize( w, h);

		// Reposition login frame
		pWidget = pResource->FindWidget( "LoginFrame");
		if ( pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();

			//jintriple3 ÇØ»óµµ¿¡ µû¸¥ ¼­¹E¼±ÅÃ Ã¢ À§Ä¡ Áß¾Ó¿¡¼­ ¾à°£ ¾Æ·¡·Î...
			/*	rect.x = (w / 2) - (rect.w / 2) + 5;
			rect.y = h - rect.h - 10;
			*/
			rect.x = (w / 2) - (rect.w / 2) + 5;
			rect.y = (int)((float)h * 0.555f);
			if( rect.h + rect.y > h )
				rect.y = h - rect.h - 10;
			pWidget->SetBounds( rect);
		}

		// Reposition connecting message
		pWidget = pResource->FindWidget( "Login_ConnectingMsg");
		if ( pWidget)
		{
			
			/*MRECT rect;
			rect = pWidget->GetRect();
			rect.x = 0;
			rect.y = (int)(h * 0.66f);
			rect.w = w;
			pWidget->SetBounds( rect);
			*/
		}
	}

	/*pWidget = pResource->FindWidget("Shop");
	if (pWidget) {
		//_AdjustMultipliedItemListManually("AllEquipmentList", "Shop_ListLabel");
		//_AdjustMultipliedItemListManually("MyAllEquipmentList", "Shop_ListLabel");
		//_AdjustMultipliedItemListManually("CashEquipmentList", "Shop_ListLabel");

		ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList( "Shop", false);
	}

	pWidget = pResource->FindWidget("Equipment");
	if (pWidget) {
		//_AdjustMultipliedItemListManually("EquipmentList", "Equip_ListLabel1");
		//_AdjustMultipliedItemListManually("AccountItemList", "Equip_ListLabel2");

		ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList( "Equip", true);
	}*/
	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList( NULL, false);

/*
	pWidget = pResource->FindWidget("Replay_FileList");
	if (pWidget) {
		enum { LF_FILENAME, LF_VERSION, LF_MAX };
		MListBox* pReplayListBox = (MListBox*)pWidget;
		MRECT rc = pReplayListBox->GetClientRect();
		if (pReplayListBox->GetFieldCount() == LF_MAX) {
			pReplayListBox->GetField(LF_FILENAME)->nTabSize = rc.0.75f *
		} else {
			ASSERT(0);
		}
	}*/

	pWidget = pResource->FindWidget("Stage");
	if (pWidget) {
		ZApplication::GetStageInterface()->GetSacrificeItemBoxPos();
		ZApplication::GetStageInterface()->SetRelayMapBoxPos(0);
	}

	pWidget = pResource->FindWidget("Lobby");
	if (pWidget) {
		ZRoomListBox* pRoomList;

		pRoomList = (ZRoomListBox*)pResource->FindWidget( "Lobby_StageList" );
		if( pRoomList != 0 )
		{
			const float tempWidth = ((float)RGetScreenWidth()) / mOldScreenWidth;
			const float tempHeight = ((float)RGetScreenHeight()) / mOldScreenHeight;
			pRoomList->Resize( tempWidth, tempHeight );
		}
	}

	pWidget = pResource->FindWidget("CharCreation");
	if (pWidget) {
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = 50 * ( RGetScreenWidth() / 800.0f);
		rect.y = (int)((RGetScreenHeight() - rect.h) / 2.0f);
		pWidget->SetBounds( rect);
	}

	pWidget = pResource->FindWidget("CombatDTInfo");
	if (pWidget) {

	}
/*
	else if(stricmp( szName, "CombatDTInfo")==0)
		ResizeWidgetRecursive( pWidget, w, h );
	else if(stricmp( szName, "CombatDT_CharacterInfo")==0)
		ResizeWidgetRecursive( pWidget, w, h );
	*/

	pWidget = pResource->FindWidget("BuyItemDetailFrame_Thumbnail");
	if (pWidget) {	// ÀÌ ½æ³×ÀÏ ÀÌ¹ÌÁö´Â ¿ÍÀÌµEÇØ»óµµ¿¡¼­µµ Á¤»ç°¢?E¸·?º¸ÀÌµµ·Ï Á¶Á¤
		MRECT rc = pWidget->GetRect();
		rc.h = rc.w;
		pWidget->SetBounds(rc);
	}
}

extern MFontR2* g_pDefFont;
void ZOptionInterface::ResizeDefaultFont(int newScreenHeight)
{
	float fontResizeRatio = newScreenHeight / 600.f;

#ifdef _FONTSIZE
	if (!RGetIsHDScreen() && RGetWidthScreen() != 0.8f)
		fontResizeRatio *= 0.90f;
	else if (!RGetIsHDScreen() && RGetWidthScreen() == 0.8f)
		fontResizeRatio *= 0.80f;
#else
	return;
#endif // 

	g_pDefFont->Destroy();

	int newFontHeight = (int)(DEFAULT_FONT_HEIGHT * fontResizeRatio + 0.5f);	//UIÀÛ¼º½Ã ±âÁØÇØ»óµµ°¡ 800*600ÀÌ¹Ç·Î 600¿¡ ´ëÇØ °è»ê
	if (newFontHeight < FONT_MINIMUM_HEIGHT)
		newFontHeight = FONT_MINIMUM_HEIGHT;	// 640*480¿¡¼­ ³Ê¹« ¹¶°³ÁöÁö ¾Ê°Ô

	if (!g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, newFontHeight, 1.0f))
	{
		mlog("Fail to Recreate default font : MFontR2 / screen resize\n");
		g_pDefFont->Destroy();
		SAFE_DELETE(g_pDefFont);
	}

	// UI¿¡¼­ ·ÎµùµÈ ÆùÆ®µé Å©±â º¯°æ
	MFontManager::Resize(fontResizeRatio, FONT_MINIMUM_HEIGHT); //¿ø·¡ Å©±â(800*600±âÁØUI)¿¡ ´ëÇØ¼­ ¸î¹èÀ²·Î º¯È­ÇØ¾ßÇÏ´ÂÁö¸¦ ¾Ë·ÁÁÜ
}

void ZOptionInterface::ScaleOptionChildren(MWidget* pParent, float scale)
{
	if (!pParent) return;

	int n = pParent->GetChildCount();
	for (int i = 0; i < n; ++i)
	{
		MWidget* pChild = pParent->GetChild(i);
		if (!pChild) continue;

		MRECT idlRect = pChild->GetIDLRect();
		if (idlRect.w > 0 && idlRect.h > 0)
		{
			MRECT r;
			r.x = (int)(idlRect.x * scale + 0.5f);
			r.y = (int)(idlRect.y * scale + 0.5f);
			r.w = (int)(idlRect.w * scale + 0.5f);
			r.h = (int)(idlRect.h * scale + 0.5f);
			pChild->SetBounds(r);
		}

		// Recurse vào children
		ScaleOptionChildren(pChild, scale);
	}
}

void ZOptionInterface::Resize(int w, int h)
{
	ResizeDefaultFont(h);
	// ÇØ»óµµ º¯°æ½Ã À§Á¬µéÀÇ Å©±â¸¦ ¹èÀ²ÇØÁØ´Ù
	if (mOldScreenHeight == 0) mOldScreenHeight = 600;
	if (mOldScreenWidth == 0) mOldScreenWidth = 800;

	ZGetGameInterface()->MultiplySize(w/800.f, h/600.f, w/float(mOldScreenWidth), h/float(mOldScreenHeight));

	AdjustMultipliedWidgetsManually();	// ¹èÀ²Àû¿EÏ°úÏª¼?Á÷Á¢ ¸Å¸¸Á®Áà¾ß ÇÏ´Â ºÎºÐµE

	ZGetGameInterface()->SetSize(w, h);

	if (ZGetCombatInterface()) ZGetCombatInterface()->Resize(w, h);

	GetRGMain().Resize(w, h);
	// Custom: If ZOptionInterface is resized (anytime res is changed), update camera distance automatically.
	/*if (ZGetGameInterface() && !ZGetGameInterface()->GetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
	{
		ZCamera* pCamera = ZGetGameInterface()->GetCamera();
		if (pCamera)
			pCamera->AdjustDist();
	}*/

	// reloadUI === Auto center & scale Option window ===
	MWidget* pOption = ZGetGameInterface()->GetIDLResource()->FindWidget("OptionGroup");
	if (pOption)
	{
		// Scale OptionGroup theo resolution, base 800x600
		float scaleX = (float)w / 800.0f;
		float scaleY = (float)h / 600.0f;
		float scale = min(scaleX, scaleY); // uniform scale, fit cả 2 chiều

		int optW = (int)(720 * scale);  // base OptionGroup W=720
		int optH = (int)(540 * scale);  // base OptionGroup H=540

		// Clamp: không vượt quá screen
		if (optW > w - 20) optW = w - 20;
		if (optH > h - 40) optH = h - 40;

		pOption->SetSize(optW, optH);
		pOption->SetPosition((w - optW) / 2, (h - optH) / 2);

		// Scale tất cả widget con bên trong OptionGroup
		ScaleOptionChildren(pOption, scale);
	}
}

void ZOptionInterface::GetOldScreenResolution()
{
	RMODEPARAMS ModeParams;
	ModeParams.nWidth	= mOldScreenWidth;
	ModeParams.nHeight	= mOldScreenHeight;
	ModeParams.nScreenType	= ZGetConfiguration()->GetVideo()->nScreenType;
	ModeParams.PixelFormat	= mnOldBpp;

	mOldScreenWidth = RGetScreenWidth();
	mOldScreenHeight = RGetScreenHeight();
	mnOldBpp = RGetPixelFormat();

	RResetDevice( &ModeParams );
	Mint::GetInstance()->SetWorkspaceSize(ModeParams.nWidth, ModeParams.nHeight);
	Mint::GetInstance()->GetMainFrame()->SetSize(ModeParams.nWidth, ModeParams.nHeight);
	Resize(ModeParams.nWidth, ModeParams.nHeight);

	D3DDISPLAYMODE ddm;
	ddm.Width = ModeParams.nWidth;
	ddm.Height = ModeParams.nHeight;
	ddm.Format = ModeParams.PixelFormat;
	ddm.RefreshRate = DEFAULT_REFRESHRATE;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox *pWidget = (MComboBox*)pResource->FindWidget( "ScreenResolution" );

	map<int, D3DDISPLAYMODE>::iterator  iter = find_if( gDisplayMode.begin(), gDisplayMode.end(), value_equals<int, D3DDISPLAYMODE>(ddm));
	if( iter != gDisplayMode.end() )
		pWidget->SetSelIndex( iter->first );
}

bool ZOptionInterface::IsDiffScreenResolution()
{
	//if (ZGetConfiguration()->GetVideo()->nScreenType == 2)
	//	return false;//the resolution never changes
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MComboBox *pWidget = (MComboBox*)pResource->FindWidget("ScreenResolution");
	if(pWidget)
	{
		int nSel = pWidget->GetSelIndex();
		D3DDISPLAYMODE ddm = (gDisplayMode.find( nSel ))->second;

		// Custom: Window mode
		if( ddm.Width == RGetScreenWidth() && ddm.Height == RGetScreenHeight() && ddm.Format == RGetPixelFormat() /*&& mbOldScreenWindowed == RIsFullScreen()*/ )
			return false;
#ifdef _DEBUG
		mlog( "%d/%d , %d/%d, %d/%d\n", ddm.Width, RGetScreenWidth(), ddm.Height, RGetScreenHeight(), ddm.Format == D3DFMT_X8R8G8B8 ? 32 : 16, RGetPixelFormat() == D3DFMT_X8R8G8B8 ? 32 : 16 );
#endif
	}
	return true;
}

bool ZOptionInterface::TestScreenResolution()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MComboBox *pWidget = (MComboBox*)pResource->FindWidget("ScreenResolution");
	if(pWidget)
	{
		RMODEPARAMS	ModeParams;

		map<int, D3DDISPLAYMODE>::iterator iter = gDisplayMode.find( pWidget->GetSelIndex() );
		if( iter == gDisplayMode.end() )
		{
			mlog("¼±ÅÃÇÑ ÇØ»óµµ°¡ Á¸ÀçÇÏÁE¾Ê¾Æ¼­ ÇØ»óµµ º¯°æ¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù..\n" );
			return false;
		}

		D3DDISPLAYMODE ddm = iter->second;

		mOldScreenWidth	= RGetScreenWidth();
		mOldScreenHeight	= RGetScreenHeight();
		mnOldBpp				= RGetPixelFormat();

		ModeParams.nWidth = ddm.Width;
		ModeParams.nHeight = ddm.Height;
		ModeParams.nScreenType = ZGetConfiguration()->GetVideo()->nScreenType;
		ModeParams.PixelFormat = ddm.Format;

		RResetDevice(&ModeParams);

		Mint::GetInstance()->SetWorkspaceSize(ModeParams.nWidth, ModeParams.nHeight);
		Mint::GetInstance()->GetMainFrame()->SetSize(ModeParams.nWidth, ModeParams.nHeight);
		Resize(ModeParams.nWidth, ModeParams.nHeight);
	}
	return true;
}

void ZOptionInterface::Update()
{
	if( mbTimer )
	{
		if( SetTimer(true) )
		{
			GetOldScreenResolution();
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pWidget = pResource->FindWidget("ViewConfirm");
			if(pWidget!= 0) pWidget->Show( false );
		}
	}
}

void ZOptionInterface::OnActionKeySet(ZActionKey* pActionKey, ZVIRTUALKEY key)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	for(int i=0; i<ZACTION_COUNT; i++){
		char szItemName[256];
		sprintf(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);
		ZActionKey* pWidget = (ZActionKey*)pResource->FindWidget(szItemName);
		if(pWidget==NULL) continue;
		if(pWidget==pActionKey) continue;

		if(pWidget->DeleteActionKey(key))
			pWidget->UpdateText();
	}
}








///////////////////// ÀÌÇÏ interface listener
BEGIN_IMPLEMENT_LISTENER(ZGetOptionFrameButtonListener, MBTN_CLK_MSG)
	// ¿É¼Ç ÇÁ·¹ÀÓ º¸¿©ÁÖ±E
	ZGetOptionInterface()->InitInterfaceOption();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("Option");
	pWidget->Show(true, true);

#ifdef LOCALE_NHNUSA
	pWidget = pResource->FindWidget( "Option_Textarea01");
	if ( pWidget)		pWidget->Show( false);
	MButton* pButton = (MButton*)pResource->FindWidget( "BoostOption");
	if ( pButton)
	{
		pButton->SetCheck( false);
		pButton->Show( false);
	}
#endif

#ifndef _MULTILANGUAGE	// ´ÙÁß¾ð¾ûÝö¿EµðÆÄÀÎÀ» ²°À»¶© ¾ð¾ûØ±ÅÃ ÄÞº¸¹Ú½º¸¦ ¼û°Ü¹ö¸°´Ù
	pWidget = (MLabel*)pResource->FindWidget( "LanguageSelectLabel");
	if ( pWidget) {
		pWidget->Show( false);
	}
	MComboBox* pCBType = (MComboBox*)pResource->FindWidget( "LanguageSelectComboBox");
	if ( pCBType) {
		pCBType->Show( false);
	}
#endif

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetSaveOptionButtonListener, MBTN_CLK_MSG)
	// Save & Close
	/*
	ZApplication::GetGameInterface()->SaveInterfaceOption();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("Option");
	if(pWidget!=NULL) pWidget->Show(false);
	//*/

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if( pWidget->m_bEventAcceleratorCall ) {

		MTabCtrl* pTab = (MTabCtrl*)pResource->FindWidget("OptionTabControl");

		if(pTab) {//Å°º¸µE¿É¼ÇÀº Å°ÀÔ·ÂÀ» ¸·¾ÆÁØ´Ù.
			if(pTab->GetSelIndex()==3)//ÀÌ¸§À» Ã£¾Æ Á¶»E
				return true;
		}
	}

	if( ZGetOptionInterface()->IsDiffNetworkPort() )
	{
		static bool bRestartAsk = false;
		if (bRestartAsk == false) {
			ZGetOptionInterface()->ShowNetworkPortConfirmDialog();
			bRestartAsk = true;
			return true;
		}
	}

	//if (ZGetOptionInterface()->IsDiffWindow())
	//{
	//	static bool bRestartAsk = false;
	//	if (bRestartAsk == false) {
	//		ZGetOptionInterface()->ShowWindowConfirmDialog();
	//		bRestartAsk = true;
	//		return true;
	//	}
	//}


	if( ZGetOptionInterface()->IsDiffScreenResolution() )
	{
		ZGetOptionInterface()->ShowResizeConfirmDialog( false );
	}
	else
	{
		bool bLanguageChanged = false;
		MComboBox* pLanguageComboBox = (MComboBox*)pResource->FindWidget("LanguageSelectComboBox");
		if (pLanguageComboBox) {
			if (ZGetConfiguration()->GetSelectedLanguageIndex() != pLanguageComboBox->GetSelIndex())
				bLanguageChanged = true;
		}

		ZGetOptionInterface()->SaveInterfaceOption();
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("Option");
		if(pWidget!=NULL) pWidget->Show(false);

		if(bLanguageChanged)	// Àç½ÃÀÛ È®ÀÎ ¸Þ¼¼ÁE¹Ú½º
		{
			ZApplication::GetGameInterface()->ShowConfirmMessage(
				ZMsg(MSG_CONFIRM_RESTART_CHANGE_LANGUAGE), ZGetLanguageChangeConfirmListenter());
		}
	}

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
	{
		if (ZGetCombatInterface())
		{
			ZGetCombatInterface()->GetCrossHair()->ChangeFromOption();
		}
	}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelOptionButtonListener, MBTN_CLK_MSG)
//	ZApplication::GetGameInterface()->SaveInterfaceOption();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// TODO: ÀÌ°Ô ÇÊ¿äÇÑ°¡ ? Å×½ºÆ® ¿ä¸Á
	if( pWidget->m_bEventAcceleratorCall ) {

		MTabCtrl* pTab = (MTabCtrl*)pResource->FindWidget("OptionTabControl");

		if(pTab) {//Å°º¸µE¿É¼ÇÀº Å°ÀÔ·ÂÀ» ¸·¾ÆÁØ´Ù.
			if(pTab->GetSelIndex()==3)//ÀÌ¸§À» Ã£¾Æ Á¶»E
				return true;
		}
	}

	MWidget* pWidget = pResource->FindWidget("Option");
	
	if(pWidget!=NULL) pWidget->Show(false);

	// ¿ø·¡ °¨¸¶°ªÀ¸·Î µ¹¸®±E
	MSlider* pSlider = (MSlider*)pResource->FindWidget("VideoGamma");
	if (pSlider != NULL) 
	{
		if (pSlider->GetValue() != Z_VIDEO_GAMMA_VALUE)
		{
			RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
		}
	}

	// Close Only
//	ZApplication::GetGameInterface()->SetState(GUNZ_PREVIOUS);
END_IMPLEMENT_LISTENER()


///////////////////////////////////////////////////
/// control

BEGIN_IMPLEMENT_LISTENER( ZGetLoadDefaultKeySettingListener, MBTN_CLK_MSG)
	ZGetConfiguration()->LoadDefaultKeySetting( );
	for(int i=0; i<ZACTION_COUNT; i++)
	{
		char szItemName[256];
		sprintf(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);
		ZActionKey* pWidget = (ZActionKey*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szItemName);
		if(pWidget==NULL) continue;
		//unsigned int nKey = 0;
		//pWidget->GetActionKey(&nKey);
		//Mint::GetInstance()->UnregisterActionKey(i);
		//Mint::GetInstance()->RegisterActionKey(i, nKey);	// Å° µûÓÏ
		//m_Keyboard.ActionKeys[i].nScanCode = nKey;	// ¿É¼Ç ÀúÀE
		pWidget->ClearActionKey();
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey);
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKeyAlt);
	}
END_IMPLEMENT_LISTENER()


//////////////////////////////////////////////////
/// video

BEGIN_IMPLEMENT_LISTENER(ZGetOptionGammaSliderChangeListener, MLIST_VALUE_CHANGED)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MSlider* pSlider = (MSlider*)pResource->FindWidget("VideoGamma");
	if (pSlider != NULL) 
	{
		unsigned short nGamma = (unsigned short)pSlider->GetValue();
		if (nGamma < 50) nGamma = 50;
		RSetGammaRamp(nGamma);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetRequestResizeListener, MBTN_CLK_MSG )
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ResizeConfirm");
	if(pWidget!= 0) pWidget->Show( false );
	//ÇØ»óµµ º¯°æÈÄ
	ZGetOptionInterface()->TestScreenResolution();
	ZGetOptionInterface()->SetTimer( true, 10 );
	ZGetOptionInterface()->ShowResizeConfirmDialog( true );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetViewConfirmCancelListener, MBTN_CLK_MSG )
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ViewConfirm");
	if(pWidget!= 0) pWidget->Show( false );
	// ÇØ»óµµ ¿ø·¡´E?º¯°E
 	ZGetOptionInterface()->SetTimer( false );
	ZGetOptionInterface()->GetOldScreenResolution();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetViewConfrimAcceptListener, MBTN_CLK_MSG )

	ZGetOptionInterface()->SetTimer( false );

	ZGetOptionInterface()->SaveInterfaceOption();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ViewConfirm");
	if(pWidget!= 0) pWidget->Show( false );
		
	pWidget = pResource->FindWidget("Option");
	if(pWidget!=NULL) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelResizeConfirmListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ResizeConfirm");
	if(pWidget!=NULL) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZSetOptimizationListener, MBTN_CLK_MSG )
	ZGetOptionInterface()->OptimizationVideoOption();
END_IMPLEMENT_LISTENER()

// Custom: Windowed mode widget
BEGIN_IMPLEMENT_LISTENER(ZGetWindowChangeRestartListener, MBTN_CLK_MSG)
ZGetOptionInterface()->SaveInterfaceOption();
ZChangeGameState(GUNZ_SHUTDOWN);
ZApplication* pApp = ZApplication::GetInstance();
pApp->Exit();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetWindowChangeCancelListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("WindowConfirm");
if (pWidget != 0) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

//////////////////////////////////////////////////
/// sound

BEGIN_IMPLEMENT_LISTENER( ZGet8BitSoundListener, MBTN_CLK_MSG )
	MButton* pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("8BitSound");
	if(pWidget)
	{
		pWidget->SetCheck(true);
		pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("16BitSound");
		if(pWidget) pWidget->SetCheck(false);
	}
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER( ZGet16BitSoundListener, MBTN_CLK_MSG )
	MButton* pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("16BitSound");
	if(pWidget)
	{
		pWidget->SetCheck(true);
		pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("8BitSound");
		if(pWidget) pWidget->SetCheck(false);
	}
END_IMPLEMENT_LISTENER()


/////////////////////////////////////////////////
//// network
BEGIN_IMPLEMENT_LISTENER( ZGetNetworkPortChangeRestartListener, MBTN_CLK_MSG )
	ZGetOptionInterface()->SaveInterfaceOption();
	ZChangeGameState(GUNZ_SHUTDOWN);
	ZApplication* pApp = ZApplication::GetInstance();
	pApp->Exit();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetNetworkPortChangeCancelListener, MBTN_CLK_MSG )
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("NetworkPortConfirm");
	if(pWidget!= 0) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

/////////////////////////////////////////////////
//// mouse
static void SetMouseSensitivitySlider(int i)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MSlider* pSlider = (MSlider*)pResource->FindWidget("MouseSensitivitySlider");
	if (pSlider) 
	{
		pSlider->SetValue(i);
	}
}
static void SetMouseSensitivityEdit(int i)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MEdit* pEdit = (MEdit*)pResource->FindWidget("MouseSensitivityEdit");
	if (pEdit)
	{
		char sz[1024] = "";
		sprintf(sz, "%d", i);
		pEdit->SetText(sz);
	}
}

BEGIN_IMPLEMENT_LISTENER( ZGetMouseSensitivitySliderListener, MLIST_VALUE_CHANGED)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MSlider* pSlider = (MSlider*)pResource->FindWidget("MouseSensitivitySlider");
	if (pSlider != NULL) 
	{
		int i = ZGetConfiguration()->ValidateMouseSensitivityInInt( pSlider->GetValue());
		SetMouseSensitivityEdit(i);
	}
END_IMPLEMENT_LISTENER()

// ÀÌ ¿¡µðÆ®¹Ú½º¿¡¼­ ¸Þ½ÃÁEµÎ°³¸¦ Ã³¸®ÇØ¾ßÇÏ±E¶§¹®¿¡ BEGIN_IMPLEMENT_LISTENER/END_IMPLEMENT_LISTENER ¸ÅÅ©·Î¸¦ ¾È½èÀ½
MListener* ZGetMouseSensitivityEditListener(void){
	class ListenerClass : public MListener
	{
	public:
		virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MEdit* pEdit= (MEdit*)pResource->FindWidget("MouseSensitivityEdit");
			{
				int i = atoi( pEdit->GetText());
				int v = ZGetConfiguration()->ValidateMouseSensitivityInInt(i);

				if(MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true)
				{
					SetMouseSensitivitySlider(v);
					return true;
				}
				else if(MWidget::IsMsg(szMessage, MEDIT_KILL_FOCUS)==true)
				{
					SetMouseSensitivitySlider(v);
					SetMouseSensitivityEdit(v);
					return true;
				}
			}
			return false;
		}
	};
	static ListenerClass	Listener;
	return &Listener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetOutlineModeComboBoxListener, MCMBBOX_CHANGED)
{
	MComboBox* pComboBox = (MComboBox*)pWidget;
	ZGetConfiguration()->GetVideo()->nOutlineMode = pComboBox->GetSelIndex();
	ZGetConfiguration()->Save(Z_LOCALE_XML_HEADER);
	return true;
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetOutlineColorComboBoxListener, MCMBBOX_CHANGED)
{
	static const DWORD s_OutlineColors[] = {
		0x00000000, 0x00FFFFFF, 0x00FFFF00, 0x00FF0000,
		0x0000FF00, 0x000000FF, 0x00FF8000, 0x00800080,
		0x00FF69B4, 0x0000CED1, 0x0000FFFF, 0x00FFD700,
	};
	MComboBox* pComboBox = (MComboBox*)pWidget;
	int nSel = pComboBox->GetSelIndex();
	if (nSel >= 0 && nSel < 12)
		ZGetConfiguration()->GetVideo()->dwOutlineColor = s_OutlineColors[nSel];
	ZGetConfiguration()->Save(Z_LOCALE_XML_HEADER);
	return true;
}
END_IMPLEMENT_LISTENER()