// Chocolate Maniac

#include "AbilitySystem/AuraAbilitySystemGlobals.h"

#include "AuraAbilityTypes.h"

FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	// ���ο� Effect Context �����Ҷ�
	// �� �Լ��� �ش� ��ü�� �ν��Ͻ�ȭ�ϴ� �� ���Ǵ� Ŭ������ �����մϴ�.
	return new FAuraGameplayEffectContext();
}
