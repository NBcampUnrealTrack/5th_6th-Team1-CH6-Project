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
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_Mining);

	SetAssetTags(DefaultTag);

	ActivationOwnedTags.AddTag(TAG_State_Combat_Attacking);
}

void UGA_Mine::StartAutoDigLoop()
{
	if (bEndInput)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	MiningOnce();
}

void UGA_Mine::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MiningData = Cast<UMiningWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!MiningData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CachedMiningAM = MiningData->MiningMontage;

	bEndInput = false;

	TargetDuration = 60.f / MiningData->DigPerMinute;
	float BaseMontageLength = CachedMiningAM->GetPlayLength();
	Playrate = FMath::Clamp(BaseMontageLength / TargetDuration,0.8f,1.8f);

	StartAutoDigLoop();
}

void UGA_Mine::OnMontageFinished()
{
	if (CurrentActorInfo->IsNetAuthority()) 
	{
		ABACharacter* Owner = Cast<ABACharacter>(SourceActor);

		UCameraComponent* Camera = Owner->GetCamera();
		if (IsValid(Camera) == false)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}

		AController* Controller = Owner->GetController();
		if (IsValid(Controller) == false)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}

		FVector Start = Camera->GetComponentLocation();
		FVector End = Start + Controller->GetControlRotation().Vector() * 700.0f;

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
					TMap<EOreType, int32> MinedOreMap;
					Ground->DigGround(MinedOreMap, HitResult.Location, MiningData->DigRadius);
					int32 MostMinedOre = static_cast<int32>(EOreType::None);
					int32 MostCount = 0;
					if (MinedOreMap.IsEmpty() == true)
					{
						MostMinedOre = -1;
					}
					else
					{
						for (const auto& Pair : MinedOreMap)
						{
							if (Pair.Key == EOreType::None)
								continue;

							if (Pair.Value > MostCount)
							{
								MostMinedOre = static_cast<int32>(Pair.Key);
								MostCount = Pair.Value;
							}
						}
					}

					UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
					FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
					Context.AddHitResult(HitResult);

					FGameplayCueParameters Parameters;
					Parameters.EffectContext = Context;
					Parameters.GameplayEffectLevel = MostMinedOre;
					Parameters.Location = HitResult.ImpactPoint;
					ASC->ExecuteGameplayCue(TAG_GameplayCue_Mining_Hit, Parameters);
				}
			}		
		}
	}
	StartAutoDigLoop();
}

void UGA_Mine::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Mine::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	bEndInput = true;
}

void UGA_Mine::MiningOnce()
{
	if (MiningData && CachedMiningAM)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, CachedMiningAM, Playrate);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Mine::OnMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Mine::EndMining);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Mine::EndMining);
		MontageTask->ReadyForActivation();
	}	
}

void UGA_Mine::EndMining()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}


