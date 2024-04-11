// Chocolate Maniac

#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	check(AttributeInfo);

	for (auto& Pair : GetAuraAS()->TagsToAttributes)
	{
		// �±��� �Ӽ� ���� �ٲ�� delegate
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				// AuraAttributeInfo ����ü�� �����ͼ� �� �Ӽ� ���� �´��� Ȯ���ϰ� 
				// �� ����ü�� ��� ������ ��ε�ĳ��Ʈ�ϱ�
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	GetAuraPS()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			AttributePointsChangedDelegate.Broadcast(Points);
		}
	);
}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	AttributePointsChangedDelegate.Broadcast(GetAuraPS()->GetAttributePoints());
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	// ASC�� ����Ʈ �����̳� GameplayEvent ���۰� ���� Attributes�� ���õ� �۾��� ������ �� �ִ� 
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	// tag�� ���� AttributeInfo�� ã��
	// TagsToAttributes.Add(�±�, �±��� �Ӽ��� ���ε� �� ��������Ʈ)
	// Pair.Key = Tag ( ex. GameplayTags.Attributes_Primary_Strength )
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	
	// Value�� ��������Ʈ�̰� �±��� �Ӽ��� ���ε� �Ǿ� �ֱ⿡ Attribute Accessor�� ���ε� ��.
	// �ϴ� ��������Ʈ�� ���� ���Ѿ� ���ε� �� �Լ����� ���� �ǰ���?
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	
	// �±׶� �Ӽ� �� ��ε�ĳ��Ʈ
	AttributeInfoDelegate.Broadcast(Info);
}

