#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSubsystem.generated.h"

class FOnlineSessionSearch;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSession, FName, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessions, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnJoinSession, FName, EOnJoinSessionCompleteResult::Type);	// Type이 그냥 enum이라 Dynamic Delegate 사용 불가

UCLASS()
class BULLETANT_API UMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void EpicLogin();

	void CreateSession(const FString& RoomName = TEXT("DefaultRoom"), int32 MaxPlayers = 4);
	void SearchSessions(int32 MaxSearchCount = 16);
	void JoinSession(int32 Index);

	void ServerTravelToLevel(const FString& LevelPath);

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	FDelegateHandle LoginHandle;

	FOnCreateSession OnCreateSession;
	FOnFindSessions OnFindSessions;
	FOnJoinSession OnJoinSession;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	static const FName SETTING_MAPNAME;
	static const FName SETTING_ROOMNAME;
	static const FName SETTING_MAXPLAYERS;
	static const FName SEARCH_PRESENCE;
	
	// 세션 이름(게임, 보이스챗 등 구분)
	static const FName NAME_GAMESESSION;
};
