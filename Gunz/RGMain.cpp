#include "stdafx.h"
#include "RGMain.h"
#include "NewChat.h"
#include "ZConfiguration.h"
#include "RealSpace2.h"
#include "ZRule.h"
#include "VoiceChat.h"
#include "FileInfo.h"
#include "ZInput.h"
#include <cstdint>
#include "dxerr.h"
#include "defer.h"
#include "hsv.h"
#include <string>
#include "FunctionalListener.h"
#include "ZCombatInterface.h"
#include "ZEmojiPicker.h"

static optional<RGMain> g_RGMain;

RGMain& GetRGMain() { return g_RGMain.value(); }
void CreateRGMain() { g_RGMain.emplace(); }
void DestroyRGMain() { g_RGMain.reset(); }

void RGMain::OnAppCreate(){}
void RGMain::OnReset() {}
void RGMain::OnDrawLobby(MDrawContext* pDC) {}
void RGMain::OnDrawGame() {}

//void RGMain::OnVulkanD3D9Create()
//{
//	HINSTANCE hVulkaD3D9 = LoadLibrary("d3d9.dll");
//
//	if (!hVulkaD3D9)
//	{
//		mlog("Error: could not load d3d9.dll");
//		ExitProcess(NULL);
//	}
//}

void RGMain::OnCreateDevice()
{
	auto&& Cfg = *ZGetConfiguration();
	auto&& c = *Cfg.GetChat();
	m_Chat.emplace(c.Font, c.BoldFont, c.FontSize);
	GetChat().SetBackgroundColor(c.BackgroundColor);
#ifdef _VOICE_CHAT
	m_VoiceChat.OnCreateDevice();
#endif
}

void RGMain::OnDrawGameInterface(MDrawContext* pDC)
{
		GetChat().OnDraw(pDC);
#ifdef _VOICE_CHAT
		m_VoiceChat.OnDraw(pDC);
#endif
}

bool RGMain::OnGameInput()
{
	return GetChat().IsInputEnabled();
}

void RGMain::Resize(int w, int h)
{
	GetChat().Resize(w, h);
}

std::pair<PlayerFoundStatus, ZCharacter*> FindSinglePlayer(const char * NameSubstring)
{
	bool Found = false;
	ZCharacter* FoundChar = nullptr;

	for (auto Item : *ZGetCharacterManager())
	{
		auto Char = (ZCharacter*)Item.second;

		if (!strstr(Char->GetUserName(), NameSubstring))
			continue;

		if (Found)
			return{ PlayerFoundStatus::TooManyFound, nullptr };

		Found = true;
		FoundChar = Char;
	}

	if (Found)
		return{ PlayerFoundStatus::FoundOne, FoundChar };

	return{ PlayerFoundStatus::NotFound, nullptr };
}

RGMain::RGMain() = default;
RGMain::~RGMain() {
	//
}

void RGMain::OnUpdate(double Elapsed)
{
	LastTime = Time;

	Time += Elapsed;

	{
		std::lock_guard<std::mutex> lock(QueueMutex);

		for (auto it = QueuedInvokations.begin(); it != QueuedInvokations.end(); it++)
		{
			auto item = *it;

			if (Time < item.Time)
				continue;

			item.fn();

			it = QueuedInvokations.erase(it);

			if (it == QueuedInvokations.end())
				break;
		}
	}

	GetChat().OnUpdate(Elapsed);
}

bool RGMain::OnEvent(MEvent* pEvent)
{
	// Route input to emoji picker if visible
	if (ZGetEmojiPicker().IsVisible())
	{
		switch (pEvent->nMessage)
		{
		case MWM_LBUTTONDOWN:
			if (ZGetEmojiPicker().OnLButtonDown(pEvent->Pos.x, pEvent->Pos.y))
				return true;
			break;
		case MWM_MOUSEMOVE:
			if (ZGetEmojiPicker().OnMouseMove(pEvent->Pos.x, pEvent->Pos.y))
				return true;
			break;
		case MWM_MOUSEWHEEL:
			if (ZGetEmojiPicker().OnMouseWheel(pEvent->nDelta))
				return true;
			break;
		case MWM_KEYDOWN:
			if (ZGetEmojiPicker().OnKeyDown(pEvent->nKey))
				return true;
			break;
		}
	}

	GetChat().OnEvent(pEvent);

	bool IsChatVisible;
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		IsChatVisible = GetChat().IsInputEnabled();
	}
	else
	{
		IsChatVisible = ZGetCombatInterface()->IsChat();
	}

	bool ret = false;

#ifdef _VOICE_CHAT
	static bool LastState = false;
	bool CurState = ZIsActionKeyPressed(ZACTION_VOICE_CHAT);

	ret = [&]
	{
		if (CurState && !LastState)
		{
			m_VoiceChat.StartRecording();
			return true;
		}
		else if (!CurState && LastState)
		{
			m_VoiceChat.StopRecording();

			return true;
		}

		return false;
	}();

	LastState = CurState;
#endif
	return IsChatVisible;

}

/*bool RGMain::IsCursorEnabled() const
{
	bool bEnabled = false;
	auto&& Cfg = *ZGetConfiguration();

	if (Cfg.GetChat()->EnableCursor && GetChat().IsInputEnabled())
		bEnabled = true;

	// Also check old chat
	if (!bEnabled && Cfg.GetChat()->EnableCursor && ZGetCombatInterface())
	{
		if (ZGetCombatInterface()->IsChat())
			bEnabled = true;
	}

	return bEnabled;
}*/
bool RGMain::IsCursorEnabled() const
{
	if (GetChat().IsInputEnabled())
		return true;

	if (ZGetCombatInterface() && ZGetCombatInterface()->IsChat())
		return true;

	return false;
}

void RGMain::OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length)
{
#ifdef _VOICE_CHAT
	m_VoiceChat.OnReceiveVoiceChat(Char, Buffer, Length);
#endif
}

void RGMain::SetListeners()
{
	auto& MySettings = TrainingSettings;

	static const std::pair<const char*, bool&> Buttons[] = 
	{
		{"TrainingLifeInfinity", MySettings.Godmode},
		{"TrainingNoStuns", MySettings.NoStuns},
		{"TrainingNoMassive", MySettings.NoMass},
		{"TrainingInfAnmo", MySettings.InBullet},
		{"TrainingNoFlip", MySettings.NoFlip},
		{"TrainingSpeed", MySettings.Speed},
		{"TrainingPaint", MySettings.Paint},
		{"TrainingWire", MySettings.WireFrame},
		{"TrainingNinja", MySettings.Ninja},
	};

	auto LastIndex = std::size(Buttons) - 0;
	for (size_t i = 0; i < LastIndex; ++i)
	{
		Listen(Buttons[i].first, SetToButtonState(Buttons[i].second));
	}

	auto&& LastButton = Buttons[LastIndex];

	Listen("", OnMessage(MBTN_CLK_MSG, [] {
		if (auto&& Widget = ZFindWidget("CombatInterface"))
		{
			Widget->Show(false);
		}
		}));

	Listen("TrainingFrame", OnMessage(MBTN_CLK_MSG, [] {
		if (auto&& Widget = ZFindWidget("Training"))
		{
			Widget->Show(true, true);
		}
		else
		{
			return;
		}

		for (auto&& Button : Buttons)
		{
			if (auto&& Widget = ZFindWidgetAs<MButton>(Button.first))
			{
				Widget->SetCheck(Button.second);
			}
		}


		}));

	Listen("TrainingCloseButton", OnMessage(MBTN_CLK_MSG, [] {
		if (auto&& Widget = ZFindWidget("Training"))
		{
			Widget->Show(false);
		}
		}));
}

void RGMain::OnGameCreate()
{
	// Update NewChat position from UI widgets (now loaded)
	GetChat().Resize(RGetScreenWidth(), RGetScreenHeight());

	if (auto&& Widget = ZFindWidget("TrainingFrame"))
	{
		if (ZGetGame()->GetMatch()->IsTraining())
		{
			Widget->Enable(true);
		}
		else
		{
			Widget->Enable(false);
		}
	    if (strstr(ZGetGameClient()->GetChannelName(), "Event"))
	    {
		Widget->Enable(false);
	    }
	}
}

#ifdef _SWORDCOLOR 1
std::pair<bool, uint32_t> RGMain::GetPlayerSwordColor(const MUID& UID)
{
	auto it = SwordColors.find(UID);

	if (it == SwordColors.end())
		return{ false, 0 };

	return{ true, it->second };
}
void RGMain::SetSwordColor(const MUID& UID, uint32_t Color)
{
	SwordColors[UID] = Color;

	auto Char = (ZCharacter*)ZGetCharacterManager()->Find(UID);

	if (!Char)
		return;

	Char->m_pVMesh->SetCustomColor(Color & 0x80FFFFFF, Color & 0x0FFFFFFF);
}
#endif