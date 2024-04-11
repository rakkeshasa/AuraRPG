// Chocolate Maniac

#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	// ��Ÿ�� �Ӹ��� ���� ����� Ƣ����� ���� ������
	// ���̾ư��� ó������ �ڵ����� �������� �ʵ��� ��
	bAutoActivate = false;
}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	else if (CombatInterface)
	{
		// ASC�� nullptr�� ��� ����Ͽ��ٰ�
		// ASC�� �����Ҷ� ��������Ʈ�� ��ε�ĳ��Ʈ�ϵ��� �Ѵ�.
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
			{
				// ���ٴ� ĸó ��Ͽ� ĸó�� �׸� ���� ������ �����Ѵ�
				// ���� ��� Ư�� ������Ʈ�� ���ٿ� ĸó�ϸ� �ش� ������Ʈ�� UPROPERTY()�� ���� �ڵ����� ������ �÷����� �ȴ�. 
				// ���ٰ� �ش� ������Ʈ�� ���� ������ �����ϰ� �ִ� ��, �ش� ������Ʈ�� �ڵ����� ������ �÷����� ���� �ʴ´�
				// WeakLambda�� ���۷��� ī��Ʈ�� �ø��� �ʰ� ���𰡿� ���� ������ �����ϴ� �Ӽ��� �����Ƿ� ������ �÷����� �� �� �ִ�
				InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
			});
	}

	if (CombatInterface)
	{
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	const bool bOwnerValid = IsValid(GetOwner());
	const bool bOwnerAlive = GetOwner()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(GetOwner());

	if (NewCount > 0 && bOwnerValid && bOwnerAlive)
	{
		Activate();
	}
	else
	{
		Deactivate();
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	Deactivate();
}
