#include "ResourceSystem/Aether/GS_AetherComp.h"

void UGS_AetherComp::AddResource(float Amount)
{
	//여기에서 초과한 경우 0.5를 곱하도록 
	float ActualAmount = CanAddResource(Amount) ? Amount * 0.5f : Amount;
	Super::AddResource(ActualAmount);
}

bool UGS_AetherComp::CanAddResource(float Amount) const
{
	return IsResourceInBound(Amount, false);
}


//void UGS_AetherComp::SpendResource(float Amount)
//{
//}
//

