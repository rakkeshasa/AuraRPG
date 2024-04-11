// Chocolate Maniac

#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// ���� ���ð��� ���� ����Ǿ�����(���� ���ð� �±� ����)
	// RegisterGameplayTagEvent(): �±� �̺�Ʈ�� listen�ϱ� ���� ���
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange,
			&UWaitCooldownChange::CooldownTagChanged);

	// ���� ���ð� ȿ���� ���� ����Ǿ�����
	// duration ��� GE�� �߰��� ������ Ŭ���̾�Ʈ�� ���� ��ο��� ȣ���.
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	// �׼��� ������ �Ϸ�Ǿ��� �� ȣ���ϸ� 
	// �׼��� ������ �� �ְ� �ǰ� ���� �ν��Ͻ��� ����� ������.
	SetReadyToDestroy();

	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		// ��Ÿ���� �󸶳� ���Ҵ���(0��-> ��Ž �� ���Ҵ�)
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		// Ȱ��ȭ�� effect �ð��� ���� ��� 
		// ������ ��Ÿ�� �±׿� ��ġ�ϴ� ����Ʈ�� �����´�
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		
		// *GetActiveEffectsTimeRemaining(): �±װ� �ִ� ��� �����÷��� ����Ʈ�� ���� �ð��� return
		// *CooldownTag.GetSingleTagContainer()�� ��ġ�ϴ� �±װ� �ִٸ� Array�� ����
		// *��κ��� ��Ȳ������ �ش� �迭�� 1���� �� ���̴�.
		// *Firebolt ��ٿ� �±״� �� ���� �ϳ��� ����
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			float TimeRemaining = TimesRemaining[0];
			// Ȥ�� 1���� �ƴ� �� ��Ÿ���� �ִ�ġ�� ���ش�.
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}

			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
