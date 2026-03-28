#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "BAPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedPlayerColor, FLinearColor);

class UHealthAttributeSet;
class UAmmoAttributeSet;
class UEXPAttributeSet;

UCLASS()
class BULLETANT_API ABAPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABAPlayerState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* NewPlayerState) override;
	virtual void BeginPlay() override;
	virtual void PostNetInit() override;
	virtual void OnRep_PlayerName() override;

public:
	void SetPlayerColorIdx(int32 NewIdx);
	UFUNCTION(BlueprintCallable)
	FLinearColor GetPlayerColor() const;

	FDelegateHandle BindOnChangedPlayerColor(const FOnChangedPlayerColor::FDelegate& Delegate);
	void UnbindOnChangedPlayerColor(const UObject* Object);
	void UnbindOnChangedPlayerColor(FDelegateHandle Handle);

	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerName(const FString& NewName);
	FORCEINLINE bool IsSetNickname() const { return bSetNickname; }

protected:
	UFUNCTION()
	void OnRep_PlayerColorIdx();

protected:
	FOnChangedPlayerColor OnChangedPlayerColor;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColorIdx)
	int32 PlayerColorIdx = -1;

	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> PlayerColorTable;

	UPROPERTY()
	uint8 bSetNickname : 1 = false;

#pragma region GAS 

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UHealthAttributeSet* GetHealthAttributeSet();
	const UAmmoAttributeSet* GetAmmoAttributeSet() const;
	const UEXPAttributeSet* GetEXPAttributeSet() const;

	void InitAbility();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbility;

	UPROPERTY()
	UAmmoAttributeSet* AmmoAttributeSet;

	UPROPERTY()
	UHealthAttributeSet* HealthAttributeSet;

	UPROPERTY()
	UEXPAttributeSet* EXPAttributeSet;


#pragma endregion

#pragma region GameResult
public:
	void AddKillCount();
	const uint32 GetKillCount();

	void AddTotalDamage(float Damage);
	const uint64 GetTotalDamage();

protected:
	UPROPERTY(Replicated)
	uint32 KillCount = 0;

	UPROPERTY(Replicated)
	uint64 TotalDamage = 0;
#pragma endregion
};
