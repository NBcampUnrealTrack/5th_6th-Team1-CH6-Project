#include "Multiplayer/MultiplayerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemNames.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/KismetSystemLibrary.h"
#include "VoiceChat.h"
#include "EOSVoiceChat.h"
#include "IOnlineSubsystemEOS.h"
#include "IEOSSDKManager.h"
#include "EOSVoiceChatUser.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/MapConfig.h"

const FName UMultiplayerSubsystem::SETTING_ROOMNAME(TEXT("ROOMNAME"));
const FName UMultiplayerSubsystem::SETTING_MAXPLAYERS(TEXT("MAXPLAYERS"));
const FName UMultiplayerSubsystem::SEARCH_PRESENCE(TEXT("SEARCH_PRESENCE"));
const FName UMultiplayerSubsystem::NAME_GAMESESSION(TEXT("GameSession"));

void UMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString MapConfigPath = TEXT("/Game/BulletAnt/Framework/DA_MapConfig.DA_MapConfig");
	MapConfig = LoadObject<UMapConfig>(nullptr, *MapConfigPath);
}

void UMultiplayerSubsystem::EpicLogin()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
		if (Identity.IsValid() == true)
		{
			if (LoginHandle.IsValid() == true)
			{
				Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginHandle);
			}

			/*LoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateLambda([this, Identity](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
				{
					UKismetSystemLibrary::PrintString(GetWorld(), bWasSuccessful == true ? TEXT("LoginTrue") : TEXT("LoginFalse"));
				}));*/

			FOnlineAccountCredentials Credentials;
			Credentials.Type = TEXT("accountportal");
			Identity->Login(0, Credentials);
		}
	}
}

void UMultiplayerSubsystem::CreateSession(const FString& RoomName, int32 MaxPlayers)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false)
		return;

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = MaxPlayers;			// 최대 인원
	SessionSettings.bAllowJoinInProgress = true;				// 게임 중 난입 허용
	SessionSettings.bAllowJoinViaPresence = true;				// 에픽 친구 시스템 연동
	SessionSettings.bShouldAdvertise = true;					// 방 목록에 노출
	SessionSettings.bUsesPresence = true;						// 유저 상태 표시
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUseLobbiesIfAvailable = true;				// 로비 사용
	SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;		// 보이스챗 가능

	// 방 목록에서 보여줄 커스텀 세팅들 (이것들 이용해서 필터링 가능)
	//SessionSettings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_ROOMNAME, RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_MAXPLAYERS, MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SEARCH_PRESENCE, true, EOnlineDataAdvertisementType::ViaOnlineService);
	// 그냥 true로 보내면 설정 안됨. FString으로 변환해서 보내야 설정됨.
	//FString StrTrue = TEXT("true");
	//SessionSettings.Set(TEXT("RTCAUTOJOIN"), StrTrue, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnCreateSessionComplete);
	SessionInterface->CreateSession(0, NAME_GAMESESSION, SessionSettings);
}

void UMultiplayerSubsystem::SearchSessions(int32 MaxSearchCount)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false)
		return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = MaxSearchCount;

	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnFindSessionsComplete);
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UMultiplayerSubsystem::JoinSession(int32 Index)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false || SessionSearch.IsValid() == false)
		return;

	if (SessionSearch->SearchResults.Num() > Index)
	{
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnJoinSessionComplete);
		SessionInterface->JoinSession(0, NAME_GAMESESSION, SessionSearch->SearchResults[Index]);		// SessionName은 내가 붙인 방의 별명
	}
}

void UMultiplayerSubsystem::ServerTravelToLobby()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->GetNetMode() == NM_Client)
		return;

	if (IsValid(MapConfig) == false)
		return;

	FString Path = MapConfig->LobbyLevel.ToSoftObjectPath().ToString();
	FString MapName = FPackageName::ObjectPathToPackageName(Path);
	// 로비로 갈 때에는 non-seamless로 가야 문제가 안 생김. 게스트 모두 접속 후에는 ServerTravelToLevel로 seamless travel.
	FString TravelURL = FString::Printf(TEXT("%s?listen"), *MapName);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerTravel => %s"), *TravelURL));
	World->ServerTravel(TravelURL);
}

void UMultiplayerSubsystem::ServerTravelToLevel(const FString& LevelPath)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->GetNetMode() == NM_Client)
		return;

	FString TravelURL = FString::Printf(TEXT("%s?listen&seamless"), *LevelPath);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerTravel => %s"), *TravelURL));
	World->ServerTravel(TravelURL);
}

void UMultiplayerSubsystem::ClientTravel(FName SessionName)
{
	// 세션 참가 시, 반드시 호스트의 현재 레벨로 ClientTravel해서 따라가야 함
	// 이후엔 호스트의 ServerTravel에 자동으로 따라감
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false)
		return;

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString) == true)
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ClientTravel => %s"), *ConnectString));
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
}

int32 UMultiplayerSubsystem::GetPlayersInChannel()
{
	return VoiceChatUser->GetPlayersInChannel(VoiceChatUser->GetChannels()[0]).Num();
}

EVoiceChatTransmitMode UMultiplayerSubsystem::GetTransmitMode()
{
	return VoiceChatUser->GetTransmitMode();
}

void UMultiplayerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnCreateSession.Broadcast(SessionName, bWasSuccessful);

	if (bWasSuccessful == false)
		return;

	CurrentSessionName = SessionName;
	//SetupNotifications();
	/*TravelHandle = OnJoinVoiceChannel.AddLambda([this, SessionName](const FString& ChannelName, const FVoiceChatResult& Result)
		{
			if (Result.IsSuccess() == true)
			{
				ServerTravelToLobby();
			}
		});*/

	//InitializeVoiceChat();

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(
		Timer,
		this,
		&ThisClass::SetVoiceChatUser,
		2.0f);
}

void UMultiplayerSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	OnFindSessions.Broadcast(bWasSuccessful);

	if (bWasSuccessful == false)
		return;
	
	// 임시
	// UI 추가되면 목록에서 선택해서 참여
	if (SessionSearch->SearchResults.IsEmpty() == false)
	{
		JoinSession(0);
	}
}

void UMultiplayerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	OnJoinSession.Broadcast(SessionName, Result);

	if (Result != EOnJoinSessionCompleteResult::Success)
		return;

	CurrentSessionName = SessionName;
	TravelHandle = OnSetVoiceChatUser.AddLambda([this, SessionName]()
		{
			ClientTravel(SessionName);
		});

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(
		Timer,
		this,
		&ThisClass::SetVoiceChatUser,
		2.0f);
}

void UMultiplayerSubsystem::SetVoiceChatUser()
{
	IOnlineSubsystemEOS* EOS = static_cast<IOnlineSubsystemEOS*>(IOnlineSubsystem::Get("EOS"));
	IOnlineIdentityPtr Identity = Online::GetIdentityInterface(GetWorld());
	if (Identity.IsValid() == true && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
		VoiceChatUser = EOS->GetVoiceChatUserInterface(*UserId);
	}

	if (VoiceChatUser)
	{
		VoiceChatUser->SetAudioInputVolume(1.0f);
		VoiceChatUser->SetSetting(TEXT("Input.VADThreshold"), TEXT("0.0"));
		VoiceChatUser->SetAudioInputDeviceMuted(false);
		VoiceChatUser->SetAudioOutputDeviceMuted(false);
		VoiceChatUser->OnVoiceChatPlayerAdded().AddLambda([this](const FString& ChannelName, const FString& PlayerName)
			{
				VoiceChatUser->SetPlayerMuted(PlayerName, true);
				VoiceChatUser->SetPlayerMuted(PlayerName, false);
			});

		VoiceChatUser->OnVoiceChatPlayerTalkingUpdated().AddLambda([this](const FString& ChannelName, const FString& PlayerName, bool bIsTalking)
			{
				/*UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Voice Log -> [Channel: %s] Player: %s is %s"),
					*ChannelName, *PlayerName, bIsTalking ? TEXT("TALKING") : TEXT("SILENT")));*/
			});

		OnSetVoiceChatUser.Broadcast();

		if (TravelHandle.IsValid() == true)
		{
			OnSetVoiceChatUser.Remove(TravelHandle);
			TravelHandle.Reset();
		}
	}
}

void UMultiplayerSubsystem::SetVolume(int32 LocalUserNum, float InVolume)
{
	IOnlineIdentityPtr Identity = Online::GetIdentityInterface(GetWorld());
	if (Identity.IsValid() == true && Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
	{
		if (VoiceChatUser)
		{
			FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(LocalUserNum);
			VoiceChatUser->SetPlayerVolume(UserId->ToString(), InVolume);
		}
	}
}

void UMultiplayerSubsystem::SetMute(int32 LocalUserNum, bool bMute)
{
	IOnlineIdentityPtr Identity = Online::GetIdentityInterface(GetWorld());
	if (Identity.IsValid() == true && Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
	{
		if (VoiceChatUser)
		{
			FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(LocalUserNum);
			VoiceChatUser->SetPlayerMuted(UserId->ToString(), bMute);
		}
	}
}
