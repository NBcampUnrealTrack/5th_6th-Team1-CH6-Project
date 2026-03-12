#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSubsystem.generated.h"

class FOnlineSessionSearch;
class IVoiceChatUser;
enum class EOnSessionParticipantLeftReason : uint8;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSession, FName, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessions, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnJoinSession, FName, EOnJoinSessionCompleteResult::Type);	// Type이 그냥 enum이라 Dynamic Delegate 사용 불가
DECLARE_MULTICAST_DELEGATE(FOnSetVoiceChatUser);

UCLASS()
class BULLETANT_API UMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void EpicLogin();

	void CreateSession(const FString& RoomName = TEXT("DefaultRoom"), int32 MaxPlayers = 4);
	void SearchSessions(int32 MaxSearchCount = 16);
	void JoinSession(int32 Index);

	// 호스트는 non-seamless travel로 로비레벨 이동 후 세션 생성
	void ServerTravelToLobby();
	// 로비 모집 종료 후에는 seamless travel로 함께 이동
	void ServerTravelToLevel(const FString& LevelPath);
	void ClientTravel(FName SessionName);

	int32 GetPlayersInChannel();
	enum class EVoiceChatTransmitMode GetTransmitMode();

	void SetVolume(int32 LocalUserNum, float InVolume);
	void SetMute(int32 LocalUserNum, bool bMute);

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void SetVoiceChatUser();


private:
	FDelegateHandle LoginHandle;

	FOnCreateSession OnCreateSession;
	FOnFindSessions OnFindSessions;
	FOnJoinSession OnJoinSession;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	class IVoiceChat* VoiceChat = nullptr;
	IVoiceChatUser* VoiceChatUser = nullptr;
	FOnSetVoiceChatUser OnSetVoiceChatUser;
	// Travel 이후에 TravelHandle 이용해서 바인딩되었던 Travel 람다식 제거
	FDelegateHandle TravelHandle;

	FName CurrentSessionName;

	UPROPERTY()
	TObjectPtr<class UMapConfig> MapConfig;

	static const FName SETTING_ROOMNAME;
	static const FName SETTING_MAXPLAYERS;
	static const FName SEARCH_PRESENCE;
	
	static const FName NAME_GAMESESSION;
};
