#include "GAS/Ability/GA_Ping.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/BAGameplayTags.h"
#include "Player/BACharacter.h"

UGA_Ping::UGA_Ping()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Event_Communicate_Ping);
	SetAssetTags(DefaultTag);
}

void UGA_Ping::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PingTrace();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Ping::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Ping::PingTrace()
{
	ABACharacter* Character = Cast<ABACharacter>(CurrentActorInfo->AvatarActor.Get());
	if (IsValid(Character) == false)
		return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (IsValid(PC) == false)
		return;

	FVector ViewLoc = FVector::ZeroVector;
	FVector Dir = FVector::ZeroVector;
	int32 ScreenX = 1920;
	int32 ScreenY = 1080;
	PC->GetViewportSize(ScreenX, ScreenY);
	PC->DeprojectScreenPositionToWorld(
		(float)ScreenX * 0.5f,
		(float)ScreenY * 0.5f,
		ViewLoc,
		Dir);

	FVector Start = ViewLoc;
	FVector End = ViewLoc + Dir.GetSafeNormal() * TraceDistance;
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Character);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			FGameplayCueParameters Params;
			Params.Instigator = PC;
			Params.Location = HitResult.Location;
			GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(TAG_GameplayCue_Communicate_Ping, Params);
		}
	}
}
