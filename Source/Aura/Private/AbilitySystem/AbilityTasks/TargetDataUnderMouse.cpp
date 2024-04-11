// Chocolate Maniac

#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);

	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
		SendMouseCursorData();
	else
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		
		// �̹� �����͸� �޾Ҵٸ�
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate) // �����Ͱ� �ȿ�
		{
			// �����Ͱ� �� ������ ��ٸ�
			SetWaitingOnRemotePlayerData();
		}
	}
	
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	/* Ŭ�󿡼� ������ ������ ������*/

	// ScopedPredictionWindow ����
	// Ŭ���̾�Ʈ���� �� ���� Ű�� ���ŵ� �� �������� ȣ��
	// ���ο� PredictionKey�� �����ϰ� 
	// �ش� Ű�� ���� Ŭ���̾�Ʈ�� ���� ���� ����ȭ ���� ������ ��
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Target, false, CursorHit);

	FGameplayAbilityTargetDataHandle DataHandle;
	// ���� ��Ʈ ����� Ÿ�� ������, �����Ͱ� ��Ʈ ����� ��ŷ�˴ϴ�.
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);

	// ������ ������ ������
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(), // Ability task�� ���� ���� ����
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	// �����Ƽ�� ���� Ȱ��ȭ�Ǿ� ���� ���� ��� ��ε�ĳ��Ʈ�ϴ� ���� ����
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	/* �����Ͱ� ���ŵǸ� �������� ȣ�� */

	// Ÿ�� �����͸� �޾�����, ĳ�ÿ� �������� ����
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	
	if (ShouldBroadcastAbilityTaskDelegates())
		ValidData.Broadcast(DataHandle);

}
