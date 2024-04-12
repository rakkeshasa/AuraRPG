# AuraRPG
Unreal Engine v5.2를 이용한 RPG게임
</BR>

간단한 소개
---

</BR></BR>
Unreal5에서 지원하는 Gameplay Ability System을 이용하여 만든 RPG게임입니다.</BR>
플레이어는 대마법사 아우라를 컨트롤하여 몬스터들을 잡아 경험치를 얻어 레벨업을 할 수 있습니다.</BR>
레벨업을 하면 스탯 포인트와 스킬 포인트가 주어지며, 최대 체력과 최대 마나가 늘어납니다.</BR>
스탯 포인트를 스탯에 투자하면 해당 스탯에 영향받는 능력치가 늘어납니다.(예를 들자면 민첩에 투자하면 크리 확률이 오르거나, 지능에 투자하면 마법 데미지와 마나가 오르는 것이 있습니다.)</BR>
스킬 포인트는 레벨에 따라 새롭게 열린 스킬에 투자하여 해당 스킬을 해금할 수 있으며, 이미 해금한 스킬에 포인트를 투자하여 데미지나 범위를 늘려 스킬이 한단계 더 업그레이드 할 수 있습니다.
</BR></BR>

플레이 영상
---
[![Video Label](http://img.youtube.com/vi/A2Z_FVkoOV8/0.jpg)](https://youtu.be/A2Z_FVkoOV8)
</BR>
👀Link: https://youtu.be/A2Z_FVkoOV8</BR>
이미지나 주소 클릭하시면 영상을 보실 수 있습니다. </BR>

코드 구현
---
### [플레이어 이동 구현]
![캐릭터 이동](https://github.com/rakkeshasa/AuraRPG/assets/77041622/d827e189-1a24-481f-a355-0e84b307e1d3)
<div align="center"><strong>Input Mapping Context에 키 값 설정해주기</strong></div></BR>

```
void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// UAuraInputComponent 클래스는 UEnhancedInputLocalPlayerSubSystem를 상속함
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	
	// 이동관련
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}

	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
```
<div align="center"><strong>C++코드에서 Enhanced Input설정하기</strong></div></BR>
<strong>BeginPlay()</strong>에서 Enhanced Input에서 사용할 IMC(Input Mapping Context)를 연결해줬습니다. 여기서 AuraContext는 InputMappingContext의 TOjbectPtr로 엔진에서 만든 IMC와 연결할 멤버 변수입니다.</BR></BR>
<strong>SetupInputComponent()</strong>에서는 이동 관련 입력이 들어오면 Move()함수에 바인딩 시켜 실행해줍니다.</BR></BR>
<strong>Move()</strong>함수는 방향 벡터를 구해 캐릭터를 회전시키고, 해당 방향으로 나아가게 합니다.</BR></BR>

### [체력 및 마나 구현]
```
class AURA_API UAuraAttributeSet : public UAttributeSet
{
public:
	/*
	* 기본 스탯
	*/
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health); 
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);	
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& oldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, oldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& oldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, oldMana);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

```
Attribute 접금자인 ATTRIBUTE_ACCESSORS매크로를 사용하여 각종 Attribute에 대한 GET함수와 SET함수를 자동으로 생성했습니다.</br></br>
체력과 마나의 Attribute는 Blueprint읽기 전용과 Replicated속성으로 만듬과 동시에 네트워크 복제시 상응되는 함수가 호출되도록 했습니다.</br></br>
<strong>GetLifetimeReplicatedProps()</strong>에서는 Replicated할 Attribute를 등록하여 복제할 프로퍼티 정보를 추가했습니다.</br>
각 속성마다 <strong>DOREPLIFETIME_CONDITION_NOTIFY()</strong>를 통하여 프로퍼티의 복제 조건과 알림을 설정하였습니다.</br>
해당 코드에서는 COND_None을 통해 항상 복제가 되도록 하고, REPNOTIFY_Always을 넣어 복제시 항상 서버와 클라에게 알리도록 했습니다.</br></br>
따라서 체력이나 마나가 변경될 때마다 복제하고 서버와 클라에게 해당 수치가 변경됐다고 알립니다.</br></br>

### [체력 및 마나 UI에 연동시키기]

![체력바](https://github.com/rakkeshasa/AuraRPG/assets/77041622/f24d6b72-9837-4261-9ac3-f7a92bff016f)
<div align="center"><strong>UserWidget을 상속받아 만든 체력창과 마나창</strong></div></BR></BR>

```
UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();

		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_AuraHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);

	// 위젯 컨트롤러 생성
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS); 
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	// 위젯에 위젯 컨트롤러 세팅
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();

	Widget->AddToViewport();
}
```
<div align="center"><strong>위에서 만든 Widget에 WidgetControlloer 연동해주기</strong></div></BR>

<Strong>GetOverlayWidgetController()</strong>에서 HUD에 위젯 컨트롤러가 없으면 생성해주고</BR>
<Strong>SetWidgetControllerParams()</strong>를 통하여 위젯 컨트롤러에 PlayerController, PlayerState, ASC, AttributeSet을 채워줬습니다.</BR></BR>

<Strong>InitOverlay()</strong>에서는 월드에 위젯을 생성해주고 위젯 컨트롤러에는 4가지 요소를 넣어 생성했습니다.</BR>
이후 위젯에 위젯 컨트롤러를 연동해주고 위젯 컨트롤러에 필요한 속성 값들을 브로드캐스트하여 세팅했습니다.</BR></BR>
<strong>BroadcastInitialValues()</strong>에 자세한 코드는 아래에 있습니다.</BR></BR>

```
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;
}

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}
```
<div align="center"><strong>위젯 컨트롤러에 초기 체력 및 마나 세팅해주기</strong></div></BR>
<strong>OverlayWidgetController</strong>클래스에서 1개의 값만(체력, 마나 등)가지는 다이나믹 델리게이트를 선언했습니다.</BR>
이후 각각의 Attribute 값이 바뀔 때 호출 될 델리게이트를 생성했습니다.</BR></BR>

<strong>BroadcastInitialValues()</strong>에서는 Attribute마다 델리게이트를 호출하여 현재 값을 브로드캐스트해
초기 값을 세팅해주도록 했습니다.</BR></BR>

![체력바 블루프린트](https://github.com/rakkeshasa/AuraRPG/assets/77041622/e0896efb-e7aa-454c-b4af-c9710beabd88)
<div align="center"><strong>C++에서 구현한 함수를 체력창에 적용하는 모습</strong></div></BR>

구현한 함수들은 맨 처음에 UserWidget기반으로 생성한 체력창과 마나창의 블루프린트에 적용하여 초기세팅을 마쳤습니다.</BR></BR>

```
UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();

		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
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
```
<div align="center"><strong>Attribute 값이 바뀔 시 브로드캐스트하여 알려주기</strong></div></BR>

<strong>BindCallbacksToDependencies()</strong>함수에서는 Attribute값이 바뀔 때 
자동으로 호출되는 GetGameplayAttributeValueChangeDelegate()를 이용하였습니다.</br></br>

<strong>GetGameplayAttributeValueChangeDelegate()</strong>는 속성 값이 바뀔 때 자동으로 호출되고 바인딩할 수 있는 대리자를 반환하는 함수입니다.</br></br>

FOnAttributeChangeData에서 변경된 델리게이트의 Attribute값에 바인딩되어 HUD를 업데이트하여 체력창과 마나창에 바뀐 수치에 맞게 체력과 마나가 채워지게 됩니다.</br></br>

### [포션 구현]


```
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// 태그가 Enemy이거나 Enemy에 적용되는 이펙트가 아니라면 통과
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) 
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr)
		return;

	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this); 
	
	// FGameplayEffectSpecHandle
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());


	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

	// 무한타입의 이펙트가 아니라면 충돌시 파괴
	if (!bIsInfinite)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;

		TArray<FActiveGameplayEffectHandle> HandlesToRemove;
		for (auto HandlePair : ActiveEffectHandles)
		{
			if (TargetASC == HandlePair.Value)
			{
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
				HandlesToRemove.Add(HandlePair.Key);
			}
		}
		for (auto& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}
```

### [공격 적중 시 피해 입히기]
