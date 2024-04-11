// Chocolate Maniac

#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

// MMC는 Custom Calculation을 하는데 좋은 도구

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// Attribute Accessors의 매크로 Property Getter
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	// Aura로부터 Aura한테
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false; // 생성되자마자 캡처할지

	RelevantAttributesToCapture.Add(VigorDef); // MMC 배열에 추가
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 저희는 더 복잡한 스탯 계산을 하고 싶어요~
	// 원인자와 타겟자 태그 갖고오기
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameter;
	EvaluationParameter.SourceTags = SourceTags;
	EvaluationParameter.TargetTags = TargetTags;

	// Vigor 값 만들기
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameter, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	// 기본 체력 + 2.5활력 + 10레벨
	return 80.f + (2.5f * Vigor) + (10.f * PlayerLevel);
}
