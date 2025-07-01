#include "Props/Trap/TriggerTrap/GS_RisingSpikeTrap.h"

EHitReactType AGS_RisingSpikeTrap::GetHitReactType() const
{
	return EHitReactType::DamageOnly;
}