#include "Player/BAPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GAS/AttributeSet/AmmoAttributeSet.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GAS/AttributeSet/EXPAttributeSet.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "Player/BACharacter.h"

ABAPlayerState::ABAPlayerState()
{
	//GAS
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));
	AmmoAttributeSet = CreateDefaultSubobject<UAmmoAttributeSet>(TEXT("AmmoSet"));
	EXPAttributeSet = CreateDefaultSubobject<UEXPAttributeSet>(TEXT("EXPSet"));
}

void ABAPlayerState::InitAbility()
{
	if (HasAuthority())
	{
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbility)
		{
			if (AbilityClass)
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, -1, this);
				AbilitySystemComponent->GiveAbility(Spec);
			}
		}
	}
}

UAbilitySystemComponent* ABAPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UHealthAttributeSet* ABAPlayerState::GetHealthAttributeSet()
{
	if (!AbilitySystemComponent) return nullptr;
	return HealthAttributeSet;
}

const UAmmoAttributeSet* ABAPlayerState::GetAmmoAttributeSet() const
{
	return AbilitySystemComponent ? AbilitySystemComponent->GetSet<UAmmoAttributeSet>() : nullptr;
}

const UEXPAttributeSet* ABAPlayerState::GetEXPAttributeSet() const
{
	return AbilitySystemComponent ? AbilitySystemComponent->GetSet<UEXPAttributeSet>() : nullptr;
}

void ABAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerColorIdx);
}

void ABAPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	ABAPlayerState* NewPS = Cast<ABAPlayerState>(NewPlayerState);
	if (IsValid(NewPS) == false)
		return;

	NewPS->PlayerColorIdx = this->PlayerColorIdx;
	NewPS->SetPlayerName(this->GetPlayerName());
}

void ABAPlayerState::BeginPlay()
{
	Super::BeginPlay();

	FGameplayTagContainer DefaultTags;
	DefaultTags.AddTag(TAG_Team_Player);
	AbilitySystemComponent->AddLooseGameplayTags(DefaultTags);
}

void ABAPlayerState::PostNetInit()
{
	Super::PostNetInit();

	APlayerController* PC = GetPlayerController();
	if (IsValid(PC) == true && PC->IsLocalController() == true)
	{
		UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
		if (IsValid(MultiplayerSubsystem) == false)
			return;

		MultiplayerSubsystem->SyncNicknameToPlayerState();
	}
}

void ABAPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	ABACharacter* BACharacter = Cast<ABACharacter>(GetPawn());
	if (IsValid(BACharacter) == false)
		return;

	BACharacter->UpdateNicknameUI();
}

void ABAPlayerState::SetPlayerColorIdx(int32 NewIdx)
{
	PlayerColorIdx = NewIdx;
	OnRep_PlayerColorIdx();
}

FLinearColor ABAPlayerState::GetPlayerColor() const
{
	return PlayerColorTable.IsValidIndex(PlayerColorIdx) == true ? PlayerColorTable[PlayerColorIdx] : FLinearColor::White;
}

FDelegateHandle ABAPlayerState::BindOnChangedPlayerColor(const FOnChangedPlayerColor::FDelegate& Delegate)
{
	return OnChangedPlayerColor.Add(Delegate);
}

void ABAPlayerState::UnbindOnChangedPlayerColor(const UObject* Object)
{
	OnChangedPlayerColor.RemoveAll(Object);
}

void ABAPlayerState::UnbindOnChangedPlayerColor(FDelegateHandle Handle)
{
	OnChangedPlayerColor.Remove(Handle);
}

void ABAPlayerState::Server_UpdatePlayerName_Implementation(const FString& NewName)
{
	bSetNickname = true;
	SetPlayerName(NewName);

	OnRep_PlayerName();

	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == false)
		return;

	MultiplayerSubsystem->UpdateSessionParticipants();
}

void ABAPlayerState::OnRep_PlayerColorIdx()
{
	OnChangedPlayerColor.Broadcast(GetPlayerColor());
}

