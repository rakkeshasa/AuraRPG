# AuraRPG
![introduction](https://github.com/rakkeshasa/AuraRPG/assets/77041622/fb21468c-7fc3-4b29-a9c2-cd305ce80184)

Unreal Engine v5.2를 이용한 RPG게임
</BR>

목차
---
- [간단한 소개](#간단한-소개)
- [플레이 영상](#플레이-영상)
- [코드 구현](#코드-구현)
  * [플레이어 이동 구현](#플레이어-이동-구현)
  * [체력 및 마나 구현](#체력-및-마나-구현)
  * [체력 및 마나 UI에 연동시키기](#체력-및-마나-UI에-연동시키기)
  * [포션 구현](#포션-구현)
  * [스탯 시스템](#스탯-시스템)
  * [FireBolt 스킬](#FireBolt-스킬)
  * [Electrocute 스킬](#Electrocute-스킬)
  * [Arcane Shard 스킬](#Arcane-Shard-스킬)
  * [데미지 주기](#데미지-주기)
  * [경험치 시스템](#경험치-시스템)
  * [스탯 창에 Attribute 연동하기](#스탯-창에-Attribute-연동하기)
 
- [사용한 플러그인](#사용한-플러그인)
- [Reference](#Reference)


## 간단한 소개
![Example](https://github.com/rakkeshasa/AuraRPG/assets/77041622/46c7c4eb-a558-43c9-9a3a-4f9723c57344)

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
플레이어 이동을 키보드 방식과 마우스 입력 방식 2가지로 구성하였습니다.</BR>
키보드 방식은 Enhanced Input을 이용하여 해당 키에 맞는 방향으로 이동합니다.</BR>
마우스 입력 방식은 마우스가 클릭한 위치를 도착지로 두고 플레이어가 자동으로 이동합니다.</BR></BR>

우선 Enhanced Input을 이용한 키보드 방식부터 살펴보겠습니다.</BR></BR>

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

```
void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	FollowTime += GetWorld()->GetDeltaSeconds();

	if (CursorHit.bBlockingHit)
		CachedDestination = CursorHit.ImpactPoint;

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection);
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* ControlledPawn = GetPawn();
	if (FollowTime <= ShortPressThreshold && ControlledPawn)
	{
		if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
		{
			Spline->ClearSplinePoints(); 
			for (const FVector& PointLoc : NavPath->PathPoints)
			{
				Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
			}

			if (NavPath->PathPoints.Num() > 0) 
			{
				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true;
			}
		}
	}
	FollowTime = 0.f;
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning)
		return;

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
			bAutoRunning = false;

	}
}
```
<div align="center"><strong>마우스 입력으로 자동 이동하기</strong></div></BR>
<strong>AbilityInputTagHeld()</strong>는 마우스를 클릭한 위치를 CachedDestination에 저장하고 해당 위치로 Pawn을 이동시키는 역할을 합니다.</BR>
FollowTime은 마우스 클릭 유지 시간을 체크하며 유지 시간이 ShortPressThreshold보다 작다면 <strong>AbilityInputTagReleased()</strong>에서 경로를 계산해주며, ShortPressThreshold보다 크면 홀딩한 마우스 위치로 Pawn을 이동시킵니다.</BR></BR>

<strong>AbilityInputTagReleased()</strong>에서는 <strong>FindPathToLocationSynchronously()</strong>함수를 이용하여 현재 Pawn의 위치와 마우스로 클릭한 위치까지의 경로를 구해줬습니다.</BR></BR>
PathPoint는 NavPath에 저장된 경로의 각 지점을 나타내며 각 PathPoint를 Spline에 넣어주고 있습니다.</BR>
Spline은 USplineComponent타입으로 두 PathPoint를 부드러운 곡선 경로로 표현하여 코너에서 Pawn이 부자연스럽게 회전하는 것을 방지하였습니다.</BR></BR>

bAutoRunning이 true가 되면 <strong>AutoRun()</strong>에서 Pawn에서 가장 가까운 Spline까지의 위치와 방향벡터를 구하여 해당 방향으로 Pawn이 움직이게 합니다.</BR>
Pawn이 각 Spline에 도달할 때 마다 목적지인 CachedDestination까지 가까워지며 목적지에 어느정도 가까워지면 더 이상 자동 이동을 안하도록 bAutoRunning을 false로 바꿉니다.</BR></BR>


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
<div align="center"><strong>Custom Calculation Class타입을 이용하여 C++클래스 연동해주기</strong></div></BR></BR>


### [FireBolt 스킬]

![불덩이](https://github.com/rakkeshasa/AuraRPG/assets/77041622/313f66b1-658f-4efa-be34-925ec0a91f7e)
<div align="center"><strong>FireBolt스킬에 쓸 투사체인 불덩이</strong></div></BR>


```
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	Projectile->FinishSpawning(SpawnTransform);
}
```
<div align="center"><strong>투사체 소환시키기</strong></div></BR>
<strong>SpawnProjectile()</strong>은 투사체를 소환시키는 함수로 FireBolt스킬에 쓰일 불덩이를 소환하는데 사용됩니다.</br></br>

우선 불덩이가 플레이어의 지팡이에서 나갈 수 있도록 지팡이의 위치를 구하고 나아갈 방향을 구합니다.</br>
이후 <strong>SpawnActorDeferred()</strong>을 이용하여 해당 투사체의 인스턴스를 생성합니다.</br></br>

<strong>ProjectileClass</strong>에는 불덩이 클래스가 들어갈 것이며 이는 엔진상에서 BP_FireBolt로 만들었습니다.</br>
<strong>SpawnTransform</strong>에는 위에서 구한 지팡이의 위치와 나아갈 방향이 들어있습니다.</br></br>

생성이 완료된 후 투사체가 데미지관련 정보를 위해 <strong>MakeDamageEffectParamsFromClassDefaults()</strong>함수를 통하여 FDamageEffectParams 구조체의 변수에 값을 채우고 해당 구조체를 return합니다.</br>
FDamageEffectParams 구조체에는 데미지 계산을 할 GameplayEffect클래스, SourceASC, TargetASC, Damage등의 변수가 있습니다.</br></br>

<strong>FinishSpawning()</strong>을 호출하여 투사체를 월드에서 지정된 위치에 소환합니다.</br></br>

![FireBolt](https://github.com/rakkeshasa/AuraRPG/assets/77041622/f5c65931-cb2a-4465-94e1-fc7d2d43a905)
<div align="center"><strong>스킬 모션 중 AnimNotify로 이벤트를 받은 후 SpawnProjectile호출</strong></div></BR>
액티브 스킬들은 애니메이션을 재생하기 때문에 딜레이가 있습니다. 따라서 액티브 스킬인 GA(Gameplay Ability)는 1프레임 내에 실행되고 끝나지 않기 때문에 Ability Task(AT)를 활용했습니다.</BR>
그 중 <Strong>PlayMontageAndWait</Strong>노드를 사용하여 애니메이션 재생이 끝나면 GA가 끝나도록 하였습니다.</BR></BR>

애니메이션이 재생하는 도중에 AnimNotify를 통해 Tag를 보냈으며, 해당 Tag를 WaitGameplayEvent노드가 받으면 C++로 구현한 SpawnProjectile()을 이용하여 투사체가 소환됩니다.</BR>
따라서 애니메이션 중에 태그가 활성화 되고 해당 태그가 활성화 되면 투사체를 월드에 소환하게 됩니다.</BR></BR>

### [Electrocute 스킬]
![electrocute](https://github.com/rakkeshasa/AuraRPG/assets/77041622/4c35c530-9d95-4239-ae38-e931dc6498ba)
<div align="center"><strong>마우스 위치에 빔을 쏘는 스킬</strong></div></BR>

```
void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);

			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));

			UKismetSystemLibrary::SphereTraceSingle(OwnerCharacter, SocketLocation, BeamTargetLocation,
				10.f, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}
```
<div align="center"><strong>마우스 위치와 Actor구하기</strong></div></BR>
<strong>TraceFirstTarget()</strong>는 마우스 위치를 입력으로 받고 지팡이에서 마우스까지 <strong>SphereTraceSingle()</strong>를 통해 충돌 검사를 합니다.</br>
충돌 검사 결과는 HitResult에 들어가게 되고 충돌하게 된 경우에는 해당 위치와 위치에 있는 Actor를 저장합니다.</br></br>

```
void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);

	TArray<AActor*> OverlappingActors;
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation());

	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);

	UAuraAbilitySystemLibrary::GetClosestTargets(
		NumAdditionalTargets, 
		OverlappingActors, 
		OutAdditionalTargets, 
		MouseHitActor->GetActorLocation());
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, 
	float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);

		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}
```
<div align="center"><strong>첫 타격 대상의 주변 대상 찾기</strong></div></BR>
Electrocute 스킬은 레벨업을 하면 타격하는 대상 수가 늘어납니다.</BR></BR> 

연쇄 번개를 맞는 대상은 우선 첫 타격 대상에서 범위 내에 있어야 하며, 그 중 가장 가까이 있는 대상에게 연쇄 번개가 나갑니다.</BR> 
<strong>GetLivePlayersWithinRadius()</strong>함수에서 OverlapMultiByObjectType()를 사용하여 연쇄 번개 범위 내에 있는 오브젝트를 찾은 후 Overlaps에 저장합니다.</BR> 
Overlaps의 요소들을 순회하면서 해당 요소가 CombatInterface를 갖고 있으면 몬스터나 플레이어이므로 OutOverlappingActors 배열에 넣어 연쇄 번개 범위에 있는 Actor들을 추출합니다.</BR></BR>

<strong>GetClosestTargets()</strong>함수에서는 OutOverlappingActors 배열에 있는 Actor들을 순회하면서 첫 타격 대상과의 거리를 비교하면서 제일 가까이 있는 타격 대상을 OutAdditionalTargets배열에 넣어줍니다.</BR></BR>

연쇄 번개 대상이 여러명이면 가장 가까운 대상을 구하고 OutOverlappingActors 배열에서 해당 대상을 뺀 후 다시 가장 가까운 대상을 구합니다.</BR></BR>

![electrocute 블프1](https://github.com/rakkeshasa/AuraRPG/assets/77041622/6a831904-3a83-49cf-91ea-75563951a03f)
<div align="center"><strong>구해준 위치와 Actor를 GameplayCue의 파라미터로 만들어주기</strong></div></BR>
함수에서 구해준 위치와 Actor를 Electorcute스킬의 GameplayCue파라미터로 만들어 GameplayCue에서 전기 이펙트를 생성할때 입력받은 위치까지 전기가 나가게 하고 
Actor가 CombatInterface를 가질 경우 Actor의 위치를 구해 전기가 Actor에게 붙게 했습니다.</BR></BR>

### [Arcane Shard 스킬]
마지막 Arcane Shard 스킬은 바닥에서 커다란 마법 조각이 솟아올라 데미지를 주는 스킬입니다.</BR>
Arcane Shard스킬에 스킬 포인트를 투자하여 스킬을 강화할 경우 소환되는 마법 조각의 개수가 증가합니다.</BR></BR>

![ArcaneShard](https://github.com/rakkeshasa/AuraRPG/assets/77041622/e379e54a-16d2-4c6e-8fc0-68ab77bd69a6)
<div align="center"><strong>강화된 Arcane Shard스킬</strong></div></BR>
Arcane Shard는 랜덤한 위치에서 여러개가 나타나는 것이 아닌 위치가 정해진 포인트에서 소환되는 형태입니다.</BR>
제일 처음 소환되는 마법 조각이 첫 번째 포인트라면 그 이후 소환되는 마법 조각들은 미리 위치를 지정한 포인트에서 소환됩니다.</BR></BR>

![point](https://github.com/rakkeshasa/AuraRPG/assets/77041622/7bfd8546-2a4a-49a8-b8aa-e66767c47980)
<div align="center"><strong>미리 지정해놓은 포인트 위치</strong></div></BR></BR>

```
TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride)
{
	TArray<USceneComponent*> ArrayCopy;

	for (USceneComponent* Pt : ImmutablePts)
	{
		if (Pt != Pt_0)
		{
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}

		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f);
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f);

		FHitResult HitResult;
		TArray<AActor*> IgnoreActors;
		UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, 
			IgnoreActors, 
			TArray<AActor*>(), 
			1500.f, 
			GetActorLocation());

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		ArrayCopy.Add(Pt);
	}
	return ArrayCopy;
}
```
<div align="center"><strong>마법 조각들이 소환될 포인트의 위치 계산해주기</strong></div></BR>

<strong>GetGroundPoints()</strong>는 미리 지정한 포인트를 첫번째 소환된 포인트 위치에서 거리와 방향을 계산하여 소환할 위치를 알려줍니다.</br></br>

ImmutablePts는 미리 지정한 포인트로 총 10개의 포인트를 지정했으며, 최대 10개의 조각이 소환될 수 있습니다.</br>
ImmutablePts를 순회하면서 제일 첫번째 소환 포인트인 PT_0가 아니라면 포인트의 위치를 다음 포인트로 세팅해줍니다.</br></br>

마법 조각들은 지면에서 생성 되기 때문에 지면에 경사가 있을 경우를 대비하여 경사가 낮은 곳과 경사가 높은 곳을 미리 지정합니다.</br>
또한 마법 조각들이 지면이 아닌 Actor의 머리 위에 소환되면 안되기 때문에 GetLivePlayersWithinRadius()함수를 통하여 CombatInterface를 가지고 있는 Actor를 충돌테스트에서 제외시켜줍니다.</br>

LineTraceSingleByProfile()함수를 사용하여 지면과의 충돌 테스트를 진행한 후 해당 지면의 높이를 구하여 땅바닥의 높이(z축)를 구해 포인트의 위치를 조정합니다.</br>
포인트의 위치를 땅바닥으로 해주고, 솟아나는 방향을 땅바닥의 노말 벡터를 구하여 마법 조각이 무조건 하늘로 향하는 것이 아니라 바닥의 수직 방향에서 솟아오르도록 했습니다.</br></br>



### [데미지 주기]

1차적으로 투사체가 소환될때 <strong>MakeDamageEffectParamsFromClassDefaults()</strong>를 통하여 FDamageEffectParams 구조체에 변수가 채워지는데
해당 구조체 내에 있는 GameplayEffect클래스의 변수가 ExecCalc_Damage클래스로 세팅이 되어있습니다.</br></br>

ExecCalc_Damage클래스는 투사체가 입힐 데미지를 계산하는 클래스입니다.</br></br>

데미지는 부가 스탯인 방어력 관통, 방어력, 치명타 확률, 치명타 데미지 등으로 계산이 되며, 투사체는 이러한 정보를 갖고 있다가
물체와 부딪혀 OnSphereOverlap()함수가 호출되면 데미지를 줍니다.</br></br>

```
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageTypeTag = Pair.Key; // 데미지 타입(Damage.Fire)
		const FGameplayTag ResistanceTag = Pair.Value; // 데미지 저항(Attributes.Resistance.Fire

		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		// 해당 공격의 데미지
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false);
		
		if (DamageTypeValue <= 0.f)
			continue;

		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;
		Damage += DamageTypeValue;
	}

	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	Damage = bBlocked ? Damage / 2.f : Damage;
}
```
<div align="center"><strong>투사체가 입힐 데미지를 계산하는 클래스</strong></div></BR>

데미지는 해당 공격이 물리 공격, 마법 공격에 따라 기본 저항치가 다릅니다.</BR></BR>

우선 해당 공격 타입의 데미지를 GE Spec에서 GetSetByCallerMagnitude()를 통해 수치를 얻어옵니다.</BR></BR>

저항 수치를 AttemptCalculateCapturedAttributeMagnitude()를 통해 얻어옵니다. 들어가는 매개변수 중 CaptureDef는 Map타입의 TagsToCaputreDefs변수에서 해당 공격 타입을 Key로 하여 해당 타입의 저항치를 Value로 얻어옵니다.</BR></BR>

얻어온 저항치는 Resistance에 넣어주고 0~100사이의 수로 클램핑해주고 해당 퍼센트만큼 데미지를 빼줍니다.</BR></BR>

이후 부가 스탯인 TargetBlockChance의 속성값을 구해 막히면 위에서 구해준 데미지에서 절반을 빼주고, 막히지 않으면 본래 데미지를 줍니다.</BR></BR>

```
void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
		UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
	}
	Destroy();
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);

	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);

	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContexthandle;
}
```
<div align="center"><strong>투사체 충돌 시 데미지 주기</strong></div></BR>
스킬로 날린 투사체가 물체에 부딪히면 <strong>OnSphereOverlap()</strong>함수가 호출됩니다.</BR></BR>

투사체를 날린 Actor가 맞으면 return하고 다른 Actor가 맞았다면 상대의 ASC를 데미지 관련 정보가 들어있는 DamageEffectParams 구조체에 넣고 해당 구조체를 ApplyDamageEffect()에 넣어 호출합니다.

<strong>ApplyDamageEffect()</strong>에서는 GE Context Handle과 GE Spec Handle을 DamageEffectParams구조체 정보 기반으로 생성하여 ASC가 GE에 접근할 수 있도록 해줍니다.</BR></BR>

<strong>AssignTagSetByCallerMagnitude()</strong>을 사용하여 Spec에 데미지 타입과 데미지 수치 정보를 넣어주고, 맞은 Actor의 ASC는 Spec정보 기반으로 GameEffect를 자기 자신한테 적용합니다.</BR></BR>


### [경험치 시스템]

```
void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		FGameplayEventData Payload;
		Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
		Payload.EventMagnitude = XPReward;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
	}
}
```
<div align="center"><strong>몬스터가 죽으면 Gameplay Event 발생시키기</strong></div></BR>
<strong>SendXPEvent()</strong>함수는 몬스터가 죽으면 발동되는 함수로 몬스터의 타입과 레벨을 구하여 해당 정보 토대로 XP수치를 구해옵니다.</BR></BR>

XP수치를 구할 때 사용하는 <strong>GetXPRewardForClassAndLevel()</strong>함수는 몬스터 타입(근거리, 원거리, 마법사)마다 주어진 XP를 찾아서 return해줍니다.</BR>
Payload에는 이벤트에 사용할 정보들인 태그와 XP수치를 넣어 Gameplay Event가 발생하면 사용할 수 있게 합니다.</BR>
<strong>SendGameplayEventToActor()</strong>함수를 이용하여 XP를 받는 쪽으로 이벤트를 발생시킬 Tag와 Payload를 담아 Gameplay Event를 보냅니다. </BR></BR>

![경험치 블프1](https://github.com/rakkeshasa/AuraRPG/assets/77041622/a1e6a82c-86d9-43c2-a2e6-9d8b0c3efd51)
<div align="center"><strong>Gameplay Event받아오기</strong></div></BR>
WaitGameplayEvent노드를 이용하여 'Attributes'태그를 루트로 하는 하위 태그들이 올 때 Gameplay Event를 받습니다.</BR>
C++코드에서는 'Attributes_Meta_IncomingXP'로 태그를 넘겨줬기 때문에 해당 태그는 Attributes의 자식 태그이므로 Gameplay Event가 발생합니다.</BR></BR>

MakeOutgoingSpec노드를 이용하여 XP속성이 담긴 GE를 조작할 수 있도록 해줍니다.</BR></BR>

![경험치 블프2](https://github.com/rakkeshasa/AuraRPG/assets/77041622/df71c5e8-2215-4585-b55a-ebcaad92dc2e)
<div align="center"><strong>받은 이벤트 토대로 자기 자신한테 GE적용해주기</strong></div></BR>
XP속성의 Magnitude계산 타입은 'Set by Caller'로 계산을 하여 수치를 구하는게 아니라 해당 속성에 넣어준 값만큼 받아서 연산을 해줍니다.</BR></BR>
AssignTagSetbyCallerMagnitude노드를 사용하여 XP속성 값에 몬스터를 잡고 넘어온 XP값을 넘겨주고 GE에서는 넘어온 XP값만큼 더하여 ApplyGameEffectSpecToSelf노드를 이용하여 자신한테 해당 GE를 적용시켜줍니다.</BR></BR>

```
void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0.f);

	if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);

		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		if (NumLevelUps > 0)
		{
			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);

			int32 AttributePointsReward = 0;
			int32 SpellPointsReward = 0;
			for (int32 i = 0; i < NumLevelUps; ++i)
			{
				SpellPointsReward += IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel + i);
				AttributePointsReward += IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel + i);
			}

			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);

			bTopOffHealth = true;
			bTopOffMana = true;

			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
		}

		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

void AAuraPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}
```
<div align="center"><strong>XP속성을 가지는 GE가 적용된 후에 호출되는 함수</strong></div></BR>

<strong>HandleIncomingXP()</strong>는 PostGameplayEffectExecute()에서 호출되는 함수로 위에서 언급한 XP속성 값이 바뀌는 GE를 적용 후에 호출됩니다.</BR></BR>
현재 레벨과 XP를 구해온 뒤 현재XP에 몬스터를 잡고 얻은 XP를 더해준 XP값의 총량을 계산하여 현재 XP값의 총량이 어느 레벨인지 계산해줍니다.</BR>
새롭게 구한 레벨과 현재 레벨을 빼서 얼마만큼 레벨을 올려야하는지 구한 후 올려야하는 레벨 만큼 스탯 포인트, 스킬 포인트를 올리고 모든 자원(체력, 마나)를 회복시켜줍니다.</BR>
이후에는 레벨업과 XP를 증가시키면서 연결된 델리게이트인 OnLevelChangedDelegate와 OnXPChangedDelegate를 브로드캐스팅합니다.</BR></BR>

```
void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		// 현재 레벨에 맞는 경험치 통의 크기(301 ~ 600 -> 600 - 300)
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		
		// 현재 경험치에서 지난 레벨 경험치 통 빼기(450 - 300 = 150)
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}
```
<div align="center"><strong>OnXPChangedDelegate와 바인딩 된 함수</strong></div></BR>
XP가 바뀌면 UI상에서 레벨을 올려주고 경험치 바를 변경해주기 위해 브로드캐스팅합니다.</BR>
DeltaLevelRequirement변수는 바뀐 레벨의 경험치 통의 크기를 구하고, XPForThisLevel변수는 바뀐 XP총량에서 이전 레벨까지의 경험치 통의 총량을 빼서 현재 레벨에서 얼마만큼의 XP를 채웠는지를 나타냅니다.</BR>
현재 레벨에서 채운 XP/현재 레벨의 경험치 통을 연산하여 경험치 바에 나타낼 퍼센트를 구해 브로드캐스팅해줍니다.</BR></BR>

![경험치 통](https://github.com/rakkeshasa/AuraRPG/assets/77041622/4b33bcde-b6a6-4d9a-91dc-441be8e52b01)
<div align="center"><strong>받은 퍼센트를 경험치 바에 적용하는 모습</strong></div></BR></BR>


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
AttributeInfoDelegate을 받으면 현재 AttributeInfo에 있는 태그와 스탯 창의 태그가 같은지 체크하고 같다면 스탯 이름과 스탯 값을 세팅해줍니다.</BR></BR>


## 사용한 플러그인
1. Enhanced Input
![EnhancedInput](https://github.com/rakkeshasa/AuraRPG/assets/77041622/a74d7cdc-b43f-43a3-b31b-2919b7745357)
Enhanced Input을 사용하기 위해 적용했습니다.</BR>

2. Motion Warping
![MotionWarping](https://github.com/rakkeshasa/AuraRPG/assets/77041622/b2ff2695-6c70-4a55-b473-bd6fb14109ac)
애니메이션 도중 원하는 방향으로 회전할 수 있도록 도와주는 플러그인입니다.</BR>

3. Gameplay Abilities
![Gameplay Abilities](https://github.com/rakkeshasa/AuraRPG/assets/77041622/7d8291d8-275d-499c-a5d6-fd63bef6ace2)
복잡한 게임플레이 상호작용을 처리할 수 있는 GameplayEffect 및 GameplayAbility 클래스를 추가해주는 플러그인입니다.</BR>

4. UMG ViewModel
![MVVM](https://github.com/rakkeshasa/AuraRPG/assets/77041622/1f04733f-a222-4d1d-b421-4dd3e50ad2cd)
로딩 화면 UI에 MVVM모델을 적용하기 위해 플러그인을 사용했습니다.</BR></BR>

## Reference
[Github GAS Document](https://github.com/tranek/GASDocumentation)</br></br>
[이득우의 언리얼 프로그래밍](https://www.inflearn.com/course/%EC%9D%B4%EB%93%9D%EC%9A%B0-%EC%96%B8%EB%A6%AC%EC%96%BC-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%98%EB%B0%8D-part-4)
