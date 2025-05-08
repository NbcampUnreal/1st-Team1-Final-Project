#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Guardian.generated.h"


UCLASS()
class GAS_API AGS_Guardian : public AGS_Player
{
	GENERATED_BODY()
	
public:
	AGS_Guardian();

	void TestMeleeAttack();

protected:
	virtual void BeginPlay() override;

	//for fever mode
	float FeverTime;
	float FeverGage;


private:
};
