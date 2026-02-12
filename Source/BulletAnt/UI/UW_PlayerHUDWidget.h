#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PlayerHUDWidget.generated.h"

class ABACharacter;
class UProgressBar;

UCLASS()
class BULLETANT_API UUW_PlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	ABACharacter* OwnerCharacter;

	virtual void NativeConstruct() override;
protected:
	UFUNCTION()
	void UpdateHealth(float Current, float Max);
	
};
