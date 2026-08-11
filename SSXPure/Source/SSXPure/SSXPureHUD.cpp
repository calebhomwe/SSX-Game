#include "SSXPureHUD.h"
#include "SSXPureSnowboarder.h"
#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"

void ASSXPureHUD::DrawHUD()
{
	Super::DrawHUD();

	APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!P)
	{
		return;
	}
	ASSXPureSnowboarder* R = Cast<ASSXPureSnowboarder>(P);
	if (!R)
	{
		return;
	}

	float Spd = R->GetSpeed() * 0.036f;
	DrawRect(FColor(0, 0, 0, 160), 25, 25, 320, 160);
	DrawText(TEXT("SPEED"), FLinearColor(0.7f, 0.7f, 0.7f), 50, 40, nullptr, 1.3f);
	DrawText(FString::Printf(TEXT("%.0f KM/H"), Spd), FLinearColor(0.0f, 1.0f, 1.0f), 50, 70, nullptr, 2.2f);
	DrawText(TEXT("SCORE"), FLinearColor(0.7f, 0.7f, 0.7f), 50, 115, nullptr, 1.3f);
	DrawText(FString::Printf(TEXT("%d"), R->GetScore()), FLinearColor(1.0f, 0.86f, 0.0f), 50, 145, nullptr, 2.2f);

	if (R->IsDisplayingTrick())
	{
		float X = Canvas->SizeX * 0.5f - 120;
		float Y = Canvas->SizeY * 0.22f;
		DrawText(R->GetCurrentTrickName(), FLinearColor(1.0f, 0.31f, 0.0f), X, Y, nullptr, 3.5f);
	}
	if (R->GetIsAirborne())
	{
		DrawText(TEXT("AIRBORNE"), FLinearColor(0.0f, 1.0f, 0.39f), Canvas->SizeX * 0.5f - 80, Canvas->SizeY * 0.15f, nullptr, 1.8f);
	}
}
