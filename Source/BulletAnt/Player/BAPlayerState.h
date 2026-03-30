#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "BAPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedPlayerColor, FLinearColor);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedPlayerName, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedPlayerLevel, float);

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

	FDelegateHandle BindOnChangedPlayerName(const FOnChangedPlayerName::FDelegate& Delegate);
	void UnbindOnChangedPlayerName(const UObject* Object);
	void UnbindOnChangedPlayerName(FDelegateHandle Handle);

	FDelegateHandle BindOnChangedPlayerLevel(const FOnChangedPlayerLevel::FDelegate& Delegate);
	void UnbindOnChangedPlayerLevel(const UObject* Object);
	void UnbindOnChangedPlayerLevel(FDelegateHandle Handle);

	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerName(const FString& NewName);
	FORCEINLINE bool IsSetNickname() const { return bSetNickname; }

	void SetPlayerLevel(float InLevel);
	FORCEINLINE float GetPlayerLevel() const { return PlayerLevel; }

protected:
	UFUNCTION()
	void OnRep_PlayerColorIdx();

	UFUNCTION()
	void OnRep_PlayerLevel();

protected:
	FOnChangedPlayerColor OnChangedPlayerColor;
	FOnChangedPlayerName OnChangedPlayerName;
	FOnChangedPlayerLevel OnChangedPlayerLevel;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColorIdx)
	int32 PlayerColorIdx = -1;

	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> PlayerColorTable;

	UPROPERTY()
	uint8 bSetNickname : 1 = false;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerLevel)
	float PlayerLevel = 1;

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

	void AddBuildCount();
	const uint32 GetBuildCount();

protected:
	UPROPERTY(Replicated)
	uint32 KillCount = 0;

	UPROPERTY(Replicated)
	uint64 TotalDamage = 0;

	UPROPERTY(Replicated)
	uint32 BuildCount = 0;
#pragma endregion
};
