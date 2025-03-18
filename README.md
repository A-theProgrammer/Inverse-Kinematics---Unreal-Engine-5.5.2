# Poseable Character with FABRIK Inverse Kinematics Animation

This project is an Unreal Engine assignment focused on creating a poseable character with scripted animations. The project demonstrates a character with an animated arm movement using Inverse Kinematics (IK), specifically implementing the FABRIK algorithm for a 3-bone IK chain. The implementation includes several features that allow real-time modifications of a skeletal mesh, smooth scripted animations along a spline path, and advanced controls such as joint limits and motion capture (mocap) integration.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Implementation Details]
  - [Poseable Mesh](#poseable-mesh)
  - [Multi-Bone IK using FABRIK](#multi-bone-ik-using-fabrik)
  - [Scripted Animation](#scripted-animation)
  - [Advanced Features](#advanced-features)
- [Setup and Installation](#setup-and-installation)
- [Usage Instructions](#usage-instructions)
- [License](#license)

---

## Overview

The primary goal of this project is to create a poseable character in Unreal Engine that demonstrates complex animation techniques:

- **Poseable Mesh:** A dynamic skeletal mesh that can be modified at runtime.
- **Inverse Kinematics (FABRIK):** Implementation of a 3-bone IK chain (upper arm, lower arm, hand) to animate the arm based on a target position.
- **Scripted Animation:** An animation system that drives the IK target along a predefined spline path with ease-in/ease-out effects.
- **Advanced Features:** Incorporates joint limits (e.g., for the elbow) and the ability to integrate motion capture data.

---

## Features

### Poseable Mesh

- **Description:**  
  Uses Unreal Engine's `UPoseableMeshComponent` to enable real-time modifications of the character's skeletal mesh.

- **Implementation:**  
  - The poseable mesh is initialized from a source skeletal mesh.
  - The mesh is attached to the character's root component.
  - A function is implemented to rotate a specific bone by name.

- **Pseudocode:**
  ```cpp
  // PSEUDO CODE: Poseable Mesh Setup
  Create PoseableMeshComponent
  Attach PoseableMeshComponent to RootComponent

  Function RotateBone(BoneName, Rotation):
      If PoseableMesh exists:
          Set bone rotation of BoneName to Rotation in world space
  ```

### Multi-Bone IK using FABRIK

- **Description:**  
  Implements the FABRIK algorithm for a three-bone chain (upper arm, lower arm, hand) to adjust the character’s arm movement toward a target position.

- **Implementation:**  
  - Retrieves the current positions of the three bones.
  - Calculates the lengths of the bone segments.
  - Determines whether the target is within reach or if the arm needs to be straightened.
  - Executes a forward and backward pass to adjust the bone positions.

- **Pseudocode:**
  ```cpp
  // PSEUDO CODE: FABRIK IK Solver
  Function SolveFABRIK(RootBone, MidBone, EndBone, TargetPosition):
      Get position of RootBone, MidBone, EndBone
      Calculate Length1 (Root to Mid) and Length2 (Mid to End)
      TotalLength = Length1 + Length2
      
      If distance(RootBone, TargetPosition) > TotalLength:
          // Target is too far; straighten the arm
          Set MidBone position = RootBone position + (direction to Target * Length1)
          Set EndBone position = TargetPosition
      Else:
          // FABRIK forward pass:
          Set EndBone position = TargetPosition
          Adjust MidBone position based on EndBone and bone lengths
          Adjust RootBone position based on MidBone and bone lengths
          // FABRIK backward pass:
          Recalculate MidBone and EndBone positions accordingly
      
      Update PoseableMesh positions for MidBone and EndBone
  ```

### Scripted Animation

- **Description:**  
  Uses a spline path to animate the IK target along a predefined route, ensuring smooth transitions with ease-in/ease-out effects.

- **Implementation:**  
  - A spline component is attached to the character.
  - The animation progresses over time by incrementing an animation progress variable.
  - The current target position is fetched from the spline based on the progress.
  - The IK solver is then called to adjust the arm based on the new target position.

- **Pseudocode:**
  ```cpp
  // PSEUDO CODE: Scripted Animation
  Create SplinePathComponent and attach it to RootComponent
  Initialize AnimationProgress to 0

  Function AnimateTarget(DeltaTime):
      Increase AnimationProgress by (DeltaTime * speedFactor)
      Clamp AnimationProgress between 0 and 1
      Get NewTargetPos from SplinePathComponent based on AnimationProgress
      Call SolveFABRIK with NewTargetPos for arm bones
  ```

### Advanced Features

- **Description:**  
  Enhances the animation system by adding joint limits to restrict bone rotations (e.g., limiting elbow bending) and supports the integration of motion capture data.

- **Implementation:**  
  - A function applies joint limits (such as clamping the elbow's rotation).
  - A separate function iterates through mocap data and applies the corresponding rotations to the bones while enforcing joint limits.

- **Pseudocode:**
  ```cpp
  // PSEUDO CODE: Advanced Features
  Function ApplyJointLimits(BoneName, OutRotation):
      If BoneName equals "Elbow":
          Clamp OutRotation.Pitch between -90 and 0 degrees
          Return true
      Else:
          Return false

  Function ApplyMocapData(MocapRotations):
      For each Bone in MocapRotations:
          Retrieve Rotation for Bone
          Apply joint limits to Rotation using ApplyJointLimits
          Set bone rotation of Bone in PoseableMesh to adjusted Rotation in world space
  ```

---

## Setup and Installation

Follow these step-by-step instructions to download and run the project on your local system:

1. **Install Git:**  
   Ensure that Git is installed on your machine. You can download it from [git-scm.com](https://git-scm.com/).

2. **Clone the Repository:**  
   Open your terminal or command prompt and run:
   ```bash
   git clone https://github.com/yourusername/your-repo-name.git
   ```
   Replace `yourusername/your-repo-name` with the actual repository URL.

3. **Install Unreal Engine:**  
   Make sure you have Unreal Engine installed. You can download it from the [Epic Games Launcher](https://www.unrealengine.com/download).

4. **Open the Project in Unreal Engine:**  
   - Launch Unreal Engine.
   - Click on **Browse** in the Unreal Project Browser.
   - Navigate to the cloned repository folder and select the project file (e.g., `.uproject` file).

5. **Build the Project:**  
   - Once the project is open, allow Unreal Engine to compile the necessary files.
   - If prompted, configure any project settings as needed.

6. **Run the Project:**  
   - Click **Play** in the Unreal Editor to run the project.
   - Observe the character with the scripted arm movement and the IK-based animations.

---

## Usage Instructions

- **Animating the Character:**  
  The project automatically animates the arm based on the spline path animation. You can modify the spline path and adjust animation parameters to see different effects.

- **Modifying Bone Rotations:**  
  Use the provided functions (illustrated in pseudocode) to change bone rotations dynamically. This is useful for integrating additional input, such as mocap data or custom animations.

- **Exploring Advanced Features:**  
  Experiment with joint limits and mocap integration to understand how the system restricts and applies realistic bone rotations.

---

## License

This project is provided under the MIT License. See the [LICENSE](LICENSE) file for more information.

---

This README provides a detailed explanation of the project, its implementation, and setup instructions. If you have any questions, feel free to open an issue on the repository.
