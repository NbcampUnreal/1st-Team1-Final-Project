#include "Props/Interactables/GS_Door.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Net/UnrealNetwork.h"


AGS_Door::AGS_Door()
{
	bReplicates = true;
	SetReplicateMovement(true);

	RootSceneComp = CreateDefaultSubobject<UBoxComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;

	DoorFrameMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMeshComp"));
	DoorFrameMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorFrameMeshComp->SetCollisionObjectType(ECC_WorldStatic);
	DoorFrameMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	DoorFrameMeshComp->SetupAttachment(RootComponent);

	DoorMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComp"));
	DoorMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMeshComp->SetCollisionObjectType(ECC_WorldStatic);
	DoorMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	DoorMeshComp->SetupAttachment(DoorFrameMeshComp);


	TriggerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	//TriggerBoxComp->SetCollisionObjectType(ECC_GameTraceChannel4);
	TriggerBoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBoxComp->SetupAttachment(DoorFrameMeshComp);
	TriggerBoxComp->SetGenerateOverlapEvents(true);
}


void AGS_Door::BeginPlay()
{
	Super::BeginPlay();

	InitDoor();
	TriggerBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_Door::OnTriggerBeginOverlap);
}

void AGS_Door::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Something overlapped: %s"), *OtherActor->GetName());

	AGS_Character* Character = Cast<AGS_Character>(OtherActor);
	if (!Character || !HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Overlap but not AGS_Character: %s"), *OtherActor->GetName());
		return;
	}
	if (!bIsOpen)
	{
		bIsOpen = true;
		Server_DoorOpen(Character);
	}
	
}


void AGS_Door::Server_DoorOpen_Implementation(AActor* TargetActor)
{
	//문 열리는 함수 호출하고 
	DoorOpen();
	GetWorldTimerManager().ClearTimer(DoorCloseTimerHandle);
	GetWorldTimerManager().SetTimer(DoorCloseTimerHandle, this, &AGS_Door::CheckForPlayerInTrigger, 2.0f, false);
}

void AGS_Door::CheckForPlayerInTrigger()
{
	TArray<AActor*> OverlappingActors;
	TriggerBoxComp->GetOverlappingActors(OverlappingActors, AGS_Character::StaticClass());

	if (OverlappingActors.Num() > 0)
		//오버랩 되는 엑터가 있으면 타이머 초기화
	{
		GetWorldTimerManager().SetTimer(DoorCloseTimerHandle, this, &AGS_Door::CheckForPlayerInTrigger, 2.0f, false);
	}
	else
	{
		//오버랩 되는 엑터가 없으면 문 닫히는 함수 호출
		DoorClose();
		bIsOpen = false;
	}


}

void AGS_Door::DoorOpen_Implementation()
{

}

void AGS_Door::DoorClose_Implementation()
{

}