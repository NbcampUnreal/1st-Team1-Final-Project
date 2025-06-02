#include "UI/Popup/GS_ExitPopup.h"
#include "CommonButtonBase.h"
#include "Kismet/KismetSystemLibrary.h"

void UGS_ExitPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (YesButton)
	{
		if (UCommonButtonBase* YesButtonBase = Cast<UCommonButtonBase>(YesButton))
		{
			YesButtonBase->OnClicked().AddUObject(this, &UGS_ExitPopup::OnYesButtonClicked);
		}
	}

	if (NoButton)
	{
		if (UCommonButtonBase* NoButtonBase = Cast<UCommonButtonBase>(NoButton))
		{
			NoButtonBase->OnClicked().AddUObject(this, &UGS_ExitPopup::OnNoButtonClicked);
		}
	}
}

void UGS_ExitPopup::OnYesButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(),EQuitPreference::Quit,false);
	SetVisibility(ESlateVisibility::Hidden);
}

void UGS_ExitPopup::OnNoButtonClicked()
{
	SetVisibility(ESlateVisibility::Hidden);
}
