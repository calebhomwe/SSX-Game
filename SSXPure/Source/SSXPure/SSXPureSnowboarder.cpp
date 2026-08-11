#include "SSXPureSnowboarder.h"
#include "SSXPureParticle.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Kismet/GameplayStatics.h"

ASSXPureSnowboarder::ASSXPureSnowboarder()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BoardMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(Root);
	BoardMesh->SetSimulatePhysics(true);
	BoardMesh->SetMassOverrideInKg(NAME_None, 70.f);
	BoardMesh->SetLinearDamping(0.2f);
	BoardMesh->SetAngularDamping(3.f);
	BoardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoardMesh->SetCollisionProfileName(TEXT("Pawn"));

	BodyMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(BoardMesh);
	HeadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BoardMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(BoardMesh);
	SpringArm->TargetArmLength = 450.f;
	SpringArm->SocketOffset = FVector(0, 0, 120);
	SpringArm->bInheritPitch = SpringArm->bInheritYaw = SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ASSXPureSnowboarder::BeginPlay()
{
	Super::BeginPlay();

	BuildBox(BoardMesh, FVector(150.f, 32.f, 6.f), FLinearColor(0.05f, 0.05f, 0.05f));
	BoardMesh->SetRelativeLocation(FVector(0, 0, 60));
	BuildBox(BodyMesh, FVector(45.f, 30.f, 95.f), FLinearColor(0.7f, 0.15f, 0.1f));
	BodyMesh->SetRelativeLocation(FVector(0, 0, 110));
	BuildBox(HeadMesh, FVector(28.f, 26.f, 28.f), FLinearColor(0.9f, 0.65f, 0.5f));
	HeadMesh->SetRelativeLocation(FVector(0, 0, 175));

	LastRot = BoardMesh->GetComponentRotation().Euler();
}

void ASSXPureSnowboarder::BuildBox(UProceduralMeshComponent* M, FVector S, FLinearColor C)
{
	TArray<FVector> V;
	TArray<int32> T;
	TArray<FVector2D> U;
	TArray<FVector> N;
	TArray<FProcMeshTangent> Tan;
	TArray<FColor> Col;

	auto AddFace = [&](FVector A, FVector B, FVector C1, FVector D, FVector Norm, FVector Tang)
	{
		int32 o = V.Num();
		V.Add(A);
		V.Add(B);
		V.Add(C1);
		V.Add(D);
		U.Add(FVector2D(0, 0));
		U.Add(FVector2D(1, 0));
		U.Add(FVector2D(0, 1));
		U.Add(FVector2D(1, 1));
		for (int32 i = 0; i < 4; i++)
		{
			N.Add(Norm);
			Tan.Add(FProcMeshTangent(Tang.X, Tang.Y, Tang.Z));
		}
		T.Add(o);
		T.Add(o + 2);
		T.Add(o + 1);
		T.Add(o + 1);
		T.Add(o + 2);
		T.Add(o + 3);
	};

	FVector h = S * 0.5f;
	AddFace(FVector(-h.X, -h.Y, -h.Z), FVector(-h.X, h.Y, -h.Z), FVector(-h.X, -h.Y, h.Z), FVector(-h.X, h.Y, h.Z), FVector(-1, 0, 0), FVector(0, 1, 0));
	AddFace(FVector(h.X, -h.Y, -h.Z), FVector(h.X, h.Y, -h.Z), FVector(h.X, -h.Y, h.Z), FVector(h.X, h.Y, h.Z), FVector(1, 0, 0), FVector(0, -1, 0));
	AddFace(FVector(-h.X, -h.Y, h.Z), FVector(h.X, -h.Y, h.Z), FVector(-h.X, h.Y, h.Z), FVector(h.X, h.Y, h.Z), FVector(0, 0, 1), FVector(1, 0, 0));
	AddFace(FVector(-h.X, -h.Y, -h.Z), FVector(h.X, -h.Y, -h.Z), FVector(-h.X, h.Y, -h.Z), FVector(h.X, h.Y, -h.Z), FVector(0, 0, -1), FVector(-1, 0, 0));
	AddFace(FVector(-h.X, -h.Y, -h.Z), FVector(h.X, -h.Y, -h.Z), FVector(-h.X, -h.Y, h.Z), FVector(h.X, -h.Y, h.Z), FVector(0, -1, 0), FVector(1, 0, 0));
	AddFace(FVector(-h.X, h.Y, -h.Z), FVector(h.X, h.Y, -h.Z), FVector(-h.X, h.Y, h.Z), FVector(h.X, h.Y, h.Z), FVector(0, 1, 0), FVector(-1, 0, 0));

	for (int32 i = 0; i < V.Num(); i++)
	{
		Col.Add(C.ToFColor(true));
	}

	M->CreateMeshSection(0, V, T, N, U, Col, Tan, true);
	M->SetMaterial(0, CreateBodyMaterial(C));
}

UMaterialInterface* ASSXPureSnowboarder::CreateBodyMaterial(FLinearColor C)
{
	UMaterial* M = NewObject<UMaterial>(this);
	M->BlendMode = BLEND_Opaque;
	M->SetShadingModel(MSM_DefaultLit);

#if WITH_EDITORONLY_DATA
	UMaterialEditorOnlyData* Data = M->GetEditorOnlyData();
	if (Data)
	{
		auto* Col = NewObject<UMaterialExpressionConstant3Vector>(M);
		Col->Constant = C;
		Data->ExpressionCollection.AddExpression(Col);
		Data->BaseColor.Expression = Col;

		auto* R = NewObject<UMaterialExpressionConstant>(M);
		R->R = 0.5f;
		Data->ExpressionCollection.AddExpression(R);
		Data->Roughness.Expression = R;
	}
#endif

	M->PostEditChange();
	return M;
}

void ASSXPureSnowboarder::SetupPlayerInputComponent(UInputComponent* PIC)
{
	Super::SetupPlayerInputComponent(PIC);
	PIC->BindAxis("MoveForward", this, &ASSXPureSnowboarder::MoveForward);
	PIC->BindAxis("MoveRight", this, &ASSXPureSnowboarder::MoveRight);
	PIC->BindAction("Jump", IE_Pressed, this, &ASSXPureSnowboarder::DoJump);
	PIC->BindAction("SpinLeft", IE_Pressed, this, &ASSXPureSnowboarder::SpinL);
	PIC->BindAction("SpinRight", IE_Pressed, this, &ASSXPureSnowboarder::SpinR);
}

void ASSXPureSnowboarder::MoveForward(float V) { InputFwd = V; }
void ASSXPureSnowboarder::MoveRight(float V) { InputRight = V; }
void ASSXPureSnowboarder::DoJump() { bJumpPressed = true; }
void ASSXPureSnowboarder::SpinL() { if (bIsAirborne) TotalSpin -= 90.f; }
void ASSXPureSnowboarder::SpinR() { if (bIsAirborne) TotalSpin += 90.f; }

float ASSXPureSnowboarder::GetSpeed() const
{
	return BoardMesh ? BoardMesh->GetComponentVelocity().Size() : 0.f;
}

void ASSXPureSnowboarder::Tick(float DT)
{
	Super::Tick(DT);
	UpdatePhysics(DT);
	UpdateTricks(DT);
	UpdateCamera(DT);
	if (TrickDisplayTimer > 0.f)
	{
		TrickDisplayTimer -= DT;
	}
	if (SprayTimer > 0.f)
	{
		SprayTimer -= DT;
	}
}

void ASSXPureSnowboarder::UpdatePhysics(float DT)
{
	if (!BoardMesh)
	{
		return;
	}

	FVector Loc = BoardMesh->GetComponentLocation();
	FVector Vel = BoardMesh->GetComponentVelocity();
	BoardSpeed = Vel.Size();

	FHitResult Hit;
	FCollisionQueryParams P;
	P.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Loc, Loc - FVector(0, 0, 250.f), ECC_WorldStatic, P);

	if (bHit)
	{
		GroundNormal = Hit.ImpactNormal;
		float Dist = (Hit.ImpactPoint - Loc).Size();
		if (Dist < 90.f && !bIsAirborne)
		{
			BoardMesh->SetWorldLocation(FMath::Lerp(Loc, Hit.ImpactPoint + GroundNormal * 58.f, DT * 12.f));
		}

		// Slope gravity: project gravity onto the surface plane.
		FVector Slope = FVector::VectorPlaneProject(FVector(0, 0, -980.f), GroundNormal);
		BoardMesh->AddForce(Slope * BoardMesh->GetMass());

		FVector Fwd = BoardMesh->GetForwardVector();
		BoardMesh->AddForce(Fwd * InputFwd * 4000.f * BoardMesh->GetMass());

		// Carving: torque around Z plus lateral force.
		float Carve = InputRight;
		BoardMesh->AddTorqueInRadians(FVector(0, 0, Carve * 2500.f));
		BoardMesh->AddForce(BoardMesh->GetRightVector() * Carve * 2000.f * BoardMesh->GetMass());

		// Air drag so speed converges instead of exploding.
		BoardMesh->AddForce(-Vel.GetSafeNormal() * Vel.SizeSquared() * 0.004f * BoardMesh->GetMass());

		if (bJumpPressed)
		{
			BoardMesh->AddImpulse(FVector(0, 0, 1) * 50000.f + Fwd * 12000.f);
			bIsAirborne = true;
			bJumpPressed = false;
			AirTime = 0.f;
		}
		else
		{
			bIsAirborne = false;
			if (BoardSpeed > 600.f && FMath::Abs(Carve) > 0.3f && SprayTimer <= 0.f)
			{
				SpawnSpray();
				SprayTimer = 0.06f;
			}
		}
	}
	else
	{
		bIsAirborne = true;
		AirTime += DT;
		BoardMesh->AddForce((BoardMesh->GetForwardVector() * InputFwd * 600.f + BoardMesh->GetRightVector() * InputRight * 400.f) * BoardMesh->GetMass());

		// Air control: lean into input, stabilize with interpolation.
		FRotator R = BoardMesh->GetComponentRotation();
		FRotator T = R;
		T.Pitch = FMath::FInterpTo(R.Pitch, InputFwd * 35.f, DT, 2.5f);
		T.Roll = FMath::FInterpTo(R.Roll, -InputRight * 35.f, DT, 2.5f);
		BoardMesh->SetWorldRotation(FMath::RInterpTo(R, T, DT, 3.f));
	}
}

void ASSXPureSnowboarder::UpdateTricks(float DT)
{
	if (!BoardMesh)
	{
		return;
	}

	FVector Cur = BoardMesh->GetComponentRotation().Euler();
	FVector D = Cur - LastRot;
	LastRot = Cur;

	auto Norm = [](float A)
	{
		while (A > 180.f) A -= 360.f;
		while (A < -180.f) A += 360.f;
		return A;
	};

	if (bIsAirborne)
	{
		TotalFlip += Norm(D.Y);
		TotalSpin += Norm(D.Z);
	}
	else if (AirTime > 0.3f)
	{
		// Landed: bank the trick.
		int32 Sp = FMath::Abs(TotalSpin) / 360.f;
		int32 Fl = FMath::Abs(TotalFlip) / 360.f;
		if (Sp > 0 || Fl > 0 || AirTime > 0.8f)
		{
			int32 TS = (Sp * 100 + Fl * 150 + FMath::FloorToInt(AirTime * 60.f)) * ComboMult;
			TotalScore += TS;

			CurrentTrickName = FString::Printf(TEXT("%d%c %d%c"),
				Sp, TotalSpin > 0 ? 'R' : 'L',
				Fl, TotalFlip > 0 ? 'B' : 'F');
			if (Sp == 0 && Fl == 0)
			{
				CurrentTrickName = TEXT("BIG AIR");
			}
			TrickDisplayTimer = 2.f;
			ComboMult = FMath::Min(ComboMult + 0.5f, 5.f);
		}
		else
		{
			ComboMult = 1.f;
		}

		TotalSpin = TotalFlip = 0.f;
		AirTime = 0.f;
	}
}

void ASSXPureSnowboarder::UpdateCamera(float DT)
{
	if (!BoardMesh || !SpringArm)
	{
		return;
	}

	FVector L = BoardMesh->GetComponentLocation();
	FVector T = L - BoardMesh->GetForwardVector() * 350.f + FVector(0, 0, 180.f);
	SpringArm->SetWorldLocation(FMath::Lerp(SpringArm->GetComponentLocation(), L, DT * 4.f));
	SpringArm->SetWorldRotation(FMath::RInterpTo(SpringArm->GetComponentRotation(), (L - T).Rotation(), DT, 2.5f));
}

void ASSXPureSnowboarder::SpawnSpray()
{
	if (!GetWorld())
	{
		return;
	}
	FVector S = BoardMesh->GetComponentLocation() - FVector(0, 0, 35.f);
	GetWorld()->SpawnActor<ASSXPureParticle>(S, BoardMesh->GetComponentRotation());
}
