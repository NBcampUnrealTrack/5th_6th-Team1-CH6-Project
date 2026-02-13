#include "GAS/Ability/GA_Mine.h"

#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "Mining/VoxelGround.h"
#include "Weapon/Data/MiningWeaponDataAsset.h"
#include "AbilitySystemComponent.h"

UGA_Mine::UGA_Mine()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Active.Mining")));
}

void UGA_Mine::StartAutoDigLoop()
{
	MiningOnce();

	float DigDelay = 60.f / MiningData->DigPerMinute;

	GetWorld()->GetTimerManager().SetTimer(
		DigTimerHandler,
		this,
		&UGA_Mine::MiningOnce,
		DigDelay,
		true
	);
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

void UGA_Mine::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DigTimerHandler);
	}

	if (MiningStateHandle.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveActiveGameplayEffect(MiningStateHandle);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Mine::MiningOnce()
{
	ABACharacter* Owner = Cast<ABACharacter>(SourceActor);
	FVector Start = Owner->GetCamera()->GetComponentLocation();
	FVector End = Start + Owner->GetController()->GetControlRotation().Vector() * 700.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SourceActor);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	DrawDebugLine(
		SourceActor->GetWorld(),
		Start,
		bHit ? HitResult.ImpactPoint : End,
		FColor::Red,
		false,
		1.f,
		0,
		1.f
	);

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


