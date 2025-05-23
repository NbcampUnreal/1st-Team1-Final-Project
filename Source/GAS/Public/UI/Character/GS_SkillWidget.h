#pragma once

#include "CoreMinimal.h"
#include "Character/Skill//GS_SkillComp.h"
#include "Blueprint/UserWidget.h"
#include "GS_SkillWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;

UCLASS()
class GAS_API UGS_SkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_SkillWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	//skill image and reset cool time 
	void Initialize(UGS_SkillBase* Skill);

	UFUNCTION()
	void OnSkillCoolTimeChanged(float InCurrentCoolTime) const;
	
	FORCEINLINE AActor* GetOwningActor()const { return OwningCharcter; }
	FORCEINLINE ESkillSlot GetSkillSlot() const {return SkillSlot;}
	
	void SetOwningActor(AGS_Character* InOwningCharacter)
	{
		OwningCharcter = InOwningCharacter;
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentCoolTimeText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> CoolTimeBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UImage> SkillImage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharcter;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget,AllowPrivateAccess))
	ESkillSlot SkillSlot;
};
