#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "BAPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedPlayerColor, FLinearColor);

class UHealthAttributeSet;
class UAmmoAttributeSet;

UCLASS()
class BULLETANT_API ABAPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABAPlayerState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* NewPlayerState) override;

public:
	void SetPlayerColorIdx(int32 NewIdx);
	UFUNCTION(BlueprintCallable)
	FLinearColor GetPlayerColor() const;

	FDelegateHandle BindOnChangedPlayerColor(const FOnChangedPlayerColor::FDelegate& Delegate);
	void UnbindOnChangedPlayerColor(const UObject* Object);
	void UnbindOnChangedPlayerColor(FDelegateHandle Handle);

protected:
	UFUNCTION()
	void OnRep_PlayerColorIdx();

protected:
	FOnChangedPlayerColor OnChangedPlayerColor;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColorIdx)
	int32 PlayerColorIdx = -1;

	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> PlayerColorTable;

#pragma region GAS 

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	const UHealthAttributeSet* GetHealthAttributeSet() const;
	const UAmmoAttributeSet* GetAmmoAttributeSet() const;

	void InitAbility();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UHealthAttributeSet* HealthAttributeSet;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbility;

	UPROPERTY()
	UAmmoAttributeSet* AmmoAttributeSet;

#pragma endregion
};
