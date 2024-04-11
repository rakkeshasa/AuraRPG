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
	bReplicates = true; // ��Ʈ��ũ �� ����

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	// GameMode�� �÷��̾ ������ �� PlayerController��� Ư���� ���͵� ����
	// ���� ���迡�� ���Ǽ����� �÷��̾ �뺯�ϴ� ����
	// ���� ���迡�� �÷��̾�� 1:1�� �����ϸ鼭 ���� �����ϴ� ����
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
	// RPC�� ���ÿ��� ȣ������� (ȣ�����)�ٸ� �ӽſ��� ���� ����Ǵ� �Լ��� ���մϴ�.
	// RPC �Լ��� ��Ʈ��ũ ������ ���� Ŭ���̾�Ʈ�� ���� ���̿� �޽����� ������ �� �ֽ��ϴ�.
	// �Լ��� RPC �� �����Ϸ��� UFUNCTION ���� Server, Client Ű���带 �ٿ��ֱ⸸ �ϸ� �˴ϴ�.
	
	// ���� ��Ʈ�ѷ��� ��쿡�� �ش� ���� �ؽ�Ʈ�� ǥ��
	// ������ �ִµ�, Ŭ���̾�Ʈ ������ �÷��̾� ��Ʈ�ѷ����� ȣ���ϴ� ��� �������� �������� ǥ�õ��� ����.
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		// Unreal�������� �ڵ������� ���ִ� �۾�
		// �츮�� �������� ������ֹǷ� ���� ȣ��
		DamageText->RegisterComponent();
		// Ÿ�ٿ��� ������ ��ġ�� ���̰� �ʹ�.
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		// ������ �ִϸ��̼Ǹ� ����ǵ���, ������ ��ġ�� ��� �Ⱥپ�ٴϵ��� �Ѵ�.
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext); // assert�� ����

	// ���� Input ���� �ý���
	// �̱��� -> ���� �ϳ�
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	// ���콺 Ŀ�� ���̵���
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default; // Ŀ���� �⺻���

	FInputModeGameAndUI InputModeData; // ���� �÷��̿� UI ���� ���� �Է� ��� ��ȯ�� ����
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // ���콺�� ����Ʈ�� ����x
	InputModeData.SetHideCursorDuringCapture(false); // �Է��߿��� ���콺�� ����
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	// �÷��̾� ��Ʈ�ѷ��� Ŀ���� �Է� ���ε��� ����� �� �ְ� ��
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	
	// �̵�����
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	
	// ���� ����
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);

	// �ɷ� ����
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// �Է¿� ���� �̵�

	// Activation Owned Tags�� �ɷ� Ȱ��ȭ �� ASC�� �ο��ǰ� �ɷ� ���� �Ǵ� ��� �� ASC���� ���ŵȴ�.
	// Electronic ��ų�� Release�±� ���� �� Ȱ��ȭ �ϸ� ������ �±׿� ���Ͽ� return
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}

	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>(); // ���� ����
	const FRotator Rotation = GetControlRotation(); // �÷��̾ ���� �ִ� ����
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f); // ���� ��ŭ ȸ��

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // ������ ���ϴ� ����
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // ���������� ���ϴ� ����

	if (APawn* ControlledPawn = GetPawn<APawn>()) // ��Ʈ�ѷ��� ������ ���� �ִ���
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	// ���콺 �� ���¸� ������ �ȵǵ���
	// ����Ʈ�� ��� ���� �ٸ� �ൿ�� ���� ����
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
		
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	// ���� ��ȯ���� �����Ѵٸ� �ٸ� �ݸ���ä�� ���
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	
	if (!CursorHit.bBlockingHit) 
		return;

	LastActor = ThisActor;
	// ThisActor = Cast<IHighlightInterface>(CursorHit.GetActor()); ��ü�ϱ�
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
		// ThisActor�� null�̵� �ƴϵ� LastActor�� ���� �ƴϴ�

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

	// ��Ŭ���� ���ȴ���
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		// ThisActor�� HighLight�������̽��� �̹� ����Ǿ� �ִ�.
		if (IsValid(ThisActor))
		{
			// Ÿ������ ����� ������ �ƴ���
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

	// Ÿ���ð� ����ƮŰ�� ������ �ʰ� �־�� �̵��� �� �ִ�
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// �ش� ���Ͱ� ���̶���Ʈ �������̽��� ������ �ִٸ�
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				// ���� ��� �ش� �Լ� ������ ������.
				// üũ����Ʈ�� ��� �ش� ��ġ�� �ڵ��̵��ϵ��� �����.
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				// Player_Block_InputPressed�±װ� ���� ���� ���̾ư��� Ȱ��ȭ
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}

			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints(); // Spline �ʱ�ȭ
				// NavPath->PathPoints �츮�� ���ϴ� ���
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					// Spline�� SplinePoint �߰��ϱ�
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}

				if (NavPath->PathPoints.Num() > 0) 
				{
					// ������: NavMesh�� �ƴ� ��ü�� Ŭ���� ��� �ɾ��
					// �ذ��: ������ ����Ʈ�� Destination����
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
	// ��Ŭ���� �ƴ϶�� ĳ���� �����̴� ����� �ƴϹǷ�
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) 
			GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	// ��Ŭ���̰� � ��ü(����)�� Ÿ������ �Ǿ��ٸ�
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC())
			GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();

		// ���콺 ��ġ ��������
		if (CursorHit.bBlockingHit)
			CachedDestination = CursorHit.ImpactPoint;


		// ���� �ִٸ�
		if (APawn* ControlledPawn = GetPawn())
		{
			// �ش� ��ġ�� �����̰� �ϱ�
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
		// ASC������ ������ֱ�
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
