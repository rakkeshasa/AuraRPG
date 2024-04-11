// Chocolate Maniac

#include "AbilitySystem/AuraAbilitySystemGlobals.h"

#include "AuraAbilityTypes.h"

FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	// 새로운 Effect Context 생성할때
	// 이 함수는 해당 객체를 인스턴스화하는 데 사용되는 클래스를 결정합니다.
	return new FAuraGameplayEffectContext();
}
