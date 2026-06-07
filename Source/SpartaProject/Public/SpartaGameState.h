#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SpartaGameState.generated.h"

class ASpawnVolume;

USTRUCT(BlueprintType)
struct FWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TimeLimit = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnInterval = 1.0f;
};

UCLASS()
class SPARTAPROJECT_API ASpartaGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ASpartaGameState();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Score;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 SpawnedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	float RemainingTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 RemainingSpawns = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	bool bInWave = false;

	UPROPERTY()
	TArray<ASpawnVolume*> SpawnVolumes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FWaveConfig> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;

	FTimerHandle HUDUpdateTimerHandle;
	FTimerHandle WaveCountdownTimerHandle;
	FTimerHandle SpawnTimerHandle;

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const;
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Level")
	void OnGameOver();

	void StartLevel();
	void StartWave(int32 WaveIndex);
	void EndWave();
	void SpawnOnce();
	void UpdateWaveTime();
	void OnCoinCollected();
	void EndLevel();
	void UpdateHUD();
};
