#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PMWPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class BULLETANT_API APMWPlayerController : public APlayerController
{
	GENERATED_BODY()

#pragma region Controller Override

public:
	APMWPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

#pragma endregion

#pragma region Inputs
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inputs|Character")
	TObjectPtr<UInputMappingContext> IMC_Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inputs|Character")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inputs|Character")
	TObjectPtr<UInputAction> LeftClickAction;

#pragma endregion
};
