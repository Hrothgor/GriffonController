// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputActionValue.h"
#include "ShapeShiftForm.h"
#include "WerewolfCharacterMoveComponent.h"
#include "WerewolfControllerCharacter.generated.h"

UCLASS()
class GRIFFONCONTROLLER_API AWerewolfControllerCharacter : public AShapeShiftForm
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;
    
    /** Jump Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* JumpAction;
    /** Move Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* MoveAction;
    /** Look Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* LookAction;
	/** Climb Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ClimbAction;

protected:
	/** Custom Movement Component **/
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly)
	UWerewolfCharacterMoveComponent *MovementComponent;
	
public:
	// Sets default values for this character's properties
	AWerewolfControllerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns CustomMovementComponent subobject **/
	UFUNCTION(BlueprintPure)
	FORCEINLINE UWerewolfCharacterMoveComponent* GetCustomCharacterMovement() const { return MovementComponent; }
	virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	///////////////////////////////
	/// CLIMB

	void Climb();

	///////////////////////////////
	/// SHAPESHIFT

	virtual void StartShapeShifting() override;
};
