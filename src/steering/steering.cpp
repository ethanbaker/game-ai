#include <SFML/Graphics.hpp>
#include <iostream>
#include "steering.hpp"
#include "../utils/kinematics/kinematics.hpp"
#include "../utils/vmath/vmath.hpp"

/** VelocityMatch Behavior */

// Find the kinematic solution that matches velocity with the target's
Accelerations VelocityMatch::find(Params params) {
    Accelerations result;

    // Calculate acceleration to reach target velocity in given time
    result.linearAcceleration = params.target.linearVelocity - params.character.linearVelocity;
    result.linearAcceleration /= this->timeToTargetVelocity;

    return result;
}

/** RotationMatch Behavior */
Accelerations RotationMatch::find(Params params) {
    Accelerations result;

    // Calculate acceleration to reach target velocity in given time
    result.angularAcceleration = params.target.angularVelocity - params.character.angularVelocity;
    result.angularAcceleration /= this->timeToTargetVelocity;

    return result;
}

/** Align (Orientation Match) Behavior */

// Find a kinematic solution to rotate the character in the direction of motion
Accelerations Align::find(Params params) {
    Accelerations a;

    // Find the distance (in radians) between the target and character orientation
    float rotation = Vmath::mapOrientationToRange(params.target.orientation - params.character.orientation);
    float rotationSize = abs(rotation);

    // Find the goal rotation we want
    float goalRotation;
    if (rotationSize < radiusOfSatisfaction) {
        return a;
    } else if (rotationSize < radiusOfDeceleration) {
        goalRotation = maxRotation * (rotationSize / radiusOfDeceleration);
    } else {
        goalRotation = maxRotation;
    }
    goalRotation *= rotation / rotationSize;

    // Use goal rotation to find the angular acceleration
    a.angularAcceleration = (goalRotation - params.character.angularVelocity) / timeToTargetRotation;

    return a;
}

/** Arrive (Position Match) Behavior */

// Find a kinematic solution to get the character onto the target
Accelerations Arrive::find(Params params) {
    Accelerations a;

    // Find the distance to the target position
    sf::Vector2f dPos = params.target.position - params.character.position;
    float len = Vmath::length(dPos);

    // Return if the length is 0
    if (len == 0) {
        return a;
    }

    // Normalize the vector
    sf::Vector2f uPos = Vmath::normalize(dPos);

    // Find our target speed based on what radius we are in
    float targetSpeed;
    if (len < radiusOfSatisfaction) {
        targetSpeed = 0;
    } else if (len < radiusOfDeceleration) {
        targetSpeed = len / radiusOfDeceleration * maxVelocity;
    } else {
        targetSpeed = maxVelocity;
    }

    // Turn the target speed into a target acceleration
    sf::Vector2f goalVelocity = uPos * targetSpeed;

    a.linearAcceleration = (goalVelocity - params.character.linearVelocity) / timeToTargetVelocity;

    return a;
}

/** Flee Behavior */

// Accelerate in the opposite direction of a target
Accelerations Flee::find(Params params) {
    Accelerations a;

    // Get the direction to the target
    sf::Vector2f direction = -Vmath::normalize(params.target.position - params.character.position);

    // Get the acceleration
    a.linearAcceleration = direction * this->maxVelocity / this->timeToTargetVelocity;

    return a;
}

/** Seek Behavior */

// Accelerate in the opposite direction of a target
Accelerations Seek::find(Params params) {
    Accelerations a;

    // Get the direction to the target
    sf::Vector2f direction = Vmath::normalize(params.target.position - params.character.position);

    // Get the acceleration
    a.linearAcceleration = direction * this->maxVelocity / this->timeToTargetVelocity;

    return a;
}