// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_CrossHairImage.generated.h"

class UImage;
class AGS_Character;

UCLASS()
class GAS_API UGS_CrossHairImage : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetOwingActor(AGS_Character* InOwningCharacter);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> CrossHairImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UTexture2D> CrossHairTexture;
	
};
