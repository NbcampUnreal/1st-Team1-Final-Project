#include "ResourceSystem/Nectar/GS_NectarComp.h"


//void UGS_NectarComp::InitializeMaxAmount(float Amount)
//{
//	Super::InitializeMaxAmount(Amount);
//	OnNectarChanged.Broadcast(GetCurrentAmount());
//}


void UGS_NectarComp::AddResource(float Amount)
{
	Super::AddResource(Amount);
	OnNectarChanged.Broadcast(GetCurrentAmount());

}

void UGS_NectarComp::SpendResource(float Amount)
{
	Super::SpendResource(Amount);
	OnNectarChanged.Broadcast(GetCurrentAmount());
}

//bool UGS_NectarComp::CanAfford(float Amount) const
//{
//	return false;
//}
