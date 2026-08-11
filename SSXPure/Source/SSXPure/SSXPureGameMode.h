#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SSXPureGameMode.generated.h"

UCLASS()
class ASSXPureGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASSXPureGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;

	/** Spawns the entire world: lights, sky, fog, terrain. Pure code, zero assets. */
	void SpawnWorldActors();

	/** Headless verification hook: -ShotTest on the command line captures a
	    real-RHI screenshot after a few seconds of simulated play, then quits. */
	void TakeShot();
	void QuitAfterShot();

private:
	FTimerHandle ShotTimerHandle;
	FTimerHandle QuitTimerHandle;
};
