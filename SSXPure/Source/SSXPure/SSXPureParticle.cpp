#include "SSXPureParticle.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"

ASSXPureParticle::ASSXPureParticle()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	TArray<FVector> V;
	TArray<int32> T;
	TArray<FVector2D> U;
	TArray<FVector> N;
	TArray<FProcMeshTangent> Tan;
	TArray<FColor> C;

	float s = 4.f;
	V.Add(FVector(-s, -s, 0));
	V.Add(FVector(s, -s, 0));
	V.Add(FVector(-s, s, 0));
	V.Add(FVector(s, s, 0));
	for (int32 i = 0; i < 4; i++)
	{
		U.Add(FVector2D(i & 1, (i >> 1) & 1));
		N.Add(FVector(0, 0, 1));
		Tan.Add(FProcMeshTangent(1.f, 0.f, 0.f));
		C.Add(FColor(245, 250, 255, 220));
	}
	T.Add(0);
	T.Add(2);
	T.Add(1);
	T.Add(1);
	T.Add(2);
	T.Add(3);
	Mesh->CreateMeshSection(0, V, T, N, U, C, Tan, true);

	UMaterial* M = NewObject<UMaterial>(this);
	M->BlendMode = BLEND_Translucent;
	M->SetShadingModel(MSM_Unlit);

#if WITH_EDITORONLY_DATA
	UMaterialEditorOnlyData* Data = M->GetEditorOnlyData();
	if (Data)
	{
		auto* Col = NewObject<UMaterialExpressionConstant3Vector>(M);
		Col->Constant = FLinearColor(0.95f, 0.97f, 1.f);
		Data->ExpressionCollection.AddExpression(Col);
		Data->EmissiveColor.Expression = Col;
	}
#endif

	M->PostEditChange();
	Mesh->SetMaterial(0, M);
}

void ASSXPureParticle::BeginPlay()
{
	Super::BeginPlay();
	Vel = FVector(FMath::RandRange(-120.f, 120.f), FMath::RandRange(-120.f, 120.f), FMath::RandRange(80.f, 250.f));
	SetActorScale3D(FVector(FMath::RandRange(0.4f, 1.2f)));
}

void ASSXPureParticle::Tick(float DT)
{
	Super::Tick(DT);
	Life -= DT;
	if (Life <= 0.f)
	{
		Destroy();
		return;
	}
	AddActorWorldOffset(Vel * DT);
	Vel.Z -= 500.f * DT;
	SetActorScale3D(FVector(Life / MaxLife));
}
