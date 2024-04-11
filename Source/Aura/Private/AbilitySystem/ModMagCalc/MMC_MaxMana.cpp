// Chocolate Maniac

#include "AbilitySystem/ModMagCalc/MMC_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	// Intelligence �Ӽ� �� ��������
	IntDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// GetAggregatedTags()�� �ش� �ҽ� �±׸� �����Ͽ� �ϳ��� Container�� ��ȯ
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// �ҽ��� Ÿ�ٿ� ���� �����÷��� �±� �����ϱ�
	FAggregatorEvaluateParameters EvaluationParameter;
	EvaluationParameter.SourceTags = SourceTags;
	EvaluationParameter.TargetTags = TargetTags;

	float Int = 0.f;
	GetCapturedAttributeMagnitude(IntDef, Spec, EvaluationParameter, Int);
	Int = FMath::Max<float>(Int, 0.f);

	int32 PlayerLevel = 1;
	// Implements�� ȣ�� �� I�� �ƴ϶� U�� ����Ѵ�.
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		// �������̽� �� �Լ� ���� I�� ���
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	return 50.f + 2.5f * Int + 15.f * PlayerLevel;
}
