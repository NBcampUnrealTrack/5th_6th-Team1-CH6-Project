#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PlayerHUDWidget.generated.h"

class ABACharacter;
class UProgressBar;
class UTextBlock;

UCLASS()
class BULLETANT_API UUW_PlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	ABACharacter* OwnerCharacter;

	UFUNCTION()
	void UpdateAmmo(float Current, float Max);

	virtual void NativeConstruct() override;
protected:
	UFUNCTION()
	void UpdateHealth(float Current, float Max);
	
};
