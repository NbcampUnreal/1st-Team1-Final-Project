#include "UI/Screen/GS_MainMenuUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "System/PlayerController/GS_MainMenuPC.h"

void UGS_MainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton) PlayButton->OnClicked.AddDynamic(this, &UGS_MainMenuUI::OnPlayButtonClicked);
	else UE_LOG(LogTemp, Error, TEXT("UGS_MainMenuUI: PlayButton is not bound in Blueprint!"));

	if (CustomGameButton) CustomGameButton->OnClicked.AddDynamic(this, &UGS_MainMenuUI::OnCustomGameButtonClicked);
	else UE_LOG(LogTemp, Error, TEXT("UGS_MainMenuUI: CustomGameButton is not bound in Blueprint!"));

	if (ExitButton) ExitButton->OnClicked.AddDynamic(this, &UGS_MainMenuUI::OnExitButtonClicked);
	else UE_LOG(LogTemp, Error, TEXT("UGS_MainMenuUI: ExitButton is not bound in Blueprint!"));


	//텍스트 초기 설정
	if (PlayText) PlayText->SetText(FText::FromString(TEXT("Play")));

	//초기 visibility 설정
	if (SeekerButton) SeekerButton->SetVisibility(ESlateVisibility::Hidden);
	if (GuardianButton) GuardianButton->SetVisibility(ESlateVisibility::Hidden);
	if (CustomGameButton) CustomGameButton->SetVisibility(ESlateVisibility::Hidden);
}

void UGS_MainMenuUI::OnPlayButtonClicked()
{
	if (!bIsPlayButtonClicked)
	{
		if (SeekerButton) SeekerButton->SetVisibility(ESlateVisibility::Visible);
		if (GuardianButton) GuardianButton->SetVisibility(ESlateVisibility::Visible);
		if (CustomGameButton) CustomGameButton->SetVisibility(ESlateVisibility::Visible);
		if (PlayText) PlayText->SetText(FText::FromString(TEXT("Close")));
		bIsPlayButtonClicked = true;
	}
	else
	{
		if (SeekerButton) SeekerButton->SetVisibility(ESlateVisibility::Hidden);
		if (GuardianButton) GuardianButton->SetVisibility(ESlateVisibility::Hidden);
		if (CustomGameButton) CustomGameButton->SetVisibility(ESlateVisibility::Hidden);
		if (PlayText) PlayText->SetText(FText::FromString(TEXT("Play")));
		bIsPlayButtonClicked = false;
	}
	
}

void UGS_MainMenuUI::OnExitButtonClicked()
{
	//캔버스 패널 안에 띄우기
}

void UGS_MainMenuUI::OnCustomGameButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UGS_MainMenuUI: OnCustomGameButtonClicked() CALLED."));
	AGS_MainMenuPC* PC = GetOwningPlayer<AGS_MainMenuPC>();
	if (PC)
	{
		UE_LOG(LogTemp, Log, TEXT("UGS_MainMenuUI: Owning PlayerController is VALID and cast to AGS_MainMenuPC. Calling HandleCustomGameRequest..."));
		PC->HandleCustomGameRequest();
	}
}

