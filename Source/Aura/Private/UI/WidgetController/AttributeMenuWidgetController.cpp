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
		// 태그의 속성 값이 바뀌면 delegate
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				// AuraAttributeInfo 구조체를 가져와서 그 속성 값이 맞는지 확인하고 
				// 그 구조체를 모든 위젯에 브로드캐스트하기
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
	// ASC는 이펙트 적용이나 GameplayEvent 전송과 같은 Attributes과 관련된 작업을 수행할 수 있다 
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	// tag를 통해 AttributeInfo을 찾음
	// TagsToAttributes.Add(태그, 태그의 속성이 바인딩 된 델리게이트)
	// Pair.Key = Tag ( ex. GameplayTags.Attributes_Primary_Strength )
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	
	// Value는 델리게이트이고 태그의 속성이 바인딩 되어 있기에 Attribute Accessor가 바인딩 됨.
	// 일단 델리게이트를 실행 시켜야 바인딩 된 함수들이 실행 되겠죠?
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	
	// 태그랑 속성 값 브로드캐스트
	AttributeInfoDelegate.Broadcast(Info);
}

