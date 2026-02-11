#include "GAS/Ability/GA_Mine.h"

#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "Mining/VoxelGround.h"

UGA_Mine::UGA_Mine()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Active.Mining")));
}

void UGA_Mine::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	MiningOnce(ActorInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_Mine::MiningOnce(const FGameplayAbilityActorInfo* ActorInfo)
{
	AActor* ActorOwner = ActorInfo->AvatarActor.Get();
	ABACharacter* Owner = Cast<ABACharacter>(ActorOwner);
	FVector Start = Owner->GetCamera()->GetComponentLocation();
	FVector End = Start + Owner->GetCamera()->GetForwardVector() * 700.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ActorOwner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			AVoxelGround* Ground = Cast<AVoxelGround>(HitActor);
			if (!Ground) Ground = Cast<AVoxelGround>(HitActor->GetOwner());

			if (IsValid(Ground) == true)
			{
				Ground->DigGround(HitResult.Location, 180.0f);
			}
		}
	}
}


