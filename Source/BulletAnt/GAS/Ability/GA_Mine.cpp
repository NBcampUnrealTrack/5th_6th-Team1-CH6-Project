#include "GAS/Ability/GA_Mine.h"

#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "Mining/VoxelGround.h"
#include "Weapon/Data/MiningWeaponDataAsset.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/BAGameplayTags.h"

UGA_Mine::UGA_Mine()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	AbilityTags.AddTag(TAG_Ability_Active_Mining);
}

void UGA_Mine::StartAutoDigLoop()
{
	MiningOnce();
}

void UGA_Mine::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor) return;

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface) return;

	MiningData = Cast<UMiningWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!MiningData) return;
	
	CachedMiningAM = MiningData->MiningMontage;

	TargetDuration = 60.f / MiningData->DigPerMinute;
	float BaseMontageLength = CachedMiningAM->GetPlayLength();
	Playrate = FMath::Clamp(BaseMontageLength / TargetDuration,0.8f,1.8f);

	const UGameplayEffect* EffectCDO = MiningData->UseStateEffect->GetDefaultObject<UGameplayEffect>();

	MiningStateHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, EffectCDO, 1.f, 1);

	if (MiningData->bAutoActive)
	{
		StartAutoDigLoop();
	}
	else
	{
		MiningOnce();
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
}

void UGA_Mine::OnMontageFinished()
{
	ABACharacter* Owner = Cast<ABACharacter>(SourceActor);
	FVector Start = Owner->GetCamera()->GetComponentLocation();
	FVector End = Start + Owner->GetController()->GetControlRotation().Vector() * 700.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SourceActor);

	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(MiningData->TraceRadius), Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			AVoxelGround* Ground = Cast<AVoxelGround>(HitActor);
			if (!Ground) Ground = Cast<AVoxelGround>(HitActor->GetOwner());

			if (IsValid(Ground) == true)
			{
				Ground->DigGround(HitResult.Location, MiningData->DigRadius);
			}
		}
	}

	StartAutoDigLoop();
}

void UGA_Mine::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MiningStateHandle.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveActiveGameplayEffect(MiningStateHandle);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Mine::MiningOnce()
{
	if (MiningData && CachedMiningAM)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, CachedMiningAM, Playrate);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Mine::OnMontageFinished);
		MontageTask->ReadyForActivation();
	}	
}


