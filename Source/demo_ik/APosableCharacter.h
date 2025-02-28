#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SplineComponent.h"  // <-- for spline animation
#include "APosableCharacter.generated.h"

/**
 * It is a skeletal mesh whose pose can be modified directly on the game thread (Poseable Mesh).
 * It is initialized from a source skeletal mesh component and used to display modifications such as IK.
 */
UCLASS()
class DEMO_IK_API AAPosableCharacter : public AActor
{
	GENERATED_BODY()

public:
	// Existing properties
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	UPoseableMeshComponent* posableMeshComponent_reference;

	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMesh* default_skeletalMesh_reference;

	UPROPERTY(EditAnywhere, Category = "waving animation")
	float waving_animationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "waving animation")
	float waving_amplitude = 30.0f;

	UPROPERTY(EditAnywhere, Category = "target")
	UStaticMesh* targetSphereAsset;

	UPROPERTY(EditAnywhere, Category = "target")
	float targetSphereScaling = 0.1f;

	UPROPERTY(EditAnywhere, Category = "target")
	FColor targetSphereColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "target|test")
	FVector targetSphereTestRelativePosition = FVector(0, 0, 0);

	// New: Hand IK and scripted animation properties

	// Toggle hand IK (using FABRIK on upperarm, lowerarm and hand)
	UPROPERTY(EditAnywhere, Category = "Hand IK")
	bool handIK_isPlaying = false;

	// For scripted animation of the IK target along a spline
	UPROPERTY(EditAnywhere, Category = "Hand IK")
	bool handIKScriptedAnimationPlaying = false;

	UPROPERTY(EditAnywhere, Category = "Hand IK")
	float handIKAnimationTime = 0.0f;

	// Total duration of the scripted IK target animation
	UPROPERTY(EditAnywhere, Category = "Hand IK")
	float handIKAnimationDuration = 5.0f;

	// Spline used to animate the IK target
	UPROPERTY(VisibleAnywhere, Category = "Hand IK")
	USplineComponent* IKTargetSpline;

	// Advanced IK Features
	UPROPERTY(EditAnywhere, Category = "Advanced IK")
	bool bEnableJointLimits = true;

	// Example joint limits for the elbow (assumed to be applied on the lower arm's pitch)
	UPROPERTY(EditAnywhere, Category = "Advanced IK")
	float ElbowMinAngle = 0.0f;    // Minimum allowed elbow bend (degrees)

	UPROPERTY(EditAnywhere, Category = "Advanced IK")
	float ElbowMaxAngle = 150.0f;  // Maximum allowed elbow bend (degrees)

	// Flag to simulate motion capture data integration
	UPROPERTY(EditAnywhere, Category = "Advanced IK")
	bool bUseMotionCaptureData = false;

	// Dummy storage for motion capture rotations per bone (if using motion capture)
	UPROPERTY(EditAnywhere, Category = "Advanced IK")
	TMap<FName, FRotator> MotionCaptureBoneRotations;

protected:
	// Existing properties
	UStaticMeshComponent* targetSphere;
	UMaterialInstanceDynamic* targetSphereMaterial;
	TArray<FRotator> waving_initialBoneRotations;
	bool session1_isPlaying = false;

	// NEW: Function for leg raise animation using inverse kinematics.
	//void legRaise_tickAnimation();

	// NEW: Flag to control the leg raise animation.
	//bool legRaise_isPlaying = false;

	// NEW: Hand IK functions (using a simple FABRIK approach)
	void handIK_tickAnimation();

	// NEW: Scripted animation for the IK target (using a spline and ease-in/ease-out)
	void handIK_animateTarget(float DeltaTime);

public:
	AAPosableCharacter();

	bool initializePosableMesh();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "waving animation")
	void waving_playStop();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "target|test")
	void testSetTargetSphereRelativePosition();

	void setTargetSphereRelativePosition(FVector newPosition);

	bool doesBoneOrSocketNameExists(FName inputName);

	void setVisibility(bool visible);

	// (Optionally you can expose these to Blueprints for testing)
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Hand IK")
	void ToggleHandIK();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Hand IK")
	void StartHandIKScriptedAnimation();

protected:
	void storeCurrentPoseRotations(TArray<FRotator>& storedPose);
	void waving_initializeStartingPose();
	void waving_tickAnimation();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
