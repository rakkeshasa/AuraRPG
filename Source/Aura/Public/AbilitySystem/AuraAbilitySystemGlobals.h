// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * 기존 Effect Context Struct는 bIsBlockedHit, bIsCriticalHit이 없다
 * 따라서 Custom을 해줘야 한다.
 * DefaultGame.ini에서 해당 custom을 쓴다고 지정을 해줘야함.
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
