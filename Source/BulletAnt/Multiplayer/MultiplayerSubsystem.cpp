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
#include "EOSSettings.h"
#include "steam/steam_api.h"
#include "Framework/BAGameInstance.h"
#include "Misc/SecureHash.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Player/BAPlayerState.h"
#include "Framework/BAGameState.h"

const FName UMultiplayerSubsystem::SETTING_ROOMNAME(TEXT("ROOMNAME"));
const FName UMultiplayerSubsystem::SETTING_MAXPLAYERS(TEXT("MAXPLAYERS"));
const FName UMultiplayerSubsystem::SETTING_PASSWORD(TEXT("PASSWORD"));
const FName UMultiplayerSubsystem::SETTING_PRIVATE(TEXT("PRIVATE"));
const FName UMultiplayerSubsystem::SETTING_PARTICIPANTNICKNAMES(TEXT("ParticipantNicknames"));
const FName UMultiplayerSubsystem::SEARCH_PRESENCE(TEXT("SEARCH_PRESENCE"));
const FName UMultiplayerSubsystem::NAME_GAMESESSION(TEXT("GameSession"));

void UMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMultiplayerSubsystem::Login()
{
	if (bLogin == true)
		return;

	IOnlineSubsystem* SteamSub = Online::GetSubsystem(GetWorld(), STEAM_SUBSYSTEM);
	if (SteamSub && SteamAPI_Init() == true)
	{
		SteamLogin();
	}
	else
	{
		EpicLogin();
	}
}

void UMultiplayerSubsystem::EpicLogin()
{
	Platform = EPlatform::EAS;

	ProcessEOSLogin(TEXT("accountportal"), TEXT(""), TEXT(""));
}

void UMultiplayerSubsystem::SteamLogin()
{
	Platform = EPlatform::Steam;

	if (SteamAPI_Init() == false || SteamAPI_IsSteamRunning() == false || SteamUser() == nullptr)
	{
		// SteamAPI_RestartAppIfNecessary는 프로세스가 스팀을 통해 정상적으로 실행되지 않았다면,
		// 현재 프로세스를 죽이고 스팀 런처를 통해 다시 실행하는 함수.
		// AppId 480 (공용 테스트 ID)로 Shipping에서 이 함수가 실행되면, 새로 켜질 떄마다 꺼지고 재실행이 반복됨.
		/*if (SteamAPI_RestartAppIfNecessary(480) == true)
		{
			FPlatformMisc::RequestExit(false);
		}*/

		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(
			Timer,
			[WeakThis = TWeakObjectPtr(this)]()
			{
				if (WeakThis.IsValid() == true)
				{
					WeakThis->SteamLogin();
				}
			},
			2.0f,
			false);
		return;
	}

	CallbackGetTicketForWebApi.Register(this, &ThisClass::OnGetAuthTicketForWebApiCompleted);

	constexpr char ApiTarget[] = "epiconlineservices";
	AuthTicketHandle = SteamUser()->GetAuthTicketForWebApi(ApiTarget);

	GetWorld()->GetTimerManager().SetTimer(
		SteamAuthTimer,
		[this]()
		{
			UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Try Get Callback"));
			SteamAPI_RunCallbacks();
		},
		0.5f,
		true);

	//uint8 Ticket[1024];
	//uint32 TicketSize = 0;

	//HAuthTicket AuthTicket =
	//	SteamUser()->GetAuthSessionTicket(Ticket, sizeof(Ticket), &TicketSize, nullptr);
	//FString Base64Ticket = FBase64::Encode(Ticket, TicketSize); //FString::FromHexBlob(Ticket, TicketSize);
	//if (TicketSize > 0)
	//{
	//	FString HexTicket;

	//	for (uint32 i = 0; i < TicketSize; i++)
	//	{
	//		HexTicket += FString::Printf(TEXT("%02x"), Ticket[i]);
	//	}

	//	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("HexTicket: %s"), *HexTicket));

	//	ProcessEOSLogin(
	//		TEXT("externalauth:SteamSessionTicket"),
	//		TEXT(""),
	//		Base64Ticket);
	//}
}

void UMultiplayerSubsystem::ProcessEOSLogin(FString CredentialType, FString CredentialId, FString AuthToken)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld(), EOS_SUBSYSTEM);

	// 게임 켜진 이후에 플랫폼 켜지면, EOSSubsystem이 안 만들어짐. 언리얼에서 만드는 기능도 제공 안해줌.
	// 반드시 게임 끄고 플랫폼 먼저 켜지면 게임 켜야 함

	if (Subsystem)
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("EOSSubsystem Valid"));
		IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
		if (Identity.IsValid() == true)
		{
			//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Identity Valid"));
			if (LoginHandle.IsValid() == true)
			{
				Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginHandle);
			}

			LoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(
				this, &ThisClass::OnLoginComplete));

			//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Start Login %s"), *AuthToken));
			FOnlineAccountCredentials Credentials;
			Credentials.Type = CredentialType;
			Credentials.Id = CredentialId;
			Credentials.Token = AuthToken;
			Identity->Login(0, Credentials);
		}
		else
		{
			//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Identity not valid"));
		}
	}
	else
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("EOS Subsystem not valid"));
	}
}

void UMultiplayerSubsystem::CreateSession()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false)
		return;

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = HostRoomSetting.MaxPlayers;			// 최대 인원
	SessionSettings.bAllowJoinInProgress = true;								// 게임 중 난입 허용
	SessionSettings.bAllowJoinViaPresence = true;								// 에픽 친구 시스템 연동
	SessionSettings.bShouldAdvertise = true;									// 방 목록에 노출
	SessionSettings.bUsesPresence = true;										// 유저 상태 표시
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUseLobbiesIfAvailable = true;								// 로비 사용
	SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;						// 보이스챗 가능

	// 방 목록에서 보여줄 커스텀 세팅들 (이것들 이용해서 필터링 가능)
	//SessionSettings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_ROOMNAME, HostRoomSetting.RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_MAXPLAYERS, HostRoomSetting.MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_PASSWORD, HashPassword(HostRoomSetting.Password), EOnlineDataAdvertisementType::ViaOnlineService);
	bool bIsPrivateRoom = HostRoomSetting.bIsPrivate;
	SessionSettings.Set(SETTING_PRIVATE, bIsPrivateRoom, EOnlineDataAdvertisementType::ViaOnlineService);
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
	if (SessionSearch->SearchResults.IsValidIndex(Index) == true)
	{
		JoinSession(SessionSearch->SearchResults[Index]);
	}
}

void UMultiplayerSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld(), EOS_SUBSYSTEM);
	if (SessionInterface.IsValid() == false || SearchResult.IsValid() == false)
		return;

	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnJoinSessionComplete);
	SessionInterface->JoinSession(0, NAME_GAMESESSION, SearchResult);		// SessionName은 내가 붙인 방의 별명
}

void UMultiplayerSubsystem::JoinSessionById(FString TargetId)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), TargetId);

	PendingSessionTargetId = TargetId;

	JoinSessionByIdHandle = OnFindSessions.AddLambda(
		[this](bool bWasSuccessful)
		{
			OnFindSessions.Remove(JoinSessionByIdHandle);
			JoinSessionByIdHandle.Reset();

			if (bWasSuccessful == false || SessionSearch.IsValid() == false)
				return;

			for (const auto& Result : SessionSearch->SearchResults)
			{
				FString FoundId = Result.Session.GetSessionIdStr();
				//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s / %s"), *FoundId, *PendingSessionTargetId));
				
				if (Result.Session.GetSessionIdStr() == PendingSessionTargetId)
				{
					//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Target Session Found! Joining..."));
					JoinSession(Result);
					return;
				}
			}
			//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Failed to find the specific session by ID."));
		});

	SearchSessions(100);
	bIsJoiningSteamInvitation = false;
}

void UMultiplayerSubsystem::ReadFriendsList()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld(), STEAM_SUBSYSTEM);
	if (Subsystem == nullptr)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Subsystem not valid"));
		return;
	}

	IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	if (FriendsInterface.IsValid() == false)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("FriendsInterface not valid"));
		return;
	}

	FriendsInterface->ReadFriendsList(0, TEXT("Default"), FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsComplete));
}

void UMultiplayerSubsystem::SyncNicknameToPlayerState()
{
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (IsValid(PC) == false)
		return;

	ABAPlayerState* PS = Cast<ABAPlayerState>(PC->PlayerState);
	if (IsValid(PS) == false)
		return;

	PS->Server_UpdatePlayerName(PlayerNickname);
}

void UMultiplayerSubsystem::UpdateSessionParticipants()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
		return;

	AGameStateBase* GS = World->GetGameState();
	TArray<FString> PlayerNames;
	if (IsValid(GS) == true)
	{
		const auto& PlayerArray = GS->PlayerArray;
		for (APlayerState* PS : PlayerArray)
		{
			ABAPlayerState* BAPS = Cast<ABAPlayerState>(PS);
			if (IsValid(BAPS) == true)
			{
				PlayerNames.Add(BAPS->IsSetNickname() == true ? PS->GetPlayerName() : TEXT("Loading..."));
			}
		}
	}

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false)
		return;

	FOnlineSessionSettings* CurrentSettings = SessionInterface->GetSessionSettings(NAME_GAMESESSION);
	if (CurrentSettings == nullptr)
		return;

	FString CombinedNames = FString::Join(PlayerNames, TEXT(","));
	CurrentSettings->Set(SETTING_PARTICIPANTNICKNAMES, CombinedNames, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionInterface->UpdateSession(NAME_GAMESESSION, *CurrentSettings);

	UKismetSystemLibrary::PrintString(GetWorld(), CombinedNames);
}

bool UMultiplayerSubsystem::GetRoomList(TArray<FRoomInfo>& OutRoomList)
{
	if (SessionSearch.IsValid() == false)
		return false;

	const auto& SearchResult = SessionSearch->SearchResults;
	if (SearchResult.IsEmpty() == true)
		return false;

	for (int32 ResultIdx = 0; ResultIdx < SearchResult.Num(); ++ResultIdx)
	{
		FRoomInfo NewInfo;
		NewInfo.RoomIdx = ResultIdx;
		const auto& SessionSetting = SearchResult[ResultIdx].Session.SessionSettings;
		if (SessionSetting.Get(SETTING_ROOMNAME, NewInfo.RoomName) == false)
		{
			NewInfo.RoomName = TEXT("DefaultRoom");
		}
		NewInfo.MaxPlayers = SessionSetting.NumPublicConnections;
		NewInfo.CurrentPlayers = NewInfo.MaxPlayers - SearchResult[ResultIdx].Session.NumOpenPublicConnections;		// NumOpenPublicConnections : 남은 자리 수
		bool bIsPrivate = false;
		SessionSetting.Get(SETTING_PRIVATE, bIsPrivate);
		NewInfo.bIsPrivate = bIsPrivate;
		SessionSetting.Get(SETTING_PASSWORD, NewInfo.HashedPassword);
		NewInfo.SearchResult = MakeShared<FOnlineSessionSearchResult>(SearchResult[ResultIdx]);

		FString CombinedNames;
		if (SessionSetting.Settings.Contains(SETTING_PARTICIPANTNICKNAMES) == true)
		{
			SessionSetting.Settings.Find(SETTING_PARTICIPANTNICKNAMES)->Data.GetValue(CombinedNames);
			CombinedNames.ParseIntoArray(NewInfo.ParticipantNicknames, TEXT(","), true);
		}

		OutRoomList.Add(NewInfo);
	}
	return true;
}

FString UMultiplayerSubsystem::HashPassword(const FString& Password)
{
	if (Password.IsEmpty() == true)
		return FString();

	return FMD5::HashAnsiString(*Password);
}

FDelegateHandle UMultiplayerSubsystem::BindOnSuccessLogin(const FOnSuccessLogin::FDelegate& Delegate)
{
	return OnSuccessLogin.Add(Delegate);
}

void UMultiplayerSubsystem::UnbindOnSuccessLogin(const UObject* Object)
{
	OnSuccessLogin.RemoveAll(Object);
}

FDelegateHandle UMultiplayerSubsystem::BindOnCreateSession(const FOnCreateSession::FDelegate& Delegate)
{
	return OnCreateSession.Add(Delegate);
}

void UMultiplayerSubsystem::UnbindOnCreateSession(const UObject* Object)
{
	OnCreateSession.RemoveAll(Object);
}

void UMultiplayerSubsystem::UnbindOnCreateSession(FDelegateHandle Handle)
{
	OnCreateSession.Remove(Handle);
}

FDelegateHandle UMultiplayerSubsystem::BindOnFindSessions(const FOnFindSessions::FDelegate& Delegate)
{
	return OnFindSessions.Add(Delegate);
}

void UMultiplayerSubsystem::UnbindOnFindSessions(const UObject* Object)
{
	OnFindSessions.RemoveAll(Object);
}

void UMultiplayerSubsystem::ServerTravelToLobby(const FRoomSetting& InSetting)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->GetNetMode() == NM_Client)
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Lobby Travel Failed 0"));
		return;
	}

	HostRoomSetting = InSetting;

	UBAGameInstance* GameInstance = Cast<UBAGameInstance>(GetGameInstance());
	UMapConfig* MapConfig = IsValid(GameInstance) == true ? GameInstance->GetMapConfig() : nullptr;
	if (IsValid(MapConfig) == false)
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Lobby Travel Failed 1"));
		return;
	}

	//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Lobby Travel 0"));
	FString Path = MapConfig->LobbyLevel.ToSoftObjectPath().ToString();
	FString MapName = FPackageName::ObjectPathToPackageName(Path);
	// 로비로 갈 때에는 non-seamless로 가야 문제가 안 생김. 게스트 모두 접속 후에는 ServerTravelToLevel로 seamless travel.
	FString TravelURL = FString::Printf(TEXT("%s?listen"), *MapName);
	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerTravel => %s"), *TravelURL));
	//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Lobby Travel 1"));
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

void UMultiplayerSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	FString StrSuccess = bWasSuccessful == true ? TEXT("Login Success") : TEXT("Login Failed");
	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s, %s"), *StrSuccess, *Error));

	if (bWasSuccessful == false)
		return;

	bLogin = true;

	if (Platform == EPlatform::Steam)
	{
		PlayerNickname = FString(UTF8_TO_TCHAR(SteamFriends()->GetPersonaName()));

		// connect 더미 데이터를 이용한 버튼 활성화 이후, RichPresence로 전달한 SessionID로 방 참여
		// 이를 위한 Callback 등록
		CallbackRichPresenceUpdate.Register(this, &ThisClass::OnSteamRichPresenceUpdate);
		CallbackJoinRequested.Register(this, &ThisClass::OnSteamJoinRequested);
		CallbackLobbyJoinRequested.Register(this, &ThisClass::OnSteamLobbyJoinRequested);

		GetWorld()->GetTimerManager().SetTimer(
			JoinInviteTimer,
			[this]()
			{
				//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Try Get Callback"));
				SteamAPI_RunCallbacks();
			},
			0.5f,
			true);
	}
	else
	{
		IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface(GetWorld());
		if (IdentityInterface.IsValid() == true)
		{
			PlayerNickname = IdentityInterface->GetPlayerNickname(LocalUserNum);
		}
	}

	IOnlineSessionPtr SessionSteam = Online::GetSessionInterface(GetWorld(), STEAM_SUBSYSTEM);
	SessionSteam->AddOnSessionUserInviteAcceptedDelegate_Handle(
		FOnSessionUserInviteAcceptedDelegate::CreateUObject(
			this, &ThisClass::OnSessionUserInviteAccepted));

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld(), EOS_SUBSYSTEM);
	SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
		FOnSessionUserInviteAcceptedDelegate::CreateUObject(
			this, &ThisClass::OnSessionUserInviteAccepted));

	OnSuccessLogin.Broadcast();
	OnSuccessLogin.Clear();
}

void UMultiplayerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnCreateSession.Broadcast(SessionName, bWasSuccessful);

	if (bWasSuccessful == false)
		return;

	CurrentSessionName = SessionName;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld(), EOS_SUBSYSTEM);
	if (SessionInterface.IsValid() == true)
	{
		FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GAMESESSION);
		if (NamedSession != nullptr && SteamFriends())
		{
			//FString Old = TCHAR_TO_UTF8(*NamedSession->GetSessionIdStr());
			FString SessionId = NamedSession->GetSessionIdStr();
			FTCHARToUTF8 Converter(*SessionId);
			FString New = Converter.Get();
			//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s / %s"), *Old, *New));
			SteamFriends()->SetRichPresence("connect", "1");	// 스팀 파싱 에러를 피하기 위한 더미
			SteamFriends()->SetRichPresence("EOS_SESSION_ID", Converter.Get());
		}
	}

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

void UMultiplayerSubsystem::OnReadFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld(), STEAM_SUBSYSTEM);
	if (Subsystem == nullptr)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Subsystem not valid"));
		return;
	}

	IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	if (FriendsInterface.IsValid() == false)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("FriendsInterface not valid"));
		return;
	}

	TArray<TSharedRef<FOnlineFriend>> OutFriends;
	FriendsInterface->GetFriendsList(0, TEXT("Default"), OutFriends);

	TArray<FString> Names;
	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("OutFriends valid"));
	for (const auto& Friend : OutFriends)
	{
		if (Friend->GetDisplayName() == TEXT("cf"))
		{
			IOnlineSubsystem* EOSSubsystem = Online::GetSubsystem(GetWorld(), EOS_SUBSYSTEM);
			if (EOSSubsystem == nullptr)
			{
				UKismetSystemLibrary::PrintString(GetWorld(), TEXT("EOS_SUBSYSTEM not valid"));
			}
			else
			{
				IOnlineSessionPtr Session = EOSSubsystem->GetSessionInterface();
				if (Session.IsValid() == false)
				{
					UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Session not valid"));
				}
				else
				{
					FNamedOnlineSession* NamedSession = Session->GetNamedSession(NAME_GAMESESSION);
					if (NamedSession == nullptr)
					{
						UKismetSystemLibrary::PrintString(GetWorld(), TEXT("NamedSession not valid"));
					}
					else
					{
						SteamFriends()->SetRichPresence("connect", TCHAR_TO_UTF8(*NamedSession->GetSessionIdStr()));
						//FUniqueNetIdRepl TargetId = Friend->GetUserId();
						//Session->SendSessionInviteToFriend(0, NAME_GAMESESSION, *TargetId.GetUniqueNetId());
						UKismetSystemLibrary::PrintString(GetWorld(), TEXT("SendSessionInviteToFriend"));
					}
				}
			}
		}
		Names.Add(Friend->GetDisplayName());
	}
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Join(Names, TEXT(",")));
}

void UMultiplayerSubsystem::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& InviterId, const FString& InviteData)
{
	SearchSessions();
}

void UMultiplayerSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful == false)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("OnSessionUserInviteAccepted failed"));
		return;
	}

	if (InviteResult.IsValid() == false)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("InviteResult not valid"));
		return;
	}

	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("JoinSession"));
	JoinSessionById(InviteResult.Session.GetSessionIdStr());
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

void UMultiplayerSubsystem::OnGetAuthTicketForWebApiCompleted(GetTicketForWebApiResponse_t* Response)
{
	GetTicketForWebApiResponse_t LocalResponse = *Response;

	AsyncTask(ENamedThreads::GameThread, [this, LocalResponse]()
		{
			if (LocalResponse.m_hAuthTicket != AuthTicketHandle)
				return;

			AuthTicketHandle = 0;

			GetWorld()->GetTimerManager().ClearTimer(SteamAuthTimer);

			if (LocalResponse.m_eResult != k_EResultOK)
			{
				UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Steam WebAPI Ticket not valid"));
				return;
			}

			FString TokenString = FString::FromHexBlob(LocalResponse.m_rgubTicket, LocalResponse.m_cubTicket);

			UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Start Login"));
			ProcessEOSLogin(
				TEXT("externalauth:SteamSessionTicket"),
				TEXT(""),
				TokenString);
		});
}

void UMultiplayerSubsystem::OnSteamRichPresenceUpdate(FriendRichPresenceUpdate_t* pCallback)
{
	// 상태가 변한 친구 확인
	HandleSteamJoin(pCallback->m_steamIDFriend);
}

void UMultiplayerSubsystem::OnSteamJoinRequested(GameRichPresenceJoinRequested_t* pCallback)
{
	HandleSteamJoin(pCallback->m_steamIDFriend);
}

void UMultiplayerSubsystem::OnSteamLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
	HandleSteamJoin(pCallback->m_steamIDFriend);
}

void UMultiplayerSubsystem::HandleSteamJoin(CSteamID HostID)
{
	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("OnSteamRichPresenceUpdate 0"));
	if (bIsJoiningSteamInvitation == true)
		return;

	// 호스트가 숨겨둔 진짜 EOS 세션 ID가 있는지 확인
	const char* RawEOSID = SteamFriends()->GetFriendRichPresence(HostID, "EOS_SESSION_ID");
	const char* ConnectKey = SteamFriends()->GetFriendRichPresence(HostID, "connect");

	UKismetSystemLibrary::PrintString(GetWorld(), FString(RawEOSID));

	// 친구가 '참여' 버튼을 눌렀거나 내 상태를 확인 중일 때 실행
	if (RawEOSID && strlen(RawEOSID) > 0)
	{
		FString TargetSessionId = FString(UTF8_TO_TCHAR(RawEOSID));
		//UKismetSystemLibrary::PrintString(GetWorld(), TargetSessionId);

		// 이미 세션에 들어가 있지 않은 경우에만 조인 시도
		IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld(), EOS_SUBSYSTEM);
		if (SessionInt->GetNamedSession(NAME_GAMESESSION) == nullptr)
		{
			UKismetSystemLibrary::PrintString(GetWorld(), TEXT("OnSteamRichPresenceUpdate 2"));
			bIsJoiningSteamInvitation = true;
			JoinSessionById(TargetSessionId);
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
