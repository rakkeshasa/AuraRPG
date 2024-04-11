// Chocolate Maniac

#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	// ctrl + k + o : ����� �̵�
	// ��Ʈ�ѷ��� ��Ʈ����Ʈ���� �˰� ����
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	// OnXpChanged ��������Ʈ�� callback ���ε� ���ֱ�
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);

	// ������ �ٲ�� callback�Լ�(����) ȣ��
	// GetGameplayAttributeValueChangeDelegate
	// Attribute�� ����� ������ �ڵ����� ȣ��Ǵ� ���ε��� �� �ִ� �븮��(HealthChanged)�� ��ȯ
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				// callback�� ����
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
	);

	if (GetAuraASC())
	{
		GetAuraASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);

		// �ɷ��� �־����ٸ�
		if (GetAuraASC()->bStartupAbilitiesGiven)
		{
			// ASC�κ��� �����ϰ� Ability������ ������ �� �ִ�.
			BroadcastAbilityInfo();
		}
		else
		{
			// ASC�� ���� Ability�� ���� ���ߴ�. 
			// ��ε�ĳ��Ʈ�� ���� ������ ���ߴ�.

			// �ݹ��� AbilitiesGivenDelegate()�� ���ε��ϱ�.
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}

		GetAuraASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				// �������� �ʵ��� &�� ����
				// ��� AssetTag�� ���� ����
				for (const FGameplayTag& Tag : AssetTags)
				{
					// �θ� �±׸� Ȯ���Ͽ� �� �±װ� TagToCheck�� ��ġ�ϴ��� Ȯ��
					// "A.1"�� MatchesTag("A")���� True�� ��ȯ, "A"�� MatchesTag("A.1")���� False�� ��ȯ
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (Tag.MatchesTag(MessageTag))
					{
						// �����Լ��� ��� �Լ��� �𸥴�. �� Ŭ���� ��ü�� �𸥴�
						// �ƴ� ���� ���� �Լ�, �Ű� ���� ��
						// ��� �Լ��� �˷��� �� �Լ��� ���Ե� Ŭ������ ĸ�����ָ� �ȴ�
						// ���⼭�� �� Ŭ�����̹Ƿ� this�� []�ȿ� ����
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}
	
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	// �������� ���� ���� �����
	if (Level <= MaxLevel && Level > 0)
	{
		// 1lvl : 0 ~ 300
		// 2lvl : 301 ~ 600
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		// ���� ������ �´� ����ġ ���� ũ��(301 ~ 600 -> 600 - 300)
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		
		// ���� ����ġ���� ���� ���� ����ġ �� ����(450 - 300 = 150)
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

