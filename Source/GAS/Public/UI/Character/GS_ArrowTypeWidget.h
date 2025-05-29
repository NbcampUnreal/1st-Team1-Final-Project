// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "GS_ArrowTypeWidget.generated.h"

class UTextBlock;
class UImage;
class AGS_Character;

UCLASS()
class GAS_API UGS_ArrowTypeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void SetOwningActor(AGS_Character* InOwningCharacter);

	void UpdateArrowImage(EArrowType InArrowType);
	void UpdateArrowCount(int32 InArrowCount);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> ArrowImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ArrowNumText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UTexture2D> NormalArrowTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UTexture2D> AxeArrowTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UTexture2D> ChildArrowTexture;
	
};
