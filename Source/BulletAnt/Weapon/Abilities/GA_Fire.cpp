#include "GA_Fire.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Weapon/BaseWeapon.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Common/FireStartInterface.h"

static const FGameplayTag TAG_Data_Combat_Damage = FGameplayTag::RequestGameplayTag(TEXT("Data.Combat.Damage")); 

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
	
	AActor* SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor) return;

	UObject* SourceObject = GetCurrentSourceObject();
	if (!SourceObject) return;

	IFireStartInterface* FireStart = Cast<IFireStartInterface>(SourceObject);
	if (!FireStart) return;

	ABaseWeapon* SourceWeapon = Cast<ABaseWeapon>(SourceObject);
	if (!SourceWeapon) return;

	URangedWeaponDataAsset* RangedData = Cast<URangedWeaponDataAsset>(SourceWeapon->GetWeaponData());
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

	ApplyDamageEffect(ActorInfo, Hit.GetActor(), RangedData->BaseDamage, RangedData->HitEventTag);
}

void UGA_Fire::ApplyDamageEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	AActor* Target,
	float Damage,
	FGameplayTag HitTag)
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

	Spec.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, Damage);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

