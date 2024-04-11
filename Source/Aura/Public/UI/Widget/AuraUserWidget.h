// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// �������Ʈ �׷������� ���� ����.
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

	// �������Ʈ���� �ش� ������ �бⰡ ����
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	// c++���� ������ �Լ��� �������Ʈ�� ����
	// �� �Լ��� c++�� �ƴ� �������Ʈ���� �����ؾ���
	UFUNCTION(BlueprintImplementableEvent) 
	void WidgetControllerSet();
};
