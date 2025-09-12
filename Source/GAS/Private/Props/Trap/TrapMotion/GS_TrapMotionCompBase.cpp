#include "Props/Trap/TrapMotion/GS_TrapMotionCompBase.h"
#include "Net/UnrealNetwork.h"

UGS_TrapMotionCompBase::UGS_TrapMotionCompBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGS_TrapMotionCompBase::BeginPlay()
{
	Super::BeginPlay();

}


void UGS_TrapMotionCompBase::StartMotion()
{
	bIsMoving = true;
	if (bReplicateMotion && GetOwnerRole() == ROLE_Authority)
	{
		BeginTrapMotion();
	}
}

void UGS_TrapMotionCompBase::StopMotion()
{
	bIsMoving = false;
	if (bReplicateMotion && GetOwnerRole() == ROLE_Authority)
	{
		EndTrapMotion();
	}
}


void UGS_TrapMotionCompBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGS_TrapMotionCompBase, bIsMoving);
	DOREPLIFETIME(UGS_TrapMotionCompBase, bReplicateMotion);
}


void UGS_TrapMotionCompBase::BeginTrapMotion()
{

}

void UGS_TrapMotionCompBase::EndTrapMotion()
{

}

void UGS_TrapMotionCompBase::ClientLerpUpdate()
{

}