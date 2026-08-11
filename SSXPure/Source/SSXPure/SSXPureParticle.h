#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SSXPureParticle.generated.h"

UCLASS()
class ASSXPureParticle : public AActor
{
	GENERATED_BODY()

public:
	ASSXPureParticle();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UProceduralMeshComponent* Mesh;

	float Life = 0.5f;
	float MaxLife = 0.5f;
	FVector Vel;
};
