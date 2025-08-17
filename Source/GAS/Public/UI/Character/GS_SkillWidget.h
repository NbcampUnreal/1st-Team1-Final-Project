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

	UFUNCTION()
	void OnSkillActivated(ESkillSlot InSkillSlot);

	UFUNCTION()
	void OnSkillCooldownBlocked(ESkillSlot InSkillSlot);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayHeartbeatAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayGlowEffect();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayCooldownBlockedAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayRedFlashEffect();

	UFUNCTION(BlueprintCallable)
	void PlaySkillActivationSound();

	UFUNCTION(BlueprintCallable)
	void PlaySkillCooldownSound();
	
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

	// 오디오 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SkillActivationSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SkillCooldownSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bEnableAudio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AudioVolume = 0.5f;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget,AllowPrivateAccess))
	ESkillSlot SkillSlot;

	UPROPERTY()
	TObjectPtr<AGS_Player> OwningCharacter;
};
