// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BAItemInterface.generated.h"

USTRUCT(BlueprintType)
struct FInteractionOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName ActionName;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBAItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BULLETANT_API IBAItemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Use(AActor* User);

	// UI 표시용
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void GetInteractionOptions(AActor* User, TArray<FInteractionOption>& OutOptions) const;

	// 실제 실행
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interaction(AActor* User, FName ActionName);
};
