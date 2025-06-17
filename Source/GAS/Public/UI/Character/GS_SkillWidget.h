#pragma once

#include "CoreMinimal.h"
#include "Character/Skill//GS_SkillComp.h"
#include "Blueprint/UserWidget.h"
#include "GS_SkillWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;
class AGS_Player;

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
	void OnSkillCoolTimeChanged(ESkillSlot InSkillSlot, float InCurrentCoolTime) const;
	
	FORCEINLINE AGS_Player* GetOwningActor()const { return OwningCharacter; }
	FORCEINLINE ESkillSlot GetSkillSlot() const {return SkillSlot;}
	
	void SetOwningActor(AGS_Player* InOwningCharacter)
	{
		OwningCharacter = InOwningCharacter;
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentCoolTimeText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> CoolTimeBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UImage> SkillImage;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget,AllowPrivateAccess))
	ESkillSlot SkillSlot;

	UPROPERTY()
	TObjectPtr<AGS_Player> OwningCharacter;
};
