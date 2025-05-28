#include "Props/Trap/NonTriggerTrap/GS_LavaTrap.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/DamageEvents.h"

AGS_LavaTrap::AGS_LavaTrap()
{
}

void AGS_LavaTrap::HandleTrapDamage(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Player damaged"));
	if (!OtherActor) return;
	AGS_Seeker* DamagedSeeker = Cast<AGS_Seeker>(OtherActor);
	if (!DamagedSeeker) return;
	if (TrapData.Effect.Damage <= 0.f) return;

	//기본 데미지 부여
	FDamageEvent DamageEvent;
	DamagedSeeker->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);

	//디버프 추가
}