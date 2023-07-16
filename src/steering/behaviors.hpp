#ifndef BEHAVIORS
#define BEHAVIORS

#include <math.h>
#include <random>
#include <iostream>
#include "steering.hpp"
#include "../utils/vmath/vmath.hpp"
#include "../engine/engine.hpp"
#include "../utils/algorithm/heuristic.hpp"
#include "../utils/graph/graph.hpp"

// AlignToVelocity behavior, which aligns an orientation to direction of velocity
class AlignToVelocity : public WeightedBehavior {
    private:
        // Parameters for AlignToVelocity
        float minSpeed = 0;
        float minOrientation = 0;
        float timeToStop = 0;

    public:
        AlignToVelocity(float minSpeed, float minOrientation, float timeToStop, Align alignBehavior) {
            // Setup the parameters
            this->minSpeed = minSpeed;
            this->minOrientation = minOrientation;
            this->timeToStop = timeToStop;

            // Setup the behavior function
            this->behavior = [this, alignBehavior](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                // Find the orientation in the direction of motion
                float x = character.linearVelocity.x;
                float y = character.linearVelocity.y;

                if (x == 0) {
                    // Don't align if x is 0
                    Accelerations a;
                    return a;
                } else if (x < 0) {
                    // Find the angle on the opposite side (as atan only returns [-90, 90])
                    float angle = atan(y / x) * 180 / M_PI;
                    params.target.orientation = angle - 180 * (angle / abs(angle));
                } else {
                    // Find the angle using atan
                    params.target.orientation = atan(y / x) * 180 / M_PI;
                }

                // If the character is already aligned or not moving, stop rotating
                if (abs(params.target.orientation - character.orientation) < this->minOrientation || Vmath::length(character.linearVelocity) < this->minSpeed) {
                    params.target.angularVelocity = 0;
                    RotationMatch matcher = RotationMatch(this->timeToStop);
                    return matcher.find(params);
                }

                // Otherwise, align the behavior
                return alignBehavior.find(params);
            };
        }
};

// Wander behavior, which randomly arrives at a certain position
class Wander : public WeightedBehavior {
    private:
        // Variables used by wander to timekeep
        sf::Clock clk;
        sf::Time timeSinceLastUpdate = sf::Time::Zero;
        sf::Vector2f targetPosition;
        bool firstWander = false;

        // Parameters for wander
        float wanderOffset;
        float wanderRadius;
        float wanderRate;
        sf::Time interval;
        Settings *settings;
        float maxWidth;
        float maxHeight;
    
    public:
        Wander(float wanderOffset, float wanderRadius, float wanderRate, Arrive arriveBehavior, sf::Time interval, Settings *settings, float maxWidth, float maxHeight) {
            // Initialize parameters
            this->wanderOffset = wanderOffset;
            this->wanderRadius = wanderRadius;
            this->wanderRate = wanderRate;
            this->interval = interval;
            this->settings = settings;
            this->maxWidth = maxWidth;
            this->maxHeight = maxHeight;

            // Set up the behavior function
            this->behavior = [this, arriveBehavior](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                // Determine if it is time to update accelerations
                this->timeSinceLastUpdate += this->clk.restart();

                if (this->timeSinceLastUpdate > this->interval || !this->firstWander) {
                    this->firstWander = true;
                    this->timeSinceLastUpdate = sf::Time::Zero;

                    // Find the target orientation for the wander
                    float wanderOrientation = Vmath::randomBinomial(360) * this->wanderRate;
                    float targetOrientation = params.character.orientation + wanderOrientation;

                    // Find the target position for the wander
                    this->targetPosition = params.character.position + this->wanderOffset * Vmath::orientationToVector(params.character.orientation);
                    this->targetPosition += this->wanderRadius * Vmath::orientationToVector(targetOrientation);
                }

                // Make sure the position fits in the game world
                this->targetPosition.x = abs(fmod(this->targetPosition.x, this->maxWidth));
                this->targetPosition.y = abs(fmod(this->targetPosition.y, this->maxHeight));

                params.target.position = this->targetPosition;

                // Go to the target position with arrive
                return arriveBehavior.find(params);
            };
        }
};

// Collision Avoidance, which avoids other moving entities
class CollisionAvoidance : public WeightedBehavior {
    private:
        // Parameters
        Engine *engine;
        float distanceThreshold;
        float timeThreshold;

    public:
        CollisionAvoidance(Engine *engine, float distanceThreshold, float timeThreshold, Flee fleeBehavior) {
            this->engine = engine;
            this->distanceThreshold = distanceThreshold;
            this->timeThreshold = timeThreshold;

            this->behavior = [this, fleeBehavior](Target character) mutable -> Accelerations {
                Accelerations a;

                // Get the closest target
                std::vector<Entity> closestEntities = this->engine->getClosestEntities(1, character);
                if (closestEntities.size() == 0) {
                    return a;
                }

                Target closest = closestEntities.at(0).getTarget();

                // Get the relative position
                sf::Vector2f dPos = character.position - closest.position;
                float distance = Vmath::length(dPos);

                // Stop if dVel is zero
                sf::Vector2f dVel = character.linearVelocity - closest.linearVelocity;
                float relSpeed = Vmath::length(dVel);
                if (relSpeed == 0) {
                    return a;
                }

                // Calculate the time of closest approach
                float timeOfClosestApproach = -Vmath::dotProduct(dPos, dVel) / (relSpeed * relSpeed);

                // Get positions at closest approach
                sf::Vector2f closestPosCharacter = character.position + character.linearVelocity * timeOfClosestApproach;
                sf::Vector2f closestPosClosest = closest.position + closest.linearVelocity * timeOfClosestApproach;

                // Determine the distance of closest approach
                float separation = Vmath::length(closestPosCharacter - closestPosClosest);

                Params params;
                if (distance < 2 * this->distanceThreshold) {
                    // Avoid using current positions if distance is too small
                    params.character.position = character.position;
                    params.target.position = closest.position;
                } else if (timeOfClosestApproach > 0 && timeOfClosestApproach < this->timeThreshold && separation < 2 * this->distanceThreshold) {
                    // Otherwise, avoid future location
                    params.character.position = closestPosCharacter;
                    params.target.position = closestPosClosest;
                } else {
                    return a;
                }

                return fleeBehavior.find(params);
            };
        }
};

// VelocityMatchToCenter matches the characters velocity to a center of a flock
class VelocityMatchToCenter : public WeightedBehavior {
    private:
        Engine *engine;
        float flockRadius;

    public:
        VelocityMatchToCenter(Engine *engine, float flockRadius, VelocityMatch velocityMatchBehavior) {
            this->engine = engine;
            this->flockRadius = flockRadius;

            this->behavior = [this, velocityMatchBehavior](Target character) mutable -> Accelerations {
                // Get the entities in a given radius
                std::vector<Entity> entities = this->engine->getEntitiesInRadius(this->flockRadius, character);

                // Calculate the flock's velocity
                sf::Vector2f velocity = sf::Vector2f(0, 0);
                for (Entity e : entities) {
                    velocity += e.linearVelocity;
                }
                velocity = velocity / (float) entities.size();

                // Delegate to VelocityMatching for that velocity
                Params params;
                params.character = character;
                params.target.linearVelocity = velocity;

                return velocityMatchBehavior.find(params);
            };
        }
};

// PositionMatchToCenter matches the characters position to a center of a flock
class PositionMatchToCenter : public WeightedBehavior {
    private:
        Engine *engine;
        float flockRadius;

    public:
        PositionMatchToCenter(Engine *engine, float flockRadius, Arrive positionMatchBehavior) {
            this->engine = engine;
            this->flockRadius = flockRadius;

            this->behavior = [this, positionMatchBehavior](Target character) mutable -> Accelerations {
                // Get the entities in a given radius
                std::vector<Entity> entities = this->engine->getEntitiesInRadius(this->flockRadius, character);

                // Calculate the flock's center position
                sf::Vector2f position = sf::Vector2f(0, 0);
                for (Entity e : entities) {
                    position += e.getPosition();
                }
                position = position / (float) entities.size();

                // Delegate to a position matching behavior
                Params params;
                params.character = character;
                params.target.position = position;

                return positionMatchBehavior.find(params);
            };
        }
};

// ConstantVelocityMatch matches a velocity to a constant value
class ConstantVelocityMatch : public WeightedBehavior {
    private:
        // Behavior parameters
        float speed = 0;
        float timeToSpeed = 0;

    public:
        ConstantVelocityMatch(float speed, float timeToSpeed) {
            this->speed = speed;
            this->timeToSpeed = timeToSpeed;

            this->behavior = [this](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                // Catch if character isn't moving
                if (Vmath::length(character.linearVelocity) == 0) {
                    params.target.linearVelocity = Vmath::orientationToVector(character.orientation) * this->speed;
                } else {
                    params.target.linearVelocity = Vmath::normalize(character.linearVelocity) * this->speed;
                }

                VelocityMatch matcher = VelocityMatch(this->timeToSpeed);
                return matcher.find(params);
            };
        }
};

// ConstantRotationMatch matches a rotation to a constant value
class ConstantRotationMatch : public WeightedBehavior {
    private:
        // Behavior parameters
        float speed = 0;
        float timeToSpeed = 0;

    public:
        ConstantRotationMatch(float speed, float timeToSpeed) {
            this->speed = speed;
            this->timeToSpeed = timeToSpeed;

            this->behavior = [this](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                params.target.angularVelocity = this->speed;

                RotationMatch matcher = RotationMatch(this->timeToSpeed);
                return matcher.find(params);
            };
        }
};


// PathfindToMouse pathfinds to each mouse click
class PathfindToMouse : public WeightedBehavior {
    private:
        // Variables for Pathfind
        sf::Vector2f lastClicked;
        std::vector<Edge<int>*> path;
        int currentIndex = 0;

        // Attributes
        float predictTime;

    public:
        PathfindToMouse(Engine *engine, SteeringBehavior *behavior, Heuristic<Grid<int>, int> *heuristic, float predictTime) {
            this->predictTime = predictTime;

            // Set up the behavior function
            this->behavior = [this, engine, behavior, heuristic](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                // Get the last clicked position of the mouse
                sf::Vector2f clicked = engine->mouse->getLastClicked();

                if (clicked.x == -1) {
                    // If the user hasn't clicked, don't go anywhere
                    return Accelerations();
                } else if (this->lastClicked != clicked) {
                    // The user has clicked somewhere new
                    this->lastClicked = clicked;
                    this->currentIndex = 0;

                    // Pathfind to that location (if it is possible)
                    this->path = engine->pathfind(character.position, this->lastClicked, heuristic);
                }

                // Don't path follow empty paths
                if (this->path.size() == 0) {
                    return Accelerations();
                }

                // Find the characters future position
                sf::Vector2f futurePosition = character.position + character.linearVelocity * this->predictTime;

                // Find the closest point on the path to the character's future position
                float minDistance = std::numeric_limits<float>::max();
                int minIndex = this->currentIndex;

                for (typename std::vector<Edge<int>*>::size_type i = this->currentIndex; i < this->path.size(); i++) {
                    float distance = Vmath::length(engine->getEnvironment()->localizeEndpoint(this->path.at(i), 0) - futurePosition);
                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = i;
                    }
                }
                this->currentIndex = minIndex;

                params.target.position = engine->getEnvironment()->localizeEndpoint(this->path.at(this->currentIndex), 1);
                return behavior->find(params);
            };
        }
};

// PathfindToPosition pathfinds to a given position
class PathfindToPositionOld : public WeightedBehavior {
    private:
        // Variables for Pathfind
        sf::Vector2f targetPosition;
        std::vector<Edge<int>*> path;
        int currentIndex = 0;

        // Attributes
        float predictTime;

    public:
        PathfindToPositionOld(Engine *engine, SteeringBehavior *behavior, Heuristic<Grid<int>, int> *heuristic, float predictTime, sf::Vector2f targetPosition) {
            this->predictTime = predictTime;
            this->targetPosition = targetPosition;

            // Set up the behavior function
            this->behavior = [this, engine, behavior, heuristic](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                // Pathfind to that location (if it is possible)
                if (this->path.size() == 0) {
                    this->currentIndex = 0;

                    this->path = engine->pathfind(character.position, this->targetPosition, heuristic);
                    if (this->path.size() == 0) {
                        return Accelerations();
                    }
                }

                // Find the characters future position
                sf::Vector2f futurePosition = character.position + character.linearVelocity * this->predictTime;

                // Find the closest point on the path to the character's future position
                float minDistance = std::numeric_limits<float>::max();
                int minIndex = this->currentIndex;

                for (typename std::vector<Edge<int>*>::size_type i = this->currentIndex; i < this->path.size(); i++) {
                    float distance = Vmath::length(engine->getEnvironment()->localizeEndpoint(this->path.at(i), 0) - futurePosition);
                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = i;
                    }
                }
                this->currentIndex = minIndex;

                params.target.position = engine->getEnvironment()->localizeEndpoint(this->path.at(this->currentIndex), 1);
                return behavior->find(params);
            };
        }
};

// PathfindToPosition pathfinds to a given position
class PathfindToPosition : public WeightedBehavior {
    private:
        // Variables for Pathfind
        sf::Vector2f targetPosition;
        std::vector<Edge<int>*> path;
        int currentIndex = 0;
        bool calculatedPath = false;

        // Attributes
        float predictTime;

    public:
        PathfindToPosition() {}

        PathfindToPosition(Engine *engine, SteeringBehavior *behavior, Heuristic<Grid<int>, int> *heuristic, float predictTime, sf::Vector2f targetPosition) {
            this->predictTime = predictTime;

            // Pathfind to that location (if it is possible)
            this->currentIndex = 0;

            // Set up the behavior function
            this->behavior = [this, engine, behavior, heuristic](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                if (!this->calculatedPath) {
                    this->calculatedPath = true;
                    this->currentIndex = 0;
                    this->path = engine->pathfind(character.position, this->targetPosition, heuristic);
                }

                if (this->path.size() == 0) {
                    return Accelerations();
                }

                // Find the characters future position
                sf::Vector2f futurePosition = character.position + character.linearVelocity * this->predictTime;

                // Find the closest point on the path to the character's future position
                float minDistance = std::numeric_limits<float>::max();
                int minIndex = this->currentIndex;

                for (typename std::vector<Edge<int>*>::size_type i = this->currentIndex; i < this->path.size(); i++) {
                    float distance = Vmath::length(engine->getEnvironment()->localizeEndpoint(this->path.at(i), 0) - futurePosition);
                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = i;
                    }
                }
                this->currentIndex = minIndex;

                params.target.position = engine->getEnvironment()->localizeEndpoint(this->path.at(this->currentIndex), 1);
                return behavior->find(params);
            };
        }

        void reset(sf::Vector2f position) {
            this->calculatedPath = false;
            this->targetPosition = position;
        }
};

// PathfindToMultiplePosition pathfinds to multiple positions
class PathfindToMultiplePosition : public WeightedBehavior {
    private:
        // Variables for Pathfind
        int positionIndex = 0;

        std::vector<Edge<int>*> path;
        int currentIndex = 0;
        bool calculatedPath = false;

        // Attributes
        float predictTime;

    public:
        PathfindToMultiplePosition(Engine *engine, SteeringBehavior *behavior, Heuristic<Grid<int>, int> *heuristic, float predictTime, std::vector<sf::Vector2f*> *targetPositions) {
            this->predictTime = predictTime;

            // Set up the behavior function
            this->behavior = [this, engine, behavior, targetPositions, heuristic](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                if (!this->calculatedPath || this->currentIndex + 1 == (int) this->path.size()) {
                    this->calculatedPath = true;
                    this->currentIndex = 0;
                    this->positionIndex = (this->positionIndex + 1) % targetPositions->size();
                    this->path = engine->pathfind(character.position, *(targetPositions->at(this->positionIndex)), heuristic);
                }

                if (this->path.size() == 0) {
                    return Accelerations();
                }

                // Find the characters future position
                sf::Vector2f futurePosition = character.position + character.linearVelocity * this->predictTime;

                // Find the closest point on the path to the character's future position
                float minDistance = std::numeric_limits<float>::max();
                int minIndex = this->currentIndex;

                for (typename std::vector<Edge<int>*>::size_type i = this->currentIndex; i < this->path.size(); i++) {
                    float distance = Vmath::length(engine->getEnvironment()->localizeEndpoint(this->path.at(i), 0) - futurePosition);
                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = i;
                    }
                }
                this->currentIndex = minIndex;

                params.target.position = engine->getEnvironment()->localizeEndpoint(this->path.at(this->currentIndex), 1);
                return behavior->find(params);
            };
        }

        void reset() {
            this->calculatedPath = false;
        }
};

// PathfindToRandomPosition pathfinds to multiple positions
class PathfindToRandomPosition : public WeightedBehavior {
    private:
        // Variables for Pathfind

        std::vector<Edge<int>*> path;
        int currentIndex = 0;
        bool calculatedPath = false;

        // Attributes
        float predictTime;

    public:
        PathfindToRandomPosition(Engine *engine, SteeringBehavior *behavior, Heuristic<Grid<int>, int> *heuristic, float predictTime, std::vector<sf::Vector2f*> *targetPositions) {
            this->predictTime = predictTime;

            // Set up the behavior function
            this->behavior = [this, engine, behavior, targetPositions, heuristic](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;

                if (!this->calculatedPath || this->currentIndex + 1 == (int) this->path.size()) {
                    this->calculatedPath = true;
                    this->currentIndex = 0;
                    int randomIndex = rand() % targetPositions->size();
                    this->path = engine->pathfind(character.position, *(targetPositions->at(randomIndex)), heuristic);
                }

                if (this->path.size() == 0) {
                    return Accelerations();
                }

                // Find the characters future position
                sf::Vector2f futurePosition = character.position + character.linearVelocity * this->predictTime;

                // Find the closest point on the path to the character's future position
                float minDistance = std::numeric_limits<float>::max();
                int minIndex = this->currentIndex;

                for (typename std::vector<Edge<int>*>::size_type i = this->currentIndex; i < this->path.size(); i++) {
                    float distance = Vmath::length(engine->getEnvironment()->localizeEndpoint(this->path.at(i), 0) - futurePosition);
                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = i;
                    }
                }
                this->currentIndex = minIndex;

                params.target.position = engine->getEnvironment()->localizeEndpoint(this->path.at(this->currentIndex), 1);
                return behavior->find(params);
            };
        }

        void reset() {
            this->calculatedPath = false;
        }
};

#endif