#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SSXPureTerrain.generated.h"

UCLASS()
class ASSXPureTerrain : public AActor
{
	GENERATED_BODY()

public:
	ASSXPureTerrain();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void GenerateChunk(FVector Center);
	float GetNoiseHeight(float X, float Y) const;
	UMaterialInterface* CreateSnowMaterial();

	UPROPERTY()
	UProceduralMeshComponent* TerrainMesh;

	FVector LastChunkCenter;
	float ChunkSize = 5000.f;
	int32 Resolution = 100;
};
