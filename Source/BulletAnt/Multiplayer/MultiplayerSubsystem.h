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

UENUM()
enum class EPlatform
{
	Steam,
	EAS,
};

USTRUCT()
struct FRoomSetting
{
	GENERATED_BODY()

	UPROPERTY()
	FString RoomName = TEXT("DefaultRoom");
	UPROPERTY()
	int32 MaxPlayers = 1;
	UPROPERTY()
	FString Password;
	UPROPERTY()
	uint8 bIsPrivate : 1 = false;
};

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
	uint8 bIsPrivate : 1 = false;
	UPROPERTY()
	FString HashedPassword;
	UPROPERTY()
	TArray<FString> ParticipantNicknames;

	TSharedPtr<FOnlineSessionSearchResult> SearchResult;
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
	void Login();

	void EpicLogin();
	void SteamLogin();

	void ProcessEOSLogin(FString CredentialType, FString CredentialId, FString AuthToken);

	void CreateSession();
	void SearchSessions(int32 MaxSearchCount = 16);
	void JoinSession(int32 Index);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void JoinSessionById(FString TargetId);

	void ReadFriendsList();

	// 세션 비참여자가 방 정보에서 참가자 정보를 확인할 수 있게, SessionSettings에서 참여자 닉네임 업데이트
	void SyncNicknameToPlayerState(class ABAPlayerState* PlayerState);
	void UpdateSessionParticipants();

	bool GetRoomList(TArray<FRoomInfo>& OutRoomList);
	FString HashPassword(const FString& Password);

	FDelegateHandle BindOnSuccessLogin(const FOnSuccessLogin::FDelegate& Delegate);
	void UnbindOnSuccessLogin(const UObject* Object);

	FDelegateHandle BindOnCreateSession(const FOnCreateSession::FDelegate& Delegate);
	void UnbindOnCreateSession(const UObject* Object);
	void UnbindOnCreateSession(FDelegateHandle Handle);
	FDelegateHandle BindOnFindSessions(const FOnFindSessions::FDelegate& Delegate);
	void UnbindOnFindSessions(const UObject* Object);
	FDelegateHandle BindOnJoinSession(const FOnJoinSession::FDelegate& Delegate);
	void UnbindOnJoinSession(const UObject* Object);
	void UnbindOnJoinSession(FDelegateHandle Handle);

	// 호스트는 non-seamless travel로 로비레벨 이동 후 세션 생성
	void ServerTravelToLobby(const FRoomSetting& InSetting);
	// 로비 모집 종료 후에는 seamless travel로 함께 이동
	void ServerTravelToLevel(const FString& LevelPath);
	void ClientTravel(FName SessionName);

	int32 GetPlayersInChannel();
	enum class EVoiceChatTransmitMode GetTransmitMode();

	void SetVolume(int32 LocalUserNum, float InVolume);
	void SetMute(int32 LocalUserNum, bool bMute);

	// 로비에서만 호출해서 갱신
	void StartSessionHeartBeat();
	UFUNCTION()
	void UpdateSessionHearBeat();
	void StopSessionHeartBeat();


	FORCEINLINE bool IsLogin() const { return bLogin; }

private:
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnReadFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& InviterId, const FString& InviteData);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	void SetVoiceChatUser();
	bool IsVoiceChatReadyForClientTravel() const;

private:
	FDelegateHandle LoginHandle;
	FDelegateHandle FindSessionsHandle;
	FDelegateHandle JoinSessionHandle;

	FOnSuccessLogin OnSuccessLogin;

	FOnCreateSession OnCreateSession;
	FOnFindSessions OnFindSessions;
	FOnJoinSession OnJoinSession;

	FRoomSetting HostRoomSetting;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FTimerHandle SessionHeartbeatTimer;

	class IVoiceChat* VoiceChat = nullptr;
	IVoiceChatUser* VoiceChatUser = nullptr;
	FOnSetVoiceChatUser OnSetVoiceChatUser;
	uint8 bVoiceDelegatesBound : 1 = false;
	uint8 bVoiceChatInitialized : 1 = false;
	FTimerHandle ClientTravelHandle;

	FName CurrentSessionName;
	FString PlayerNickname;

	UPROPERTY()
	EPlatform Platform = EPlatform::Steam;

	uint8 bLogin : 1 = false;

	static const FName SETTING_ROOMNAME;
	static const FName SETTING_MAXPLAYERS;
	static const FName SETTING_PASSWORD;
	static const FName SETTING_PRIVATE;
	static const FName SETTING_PARTICIPANTNICKNAMES;
	static const FName SETTING_LASTHEARTBEAT;		// 유효한 세션인지 확인하기 위해, 주기적으로 호스트가 갱신 <= 오래됐으면 호스트는 나갔으나 살아있는 좀비 세션
	static const FName SEARCH_PRESENCE;
	
	static const FName NAME_GAMESESSION;

#pragma region Steam

public:
	void OnGetAuthTicketForWebApiCompleted(struct GetTicketForWebApiResponse_t* Response);
	void OnSteamRichPresenceUpdate(FriendRichPresenceUpdate_t* pCallback);
	void OnSteamJoinRequested(GameRichPresenceJoinRequested_t* pCallback);
	void OnSteamLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback);
	
private:
	void HandleSteamJoin(CSteamID HostID);

private:
	FTimerHandle SteamAuthTimer;
	HAuthTicket AuthTicketHandle;
	CCallbackManual<UMultiplayerSubsystem, GetTicketForWebApiResponse_t> CallbackGetTicketForWebApi;

	FTimerHandle JoinInviteTimer;
	CCallbackManual<UMultiplayerSubsystem, FriendRichPresenceUpdate_t> CallbackRichPresenceUpdate;
	CCallbackManual<UMultiplayerSubsystem, GameRichPresenceJoinRequested_t> CallbackJoinRequested;
	CCallbackManual<UMultiplayerSubsystem ,GameLobbyJoinRequested_t> CallbackLobbyJoinRequested;
	FDelegateHandle JoinSessionByIdHandle;

	FString PendingSessionTargetId;
	uint8 bPendingJoinById : 1 = false;

	uint8 bIsJoiningSteamInvitation : 1 = false;

#pragma endregion

};
