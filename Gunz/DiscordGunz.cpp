#include "stdafx.h"
#include "DiscordGunz.h"
#include <Windows.h>
#include <chrono>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include "discord_rpc.h"
#include "ZRuleQuestChallenge.h"

static bool gInit, gRPC = true;

static void handleDiscordReady(const DiscordUser* connectedUser)
{
	printf("\nDiscord: connected to user %s#%s - %s\n",
		connectedUser->username,
		connectedUser->discriminator,
		connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
	printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
	printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

static void handleDiscordJoin(const char* secret)
{
	printf("\nDiscord: join (%s)\n", secret);
}

static void handleDiscordSpectate(const char* secret)
{
	printf("\nDiscord: spectate (%s)\n", secret);
}

void ZDiscord::OnRunDiscord()
{
	if (gRPC)
	{
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));

		handlers.ready = handleDiscordReady;
		handlers.disconnected = handleDiscordDisconnected;
		handlers.errored = handleDiscordError;
		handlers.joinGame = handleDiscordJoin;
		handlers.spectateGame = handleDiscordSpectate;

		Discord_Initialize("977280318790856744", &handlers, 1, NULL);
	}
}

void ZDiscord::OnUpdateDiscord()
{
	if (gRPC)
	{
		static int64_t eptime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));
		discordPresence.state = "Play free!";
		discordPresence.details = "Gunz The Duel";
		discordPresence.largeImageKey = "logo512";
		discordPresence.largeImageText = "Join Now https://gunz.vn/";
		discordPresence.smallImageKey = "teamgladiator-mode";
		discordPresence.smallImageText = "Gunz: The Duel";
		discordPresence.partySize = 0;
		discordPresence.partyMax = 0;
		discordPresence.partyId = 0;
		discordPresence.spectateSecret = 0;
		discordPresence.joinSecret = 0;

		Discord_UpdatePresence(&discordPresence);

		GunzState GunzState2 = ZGetGameInterface()->GetState();

		switch (GunzState2)
		{
		case GUNZ_NA:
		{
			discordPresence.details = "Running Gunz: The Duel...";
			discordPresence.smallImageKey = "logo512";
		}
		break;

		case GUNZ_LOGIN:
		{
			discordPresence.details = "Login Server (Gunz: The Duel)";
			discordPresence.state = "I'm trying to log in...";
		}
		break;

		case GUNZ_LOBBY:
		{
			char buf[512];
			char buf2[512];
			sprintf_safe(buf, "%s > %s", ZMsg(MSG_WORD_LOBBY), ZGetGameClient()->GetChannelName());
			discordPresence.details = buf;

			sprintf_safe(buf2, "Lv. %d %s ", ZGetMyInfo()->GetLevel(), ZGetMyInfo()->GetCharName());
			discordPresence.state = buf2;
		}
		break;

		case GUNZ_STAGE:
		{
			discordPresence.details = "Preparing for the game...";

			char szText1[512];
			sprintf_safe(szText1, "[%03d] %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
			discordPresence.state = szText1;

			char szMapGStageName[256];
			sprintf_safe(szMapGStageName, "%s", ZGetGameClient()->GetMatchStageSetting()->GetMapName());
			discordPresence.largeImageText = szMapGStageName;

			char szMapStageImage[256];
			sprintf_safe(szMapStageImage, "%d", ZGetGameClient()->GetMatchStageSetting()->GetMapIndex());
			discordPresence.largeImageKey = szMapStageImage;

		}
		break;

		case GUNZ_CHARSELECTION:
			char szSelect[512];
			sprintf_safe(szSelect, "In Select Character.");

			discordPresence.details = szSelect;
			discordPresence.state = "I'm trying to select my char...";
			break;

		case GUNZ_CHARCREATION:
			char szCreation[512];
			sprintf_safe(szCreation, "In Create Character.");

			discordPresence.details = szCreation;
			discordPresence.state = "I'm trying to create a character...";
			break;

		case GUNZ_GAME:
		{
			MMATCH_GAMETYPE nGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
			MCHANNEL_TYPE nChannelType = ZGetGameClient()->GetChannelType();

			if (ZGetGame()->IsReplay()) {
				discordPresence.state = "Watching Replay!";
				break;
			}

			bool bClanGame = ZGetGameClient()->IsCWChannel();
			bool bLadderGame = ZGetGameClient()->IsLadderWarsChannel();
			bool bClanServer = ZGetGameClient()->GetServerMode();

			int nStageType = ZGetGameClient()->GetMatchStageSetting()->GetStageType();

			ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(ZGetMyUID());
			char szCharLevel[256];
			char zsCharClan[256];
			char zsNameStage[256];
			sprintf_safe(szCharLevel, "%d Lv. %s", pChar->GetProperty()->nLevel, pChar->GetProperty()->GetName());
			sprintf_safe(zsCharClan, "%d Clan %d", pChar->GetProperty()->GetName(), pChar->GetProperty()->GetClanName());
			sprintf_safe(zsNameStage, "[%03d] %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());


			if (bClanServer)
			{
				switch (nStageType)
				{
				case (MSM_LADDER):
					discordPresence.details = "Clan War";
					discordPresence.state = "I'm playing Clan War!";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "teamdeath-mode";
					discordPresence.smallImageText = " Team Deathmatch";
				default:
					break;
				}
			}
			if (bClanGame && bClanServer)
			{
				switch (nStageType)
				{
				case (MST_LADDER):
					discordPresence.details = "Ladder (Clan War)";
					discordPresence.state = "I'm playing Ladder!";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "teamdeath-mode";
					discordPresence.smallImageText = " Team Deathmatch";
					break;
				case (MST_LADDERWARS):
					discordPresence.details = "Ladder War (Competitive)";
					discordPresence.state = "I'm playing Ladder War!";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "deity_mode";
					discordPresence.smallImageText = "Competitive!";
				default:
					break;
				}
			}
			else
			{
				switch (nGameType)
				{
				case (MMATCH_GAMETYPE_DEATHMATCH_SOLO):
					discordPresence.details = "Deathmatch (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "death-mode";
					discordPresence.smallImageText = "Deathmatch";
					if (ZGetGameTypeManager()->IsSoloGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetRoundKills() + 0;
						int sectorCount = ZGetGameClient()->GetMatchStageSetting()->GetRoundMax();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_DEATHMATCH_TEAM):
					discordPresence.details = "Deathmatch (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "teamdeath-mode";
					discordPresence.smallImageText = " Team Deathmatch";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_GLADIATOR_SOLO):
					discordPresence.details = "Gladiator (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "Gladiator-mode";
					discordPresence.smallImageText = " Gladiator";
					if (ZGetGameTypeManager()->IsSoloGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetRoundKills() + 0;
						int sectorCount = ZGetGameClient()->GetMatchStageSetting()->GetRoundMax();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_GLADIATOR_TEAM):
					discordPresence.details = "Gladiator (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "teamGladiator-mode";
					discordPresence.smallImageText = " Gladiator Team";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_ASSASSINATE):
					discordPresence.details = "Assassinate (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "Assasin-mode";
					discordPresence.smallImageText = " Assassinate Team";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_TRAINING):
					discordPresence.details = "Training (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "train_mode";
					discordPresence.smallImageText = " Training";
					break;

				case (MMATCH_GAMETYPE_SURVIVAL):
					discordPresence.details = "Survival (Mission)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "survival-mode";
					discordPresence.smallImageText = " Survival";
					if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex() + 1;
						int sectorCount = ZGetQuest()->GetGameInfo()->GetMapSectorCount();
						int repeatCount = ZGetQuest()->GetGameInfo()->GetRepeatCount();

						currSector += ZGetQuest()->GetGameInfo()->GetCurrRepeatIndex() * sectorCount;
						sectorCount *= repeatCount;

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_QUEST):
					discordPresence.details = "Quest (Mission)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "quest-mode";
					discordPresence.smallImageText = " Mission!";
					if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex() + 1;
						int sectorCount = ZGetQuest()->GetGameInfo()->GetMapSectorCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_QUEST_CHALLENGE):
					discordPresence.details = "Challenge Quest (Mission)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "challengequest_guerilla-mode";
					discordPresence.smallImageText = " Challenge!";
					if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
					{
						ZRuleQuestChallenge* pCurrSector = NULL;
						if (ZGetGame()->GetMatch()->GetRule())
						{
							pCurrSector = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
						}
						int currSector = pCurrSector->GetCurrSector() + 1;
						int sectorCount = ZGetGameClient()->GetMatchStageSetting()->GetRoundMax();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;

				case (MMATCH_GAMETYPE_BERSERKER):
					discordPresence.details = "Berserker (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "berserk-mode";
					discordPresence.smallImageText = " Berserker";
					break;
				case (MMATCH_GAMETYPE_DEATHMATCH_TEAM2):
					discordPresence.details = "Extreme (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "unlimitdeath-mode";
					discordPresence.smallImageText = " Extreme";
					break;
				case (MMATCH_GAMETYPE_DUEL):
					discordPresence.details = "Duel Match (PvP)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "duel-mode";
					discordPresence.smallImageText = " Duel PvP";
					break;
				case (MMATCH_GAMETYPE_DUELTOURNAMENT):
					discordPresence.details = "Duel (Tournament)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "duel-mode";
					discordPresence.smallImageText = " Tournament";
					break;
				case (MMATCH_GAMETYPE_TEAM_TRAINING):
					discordPresence.details = "Training (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "teamtrain_mode";
					discordPresence.smallImageText = " Training Team";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_CTF):
					discordPresence.details = "Capture The Flag (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "ctf-mode";
					discordPresence.smallImageText = " CTF";
					break;
				case (MMATCH_GAMETYPE_INFECTED):
					discordPresence.details = "Infected (Mode)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "infected-mode";
					discordPresence.smallImageText = " Zombie";
					break;
				case (MMATCH_GAMETYPE_GUNGAME):
					discordPresence.details = "Gun Game (Mode)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "gungame-mode";
					discordPresence.smallImageText = " Gun Game";
					break;
				case (MMATCH_GAMETYPE_SPY):
					discordPresence.details = "Spy (Mode)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "spy_mode";
					discordPresence.smallImageText = " Spy Mode";
					break;
				case (MMATCH_GAMETYPE_VAMPIRE):
					discordPresence.details = "Vampire (Mode)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "vampire-mode";
					discordPresence.smallImageText = " Vampire";
					break;
				case (MMATCH_GAMETYPE_SKILLMAP):
					discordPresence.details = "Skill Map (Skill)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "ninja-mode";
					discordPresence.smallImageText = " Skill Mode";
					break;
				case (MMATCH_GAMETYPE_DROPMAGIC):
					discordPresence.details = "DropMagic Box (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "battle-mode";
					discordPresence.smallImageText = " Drop Magic";
					break;
				case (MMATCH_GAMETYPE_PAINTBALL_SOLO):
					discordPresence.details = "Paint Ball (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "paint-mode";
					discordPresence.smallImageText = " Paint Ball";
					break;
				case (MMATCH_GAMETYPE_PAINTBALL_TEAM):
					discordPresence.details = "Paint Ball (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "paint-mode";
					discordPresence.smallImageText = " Paint Ball Team";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_CLASSIC_SOLO):
					discordPresence.details = "Classic (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "death-mode";
					discordPresence.smallImageText = " Classic";
					break;
				case (MMATCH_GAMETYPE_CLASSIC_TEAM):
					discordPresence.details = "Classic (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "teamdeath-mode";
					discordPresence.smallImageText = " Classi Team";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_MODE_STAFF):
					discordPresence.details = "Staff Mode (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "reverse-mode";
					discordPresence.smallImageText = " Staff Weapons Broken";
					break;
				case (MMATCH_GAMETYPE_SNIPERMODE):
					discordPresence.details = "Sniper Mode (Team)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "nks-mode";
					discordPresence.smallImageText = " Sniper Mode";
					if (ZGetGameTypeManager()->IsTeamGame(ZGetGame()->GetMatch()->GetMatchType()))
					{
						int currSector = ZGetGame()->GetMatch()->GetCurrRound() + 1;
						int sectorCount = ZGetGame()->GetMatch()->GetRoundCount();

						discordPresence.partySize = currSector;
						discordPresence.partyMax = sectorCount;
					}
					break;
				case (MMATCH_GAMETYPE_TURBO):
					discordPresence.details = "Turbo Mode (Solo)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "death-mode";
					discordPresence.smallImageText = " Turbo";
					break;
				default:
					discordPresence.details = "Unknown (????)";
					discordPresence.startTimestamp = eptime;
					discordPresence.endTimestamp = 0;
					discordPresence.smallImageKey = "death-mode";
					discordPresence.smallImageText = " Unknown Mode";
					break;
				}
			}

			char szText[256];
			sprintf(szText, "Kill: %d | Dead: %d", ZGetGame()->m_pMyCharacter->GetStatus().Ref().nKills, ZGetGame()->m_pMyCharacter->GetStatus().Ref().nDeaths);
			//sprintf_safe(szText, "[%03d] %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
			discordPresence.state = szText;

			char szChar[256];
			sprintf_safe(szChar, "%s Lv. %d", pChar->GetProperty()->GetName(), pChar->GetProperty()->nLevel);
			discordPresence.details = szChar;

			char szMapGameImage[256];
			char szMapGameName[256];
			sprintf_safe(szMapGameImage, "%d", ZGetGameClient()->GetMatchStageSetting()->GetMapIndex());
			sprintf_safe(szMapGameName, "%s", ZGetGameClient()->GetMatchStageSetting()->GetMapName());
			discordPresence.largeImageKey = szMapGameImage;
			discordPresence.largeImageText = szMapGameName;

		}
		break;
		case GUNZ_SHUTDOWN:
			char zsDesconn[512];
			sprintf_safe(zsDesconn, "Disconnected.");

			discordPresence.details = zsDesconn;
			discordPresence.state = "I won't play until later.";
			discordPresence.largeImageText = "Gunz The Duel";
			discordPresence.largeImageKey = "logo512";
			break;
		}

		Discord_UpdatePresence(&discordPresence);
	}
	else
	{
	    Discord_ClearPresence();
	}
}

bool ZDiscord::SetStateDiscord(GunzState nState)
{
	if (m_nStateDiscord == nState)
		return true;

	m_nStateDiscord = nState;

	return true;
}