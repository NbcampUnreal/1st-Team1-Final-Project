#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"

AGS_NonTrigTrapBase::AGS_NonTrigTrapBase()
{
	/*ActivateSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("ActivateSphereComp"));
	ActivateSphereComp->SetupAttachment(MeshParentSceneComp);
	ActivateSphereComp->PrimaryComponentTick.bCanEverTick = false;
	ActivateSphereComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	ActivateSphereComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	ActivateSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivateSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivateSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);*/
}

//void AGS_NonTrigTrapBase::BeginPlay()
//{
//	Super::BeginPlay();
//	//ActivateSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_NonTrigTrapBase::OnActivSCompBeginOverlap);
//}

////Sphere Comp에 Begin Overlap 시,
//void AGS_NonTrigTrapBase::OnActivSCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
//	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
//	bool bFromSweep, const FHitResult& SweepResult)
//{
//	if (OtherActor && OtherActor != this)
//	{
//		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
//		if (Seeker && !bIsActivated)
//		{
//			bIsActivated = true;
//			if (!HasAuthority())
//			{
//				Server_ActivateTrap(OtherActor);
//			}
//			else
//			{
//				ActivateTrap(OtherActor);
//			}
//
//			if (!GetWorld()->GetTimerManager().IsTimerActive(CheckOverlapTimerHandle))
//			{
//				StartDeactivateTrapCheck();
//			}
//		}
//	}
//}
//void AGS_NonTrigTrapBase::Server_ActivateTrap_Implementation(AActor* TargetActor)
//{
//	ActivateTrap(TargetActor);
//}
//
//
//void AGS_NonTrigTrapBase::ActivateTrap_Implementation(AActor* TargetActor)
//{
//
//}
//
//
//
//
////Sphere Comp에 End Overlap 시,
//
//void AGS_NonTrigTrapBase::StartDeactivateTrapCheck()
//{
//	GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AGS_NonTrigTrapBase::CheckOverlappingSeeker, 10.0f, true);
//}
//
//void AGS_NonTrigTrapBase::CheckOverlappingSeeker()
//{
//	TArray<AActor*> OverlappingActors;
//	ActivateSphereComp->GetOverlappingActors(OverlappingActors, AGS_Seeker::StaticClass());
//
//	if (OverlappingActors.Num() == 0)
//	{
//		DeActivateTrap();
//		GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
//		bIsActivated = false;
//	}
//}



//void AGS_NonTrigTrapBase::DeActivateTrap_Implementation()
//{
//
//}



//bool AGS_NonTrigTrapBase::CanStartMotion() const
//{
//	//UE_LOG(LogTemp, Warning, TEXT("[CanStartMotion] bIsActivated: %s"), bIsActivated ? TEXT("true") : TEXT("false"));
//	return bIsActivated;
//}

//bool AGS_NonTrigTrapBase::CanStopMotion() const
//{//false가 되면 제거 
//	return !bIsActivated;
//}
