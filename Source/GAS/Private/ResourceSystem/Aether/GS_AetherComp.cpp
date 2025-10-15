#include "ResourceSystem/Aether/GS_AetherComp.h"
#include "Net/UnrealNetwork.h"

UGS_AetherComp::UGS_AetherComp()
{
	SetIsReplicatedByDefault(true);
}

void UGS_AetherComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGS_AetherComp, ReplicatedAetherAmount);
}

void UGS_AetherComp::InitializeMaxAmount(float Amount)
{
	Super::InitializeMaxAmount(Amount);
	CurrentAmount = 0.0f;
	//UE_LOG(LogTemp, Error, TEXT("[Aether] InitializeMaxAmount called : %f"), CurrentAmount);
	SetCurrentAether(GetCurrentAmount());
}

void UGS_AetherComp::AddResource(float Amount)
{
	//UE_LOG(LogTemp, Error, TEXT("[UGS_AetherComp] AddResource %f"), Amount);
	//여기에서 초과한 경우 0.5를 곱하도록 
	float ActualAmount = CanAddResource(Amount) ? Amount : Amount * 0.5f;
	Super::AddResource(ActualAmount);
	SetCurrentAether(GetCurrentAmount());
}

bool UGS_AetherComp::CanAddResource(float Amount) const
{
	return IsResourceInBound(Amount, false);
}


//void UGS_AetherComp::SpendResource(float Amount)
//{
//}
//

void UGS_AetherComp::SetCurrentAether(float NewValue)
{//서버만 실행됨
	if (GetOwnerRole() == ROLE_Authority)
	{
		/*UE_LOG(LogTemp, Error, TEXT("[SetCurrentAether] 서버 실행 - NewValue: %f, 기존 ReplicatedAetherAmount: %f"),
			NewValue,
			ReplicatedAetherAmount);*/
		ReplicatedAetherAmount = NewValue;
	}
}



void UGS_AetherComp::OnRep_ReplicatedAmount()
{
	//UE_LOG(LogTemp, Error, TEXT("[OnRep_ReplicatedAmount] exist"));
	OnAetherChanged.Broadcast(ReplicatedAetherAmount);
}
