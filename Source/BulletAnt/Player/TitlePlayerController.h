
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

class UUW_TitleLayout;
class UUserWidget;
class UUISubsystem;
class UUW_TitleScreen;

UCLASS()
class BULLETANT_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATitlePlayerController();

	virtual void BeginPlay() override;

public:
	void ShowLoginPanel(bool bInShow);

private:
	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

	UPROPERTY()
	TObjectPtr<UUW_TitleScreen> TitleScreenWidget;

	UPROPERTY()
	uint8 bShowLoginPanel : 1 = true;
};
