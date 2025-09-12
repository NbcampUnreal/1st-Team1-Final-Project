#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_CommandWidget.generated.h"

class UTextBlock;
class UImage;
class UGS_SkillBase;
class AGS_Player;

UCLASS()
class GAS_API UGS_CommandWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_CommandWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	// 단축키 아이콘 + 키 텍스트 설정
	UFUNCTION(BlueprintCallable)
	void SetKeyCommand(UTexture2D* IconTexture, const FString& KeyText);

protected:
	// 스킬 아이콘 이미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UImage> SkillIcon;

}; 