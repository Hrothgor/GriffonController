// Fill out your copyright notice in the Description page of Project Settings.

#include "WerewolfCharacterMoveComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

void UWerewolfCharacterMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	ClimbQueryParams.AddIgnoredActor(GetOwner());
	AnimInstance = GetCharacterOwner()->GetMesh()->GetAnimInstance();
}

void UWerewolfCharacterMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
													  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SweepAndStoreWallHits();
}

void UWerewolfCharacterMoveComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (bWantsToClimb)
	{
		SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Climbing);
	}
}

///////////////////
/// DETECTING SURFACES TO CLIMB AND START OR NOT CLIMBING

void UWerewolfCharacterMoveComponent::SweepAndStoreWallHits()
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CollisionCapsuleRadius, CollisionCapsuleHalfHeight);

	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 20;

	// Avoid using the same Start/End location for a Sweep, as it doesn't trigger hits on Landscapes.
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	TArray<FHitResult> Hits;
	const bool HitWall = GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
		  ECC_WorldStatic, CollisionShape, ClimbQueryParams);

	if (IsDebug == true && GEngine)
	{
		DrawDebugCapsule(GetWorld(), End, CollisionCapsuleHalfHeight, CollisionCapsuleRadius, FQuat::Identity, FColor::Silver);
		for (FHitResult Hit : Hits)
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8, 32, FColor::Blue);
	}

	HitWall ? CurrentWallHits = Hits : CurrentWallHits.Reset();
}

bool UWerewolfCharacterMoveComponent::CanStartClimbing()
{
	for (FHitResult& Hit : CurrentWallHits)
	{
		const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D(); // Normal without Z

		const float HorizontalDot = FVector::DotProduct(UpdatedComponent->GetForwardVector(), -HorizontalNormal); // If actor is looking in the direction of the whole (in yaw)
		const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal); // If the whole is not too steep (> 90 degrees)

		const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(HorizontalDot));

		const bool bIsCeiling = FMath::IsNearlyZero(VerticalDot); // Perpendicular (90 degrees)

		if (HorizontalDegrees <= MinHorizontalDegreesToStartClimbing
			&& !bIsCeiling && IsFacingSurface(VerticalDot))
				return true;
	}

	return false;
}

bool UWerewolfCharacterMoveComponent::IsFacingSurface(const float Steepness) const
{
	// Steepness is 0 when 90 degrees and 1 when wall perfectly perpendicular to ground

	constexpr float BaseLength = 80;
	const float SteepnessMultiplier = 1 + (1 - Steepness) * 5;

	return EyeHeightTrace(BaseLength * SteepnessMultiplier);
}

bool UWerewolfCharacterMoveComponent::EyeHeightTrace(const float TraceDistance) const
{
 	FHitResult UpperEdgeHit;

	const float BaseEyeHeight = GetCharacterOwner()->BaseEyeHeight;
	const float EyeHeightOffset = IsClimbing() ? BaseEyeHeight + ClimbingCollisionShrinkAmount * 2 : BaseEyeHeight;

	const FVector Start = UpdatedComponent->GetComponentLocation() +
			(UpdatedComponent->GetUpVector() * EyeHeightOffset);
	const FVector End = Start + (UpdatedComponent->GetForwardVector() * TraceDistance);

	if (IsDebug == true && GEngine)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, -1, 0, 5);
	}
	
	return GetWorld()->LineTraceSingleByChannel(UpperEdgeHit, Start, End, ECC_WorldStatic, ClimbQueryParams);
}

///////////////////
/// START/STOP CLIMBING INPUT

void UWerewolfCharacterMoveComponent::TryClimbing()
{
	if (CanStartClimbing())
	{
		bWantsToClimb = true;
	}
}

void UWerewolfCharacterMoveComponent::CancelClimbing()
{
	bWantsToClimb = false;
}

bool UWerewolfCharacterMoveComponent::IsClimbing() const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Climbing;
}

FVector UWerewolfCharacterMoveComponent::GetClimbSurfaceNormal() const
{
	return CurrentClimbingNormal;
}

///////////////////
/// CLIMBING PHYSICS

void UWerewolfCharacterMoveComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == ECustomMovementMode::CMOVE_Climbing)
	{
		PhysClimbing(deltaTime, Iterations);
	}

	Super::PhysCustom(deltaTime, Iterations);
}

void UWerewolfCharacterMoveComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
	uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;

		// Shrink capsule or too big
		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() - ClimbingCollisionShrinkAmount);
	}

	const bool bWasClimbing = PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Climbing;
	if (bWasClimbing)
	{
		bOrientRotationToMovement = true;

		// After exiting climbing mode, reset rotation
		const FRotator StandRotation = FRotator(0,  UpdatedComponent->GetComponentRotation().Yaw, 0);
		UpdatedComponent->SetRelativeRotation(StandRotation);

		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() + ClimbingCollisionShrinkAmount);

		// After exiting climbing mode, reset velocity and acceleration
		StopMovementImmediately();
	}
	
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UWerewolfCharacterMoveComponent::PhysClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	ComputeSurfaceInfo();

	if (ShouldStopClimbing())
	{
		StopClimbing(deltaTime, Iterations);
		return;
	}

	ComputeClimbingVelocity(deltaTime);

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();

	MoveAlongClimbingSurface(deltaTime);

	TryClimbUpLedge();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

	SnapToClimbingSurface(deltaTime);
}

void UWerewolfCharacterMoveComponent::ComputeSurfaceInfo()
{
	CurrentClimbingNormal = FVector::ZeroVector;
	CurrentClimbingPosition = FVector::ZeroVector;

	if (CurrentWallHits.IsEmpty())
	{
		return;
	}

	// Here instead of just taking the average of each hit
	// We first do an other check by throwing a sphere to each hit
	// to be sure that we are not taking in account overlapping object
	// it will only consider the first object hit by the sphere hit
	// it give a way more precise surface detection
	
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(6);

	for (const FHitResult& WallHit : CurrentWallHits)
	{
		const FVector End = Start + (WallHit.ImpactPoint - Start).GetSafeNormal() * 120;

		FHitResult AssistHit;
		GetWorld()->SweepSingleByChannel(AssistHit, Start, End, FQuat::Identity,
			ECC_WorldStatic, CollisionSphere, ClimbQueryParams);

		CurrentClimbingPosition += AssistHit.ImpactPoint;
		CurrentClimbingNormal += AssistHit.Normal;
	}

	CurrentClimbingPosition /= CurrentWallHits.Num();
	CurrentClimbingNormal = CurrentClimbingNormal.GetSafeNormal();
}

void UWerewolfCharacterMoveComponent::ComputeClimbingVelocity(float deltaTime)
{
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		constexpr float Friction = 0.0f;
		constexpr bool bFluid = false;
		CalcVelocity(deltaTime, Friction, bFluid, BrakingDecelerationClimbing);
	}

	ApplyRootMotionToVelocity(deltaTime);
}

bool UWerewolfCharacterMoveComponent::ShouldStopClimbing() const
{
	const bool bIsOnCeiling = FVector::Parallel(CurrentClimbingNormal, FVector::UpVector, 0.95);

	return !bWantsToClimb || CurrentClimbingNormal.IsZero() || bIsOnCeiling;
}

void UWerewolfCharacterMoveComponent::StopClimbing(float deltaTime, int32 Iterations)
{
	bWantsToClimb = false;
	SetMovementMode(EMovementMode::MOVE_Falling);
	StartNewPhysics(deltaTime, Iterations);
}

void UWerewolfCharacterMoveComponent::MoveAlongClimbingSurface(float deltaTime)
{
	const FVector Adjusted = Velocity * deltaTime;

	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Adjusted, GetClimbingRotation(deltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
}

void UWerewolfCharacterMoveComponent::SnapToClimbingSurface(float deltaTime) const
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();

	const FVector ForwardDifference = (CurrentClimbingPosition - Location).ProjectOnTo(Forward);
	const FVector Offset = -CurrentClimbingNormal * (ForwardDifference.Length() - DistanceFromSurface);

	constexpr bool bSweep = true;
	UpdatedComponent->MoveComponent(Offset * ClimbingSnapSpeed * deltaTime, Rotation, bSweep);
}

FQuat UWerewolfCharacterMoveComponent::GetClimbingRotation(float deltaTime) const
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return Current;
	}
	
	const FQuat Target = FRotationMatrix::MakeFromX(-CurrentClimbingNormal).ToQuat();

	return FMath::QInterpTo(Current, Target, deltaTime, ClimbingRotationSpeed);
}

///////////////////
/// CLIMBING PHYSICS VALUES

float UWerewolfCharacterMoveComponent::GetMaxSpeed() const
{
	return IsClimbing() ? MaxClimbingSpeed : Super::GetMaxSpeed();
}

float UWerewolfCharacterMoveComponent::GetMaxAcceleration() const
{
	return IsClimbing() ? MaxClimbingAcceleration : Super::GetMaxAcceleration();
}

bool UWerewolfCharacterMoveComponent::TryClimbUpLedge() const
{
	if (AnimInstance == nullptr || LedgeClimbMontage == nullptr)
		return false;
	
	if (AnimInstance->Montage_IsPlaying(LedgeClimbMontage))
		return false;

	const float UpSpeed = FVector::DotProduct(Velocity, UpdatedComponent->GetUpVector());
	const bool bIsMovingUp = UpSpeed >= MaxClimbingSpeed / 3;

	if (bIsMovingUp && HasReachedEdge() && CanMoveToLedgeClimbLocation())
	{
		const FRotator StandRotation = FRotator(0, UpdatedComponent->GetComponentRotation().Yaw, 0);
		UpdatedComponent->SetRelativeRotation(StandRotation);

		AnimInstance->Montage_Play(LedgeClimbMontage);

		return true;
	}

	return false;
}

bool UWerewolfCharacterMoveComponent::HasReachedEdge() const
{
		const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
    	const float TraceDistance = Capsule->GetUnscaledCapsuleRadius() * 2.5;
    
    	return !EyeHeightTrace(TraceDistance);
}

bool UWerewolfCharacterMoveComponent::IsLocationWalkable(const FVector& CheckLocation) const
{
	return true;
}

bool UWerewolfCharacterMoveComponent::CanMoveToLedgeClimbLocation() const
{
	return true;
}
