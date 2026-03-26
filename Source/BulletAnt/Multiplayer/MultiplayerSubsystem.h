#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END
#include "MultiplayerSubsystem.generated.h"

class FOnlineSessionSearch;
class IVoiceChatUser;
enum class EOnSessionParticipantLeftReason : uint8;

USTRUCT()
struct FRoomInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 RoomIdx = 0;
	UPROPERTY()
	FString RoomName;
	UPROPERTY()
	int32 CurrentPlayers = 0;
	UPROPERTY()
	int32 MaxPlayers = 0;
	UPROPERTY()
	TArray<FString> ParticipantNicknames;

	FOnlineSessionSearchResult SearchResult;
};

DECLARE_MULTICAST_DELEGATE(FOnSuccessLogin);
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

public:
	void EpicLogin();
	void SteamLogin();

	void ProcessEOSLogin(FString CredentialType, FString CredentialId, FString AuthToken);

	void CreateSession(const FString& RoomName = TEXT("DefaultRoom"), int32 MaxPlayers = 8);
	void SearchSessions(int32 MaxSearchCount = 16);
	void JoinSession(int32 Index);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	// 세션 비참여자가 방 정보에서 참가자 정보를 확인할 수 있게, SessionSettings에서 참여자 닉네임 업데이트
	void UpdateSessionParticipants(const TArray<FString>& ParticipantNicknames);

	bool GetRoomList(TArray<FRoomInfo>& OutRoomList);

	FDelegateHandle BindOnSuccessLogin(const FOnSuccessLogin::FDelegate& Delegate);
	void UnbindOnSuccessLogin(const UObject* Object);

	FDelegateHandle BindOnCreateSession(const FOnCreateSession::FDelegate& Delegate);
	void UnbindOnCreateSession(const UObject* Object);
	void UnbindOnCreateSession(FDelegateHandle Handle);
	FDelegateHandle BindOnFindSessions(const FOnFindSessions::FDelegate& Delegate);
	void UnbindOnFindSessions(const UObject* Object);

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

	FOnSuccessLogin OnSuccessLogin;

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

	static const FName SETTING_ROOMNAME;
	static const FName SETTING_MAXPLAYERS;
	static const FName SETTING_PARTICIPANTNICKNAMES;
	static const FName SEARCH_PRESENCE;
	
	static const FName NAME_GAMESESSION;

#pragma region Steam

public:
	void OnGetAuthTicketForWebApiCompleted(struct GetTicketForWebApiResponse_t* Response);

private:
	FTimerHandle SteamAuthTimer;

	HAuthTicket AuthTicketHandle;
	CCallbackManual<UMultiplayerSubsystem, GetTicketForWebApiResponse_t> CallbackGetTicketForWebApi;

#pragma endregion

};
