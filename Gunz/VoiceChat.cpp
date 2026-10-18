#include "stdafx.h"
#include "VoiceChat.h"
#include "RGMain.h"

#ifdef _VOICE_CHAT

template <typename SampleType = VoiceChat::SampleFormat>
unsigned long GetSampleFormat()
{
	static_assert(false, "Unkwown type");
}

template <> unsigned long GetSampleFormat<char>()			{ return paInt8; }
template <> unsigned long GetSampleFormat<unsigned char>()	{ return paUInt8; }
template <> unsigned long GetSampleFormat<short>()			{ return paInt16; }
template <> unsigned long GetSampleFormat<int>()			{ return paInt32; }
template <> unsigned long GetSampleFormat<float>()			{ return paFloat32; }

VoiceChat* VoiceChat::Instance = nullptr;

VoiceChat::VoiceChat()
{
	if (Instance != nullptr)
		throw std::runtime_error("VoiceChat multiply created");

	Instance = this;

#ifdef WAVEIN
	thr = std::thread([this]()
	{
		while (true)
		{
			ThreadLoop();
		}
	});
#endif

	int error;

	CanPlay = true;

	error = Pa_Initialize();
	if (error != paNoError)
	{
		MLog("Pa_Initialize failed with error code %d: %s\n", error, Pa_GetErrorText(error));

		CanPlay = false;

#ifndef WAVEIN
		CanRecord = true;
#endif

		return;
	}

	[&] {
		CanRecord = true;

#ifdef WAVEIN
		WAVEFORMATEX Format;
		Format.wFormatTag = WAVE_FORMAT_PCM;
		Format.nChannels = 1;
		Format.nSamplesPerSec = SampleRate;
		Format.wBitsPerSample = 16;
		Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nChannels * Format.wBitsPerSample / 8;
		Format.nBlockAlign = Format.nChannels * Format.wBitsPerSample / 8;
		Format.cbSize = 0;

		auto ret = waveInOpen(&WaveIn, WAVE_MAPPER, &Format, (DWORD_PTR)GetThreadId(thr.native_handle()), 0, WAVE_FORMAT_DIRECT | CALLBACK_THREAD);

		if (ret != MMSYSERR_NOERROR)
		{
			char Reason[256];

			waveInGetErrorText(ret, Reason, sizeof(Reason));

			MLog("waveInOpen failed: %s\n", Reason);

			CanRecord = false;

			return;
		}
#else
		error = Pa_OpenDefaultStream(&InputStream, NumChannels, 0, GetSampleFormat(), SampleRate, FrameSize, &RecordCallbackWrapper, nullptr);

		if (error != paNoError)
		{
			MLog("Pa_OpenStream failed with error code %d: %s\n", error, Pa_GetErrorText(error));

			CanRecord = false;

			return;
		}
#endif

		pOpusEncoder = opus_encoder_create(SampleRate, NumChannels, OPUS_APPLICATION_VOIP, &error);

		if (error != OPUS_OK)
		{
			MLog("opus_encoder_create failed with error code %d: %s\n", error, opus_strerror(error));

			CanRecord = false;

			return;
		}
	}();

	[&] {
		CanDecode = true;

		pOpusDecoder = opus_decoder_create(SampleRate, 1, &error);

		if (error != OPUS_OK)
		{
			MLog("opus_decoder_create failed with error code %d: %s\n", error, opus_strerror(error));

			CanDecode = false;
		}
	}();
}

VoiceChat::~VoiceChat()
{
#ifdef WAVEIN
	waveInClose(WaveIn);
#else
	if(pOpusEncoder)
		opus_encoder_destroy(pOpusEncoder);
	if(pOpusDecoder)
		opus_decoder_destroy(pOpusDecoder);
#endif
}
void VoiceChat::StartRecording()
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bVoiceChat)
	{
		if (Recording || !CanRecord || ZGetGame()->IsReplay())
			return;

	if (ZGetGame()->m_pMyCharacter->IsDie())
			return;

	if (ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true) return;

	if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible() == true)
		return;

		if (!Pa_IsStreamActive(InputStream))
		{
			Pa_StopStream(InputStream);
			Pa_StartStream(InputStream);
		}
		Recording = true;
	}	
}

void VoiceChat::StopRecording()
{
	/*if (ZGetGame()->GetMatch()->IsTeamPlay() ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_TRAINING ||
		ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsLadderWarsChannel())*/
	{
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bVoiceChat)
		{
			if (!Recording || !CanRecord || ZGetGame()->IsReplay())
				return;

			if (ZGetGame()->m_pMyCharacter->IsDie()) // No Talking dead..
				return;

#ifdef WAVEIN
			waveInReset(WaveIn);
			waveInStop(WaveIn);
#else
			Pa_StopStream(InputStream);
#endif

			Recording = false;
		}
	}
}
// Custom: Volumen VoiceChat.
void VoiceChat::OnReceiveVoiceChat(ZCharacter* Char, const uint8_t* Buffer, int Length)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bVoiceChat)
	{
		if (Z_ETC_VOICE_VOLUME / 10000.f)
		{

			//nVoiceVolumen = 1.0f;

			if (!CanDecode)
				return;

			if (MutedPlayers.find(Char->GetUID()) != MutedPlayers.end())
				return;

			if (ZGetGame()->m_pMyCharacter->IsDie()) // No Talking dead..
				return;

			MicFrame mf;
			int ret = opus_decode(pOpusDecoder, Buffer, Length, mf.pcm, FrameSize, 0);

			if (ret < 0)
			{
				MLog("opus_decode failed with error code %d: %s\n", ret, opus_strerror(ret));
				return;
			}

			auto it = MicStreams.find(Char);

			if (it == MicStreams.end())
			{
				PaStream* Stream = nullptr;
				auto error = Pa_OpenDefaultStream(&Stream, 0, 1, paInt16, SampleRate, FrameSize, PlayCallback, Char);

				if (error != paNoError)
				{
					MLog("Pa_OpenStream failed with error code %d: %s\n", error, Pa_GetErrorText(error));
				}

				it = MicStreams.emplace(Char, MicStuff(Stream)).first;
				return;
			}

			if (!it->second.Stream) // Dead stream object that failed to create
				return;

			{
				std::lock_guard<std::mutex> lock(it->second.QueueMutex);

				it->second.Data.push(mf);
			}

			if (!Pa_IsStreamActive(it->second.Stream) && it->second.Data.size() > 2)
			{
				Pa_StopStream(it->second.Stream);
				Pa_StartStream(it->second.Stream);
				it->second.Streaming = true;
			}
		}
	}
}
	

void VoiceChat::OnDestroyCharacter(ZCharacter * Char)
{
	auto it = MicStreams.find(Char);

	if (it == MicStreams.end())
		return;

	MicStreams.erase(it);
}

bool VoiceChat::MutePlayer(const MUID & UID)
{
	auto it = MutedPlayers.find(UID);

	if (it == MutedPlayers.end())
	{
		MutedPlayers.insert(UID);
		return true;
	}

	MutedPlayers.erase(it);

	return false;
}

#ifdef WAVEIN
void VoiceChat::ThreadLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, MM_WIM_DATA, MM_WIM_DATA) == 1)
	{
		WAVEHDR &wavehdr = *LPWAVEHDR(msg.lParam);

		EncodedSizes[CurrentBufferIndex] = opus_encode(pOpusEncoder, (const opus_int16 *)FrameBuffers[CurrentBufferIndex].data(), FrameSize, &EncodedFrames[CurrentBufferIndex][0], sizeof(EncodedFrames[0]));

		if (EncodedSizes[CurrentBufferIndex] < 0)
		{
			MLog("opus_encode failed with error code %d: %s\n", EncodedSizes[CurrentBufferIndex], opus_strerror(EncodedSizes[CurrentBufferIndex]));
			return;
		}

		//MLog("Encoded data, size %d\n", EncodedSizes[CurrentBufferIndex]);

		auto lambda = [EncodedFrame = EncodedFrames[CurrentBufferIndex], Size = EncodedSizes[CurrentBufferIndex]]
		{
			ZPostVoiceChat(EncodedFrame.data(), Size);
		};

		GetRGMain().Invoke(lambda, 2);

		CurrentBufferIndex = (CurrentBufferIndex + 1) % NumBuffers;

		wavehdr.lpData = LPSTR(FrameBuffers[CurrentBufferIndex].data());

		waveInAddBuffer(WaveIn, &wavehdr, sizeof(WAVEHDR));
	}
}
#else

int VoiceChat::RecordCallbackWrapper(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	return GetInstance()->RecordCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags, userData);
}

int VoiceChat::RecordCallback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bVoiceChat)
	{
		if (!Recording)
			return paComplete;

		std::array<unsigned char, FrameSize> EncodedFrame;
		auto Size = opus_encode(pOpusEncoder, (const opus_int16*)inputBuffer, FrameSize, EncodedFrame.data(), sizeof(EncodedFrame));

		if (Size < 0)
		{
			MLog("opus_encode failed with error code %d: %s\n", Size, opus_strerror(Size));
			return paContinue;
		}

		if (ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true) return paAbort;

		if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible() == true)
			return paAbort;

#ifdef _ICONCHAT
		ZCharacter* pTarget = NULL;
		char* name = "";

		if (statusFlags == STATUS_TALKING)
		{
			for (ZObjectManager::iterator itor = ZGetGame()->m_ObjectManager.begin(); itor != ZGetGame()->m_ObjectManager.end(); ++itor)
			{
				pTarget = (ZCharacter*)(*itor).second;
				if (pTarget && pTarget->GetCharInfo())
				{
					if (strstr(pTarget->GetCharInfo()->szName, name))
					{
						if (ZGetConfiguration()->GetEtc()->bVoiceChat == true)
						{
							ZCharaterStatusBitPacking& uStatus = pTarget->m_dwStatusBitPackingValue.Ref();
							uStatus.m_bChatVoice = true;
							ZGetEffectManager()->AddChatVoiceIcon(pTarget);
							ZPostPeerChatVoiceIcon(true);
						}
						MEMBER_SET_CHECKCRC(pTarget->GetStatus(), isTalking, true);
						break;
					}
				}
				else if (statusFlags == STATUS_NOT_TALKING)
				{
					for (ZObjectManager::iterator itor = ZGetGame()->m_ObjectManager.begin(); itor != ZGetGame()->m_ObjectManager.end(); ++itor)
					{
						pTarget = (ZCharacter*)(*itor).second;
						if (pTarget && pTarget->GetCharInfo())
						{
							if (strstr(pTarget->GetCharInfo()->szName, name))
							{
								ZCharaterStatusBitPacking& uStatus = pTarget->m_dwStatusBitPackingValue.Ref();
								uStatus.m_bChatVoice = false;
								MEMBER_SET_CHECKCRC(pTarget->GetStatus(), isTalking, false);
								ZPostPeerChatVoiceIcon(false);
								break;
							}
						}
					}
				}
			}
#endif
			//MLog("Encoded data, size %d\n", Size);

			auto lambda = [EncodedFrame, Size]
			{
				ZPostVoiceChat(EncodedFrame.data(), Size);
			};

			GetRGMain().Invoke(lambda);

			return paContinue;
		}
	}
}


#endif

int VoiceChat::PlayCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bVoiceChat)
	{
		if (Z_ETC_VOICE_VOLUME / 10000.f)
		{
			auto it = GetInstance()->MicStreams.find((ZCharacter*)userData);
			if (it == GetInstance()->MicStreams.end())
				return paComplete;

			if (ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true) return paComplete;

			if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible() == true)
				return paComplete;
			{
				std::lock_guard<std::mutex> lock(it->second.QueueMutex);
				auto& Queue = it->second.Data;
				if (Queue.empty())
				{
					it->second.Streaming = false;
					return paComplete;
				}
				auto& p = Queue.front();
				memcpy(outputBuffer, p.pcm, sizeof(p.pcm));
				Queue.pop();
			}
			return paContinue;
		}
	}
}


void VoiceChat::OnCreateDevice()
{
	if (RGetDevice() == nullptr)
	{
		MLog("VoiceChat: Device is NULL, skip create\n");
		return;
	}

	auto Success = SpeakerBitmap.Create(
		"SpeakerIcon",
		RGetDevice(),
		"Interface/default/icon_voice/SpeakerIcon.png"
	);

	if (!Success)
		MLog("Failed to create speaker icon texture\n");
}

void VoiceChat::OnDraw(MDrawContext* pDC)
{
    // Kiểm tra an toàn cơ bản
    if (!ZGetGame() || !ZGetGame()->m_pMyCharacter) return;
    if (ZGetGame()->m_pMyCharacter->IsDie()) return;

    if (ZGetConfiguration()->GetEtc()->bVoiceChat)
    {
        int i = 0;

        // Giữ nguyên Lambda DrawStuff nhưng thêm kiểm tra NULL
        auto DrawStuff = [&](ZCharacter* Player)
        {
            if (!Player) return;
			const char* szUserName = Player->GetUserName();
			if (!szUserName) return;

            v2 TopLeft{ float(RELWIDTH(1920 - 400)), float(RELHEIGHT(1080 / 2 + i * 100)) };
            v2 Extents{ float(RELWIDTH(300)), float(RELHEIGHT(50)) };

            auto color = Player->GetTeamID() == MMT_BLUE ? 0xC000A5C3 : 0xC0FF0000;

            pDC->SetColor(color);
            pDC->FillRectangle(TopLeft.x, TopLeft.y, Extents.x, Extents.y);

            v2 TextOffset{ float(RELWIDTH(50)), float(RELHEIGHT(10)) };
            auto v = TopLeft + TextOffset; // Phép cộng v2 sẽ an toàn vì TopLeft đã có giá trị thực

            pDC->SetColor(ARGB(255, 255, 255, 255));
            pDC->Text(v.x, v.y, Player->GetUserName());

            pDC->SetBitmap(&SpeakerBitmap);
            pDC->Draw(TopLeft.x + RELWIDTH(10), TopLeft.y + RELHEIGHT(10), RELWIDTH(30), RELHEIGHT(30));

            i++;
        };

        // 1. Vẽ cho chính mình
        if (Recording)
            DrawStuff(ZGetGame()->m_pMyCharacter);

        // 2. Vẽ cho người khác dựa trên MUID
        for (auto& item : MicStreams)
        {
            if (!item.second.Streaming)
                continue;
                DrawStuff(item.first);
            
        }
    }
}
#endif