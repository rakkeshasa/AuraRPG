// Chocolate Maniac

#include "AbilitySystem/ModMagCalc/MMC_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	// Intelligence 속성 값 가져오기
	IntDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// GetAggregatedTags()는 해당 소스 태그를 집계하여 하나의 Container로 반환
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 소스와 타겟에 대한 게임플레이 태그 세팅하기
	FAggregatorEvaluateParameters EvaluationParameter;
	EvaluationParameter.SourceTags = SourceTags;
	EvaluationParameter.TargetTags = TargetTags;

	float Int = 0.f;
	GetCapturedAttributeMagnitude(IntDef, Spec, EvaluationParameter, Int);
	Int = FMath::Max<float>(Int, 0.f);

	int32 PlayerLevel = 1;
	// Implements를 호출 시 I가 아니라 U를 사용한다.
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		// 인터페이스 내 함수 사용시 I를 사용
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	return 50.f + 2.5f * Int + 15.f * PlayerLevel;
}
