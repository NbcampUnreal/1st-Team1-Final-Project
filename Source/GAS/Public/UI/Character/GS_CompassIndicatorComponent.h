// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/GS_PlayerRole.h"
#include "GS_CompassIndicatorComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class GAS_API UGS_CompassIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_CompassIndicatorComponent();

	// Blueprint Callable Functions
	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	FVector GetWorldLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	bool IsValidForCompass() const;

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	ESeekerJob GetSeekerJob() const;

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	FString GetPlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	bool IsPlayerAlive() const;

	// Static function to get all compass indicators in the world
	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	static TArray<UGS_CompassIndicatorComponent*> GetAllCompassIndicators(const UObject* WorldContext);

protected:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass Indicator|Display")
	bool bShowOnCompass = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass Indicator|Display", meta=(Tooltip="If the owner is a Player, this will check the PlayerState's alive status. For non-players like monsters, disable this."))
	bool bCheckPlayerStatus = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Compass Indicator|State")
	bool bIsManuallyHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass Indicator|Cosmetic")
	UTexture2D* CustomIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass Indicator|Cosmetic")
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass Indicator|Range")
	float MaxDisplayDistance = 10000.0f;

public:
	// Use this function from Blueprint (e.g., on monster death) to hide the icon.
	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	void SetIndicatorHidden(bool bHidden) { bIsManuallyHidden = bHidden; }

	// Getter/Setter functions for Blueprint
	UFUNCTION(BlueprintPure, Category = "Compass Indicator")
	bool GetShowOnCompass() const { return bShowOnCompass; }

	UFUNCTION(BlueprintPure, Category = "Compass Indicator")
	float GetMaxDisplayDistance() const { return MaxDisplayDistance; }

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	void SetShowOnCompass(bool bShow) { bShowOnCompass = bShow; }

	UFUNCTION(BlueprintPure, Category = "Compass Indicator")
	UTexture2D* GetCustomIcon() const { return CustomIcon; }

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	void SetCustomIcon(UTexture2D* Icon) { CustomIcon = Icon; }

	UFUNCTION(BlueprintPure, Category = "Compass Indicator")
	FLinearColor GetIconColor() const { return IconColor; }

	UFUNCTION(BlueprintCallable, Category = "Compass Indicator")
	void SetIconColor(FLinearColor Color) { IconColor = Color; }
}; 