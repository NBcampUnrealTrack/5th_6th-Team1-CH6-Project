#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BAGameInstance.generated.h"

UCLASS()
class BULLETANT_API UBAGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void OnStart() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bLoginOnStart : 1 = true;						// 임시 - 로그인 방식 많아지고 버튼 선택할 수 있게 되면 제거
};
