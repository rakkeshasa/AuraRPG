// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * ���� Effect Context Struct�� bIsBlockedHit, bIsCriticalHit�� ����
 * ���� Custom�� ����� �Ѵ�.
 * DefaultGame.ini���� �ش� custom�� ���ٰ� ������ �������.
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
