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
	//if (bIsInitialized)
	//{
	//	//후에 가디언 종류 바꿀 떄마다 Initialize 제대로 되는지 확인 필요
	//	UE_LOG(LogTemp, Warning, TEXT("[Nectar] Already Initialized. Can only be initialized once"));
	//	return;
	//}
	MaxAmount = Amount;
	//bIsInitialized = true;
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
	CurrentAmount += Amount; //FMath::Clamp(CurrentAmount + Amount, 0.0f, MaxAmount);
	UE_LOG(LogTemp, Warning, TEXT("[add] Amount : %.2f"), Amount);
	UE_LOG(LogTemp, Warning, TEXT("Current Amount : %.2f"), CurrentAmount);
}

void UGS_ResourceBaseComp::SpendResource(float Amount)
{
	CurrentAmount -= Amount; //FMath::Clamp(CurrentAmount - Amount, 0.0f, MaxAmount);
	UE_LOG(LogTemp, Warning, TEXT("Current Amount : %.2f"), CurrentAmount);

}

//bool UGS_ResourceBaseComp::CanSpend(float Amount) const
//{
//	if (CurrentAmount - Amount < 0)
//	{
//		return false;
//	}
//	else
//	{
//		return true;
//	}
//}

bool UGS_ResourceBaseComp::IsResourceInBound(float Amount, bool bIsSpending) const
{
	//계산 결과가 범위 내라면 true 반환 아닌 경우 false 반환
	//전자는 소모 했을 때 0 이하인지 확인해야하는 경우 
	//후자는 얻었을 때 MaxAmount를 넘는지 확인해야하는 경우
	return bIsSpending
		? (CurrentAmount - Amount >= 0)
		: (CurrentAmount + Amount <= MaxAmount);
}
