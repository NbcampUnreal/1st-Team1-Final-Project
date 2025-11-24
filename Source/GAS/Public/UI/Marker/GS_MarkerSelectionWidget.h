#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/GS_MarkerTypes.h"
#include "GS_MarkerSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkerSelected, EMarkerType, SelectedType);

UCLASS()
class GAS_API UGS_MarkerSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Marker")
	FOnMarkerSelected OnMarkerSelected;

	UFUNCTION(BlueprintCallable, Category = "Marker")
	void SelectMarker(EMarkerType MarkerType);
};

