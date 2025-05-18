#include "UI/GS_CustomLobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "System/GS_PlayerState.h"


void UGS_CustomLobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (JobSelectionButton) JobSelectionButton->OnClicked.AddDynamic(this, &UGS_CustomLobbyUI::OnJobSelectionButtonClicked);
	if (PerkOrDungeonButton) PerkOrDungeonButton->OnClicked.AddDynamic(this, &UGS_CustomLobbyUI::OnPerkOrDungeonButtonClicked);
	if (ReadyButton) ReadyButton->OnClicked.AddDynamic(this, &UGS_CustomLobbyUI::OnReadyButtonClicked);
	if (RoleChangeButton) RoleChangeButton->OnClicked.AddDynamic(this, &UGS_CustomLobbyUI::OnRoleChangeButtonClicked);

	if (PerkDungeonText) UpdateRoleSpecificText(EPlayerRole::PR_None);
}

void UGS_CustomLobbyUI::OnJobSelectionButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestOpenJobSelectionPopup();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnPerkOrDungeonButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestOpenPerkOrDungeonPopup();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnReadyButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestToggleReadyStatus();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnRoleChangeButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestToggleRole();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::UpdateRoleSpecificText(EPlayerRole NewRole)
{
	if (PerkDungeonText)
	{
		FText NewTextToShow;
		switch (NewRole)
		{
		case EPlayerRole::PR_Seeker:
			NewTextToShow = FText::FromString(TEXT("Perk (Seeker)"));
			break;
		case EPlayerRole::PR_Guardian:
			NewTextToShow = FText::FromString(TEXT("Dungeon (Guardian)"));
			break;
		case EPlayerRole::PR_None:
		default:
			break;
		}
		PerkDungeonText->SetText(NewTextToShow);
		UE_LOG(LogTemp, Log, TEXT("UGS_CustomLobbyUI: PerkDungeonText updated for Role: %s"), *UEnum::GetValueAsString(NewRole));
	}
}

void UGS_CustomLobbyUI::UpdateReadyButtonText(bool bIsReady)
{
	if (ReadyText)
	{
		FText NewTextToShow = bIsReady ? FText::FromString(TEXT("Cancel")) : FText::FromString(TEXT("Ready"));
		ReadyText->SetText(NewTextToShow);
		UE_LOG(LogTemp, Log, TEXT("UGS_CustomLobbyUI: ReadyButton text updated to: %s"), *NewTextToShow.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UGS_CustomLobbyUI: ReadyText is not bound in Blueprint!"));
	}
}
