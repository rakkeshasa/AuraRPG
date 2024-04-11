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

### [Attribute 구현]

### [투사체 구현]

### [공격 적중 시 피해 입히기]
