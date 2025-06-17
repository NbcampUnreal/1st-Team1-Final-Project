#include "Props/Trap/TrapMotion/GS_LoopingTrapMotionComp.h"
#include "TimerManager.h"


UGS_LoopingTrapMotionComp::UGS_LoopingTrapMotionComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGS_LoopingTrapMotionComp::BeginPlay()
{
	Super::BeginPlay();

	if (TargetComponent)
	{
		InitialLocation = TargetComponent->GetRelativeLocation();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[LoopingTrapMotionComp] TargetComponent is NULL in %s"), *GetOwner()->GetName());
	}
}



void UGS_LoopingTrapMotionComp::BeginTrapMotion()
{
	if (!TargetComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer_NoTargetComponent"));
		return;
	}

	switch (TrapType)
	{
	case ELoopingTrapType::SwingBladeTrap:
	{
		DoSwingBladeMotion();
		break;
	}
	case ELoopingTrapType::SawTrap:
	{
		//UE_LOG(LogTemp, Warning, TEXT("Timer_SawTrap worked till case"));
		DoSawMotion();
		break;
	}
	}

}

//void UGS_LoopingTrapMotionComp::EndTrapMotion()
//{
//	GetWorld()->GetTimerManager().ClearTimer(LoopTimerHandle);
//}

//void UGS_LoopingTrapMotionComp::TimerMotion()
//{
//
//}


void UGS_LoopingTrapMotionComp::DoSwingBladeMotion()
{
	AccumulatedTime += MotionInterval;
	float Angle = FMath::Sin(AccumulatedTime) * RotationSpeed;
	TargetComponent->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
}

void UGS_LoopingTrapMotionComp::DoSawMotion()
{
	FRotator CurrentRotation = TargetComponent->GetRelativeRotation();
	if (!TargetComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("TargetComponent is NULL!"));
	}
	

	AccumulatedTime += MotionInterval;

	float MoveCycleTime = 2.0f;
	float CycleProgress = FMath::Fmod(AccumulatedTime, MoveCycleTime);

	static float LastCycleProgress = 0.f;
	if (CycleProgress < 0.01f && LastCycleProgress >(MoveCycleTime - 0.01f))
	{
		CurrentDirection *= -1;
	}
	LastCycleProgress = CycleProgress;

	//회전
	FRotator DeltaRot = FRotator(RotationSpeed * MotionInterval * CurrentDirection * -1, 0.f, 0.f);
	TargetComponent->AddLocalRotation(DeltaRot);
	FRotator NewRotation = TargetComponent->GetRelativeRotation();
	//좌우 이동

	float Frequency = 2 * PI / MoveCycleTime;
	float XOffset = MoveDistance * FMath::Sin(AccumulatedTime * Frequency);

	FVector NewLocation = InitialLocation;
	NewLocation.X += XOffset;
	TargetComponent->SetRelativeLocation(NewLocation);

	Multicast_UpdateTransform(NewLocation, NewRotation);
}


void UGS_LoopingTrapMotionComp::Multicast_UpdateTransform_Implementation(FVector Loc, FRotator Rot)
{
	UE_LOG(LogTemp, Warning, TEXT("[Multicast] Loc: %s, Rot: %s"), *Loc.ToString(), *Rot.ToString());
	if (!TargetComponent)
	{
		return;
	}
	CurrentLoc = TargetComponent->GetRelativeLocation();
	TargetLoc = Loc;

	CurrentRot = TargetComponent->GetRelativeRotation();
	TargetRot = Rot;

	InterpAlpha = 0.f;

}



void UGS_LoopingTrapMotionComp::ClientLerpUpdate()
{
	if (!TargetComponent)
	{
		return;
	}

	InterpAlpha += (MotionInterval / InterpStep);
	InterpAlpha = FMath::Clamp(InterpAlpha, 0.f, 1.f);

	FVector NewLoc = FMath::Lerp(CurrentLoc, TargetLoc, InterpAlpha);
	FRotator NewRot = FMath::Lerp(CurrentRot, TargetRot, InterpAlpha);

	TargetComponent->SetRelativeLocation(NewLoc);
	TargetComponent->SetRelativeRotation(NewRot);
	//UE_LOG(LogTemp, Warning, TEXT("[LerpUpdate] CurrentLoc: %s, TargetLoc: %s, Alpha: %f"), *CurrentLoc.ToString(), *TargetLoc.ToString(), InterpAlpha);
}