#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GS_HPTextWidgetComp.generated.h"

UCLASS()
class GAS_API UGS_HPTextWidgetComp : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void InitWidget() override;

};
