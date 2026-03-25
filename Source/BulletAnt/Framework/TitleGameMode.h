#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TitleGameMode.generated.h"

UCLASS()
class BULLETANT_API ATitleGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ATitleGameMode();

	virtual void BeginPlay() override;

	void StartLogin();
	void OnSuccessLogin();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bAlreadyLogin : 1 = false;
};
