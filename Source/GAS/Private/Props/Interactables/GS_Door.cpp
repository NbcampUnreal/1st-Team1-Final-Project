#include "Props/Interactables/GS_Door.h"
#include "Character/Player/Seeker/GS_Seeker.h"

AGS_Door::AGS_Door()
{
	bReplicates = true;
	SetReplicateMovement(true);

	PrimaryActorTick.bCanEverTick = false;

	RootSceneComp = CreateDefaultSubobject<UBoxComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;
	
	TriggerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBoxComp->SetCollisionObjectType(ECC_GameTraceChannel4);
	TriggerBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);



}


//나중에 seeker만 인식하도록 바꾸기



void AGS_Door::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (!Seeker || !HasAuthority())
	{
		return;
	}
	Server_DoorOpen(Seeker);
}


void AGS_Door::Server_DoorOpen_Implementation(AActor* TargetActor)
{

}