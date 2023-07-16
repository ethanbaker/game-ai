// pure virtual steering class to be implemented later
#ifndef STEERING
#define STEERING

#include <SFML/Graphics.hpp>
#include "../utils/kinematics/kinematics.hpp"

// Pure virtual class SteeringBehavior for other steering behavior classes to implement
class SteeringBehavior {
    public:
        // Find a kinematic solution for given parameters
        virtual Accelerations find(Params params) = 0;
};

// WeightedBehavior is a struct containing a steering behavior and an associated weight
struct WeightedBehavior {
    public:
        // User set
        std::function <Accelerations(Target)> behavior;
        float weight = 0;
        sf::Time interval = sf::Time::Zero;

        // Used for acceleration keeping
        sf::Time elapsed = sf::Time::Zero;
        Accelerations curAccelerations;
};

// VelocityMatch matches the velocity of the character to the velocity of the target
class VelocityMatch : public SteeringBehavior {
    // Behavior Parameters
    private:
        float timeToTargetVelocity; // Time it takes for the character to reach the target velocity

    public:
        // Constructor initializes parameters
        VelocityMatch(float timeToTargetVelocity) {
            this->timeToTargetVelocity = timeToTargetVelocity;
        }
        Accelerations find(Params params);
};

// RotationMatch matches the angular velocity of the character to the angular velocity of the target
class RotationMatch : public SteeringBehavior {
    // Behavior Parameters
    private:
        float timeToTargetVelocity; // Time it takes for the character to reach the target velocity

    public:
        // Constructor initializes parameters
        RotationMatch(float timeToTargetVelocity) {
            this->timeToTargetVelocity = timeToTargetVelocity;
        }
        Accelerations find(Params params);
};

// Align matches the rotation of the character to the direction of its motion
class Align : public SteeringBehavior {
    // Behavior Parameters
    private:
        float radiusOfSatisfaction; // Radius to determine when small position errors should be ignored
        float radiusOfDeceleration; // Radius to determine when to be slowing down
        float maxRotation; // Maximum rotation of the character
        float timeToTargetRotation; // Unit of time to find target acceleration in order to achieve target rotation

    public:
        // Constructor initializes parameters
        Align(float radiusOfSatisfaction, float radiusOfDeceleration, float maxRotation, float timeToTargetRotation) {
            this->radiusOfSatisfaction = radiusOfSatisfaction;
            this->radiusOfDeceleration = radiusOfDeceleration;
            this->maxRotation = maxRotation;
            this->timeToTargetRotation = timeToTargetRotation;
        }

        Accelerations find(Params params);
};

// Arrive matches the position of a given target by stopping directly on it
class Arrive : public SteeringBehavior {
    // Behavior Parameters
    private:
        float radiusOfSatisfaction; // Radius to determine when small position errors should be ignored
        float radiusOfDeceleration; // Radius to determine when to be slowing down
        float maxVelocity; // Maximum velocity of the character
        float timeToTargetVelocity; // Unit of time to find target acceleration in order to achieve target velocity

    public:
        // Constructor initializes parameters
        Arrive(float radiusOfSatisfaction, float radiusOfDeceleration, float maxVelocity, float timeToTargetVelocity) {
            this->radiusOfSatisfaction = radiusOfSatisfaction;
            this->radiusOfDeceleration = radiusOfDeceleration;
            this->maxVelocity = maxVelocity;
            this->timeToTargetVelocity = timeToTargetVelocity;
        }

        Accelerations find(Params params);
};

// Flee moves in the opposite direction of the target
class Flee : public SteeringBehavior {
    // Behavior Parameters
    private:
        float maxVelocity; // Maximum velocity of the character
        float timeToTargetVelocity; // Unit of time to find target acceleration in order to achieve target velocity

    public:
        // Constructor initializes parameters
        Flee(float maxVelocity, float timeToTargetVelocity) {
            this->maxVelocity = maxVelocity;
            this->timeToTargetVelocity = timeToTargetVelocity;
        }

        Accelerations find(Params params);
};

// Seek moves in the same direction as the target
class Seek : public SteeringBehavior {
    // Behavior Parameters
    private:
        float maxVelocity; // Maximum velocity of the character
        float timeToTargetVelocity; // Unit of time to find target acceleration in order to achieve target velocity

    public:
        // Constructor initializes parameters
        Seek(float maxVelocity, float timeToTargetVelocity) {
            this->maxVelocity = maxVelocity;
            this->timeToTargetVelocity = timeToTargetVelocity;
        }

        Accelerations find(Params params);
};

#endif