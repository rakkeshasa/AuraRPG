

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraPassiveAbility.h"
#include "LifeSiphon.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API ULifeSiphon : public UAuraPassiveAbility
{
	GENERATED_BODY()
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
