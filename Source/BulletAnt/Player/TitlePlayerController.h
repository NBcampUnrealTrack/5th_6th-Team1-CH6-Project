
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

private:
	UFUNCTION()
	void HandleJoinRequested(const FText& InIpPort);

	UFUNCTION()
	void HandleOptionRequested();

private:
	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

	UPROPERTY()
	TObjectPtr<UUW_TitleScreen> TitleScreenWidget;
};
