#include "GA_Fire.h"

#include "BulletAnt/Weapon/BaseRangedWeapon.h"
#include "BulletAnt/Weapon/Data/WeaponDataAsset.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

UGA_Fire::UGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Weapon.Fire")));
}

void UGA_Fire::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	FireOnce(ActorInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_Fire::FireOnce(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo) return;
	
	AActor* SourceActor = Cast<AActor>(GetCurrentSourceObject());
	if (!SourceActor) return;

	IFireStartInterface* FireStart = Cast<IFireStartInterface>(SourceActor);
	if (!FireStart) return;

	URangedWeaponDataAsset* RangedData = Cast<URangedWeaponDataAsset>(WeaponData);
	if (!RangedData) return;

	const FVector Start = FireStart->GetFireStartLocation();
	const FVector End = Start + FireStart->GetFireDirection() * RangedData->Range;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SourceActor);

	bool bHit = SourceActor->GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	DrawDebugLine(
		SourceActor->GetWorld(),
		Start,
		bHit ? Hit.ImpactPoint : End,
		FColor::Red,
		false,
		1.f,
		0,
		1.f
	);

	if (!bHit) return;

	ApplyDamageEffect(ActorInfo, Hit.GetActor(), RangedData->BaseDamage);
}

void UGA_Fire::ApplyDamageEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	AActor* Target,
	float Damage)
{
	if (!Target) return;

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC) return;

	UAbilitySystemComponent* TargetASC =
		Target->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC) return;

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, SourceASC->MakeEffectContext());

	if (!Spec.IsValid()) return;
	UE_LOG(LogTemp, Error, TEXT("ApplyDamageEffect 4"));
	Spec.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(TEXT("Event.Combat.Hit")),
		Damage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

