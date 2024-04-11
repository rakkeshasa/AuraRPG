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
	// 블루프린트 그래프에서 실행 가능.
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

	// 블루프린트에서 해당 변수를 읽기가 가능
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	// c++에서 구현한 함수를 블루프린트로 구현
	// 이 함수는 c++이 아닌 블루프린트에서 구현해야함
	UFUNCTION(BlueprintImplementableEvent) 
	void WidgetControllerSet();
};
