#include "Multiplayer/MultiplayerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemNames.h"
#include "Kismet/KismetSystemLibrary.h"

const FName UMultiplayerSubsystem::SETTING_MAPNAME(TEXT("MAPNAME"));
const FName UMultiplayerSubsystem::SETTING_ROOMNAME(TEXT("ROOMNAME"));
const FName UMultiplayerSubsystem::SETTING_MAXPLAYERS(TEXT("MAXPLAYERS"));
const FName UMultiplayerSubsystem::SEARCH_PRESENCE(TEXT("SEARCH_PRESENCE"));
const FName UMultiplayerSubsystem::NAME_GAMESESSION(TEXT("GameSession"));

void UMultiplayerSubsystem::EpicLogin()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			if (LoginHandle.IsValid())
			{
				Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginHandle);
			}

			LoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateLambda([this, Identity](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
				{
					UKismetSystemLibrary::PrintString(GetWorld(), bWasSuccessful == true ? TEXT("LoginTrue") : TEXT("LoginFalse"));
					if (bWasSuccessful == true)
					{
						Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginHandle);
						LoginHandle.Reset();
					}
				}));

			// 2. 그 다음 로그인을 시도합니다.
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
	SessionSettings.NumPublicConnections = MaxPlayers;	// 최대 인원
	SessionSettings.bAllowJoinInProgress = true;		// 게임 중 난입 허용
	SessionSettings.bAllowJoinViaPresence = true;		// 에픽 친구 시스템 연동
	SessionSettings.bShouldAdvertise = true;			// 방 목록에 노출
	SessionSettings.bUsesPresence = true;				// 유저 상태 표시

	// 방 목록에서 보여줄 커스텀 정보들 (이것들 이용해서 필터링 가능)
	//SessionSettings.Set(SETTING_MAPNAME, FString("Lobby"), EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_ROOMNAME, RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_MAXPLAYERS, MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);

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

	//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
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

void UMultiplayerSubsystem::ServerTravelToLevel(const FString& LevelPath)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->GetNetMode() == NM_Client)
		return;

	FString TravelURL = FString::Printf(TEXT("%s?listen"), *LevelPath);
	World->ServerTravel(TravelURL);
}

void UMultiplayerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnCreateSession.Broadcast(SessionName, bWasSuccessful);

	// 임시
	if (bWasSuccessful == true)
	{
		ServerTravelToLevel(TEXT("/Game/SpaceBase/Maps/MainLevel?listen"));
	}
}

void UMultiplayerSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	OnFindSessions.Broadcast(bWasSuccessful);
	
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

	// 세션 참가 시, 반드시 호스트의 현재 레벨로 ClientTravel해서 따라가야 함
	// 이후엔 호스트의 ServerTravel에 자동으로 따라감
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() == false)
		return;

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		UKismetSystemLibrary::PrintString(GetWorld(), ConnectString);
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
}
