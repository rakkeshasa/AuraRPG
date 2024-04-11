// Chocolate Maniac

#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

// MMC�� Custom Calculation�� �ϴµ� ���� ����

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// Attribute Accessors�� ��ũ�� Property Getter
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	// Aura�κ��� Aura����
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false; // �������ڸ��� ĸó����

	RelevantAttributesToCapture.Add(VigorDef); // MMC �迭�� �߰�
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// ����� �� ������ ���� ����� �ϰ� �;��~
	// �����ڿ� Ÿ���� �±� �������
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameter;
	EvaluationParameter.SourceTags = SourceTags;
	EvaluationParameter.TargetTags = TargetTags;

	// Vigor �� �����
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameter, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	// �⺻ ü�� + 2.5Ȱ�� + 10����
	return 80.f + (2.5f * Vigor) + (10.f * PlayerLevel);
}
