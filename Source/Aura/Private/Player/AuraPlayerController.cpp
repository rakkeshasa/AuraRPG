// Chocolate Maniac

#include "Player/AuraPlayerController.h"

#include "Aura/Aura.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "Interaction/HighlightInterface.h"
#include "UI/Widget/DamageTextComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true; // 네트워크 상 복제

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	// GameMode는 플레이어가 입장할 때 PlayerController라는 특별한 액터도 생성
	// 게임 세계에서 현실세계의 플레이어를 대변하는 액터
	// 게임 세계에서 플레이어와 1:1로 소통하면서 폰을 조정하는 역할
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
	UpdateMagicCircleLocation();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	// https://docs.unrealengine.com/4.26/ko/InteractiveExperiences/Networking/Actors/RPCs/
	// RPC는 로컬에서 호출되지만 (호출과는)다른 머신에서 원격 실행되는 함수를 말합니다.
	// RPC 함수는 네트워크 연결을 통해 클라이언트와 서버 사이에 메시지를 전송할 수 있습니다.
	// 함수를 RPC 로 선언하려면 UFUNCTION 선언에 Server, Client 키워드를 붙여주기만 하면 됩니다.
	
	// 로컬 컨트롤러인 경우에만 해당 피해 텍스트가 표시
	// 서버에 있는데, 클라이언트 소유의 플레이어 컨트롤러에서 호출하는 경우 서버에는 데미지가 표시되지 않음.
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		// Unreal엔진에서 자동적으로 해주던 작업
		// 우리는 동적으로 만들어주므로 직접 호출
		DamageText->RegisterComponent();
		// 타겟에게 데미지 수치를 붙이고 싶다.
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		// 떼내고 애니메이션만 실행되도록, 데미지 수치가 계속 안붙어다니도록 한다.
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext); // assert와 같음

	// 향상된 Input 서브 시스템
	// 싱글톤 -> 오직 하나
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	// 마우스 커서 보이도록
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default; // 커서는 기본모양

	FInputModeGameAndUI InputModeData; // 게임 플레이와 UI 상태 간의 입력 모드 전환을 관리
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스가 뷰포트에 고정x
	InputModeData.SetHideCursorDuringCapture(false); // 입력중에도 마우스가 보임
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	// 플레이어 컨트롤러가 커스텀 입력 바인딩을 사용할 수 있게 함
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	
	// 이동관련
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	
	// 공격 관련
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);

	// 능력 관련
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// 입력에 따른 이동

	// Activation Owned Tags는 능력 활성화 시 ASC에 부여되고 능력 종료 또는 취소 시 ASC에서 제거된다.
	// Electronic 스킬에 Release태그 빼고 다 활성화 하면 나머지 태그에 대하여 return
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}

	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>(); // 방향 벡터
	const FRotator Rotation = GetControlRotation(); // 플레이어가 보고 있는 방향
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f); // 방향 만큼 회전

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // 앞으로 향하는 벡터
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // 오른쪽으로 향하는 벡터

	if (APawn* ControlledPawn = GetPawn<APawn>()) // 컨트롤러가 소유한 폰이 있는지
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	// 마우스 블럭 상태면 조준이 안되도록
	// 라이트닝 쏘는 동안 다른 행동을 막기 위함
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
		
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	// 샤드 소환문이 존재한다면 다른 콜리전채널 사용
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	
	if (!CursorHit.bBlockingHit) 
		return;

	LastActor = ThisActor;
	// ThisActor = Cast<IHighlightInterface>(CursorHit.GetActor()); 대체하기
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}

	if (LastActor != ThisActor)
	{
		// ThisActor가 null이든 아니든 LastActor가 이젠 아니다

		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}

void AAuraPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void AAuraPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}

	// 좌클릭이 눌렸는지
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		// ThisActor에 HighLight인터페이스가 이미 내재되어 있다.
		if (IsValid(ThisActor))
		{
			// 타게팅한 대상이 적인지 아닌지
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
			// bAutoRunning = false;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}

	if (GetASC()) 
		GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) 
			GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC())
		GetASC()->AbilityInputTagReleased(InputTag);

	// 타게팅과 쉬프트키를 누르지 않고 있어야 이동할 수 있다
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 해당 액터가 하이라이트 인터페이스를 가지고 있다면
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				// 적일 경우 해당 함수 내용을 비워줬다.
				// 체크포인트일 경우 해당 위치로 자동이동하도록 해줬다.
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				// Player_Block_InputPressed태그가 없을 때만 나이아가라 활성화
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}

			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints(); // Spline 초기화
				// NavPath->PathPoints 우리가 원하는 경로
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					// Spline에 SplinePoint 추가하기
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}

				if (NavPath->PathPoints.Num() > 0) 
				{
					// 문제점: NavMesh가 아닌 물체를 클릭시 계속 걸어간다
					// 해결법: 마지막 포인트를 Destination으로
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	// 좌클릭이 아니라면 캐릭터 움직이는 기능이 아니므로
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) 
			GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	// 좌클릭이고 어떤 물체(몬스터)가 타겟팅이 되었다면
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC())
			GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();

		// 마우스 위치 가져오기
		if (CursorHit.bBlockingHit)
			CachedDestination = CursorHit.ImpactPoint;


		// 폰이 있다면
		if (APawn* ControlledPawn = GetPawn())
		{
			// 해당 위치로 움직이게 하기
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
		// ASC없으면 만들어주기
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));

	return AuraAbilitySystemComponent;
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

void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}
}

void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}
