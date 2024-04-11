// Chocolate Maniac

#include "Character/AuraEnemy.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Aura/Aura.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameplayTags.h"
#include "AI/AuraAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // �޽� ������Ʈ->visibility�� block����

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); // �������� ������Ʈ ����ȭ(����)
	// ���ʹ� AI��Ʈ���̴ϱ� Minimal
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	// ������ �������� ȸ���� ���� ĳ���� �����Ʈ ������Ʈ�� �Ӽ��� �̿�
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet"); // �ش� ������Ʈ �Ӽ�set
	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	/* MarkRenderStateDirty() */
	// ������ ���¸� ��Ƽ�� ǥ���ϸ� �������� ���� �� ������ ������� ���۵˴ϴ�.
	// �������� �Ϸ��� ���� ������Ʈ�� ���� ���¸� �����ؾ� �մϴ�. 
	// ���� ���´� ���� �����͸� ������Ʈ�ؾ� �ϴ� ������Ʈ�� ���� ���� ����Ǿ��ٴ� ����� ������ �˸��ϴ�. 
	// �̷��� ������ �߻��ϸ� ���� ���´� '��Ƽ(dirty)'�� ǥ�õ˴ϴ�.
	// ������Ʈ�� ���� ���̴� (�׷��� �������Ǵ�) ���� ���(register)�� ���� ��츸�Դϴ�. 
	// ������Ʈ�� ������Ƽ�� �����ϴ� ���� ������ �ڵ�� �ݵ�� ������Ʈ���� MarkRenderStateDirty() �� ȣ���ؾ� ���泻���� ������ �����忡 �����մϴ�.
	GetMesh()->MarkRenderStateDirty();
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->MarkRenderStateDirty();

	BaseWalkSpeed = 250.f;
}

void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	AuraAIController = Cast<AAuraAIController>(NewController);
	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AuraAIController->RunBehaviorTree(BehaviorTree);

	// �������� Ű ���� false�� ����
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();

	// ������ �ƴ� �� ȣ��X
	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	
	// ���� ��Ʈ�ѷ� �������ֱ�
	if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		AuraUserWidget->SetWidgetController(this);
	}

	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
		// RegisterGameplayTagEvent: �߰� �Ǵ� ���ŵǴ� �±׿� ���� �̺�Ʈ�� ����� �� �ִ�.
		// EGameplayTagEventType::NewOrRemoved: �±װ� ���� �߰��ǰų� ���� ���� ���� ��
		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this, // UserObject
			&AAuraEnemy::HitReactTagChanged // Callback
		); 

		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
	
}

void AAuraEnemy::InitAbilityActorInfo()
{
	// ASC�� ���� ���Ͱ� OwnerActor, ASC�� ������ ǥ�� ���Ͱ� AvatarActor
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// AbilityActorInfo�� �������ڸ��� AbilityActorInfoSet()�Լ� ȣ��
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	
	// 335
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraEnemy::StunTagChanged);

	// ���Ӹ��� Client���� UGameplayStatics::GetGameMode()�� ����Ͽ� �׼����ϴ� ��쿡�� �������� ��ȿ
	if (HasAuthority())
	{
		// ���������� ����Ǿ�� �Ѵ�.
		InitializeDefaultAttributes();
	}

	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void AAuraEnemy::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void AAuraEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);

	if (AuraAIController && AuraAIController->GetBlackboardComponent())
	{
		// ������Ű Stunned �����ϱ�
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}

void AAuraEnemy::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);
}

void AAuraEnemy::UnHighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::SetMoveToLocation_Implementation(FVector& OutDestination)
{
}

int32 AAuraEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void AAuraEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);

	// ������ ������ Ű Dead�� true�� �����ϱ�
	if (AuraAIController) AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	
	SpawnLoot();
	Super::Die(DeathImpulse);
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	// �¾��� ���� �ӵ��� 0, �ƴϸ� �⺻ �ӵ���
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	
	// AuraAIController�� ���������� ��ȿ��. Ŭ��X
	if (AuraAIController && AuraAIController->GetBlackboardComponent())
	{
		// ������ ������ Ű�� true�� �ȸ����� false��
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
	
}
