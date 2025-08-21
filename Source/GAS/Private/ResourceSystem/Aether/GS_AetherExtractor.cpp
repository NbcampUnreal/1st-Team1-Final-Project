#include "ResourceSystem/Aether/GS_AetherExtractor.h"


AGS_AetherExtractor::AGS_AetherExtractor()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AGS_AetherExtractor::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(
		DelayHandle,
		this,
		&ThisClass::InitializeAetherComp,
		1.5f,  // 딜레이 후 초기화 시도
		false
	);


	//CachedAetherComp = FindGuardianAetherComp();

	//GetWorld()->GetTimerManager().SetTimer(
	//	AetherExtractTimerHandle,
	//	this,
	//	&ThisClass::ExtractAether,
	//	ExtractionInterval,
	//	true
	//);

}

UGS_AetherComp* AGS_AetherExtractor::FindGuardianAetherComp()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController is null"));
			continue;
		}

		AGS_PlayerState* PS = Cast<AGS_PlayerState>(PC->PlayerState);
		/*if (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("AetherComp found"));
			return PS->GetAetherComp();
		}*/

		if (!PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState is not GS_PlayerState"));
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("Current PlayerRole: %s"),
			PS ? *UEnum::GetValueAsString(PS->CurrentPlayerRole) : TEXT("No GS_PlayerState"));


		if (PS->CurrentPlayerRole != EPlayerRole::PR_Guardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerRole is not Guardian"));
			continue;
		}

		if (!PS->GetAetherComp())
		{
			UE_LOG(LogTemp, Warning, TEXT("AetherComp is null on Guardian PlayerState"));
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("AetherComp found successfully!"));
		return PS->GetAetherComp();
	}
	UE_LOG(LogTemp, Warning, TEXT("AetherComp not found"));
	return nullptr;
}


void AGS_AetherExtractor::InitializeAetherComp()
{

	CachedAetherComp = FindGuardianAetherComp();

	GetWorld()->GetTimerManager().SetTimer(
		AetherExtractTimerHandle,
		this,
		&ThisClass::ExtractAether,
		ExtractionInterval,
		true
	);
}



void AGS_AetherExtractor::ExtractAether()
{
	if (IsValid(CachedAetherComp))
	{
		CachedAetherComp->AddResource(ExtractionAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AetherComp in PlayerState is lost"));
	}
}
