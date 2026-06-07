#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

ASpartaGameState::ASpartaGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		ASpawnVolume::StaticClass(),
		FoundVolumes
	);

	SpawnVolumes.Empty();
	for (AActor* Actor : FoundVolumes)
	{
		if (ASpawnVolume* Volume = Cast<ASpawnVolume>(Actor))
		{
			SpawnVolumes.Add(Volume);
		}
	}

	StartLevel();
	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("BeginPlay screen msg"));
	}

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay log msg"));
}

int32 ASpartaGameState::GetScore() const
{
	return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

void ASpartaGameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->ShowGameHUD();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("StartLevel -> calling StartWave(0). Waves.Num=%d"), Waves.Num());
	StartWave(0);
	UE_LOG(LogTemp, Warning, TEXT("StartLevel -> calling StartWave(0). Waves.Num=%d"), Waves.Num());
}

void ASpartaGameState::StartWave(int32 WaveIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("StartWave called. WaveIndex=%d Waves.Num=%d"), WaveIndex, Waves.Num());
	
	if (!Waves.IsValidIndex(WaveIndex)) return;

	CurrentWaveIndex = WaveIndex;
	const FWaveConfig& Config = Waves[CurrentWaveIndex];

	RemainingTime = Config.TimeLimit;
	RemainingSpawns = Config.SpawnCount;
	bInWave = true;

	UE_LOG(LogTemp, Log, TEXT("Wave %d 시작!"), CurrentWaveIndex + 1);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.f, FColor::Green,
			FString::Printf(TEXT("Wave %d 시작!"), CurrentWaveIndex + 1)
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("StartWave: TimeLimit=%.2f SpawnCount=%d Interval=%.2f"),
		Config.TimeLimit, Config.SpawnCount, Config.SpawnInterval);

	SpawnOnce();

	// 카운트다운 타이머 ON (0.1초마다 시간 감소)
	GetWorldTimerManager().ClearTimer(WaveCountdownTimerHandle);
	GetWorldTimerManager().SetTimer(
		WaveCountdownTimerHandle,
		this,
		&ASpartaGameState::UpdateWaveTime,
		0.1f,
		true
	);

	// 스폰 타이머 ON
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ASpartaGameState::SpawnOnce,
		Config.SpawnInterval,
		true
	);


	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
		FString::Printf(TEXT("WaveStart T=%.1f S=%d I=%.2f"),
			RemainingTime, RemainingSpawns, Config.SpawnInterval));
}

void ASpartaGameState::EndWave()
{
	bInWave = false;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveCountdownTimerHandle);

	const int32 NextWaveIndex = CurrentWaveIndex + 1;

	if (Waves.IsValidIndex(NextWaveIndex))
	{
		StartWave(NextWaveIndex);
	}
	else
	{
		EndLevel();
	}
}

void ASpartaGameState::SpawnOnce()
{
	if (!bInWave) return;

	if (RemainingSpawns <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	if (SpawnVolumes.Num() > 0 && SpawnVolumes[0])
	{
		for (int32 i = 0; i <= 9; ++i)
		{
			AActor* SpawnedActor = SpawnVolumes[0]->SpawnRandomItem();

			if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
			{
				SpawnedCoinCount++;
			}
		}
	}

	RemainingSpawns--;
}

void ASpartaGameState::UpdateWaveTime()
{
	if (!bInWave) return;

	RemainingTime -= 0.1f;

	if (RemainingTime <= 0.f)
	{
		EndWave();
	}
}

void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Coin Collected: %d / %d"),
		CollectedCoinCount,
		SpawnedCoinCount);
}

void ASpartaGameState::EndLevel()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			AddScore(Score);
			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}

	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver();
		return;
	}

	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else
	{
		OnGameOver();
	}
}

void ASpartaGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->SetPause(true);
			SpartaPlayerController->ShowMainMenu(true);
		}
	}
}

void ASpartaGameState::UpdateHUD()
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("UpdateHUD tick. RemainingTime=%.2f"), RemainingTime);
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
				}

				if (UTextBlock* StageText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Stage"))))
				{
					StageText->SetText(FText::FromString(FString::Printf(TEXT("Stage: %d"), CurrentWaveIndex + 1)));

				}

				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
						if (SpartaGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
						}
					}
				}

				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex+1)));
				}
			}
		}
	}
}