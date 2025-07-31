#include "ResourceSystem/GS_ResourceBaseComp.h"


UGS_ResourceBaseComp::UGS_ResourceBaseComp()
{

	PrimaryComponentTick.bCanEverTick = false;

}


void UGS_ResourceBaseComp::BeginPlay()
{
	Super::BeginPlay();

}

void UGS_ResourceBaseComp::InitializeMaxAmount(float Amount)
{
	if (bIsInitialized)
	{
		//후에 가디언 종류 바꿀 떄마다 Initialize 제대로 되는지 확인 필요
		UE_LOG(LogTemp, Warning, TEXT("[Nectar] Already Initialized. Can only be initialized once"));
		return;
	}

	MaxAmount = Amount;
	CurrentAmount = MaxAmount;
	bIsInitialized = true;
}

float UGS_ResourceBaseComp::GetCurrentAmount() const
{
	return CurrentAmount;
}

float UGS_ResourceBaseComp::GetMaxAmount() const
{
	return MaxAmount;
}




void UGS_ResourceBaseComp::AddResource(float Amount)
{
	CurrentAmount = FMath::Clamp(CurrentAmount + Amount, 0.0f, MaxAmount);
	UE_LOG(LogTemp, Warning, TEXT("[add] Amount : %.2f"), Amount);
	UE_LOG(LogTemp, Warning, TEXT("Current Amount : %.2f"), CurrentAmount);
}

void UGS_ResourceBaseComp::SpendResource(float Amount)
{
	CurrentAmount = FMath::Clamp(CurrentAmount - Amount, 0.0f, MaxAmount);
	UE_LOG(LogTemp, Warning, TEXT("Current Amount : %.2f"), CurrentAmount);

}

bool UGS_ResourceBaseComp::CanAfford(float Amount) const
{
	if (CurrentAmount - Amount < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
