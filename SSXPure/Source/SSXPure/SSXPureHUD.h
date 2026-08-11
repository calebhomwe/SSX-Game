#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SSXPureHUD.generated.h"

UCLASS()
class ASSXPureHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
