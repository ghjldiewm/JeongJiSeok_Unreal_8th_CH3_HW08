#pragma once

#include "CoreMinimal.h"
#include "ItemSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FItemSpawnRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	// 어떤 아이템 클래스 스폰?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ItemClass; // 하드 레퍼런스로 클래스를 메모리에 항상 로드해놓음 (권장하지않음)
	// 스폰 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance;
};
