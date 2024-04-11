// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "Aura/Aura.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/HighlightInterface.h"
#include "Interaction/SaveInterface.h"
#include "Checkpoint.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class AURA_API ACheckpoint : public APlayerStart, public ISaveInterface, public IHighlightInterface
{
	GENERATED_BODY()
public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

	/* Save Interface */
	virtual bool ShouldLoadTransform_Implementation() override { return false; };
	virtual void LoadActor_Implementation() override;
	/* end Save Interface */

	// üũ����Ʈ�� �����ߴ���(410)
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bReached = false;

	UPROPERTY(EditAnywhere)
	bool bBindOverlapCallback = true;

protected:

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void BeginPlay() override;

	/* Highlight Interface */
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	virtual void HighlightActor_Implementation() override;
	virtual void UnHighlightActor_Implementation() override;
	/* Highlight Interface */

	// USceneComponent: FScene�� �߰��ؾ��ϴ�(����Ʈ, �޽�, ���� ��) ������Ʈ�� ���̽� Ŭ�����Դϴ�.
	// �� ������Ʈ�� ������ Ư�� ������ ��ġ�� �����ϴ� ���� ������Ʈ�Դϴ�. 
	// �� ��ġ�� ������Ʈ�� ��ġ, ȸ��, �������� �����ϴ� Ʈ������(FTransform Ŭ����)���� ���ǵ˴ϴ�. 
	// �� ������Ʈ�� ���� �پ Ʈ���� ������ �� ������, ���ʹ� �ϳ��� �� ������Ʈ�� "��Ʈ"�� �����Ͽ� ������ ���� ��ġ, ȸ��, �������� �ش� ������Ʈ�κ��� ������ �� �ֽ��ϴ�.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MoveToComponent;

	UPROPERTY(EditDefaultsOnly)
	int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;

	// üũ����Ʈ�� ���� �� �ش� üũ����Ʈ ���͸����� �������� �����ϱ� ����
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintCallable)
	void HandleGlowEffects();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
