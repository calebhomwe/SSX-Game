#include "SSXPureGameMode.h"
#include "SSXPureSnowboarder.h"
#include "SSXPurePlayerController.h"
#include "SSXPureHUD.h"
#include "SSXPureTerrain.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"

ASSXPureGameMode::ASSXPureGameMode()
{
	DefaultPawnClass = ASSXPureSnowboarder::StaticClass();
	PlayerControllerClass = ASSXPurePlayerController::StaticClass();
	HUDClass = ASSXPureHUD::StaticClass();
}

void ASSXPureGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ASSXPureGameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnWorldActors();

	// Headless visual-verification hook: UnrealEditor-Cmd.exe <proj> -game -ShotTest
	if (FParse::Param(FCommandLine::Get(), TEXT("ShotTest")))
	{
		GetWorldTimerManager().SetTimer(ShotTimerHandle, this, &ASSXPureGameMode::TakeShot, 6.0f, false);
		GetWorldTimerManager().SetTimer(QuitTimerHandle, this, &ASSXPureGameMode::QuitAfterShot, 18.0f, false);
	}
}

void ASSXPureGameMode::SpawnWorldActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Procedural sky: physical atmosphere, then sky light captured from it.
	ASkyAtmosphere* Atmosphere = World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, Params);

	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0, 0, 2000), FRotator(-45, -30, 0), Params);
	if (Sun)
	{
		Sun->GetLightComponent()->SetIntensity(3.2f);
		Sun->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.96f, 0.9f));
	}

	ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (Sky)
	{
		Sky->GetLightComponent()->SetIntensity(0.8f);
		Sky->GetLightComponent()->RecaptureSky();
	}

	// Distance haze so the mountain reads as huge.
	AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (Fog)
	{
		Fog->GetComponent()->FogDensity = 0.0008f;
		Fog->GetComponent()->FogInscatteringColor = FLinearColor(0.85f, 0.9f, 1.0f);
	}

	World->SpawnActor<ASSXPureTerrain>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
}

void ASSXPureGameMode::TakeShot()
{
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("HighResShot 2560x1440"));
	}
}

void ASSXPureGameMode::QuitAfterShot()
{
	FPlatformMisc::RequestExit(false);
}
