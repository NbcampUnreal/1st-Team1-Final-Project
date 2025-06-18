#include "Props/Trap/NonTriggerTrap/GS_BearTrap.h"
#include "Character/Player/Seeker/GS_Seeker.h"




void AGS_BearTrap::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnDamageBoxOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex,
								bFromSweep, SweepResult);

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor))
	{
		FVector SeekerLocation = Seeker->GetActorLocation();
		FVector TrapLocation = GetActorLocation();

		FVector NewLocation = FVector(TrapLocation.X, TrapLocation.Y, SeekerLocation.Z);
		Seeker->SetActorLocation(NewLocation);
	}


}