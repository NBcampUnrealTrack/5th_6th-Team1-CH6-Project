// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/GAS/Effect/MoveGameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UMoveGameplayEffect::UMoveGameplayEffect()
{
	// 1. 지속 시간 설정 (무한)
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 2. 부여할 태그 설정
	//FGameplayTag ChaseTag = FGameplayTag::RequestGameplayTag(FName("State.Enemy.Move"));
	//UTargetTagsGameplayEffectComponent TargetTagsGameplayEffectComponent;
	//GEComponents.Add(TargetTagsGameplayEffectComponent);
	
	
	// GrantedTags에 태그 추가 (이 효과가 적용된 동안 유지됨)
	// InheritableOwnedTagsContainer.AddTag(ChaseTag);

	// 3. (옵션) 이동 속도 변경 등 스탯 수정이 필요하다면 여기서 추가
	/*
	FGameplayModifierInfo ModInfo;
	ModInfo.Attribute = UMyAttributeSet::GetMoveSpeedAttribute();
	ModInfo.ModifierOp = EGameplayModOp::Additive;
	ModInfo.ModifierMagnitude = FScalableFloat(200.0f);
	Modifiers.Add(ModInfo);
	*/
	
	// Effect 종료 후 태그 제거
}
