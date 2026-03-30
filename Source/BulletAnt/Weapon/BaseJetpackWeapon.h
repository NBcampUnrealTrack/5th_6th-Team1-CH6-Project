#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "BaseJetpackWeapon.generated.h"

class UNiagaraComponent;

UCLASS()
class BULLETANT_API ABaseJetpackWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jetpack", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* LeftFlame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jetpack", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* RightFlame;
public:
	ABaseJetpackWeapon();

	UFUNCTION()
	void OnRep_bJetpackActive();

	void SetbJetpackActive(bool InbJetpackActive);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_bJetpackActive)
	bool bJetpackActive = false;
};
