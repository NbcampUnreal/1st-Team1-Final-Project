#include "UI/Character/GS_HPTextWidgetComp.h"

#include "UI/Character/GS_HPText.h"

void UGS_HPTextWidgetComp::InitWidget()
{
	Super::InitWidget();

	UGS_HPText* HPText = Cast<UGS_HPText>(GetWidget());
	if (IsValid(HPText))
	{
		HPText->SetOwningActor(GetOwner());
	}
}
