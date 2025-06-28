#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GS_SteamNameWidgetComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_SteamNameWidgetComp : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void InitWidget() override;
};
