# AuraRPG
Unreal Engine v5.2를 이용한 RPG게임
</BR>

목차
---
- [간단한 소개](#간단한-소개)
- [플레이 영상](#플레이-영상)
- [코드 구현](#코드-구현)
  * [플레이어 이동 구현](#플레이어-이동-구현)
- 

## 간단한 소개

</BR></BR>
Unreal5에서 지원하는 Gameplay Ability System을 이용하여 만든 RPG게임입니다.</BR>
플레이어는 대마법사 아우라를 컨트롤하여 몬스터들을 잡아 경험치를 얻어 레벨업을 할 수 있습니다.</BR>
레벨업을 하면 스탯 포인트와 스킬 포인트가 주어지며, 최대 체력과 최대 마나가 늘어납니다.</BR>
스탯 포인트를 스탯에 투자하면 해당 스탯에 영향받는 능력치가 늘어납니다.(예를 들자면 민첩에 투자하면 크리 확률이 오르거나, 지능에 투자하면 마법 데미지와 마나가 오르는 것이 있습니다.)</BR>
스킬 포인트는 레벨에 따라 새롭게 열린 스킬에 투자하여 해당 스킬을 해금할 수 있으며, 이미 해금한 스킬에 포인트를 투자하여 데미지나 범위를 늘려 스킬이 한단계 더 업그레이드 할 수 있습니다.
</BR></BR>

## 플레이 영상
[![Video Label](http://img.youtube.com/vi/A2Z_FVkoOV8/0.jpg)](https://youtu.be/A2Z_FVkoOV8)
</BR>
👀Link: https://youtu.be/A2Z_FVkoOV8</BR>
이미지나 주소 클릭하시면 영상을 보실 수 있습니다. </BR>

## 코드 구현

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
Attribute 접근자인 ATTRIBUTE_ACCESSORS매크로를 사용하여 각종 Attribute에 대한 GET함수와 SET함수를 자동으로 생성했습니다.</br></br>
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

![포션](https://github.com/rakkeshasa/AuraRPG/assets/77041622/d74cde77-78e2-4b77-b1a8-0d8efbbf579b)
<div align="center"><strong>체력 포션에 적용한 GameEffect</strong></div></BR>

```
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
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
	if (!bIsInfinite)
	{
		Destroy();
	}
}
```
<div align="center"><strong>Target에게 Gameplay Effect 적용시키기</strong></div></BR>
<strong>ApplyEffectToTarget()</strong>에서는 포션이 플레이어와 충돌할 시 Gameplay Effect를 플레이어에게 적용시키게 했습니다.</br></br>

<strong>Gameplay Effect(GE)</strong>는 <strong>GE Context</strong>와 <strong>GE Spec</strong>으로 2가지 데이터로 구성되어 있습니다.</br>
<strong>Gameplay Ability System(GAS)</strong>는 두 데이터를 직접 제어하는게 아니라 <strong>Handle</strong>을 통해 데이터를 제어합니다.</br></br>

<strong>GE Context</strong>에는 GE가 계산에 필요한 데이터를 갖고 있으며 Instigator(가해자), Causer(가해수단), HitResult(충돌판정)과 같은 정보를 갖고 있습니다. 코드에서는 현재 Actor인 포션을 GameplayEffectContextHandle에 넣어 타겟이 어떤 Actor에게 영향을 받는지 알 수 있게 했습니다.</br></br>

<strong>GE Spec</strong>에는 Level, Modifier, 태그 등 GE에 관한 정보를 답고 있습니다. 코드에서는 영향받을 GE의 클래스와 포션의 레벨과 윗 줄에서 생성한 GE Context Handle을 입력하여 GE Spec을 만들어 GE Spec이 해당 정보를 갖고 있게 했습니다.</br></br>

<strong>ApplyGameplayEffectSpecToSelf()</strong>를 이용하여 GE Spec의 정보 토대로 Target(플레이어)의 ASC에 적용하도록 했습니다.</br>
마지막에는 GE의 Duration이 Infinite가 아닌 경우에만 Actor가 Destroy()되도록 했습니다. Infinite 타입은 불 장판과 같은 함정에 쓰여 플레이어에게 영향을 주고 함정이 사라지는 것을 방지하기 위해 Infinite 타입을 제외했습니다.</br></br>
이렇게 함으로써 ASC가 별도의 Gameplay Ability를 발동시키지 않고도 Gameplay Effect를 발동시켜 Target에게 영향을 줄 수 있게 했습니다.</br></br>

![포션 블프](https://github.com/rakkeshasa/AuraRPG/assets/77041622/37b33e80-adfa-49f1-9318-570d3388aeb8)
<div align="center"><strong>C++로 만든 함수를 블루프린트에 적용해주는 모습</strong></div></BR>

```
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute()) // health 속성인지, ATTRIBUTE_ACCESSOR제공
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}

	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
}
```
<div align="center"><strong>속성이 변경되기 전에 체크해주기</strong></div></BR>
포션을 먹을 시 최대 수치를 넘어서는 경우가 발생하였는데 이는 속성값이 변경되기 전에 호출되는 <strong>PreAttributeChange()</strong>함수에서 클램핑 하여 처리하였습니다.</BR></BR>

<strong>PostGameplayEffectExecute()</strong>는 Gameplay Effect가 적용 직후에 호출되는 함수입니다.</BR>
포션을 먹고 바뀐 체력이나 마나량을 클램핑 하여 Set해달라고 요청을 합니다.</BR></BR>

이후 <strong>PreAttributeChange()</strong>에서 바뀐 수치를 한번 더 클램핑하여 체크하고 해당 수치로 Set을 해줍니다.</BR></BR>

### [스탯 시스템]
![1차 스탯](https://github.com/rakkeshasa/AuraRPG/assets/77041622/298898cd-811e-4fcf-9119-801b2902d433)
<div align="center"><strong>Gameplay Effect로 기본 스탯 만들어주기</strong></div></BR>

```
void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
}
```
<div align="center"><strong>GE 자기 자신한테 적용하기</strong></div></BR>
체력과 마나와 같이 기본 스탯을 Attribute로 생성해주고 해당 Attribute를 GE에 세팅을 해줬습니다.</BR></BR>
<strong>ApplyEffectToSelf()</strong>함수는 AuraCharacterBase클래스에 속해있으므로 AddSourceObject(this)를 통해 Source도 Target도 자기 자신으로 해서 GE에 포함된 Attribute가 자신한테 적용되도록 했습니다.</BR></BR>

![부가 스탯](https://github.com/rakkeshasa/AuraRPG/assets/77041622/5c63adf1-ee0b-4a05-b6c9-750de92a6c0c)
<div align="center"><strong>1차 스탯에 영향받는 2차 스탯</strong></div></BR>
MaxHealth와 MasMana를 제외한 2차 스탯의 Attribute는 <strong>Magnitue Calculation Type</strong>을 <strong>Attribute Based</strong>로 설정하여 1차 스탯에 연산을 한 결과값을 가지도록 했습니다.</BR>
1차 스탯이 바뀌면 2차 스탯이 바뀐 후 계속 유지되어야 하므로 Duration Policy를 <strong>Infinite</strong>로 설정했습니다.</BR></BR>

```
UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameter;
	EvaluationParameter.SourceTags = SourceTags;
	EvaluationParameter.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameter, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	// 기본 체력 + 2.5활력 + 10레벨
	return 80.f + (2.5f * Vigor) + (10.f * PlayerLevel);
}
```
<div align="center"><strong>최대 체력과 최대 마나 연산하기</strong></div></BR>
최대 체력과 최대 마나의 경우에는 1차 스탯뿐만 아니라 플레이어가 레벨업 할 때마다 수치를 늘려주고자 다른 연산 타입을 사용했습니다.</BR>
최소 2가지 이상의 요소가 Attribute 값을 결정하기에 따로 C++클래스로 만들었습니다.</BR></BR>

생성자에서는 연산에 사용할 Attribute인 Vigor를 VigroDef에 캡쳐합니다.</BR></BR>

<strong>CalculateBaseMagnitude_Implementation()</strong>에서 Source와 Target의 태그를 GE Spec에서 가져와 EvaluationParameter에 세팅해줍니다.</BR></BR>

<strong>GetCapturedAttributeMagnitude()</strong>에 생성자에서 캡처한 Vigor와 GE Spec, EvaluationParameter, Vigor 수치를 넣어 Vigor속성의 값을 가져옵니다.</BR></BR>

이후 Interface에서 플레이어의 레벨을 가져오고 최대 체력 수식을 return하여 플레이어의 레벨과 Vigor의 수치에 맞게 최대 체력이 정해집니다.</BR></BR>

![최대체력](https://github.com/rakkeshasa/AuraRPG/assets/77041622/897db214-3066-4e6f-a24a-03c3daa995df)
<div align="center"><strong>Custom Calculation Class타입을 이용하여 C++클래스 연동해주기</strong></div></BR>


### [공격 적중 시 피해 입히기]


### [스탯 창에 Attribute 연동하기]
![스탯 미니 창](https://github.com/rakkeshasa/AuraRPG/assets/77041622/ccd566da-7c1d-44e7-bad9-5a40f8e74d70)
<div align="center"><strong>스탯 창에서 1개의 스탯을 담당하는 위젯</strong></div></BR>

```
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FAuraAttributeInfo&, Info);

UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
FAttributeInfoSignature AttributeInfoDelegate;

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
```
<div align="center"><strong>Tag에 맞는 스탯 갖고와서 브로드캐스트하기</strong></div></BR>
<strong>FAuraAttributeInfo</strong>구조체에는 AttributeTag, AttributeName, AttributeDescription, AttributeValue 변수가 있습니다.</br></br>

<strong>BroadcastInitialValues()</strong>에서는 스탯 창에 들어갈 Attribute들을 AttributeSet으로부터 가져옵니다.</br>
TagsToAttributes는 TMap타입의 변수로 키 값은 Attribute에 연결된 태그이며, Value는 해당 Attribute입니다.</br></br>

<strong>BroadcastAttributeInfo()</strong>에서는 Tag이름을 토대로 Attribute를 찾고 AttributeInfo구조체를 가져옵니다.</br>
AttributeInfo구조체의 AttributeValue변수에 현재 Attribute값을 넘겨주고 브로드캐스트합니다.</br></br>

![스탯 창 블브](https://github.com/rakkeshasa/AuraRPG/assets/77041622/3a850bc6-4da8-4c77-992d-a85cc90aebed)
<div align="center"><strong>받은 정보 토대로 텍스트 세팅해주기</strong></div></BR>
AttributeInfoDelegate을 받으면 현재 AttributeInfo에 있는 태그와 스탯 창의 태그가 같은지 체크하고 같다면 스탯 이름과 스탯 값을 세팅해줍니다.</BR>
