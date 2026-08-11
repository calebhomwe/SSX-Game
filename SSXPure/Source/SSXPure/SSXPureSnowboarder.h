#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProceduralMeshComponent.h"
#include "SSXPureSnowboarder.generated.h"

UCLASS()
class ASSXPureSnowboarder : public APawn
{
	GENERATED_BODY()

public:
	ASSXPureSnowboarder();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PIC) override;

	UFUNCTION(BlueprintCallable)
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable)
	int32 GetScore() const { return TotalScore; }

	UFUNCTION(BlueprintCallable)
	bool IsDisplayingTrick() const { return TrickDisplayTimer > 0.f; }

	UFUNCTION(BlueprintCallable)
	FString GetCurrentTrickName() const { return CurrentTrickName; }

	UFUNCTION(BlueprintCallable)
	bool GetIsAirborne() const { return bIsAirborne; }

protected:
	virtual void BeginPlay() override;

	void BuildBox(UProceduralMeshComponent* M, FVector Size, FLinearColor Color);
	UMaterialInterface* CreateBodyMaterial(FLinearColor Color);

	void MoveForward(float V);
	void MoveRight(float V);
	void DoJump();
	void SpinL();
	void SpinR();

	void UpdatePhysics(float DT);
	void UpdateTricks(float DT);
	void UpdateCamera(float DT);
	void SpawnSpray();

	UPROPERTY()
	UProceduralMeshComponent* BoardMesh;

	UPROPERTY()
	UProceduralMeshComponent* BodyMesh;

	UPROPERTY()
	UProceduralMeshComponent* HeadMesh;

	UPROPERTY()
	class USpringArmComponent* SpringArm;

	UPROPERTY()
	class UCameraComponent* Camera;

	float InputFwd = 0.f;
	float InputRight = 0.f;
	bool bJumpPressed = false;
	bool bIsAirborne = false;
	float AirTime = 0.f;
	float BoardSpeed = 0.f;
	float SprayTimer = 0.f;
	FVector GroundNormal = FVector::UpVector;
	FVector LastRot;

	float TotalSpin = 0.f;
	float TotalFlip = 0.f;
	float TrickDisplayTimer = 0.f;
	float ComboMult = 1.f;
	int32 TotalScore = 0;
	FString CurrentTrickName;
};
