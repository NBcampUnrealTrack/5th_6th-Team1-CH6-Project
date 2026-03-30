#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class BULLETANT_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	virtual void BeginPlay() override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// seamless travel에서는 Login/Logout 발생 안 함.
	// 호스트, 게스트 처음 접속했을 때에만 PostLogin / 나가면 Logout
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	void UpdateSessionParticipants();

protected:
	void CreateRoom();

	void OnCreateRoom(FName SessionName, bool bWasSuccessful);

protected:
	FDelegateHandle CreateRoomHandle;
};
