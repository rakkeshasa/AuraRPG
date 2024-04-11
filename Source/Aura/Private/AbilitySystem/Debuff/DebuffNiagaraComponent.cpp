// Chocolate Maniac

#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	// 불타는 머리를 가진 고블린이 튀어나오는 일이 없도록
	// 나이아가라가 처음부터 자동으로 실행하지 않도록 함
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
		// ASC가 nullptr일 경우 대기하였다가
		// ASC가 존재할때 델리게이트를 브로드캐스트하도록 한다.
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
			{
				// 람다는 캡처 목록에 캡처한 항목에 대한 참조를 저장한다
				// 예를 들어 특정 컴포넌트를 람다에 캡처하면 해당 컴포넌트는 UPROPERTY()를 통해 자동으로 가비지 컬렉션이 된다. 
				// 람다가 해당 컴포넌트에 대한 참조를 보유하고 있는 한, 해당 컴포넌트는 자동으로 갈비지 컬렉션이 되지 않는다
				// WeakLambda는 레퍼런스 카운트를 늘리지 않고도 무언가에 대한 참조를 보유하는 속성이 있으므로 가비지 컬렉션이 될 수 있다
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
