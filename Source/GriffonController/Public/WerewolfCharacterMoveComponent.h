// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ObjectMacros.h"
#include "WerewolfCharacterMoveComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Climbing      UMETA(DisplayName = "Climbing"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class GRIFFONCONTROLLER_API UWerewolfCharacterMoveComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	
	////////////////////////////////////////////////////////////
	/// CLIMBING

	UPROPERTY(EditAnywhere)
	bool IsDebug = true;
	
	///////////////////
	/// DETECTING SURFACES TO CLIMB AND START OR NOT CLIMBING

	void SweepAndStoreWallHits();

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere)
	int CollisionCapsuleHalfHeight = 80;
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere)
	int CollisionCapsuleRadius = 50;

	TArray<FHitResult> CurrentWallHits;
	FCollisionQueryParams ClimbQueryParams;
	
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="1.0", ClampMax="75.0"))
	float MinHorizontalDegreesToStartClimbing = 30;

	bool CanStartClimbing();
	bool IsFacingSurface(const float Steepness) const;
	bool EyeHeightTrace(const float TraceDistance) const;

	///////////////////
	/// START/STOP CLIMBING INPUT

	void TryClimbing();
	void CancelClimbing();

	bool bWantsToClimb = false;

	UFUNCTION(BlueprintPure)
	bool IsClimbing() const;
	UFUNCTION(BlueprintPure)
	FVector GetClimbSurfaceNormal() const;

	///////////////////
	/// CLIMBING PHYSICS
	/// INSPIRATION FROM PhysFlying for the workflow

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void PhysClimbing(float deltaTime, int32 Iterations);

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="80.0"))
	float ClimbingCollisionShrinkAmount = 30;
	
	void ComputeSurfaceInfo();
	void ComputeClimbingVelocity(float deltaTime);
	bool ShouldStopClimbing() const;
	void StopClimbing(float deltaTime, int32 Iterations);
	void MoveAlongClimbingSurface(float deltaTime);
	void SnapToClimbingSurface(float deltaTime) const;
	FQuat GetClimbingRotation(float deltaTime) const;

	FVector CurrentClimbingNormal;
	FVector CurrentClimbingPosition;

	///////////////////
	/// CLIMBING PHYSICS VALUES
	
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="10.0", ClampMax="500.0"))
	float MaxClimbingSpeed = 120.f;
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="10.0", ClampMax="2000.0"))
	float MaxClimbingAcceleration = 380.f;
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="3000.0"))
	float BrakingDecelerationClimbing = 550.f;
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="1.0", ClampMax="12.0"))
	int ClimbingRotationSpeed = 6;
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="60.0"))
    float ClimbingSnapSpeed = 4.f;
    UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="80.0"))
    float DistanceFromSurface = 45.f;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;

	///////////////////
	/// CLIMBING UP LEDGES

	UPROPERTY(Category="Character Movement: Climbing", EditDefaultsOnly)
	UAnimMontage* LedgeClimbMontage;
	UPROPERTY()
	UAnimInstance* AnimInstance;

	bool TryClimbUpLedge() const;

	bool HasReachedEdge() const;
	bool IsLocationWalkable(const FVector& CheckLocation) const;
	bool CanMoveToLedgeClimbLocation() const;
};
