#include "SSXPureTerrain.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant.h"

ASSXPureTerrain::ASSXPureTerrain()
{
	PrimaryActorTick.bCanEverTick = true;
	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;
	TerrainMesh->bUseAsyncCooking = false;
	TerrainMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TerrainMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void ASSXPureTerrain::BeginPlay()
{
	Super::BeginPlay();
	GenerateChunk(FVector::ZeroVector);
	LastChunkCenter = FVector::ZeroVector;
}

void ASSXPureTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player)
	{
		return;
	}

	FVector P = Player->GetActorLocation();
	FVector Current = FVector(
		FMath::RoundToFloat(P.X / ChunkSize) * ChunkSize,
		FMath::RoundToFloat(P.Y / ChunkSize) * ChunkSize,
		0);

	if (!Current.Equals(LastChunkCenter, 10.f))
	{
		LastChunkCenter = Current;
		GenerateChunk(Current);
	}
}

float ASSXPureTerrain::GetNoiseHeight(float X, float Y) const
{
	// Layered Perlin: broad mountain ridges + mid hills + fine snow detail.
	float H = FMath::PerlinNoise2D(FVector2D(X * 0.004f, Y * 0.004f)) * 800.f;
	H += FMath::PerlinNoise2D(FVector2D(X * 0.02f, Y * 0.02f)) * 200.f;
	H += FMath::PerlinNoise2D(FVector2D(X * 0.1f, Y * 0.1f)) * 40.f;

	// Occasional ridgelines that act as natural kickers.
	float Ramp = FMath::PerlinNoise2D(FVector2D(X * 0.002f + 50, Y * 0.002f));
	if (Ramp > 0.65f)
	{
		H += FMath::Max(0.f, FMath::Sin(X * 0.015f) * 300.f);
	}

	return H;
}

UMaterialInterface* ASSXPureTerrain::CreateSnowMaterial()
{
	UMaterial* Mat = NewObject<UMaterial>(this, TEXT("SnowMat"));
	Mat->BlendMode = BLEND_Opaque;
	Mat->SetShadingModel(MSM_DefaultLit);

#if WITH_EDITORONLY_DATA
	// UE 5.8: the expression graph lives on the editor-only data object.
	UMaterialEditorOnlyData* Data = Mat->GetEditorOnlyData();
	if (Data)
	{
		UMaterialExpressionConstant3Vector* Color = NewObject<UMaterialExpressionConstant3Vector>(Mat);
		Color->Constant = FLinearColor(0.88f, 0.92f, 0.98f);
		Data->ExpressionCollection.AddExpression(Color);
		Data->BaseColor.Expression = Color;

		UMaterialExpressionConstant* Rough = NewObject<UMaterialExpressionConstant>(Mat);
		Rough->R = 0.9f;
		Data->ExpressionCollection.AddExpression(Rough);
		Data->Roughness.Expression = Rough;
	}
#endif

	Mat->PostEditChange();
	return Mat;
}

void ASSXPureTerrain::GenerateChunk(FVector Center)
{
	TArray<FVector> Verts;
	TArray<int32> Tris;
	TArray<FVector2D> UV0;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> Colors;

	float Half = ChunkSize * 0.5f;
	float Step = ChunkSize / Resolution;

	for (int32 Y = 0; Y <= Resolution; Y++)
	{
		for (int32 X = 0; X <= Resolution; X++)
		{
			float WX = Center.X - Half + X * Step;
			float WY = Center.Y - Half + Y * Step;
			Verts.Add(FVector(WX, WY, GetNoiseHeight(WX, WY)));
			UV0.Add(FVector2D((float)X / Resolution, (float)Y / Resolution));
			Normals.Add(FVector::UpVector);
			Tangents.Add(FProcMeshTangent(1.f, 0.f, 0.f));
			Colors.Add(FColor(255, 255, 255));
		}
	}

	for (int32 Y = 0; Y < Resolution; Y++)
	{
		for (int32 X = 0; X < Resolution; X++)
		{
			int32 TL = Y * (Resolution + 1) + X;
			int32 BL = (Y + 1) * (Resolution + 1) + X;
			int32 TR = TL + 1;
			int32 BR = BL + 1;
			Tris.Add(TL);
			Tris.Add(BL);
			Tris.Add(TR);
			Tris.Add(TR);
			Tris.Add(BL);
			Tris.Add(BR);
		}
	}

	// Smooth-shaded normals from the actual triangle soup.
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Normals[i] = FVector::ZeroVector;
	}
	for (int32 i = 0; i < Tris.Num(); i += 3)
	{
		FVector N = FVector::CrossProduct(
			Verts[Tris[i + 1]] - Verts[Tris[i]],
			Verts[Tris[i + 2]] - Verts[Tris[i]]).GetSafeNormal();
		Normals[Tris[i]] += N;
		Normals[Tris[i + 1]] += N;
		Normals[Tris[i + 2]] += N;
	}
	for (auto& N : Normals)
	{
		N.Normalize();
	}

	TerrainMesh->ClearMeshSection(0);
	TerrainMesh->CreateMeshSection(0, Verts, Tris, Normals, UV0, Colors, Tangents, true);
	if (!TerrainMesh->GetMaterial(0))
	{
		TerrainMesh->SetMaterial(0, CreateSnowMaterial());
	}
}
