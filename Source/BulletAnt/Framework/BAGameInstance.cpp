#include "Framework/BAGameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

void UBAGameInstance::Init()
{
	EpicLogin();
}

void UBAGameInstance::EpicLogin()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			FDelegateHandle LoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateLambda([](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
				{
					UE_LOG(LogTemp, Fatal, TEXT("콜백 수신 성공 여부: %s"), bWasSuccessful ? TEXT("True") : *Error);
				}));

			// 2. 그 다음 로그인을 시도합니다.
			FOnlineAccountCredentials Credentials;
			Credentials.Type = TEXT("accountportal");
			Credentials.Id = TEXT("");
			Credentials.Token = TEXT("");
			Identity->Login(0, Credentials);
		}
	}
}
