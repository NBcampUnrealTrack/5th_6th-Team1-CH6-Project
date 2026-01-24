#include "GA_Fire.h"

#include "BulletAnt/Weapon/BaseWeapon.h"
#include "BulletAnt/Weapon/Data/WeaponDataAsset.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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

	ServerFire(ActorInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_Fire::ServerFire(const FGameplayAbilityActorInfo* ActorInfo)
{
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor) return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;

	UObject* SourceObj = GetCurrentSourceObject();
	ABaseWeapon* Weapon = Cast<ABaseWeapon>(SourceObj);
	if (!Weapon || !Weapon->GetWeaponData()) return;

	float Range = Weapon->GetWeaponData()->Range;
	float Damage = Weapon->GetWeaponData()->BaseDamage;

	FVector TraceStart = GetTraceStart(AvatarActor, Weapon);
	FVector TraceEnd = GetTraceEnd(TraceStart, Range);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AvatarActor);

	bool bHit = AvatarActor->GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

#if WITH_EDITOR
	DrawDebugLine(
		AvatarActor->GetWorld(),
		TraceStart,
		bHit ? Hit.ImpactPoint : TraceEnd,
		FColor::Red,
		false,
		1.0f,
		0,
		1.0f
	);
#endif

	if (!bHit) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	if (!DamageEffectClass) return;

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.Damage")),
			Damage
		);

		ASC->ApplyGameplayEffectSpecToTarget(
			*SpecHandle.Data.Get(),
			HitActor->FindComponentByClass<UAbilitySystemComponent>()
		);
	}
}

FVector UGA_Fire::GetTraceStart(const AActor* AvatarActor, ABaseWeapon* Weapon) const
{
	UCameraComponent* Camera = AvatarActor->FindComponentByClass<UCameraComponent>();
	if (Camera)
	{
		return Camera->GetComponentLocation();
	}

	return Weapon->GetWeaponMesh()->GetSocketLocation(
		Weapon->GetMuzzleSocketName()
	);
}

FVector UGA_Fire::GetTraceEnd(const FVector& Start, float Range) const
{
	return Start + (GetAvatarActorFromActorInfo()->GetActorForwardVector() * Range);
}
