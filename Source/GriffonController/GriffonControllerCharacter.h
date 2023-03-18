// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GriffonControllerCharacter.generated.h"


UCLASS(config=Game)
class AGriffonControllerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FlyAction;

public:
	AGriffonControllerCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    void Tick(float DeltaSeconds) override;

	void StartFlying();
	void StopFlying();

	void StopFlapping();
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsFlying = false;
	bool bCanFly = true;
	UPROPERTY(BlueprintReadOnly)
	bool bIsFlapping = false;
	
	void FlyPhysicsCompute(float DeltaSeconds);
	
	// FLY VALUE
	UPROPERTY(EditAnywhere)
	float AirControlValue = 100;
	float DefaultAirControlValue;
	UPROPERTY(EditAnywhere)
	float BrakingFrictionValue = 2;
	float DefaultBrakingFrictionValue;
	UPROPERTY(EditAnywhere)
	FRotator RotationRateValue = {0, 0, 90};
	FRotator DefaultRotationRateValue;
	UPROPERTY(EditAnywhere)
	float MaxAccelerationValue = 600;
	float DefaultMaxAccelerationValue;
	UPROPERTY(EditAnywhere)
	float MaxWalkSpeedValue = 4000;
	float DefaultMaxWalkSpeedValue;

	// FLY VARIABLES
	UPROPERTY(EditAnywhere)
	UCurveFloat *FlightVelocityLiftMultiplierCurve;
	UPROPERTY(EditAnywhere)
	UCurveFloat *FlightVelocityAngleMultiplierCurve;

	UPROPERTY(BlueprintReadOnly)
	float ControlInclination;
	UPROPERTY(BlueprintReadOnly)
	float LiftNormalized;
	UPROPERTY(BlueprintReadOnly)
	float FlySpeedGliding;
	UPROPERTY(BlueprintReadOnly)
	float VelocityAngle;
	UPROPERTY(BlueprintReadOnly)
	float FlyingVelocity;
	
	// DEBUG
	void DrawDebug();
	
	UPROPERTY(EditAnywhere)
	bool IsDebug = true;
};

