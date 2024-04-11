// Chocolate Maniac

#include "UI/WidgetController/AuraWidgetController.h"

#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UAuraWidgetController::BroadcastInitialValues()
{

}

void UAuraWidgetController::BindCallbacksToDependencies()
{

}

void UAuraWidgetController::BroadcastAbilityInfo()
{
	//�־��� �ɷ¿� ���� ������ ���, �ɷ� ������ ��ȸ�Ͽ� ������ ��ε�ĳ��Ʈ�ϱ�.
	// ��������Ʈ�� ���� AbilityInfo�� ��ε�ĳ��Ʈ�ϴ� ��� Ability�� ���� ��ε�ĳ�����ϱ�
	if (!GetAuraASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
		{
			//�־��� �ɷ� ��翡 ���� �ɷ� �±׸� �ľ��ϱ�.
			// Tag�� �ִٸ� FindAbilityInfoForTag()�� ���� AbilityInfo�� ������ �� �ִ�.
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AuraAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
			Info.StatusTag = AuraAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);

			// FAuraAbilityInfo ��ε�ĳ�����ϱ�
			AbilityInfoDelegate.Broadcast(Info);
		});
	// �� Ability���� �����Լ� ȣ��
	GetAuraASC()->ForEachAbility(BroadcastDelegate);
}

AAuraPlayerController* UAuraWidgetController::GetAuraPC()
{
	if (AuraPlayerController == nullptr)
	{
		AuraPlayerController = Cast<AAuraPlayerController>(PlayerController);
	}
	return AuraPlayerController;
}

AAuraPlayerState* UAuraWidgetController::GetAuraPS()
{
	if (AuraPlayerState == nullptr)
	{
		AuraPlayerState = Cast<AAuraPlayerState>(PlayerState);
	}
	return AuraPlayerState;
}

UAuraAbilitySystemComponent* UAuraWidgetController::GetAuraASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	}
	return AuraAbilitySystemComponent;
}

UAuraAttributeSet* UAuraWidgetController::GetAuraAS()
{
	if (AuraAttributeSet == nullptr)
	{
		AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);
	}
	return AuraAttributeSet;
}