#include "GAS/Ability/GA_Ping.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/BAGameplayTags.h"
#include "Player/BACharacter.h"
#include "Player/BAPlayerState.h"

UGA_Ping::UGA_Ping()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

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

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(ASC) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ASC->AbilityTargetDataSetDelegate(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()
	).AddUObject(this, &UGA_Ping::OnTargetDataReady);

	if (IsLocallyControlled() == true)
	{
		SendTargetData();
	}
}

void UGA_Ping::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Ping::SendTargetData()
{
	ABACharacter* Character = Cast<ABACharacter>(GetAvatarActorFromActorInfo());
	if (IsValid(Character) == false)
		return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (IsValid(PC) == false)
		return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector End = CamLoc + CamRot.Vector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params);

	FVector TargetLoc = Hit.bBlockingHit ? Hit.Location : End;

	FGameplayAbilityTargetData_LocationInfo* Data = new FGameplayAbilityTargetData_LocationInfo();
	Data->TargetLocation.LiteralTransform = FTransform(TargetLoc);

	FGameplayAbilityTargetDataHandle Handle;
	Handle.Add(Data);

	GetAbilitySystemComponentFromActorInfo()->ServerSetReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey(),
		Handle,
		FGameplayTag(),
		GetAbilitySystemComponentFromActorInfo()->ScopedPredictionKey);
}

void UGA_Ping::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Tag)
{
	if (CurrentActorInfo->IsNetAuthority() == false)
		return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	ASC->ConsumeClientReplicatedTargetData(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey());

	if (Data.Num() == 0)
		return;

	const FGameplayAbilityTargetData_LocationInfo* LocData = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Data.Get(0));
	if (!LocData)
		return;

	FVector PingLocation = LocData->TargetLocation.LiteralTransform.GetLocation();

	ABACharacter* Character = Cast<ABACharacter>(GetAvatarActorFromActorInfo());
	if (IsValid(Character) == false)
		return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (IsValid(PC) == false)
		return;

	ABAPlayerState* PS = PC->GetPlayerState<ABAPlayerState>();

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(PS, nullptr);
	Context.AddOrigin(PingLocation);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Ping, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	/*FGameplayCueParameters Params;
	Params.EffectContext = ASC->MakeEffectContext();
	Params.Instigator = PC;
	Params.Location = PingLocation;

	ASC->ExecuteGameplayCue(TAG_GameplayCue_Communicate_Ping, Params);*/

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
