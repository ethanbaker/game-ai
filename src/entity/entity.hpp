// entity represents an entity (moveable or immovable) in the scene
#ifndef ENTITY
#define ENTITY

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../utils/kinematics/kinematics.hpp"
#include "../steering/steering.hpp"
#include "../breadcrumbs/breadcrumbs.hpp"
#include "../utils/tree/decision-tree/decision-tree.hpp"
#include "../utils/tree/behavior-tree/behavior-tree.hpp"
#include "../environment/environment.hpp"

// Forward declaration for Engine class
class Engine;

// StatePacket represents a packet of information this entity represented in the game at a given moment in time
class StatePacket {
    public:
        int timestamp;    
        std::string action;
        float x;
        float y;
        float characterX;
        float characterY;
        float obstacleDistTop;
        float obstacleDistRight;
        float obstacleDistBottom;
        float obstacleDistLeft;

        // Get the value of an attribute as a string
        std::string getValue(std::string attribute) {
            if (attribute == "timestamp") {
                return std::to_string(this->timestamp);
            }
            if (attribute == "action") {
                return this->action;
            }
            if (attribute == "x") {
                return std::to_string(this->x);
            }
            if (attribute == "y") {
                return std::to_string(this->y);
            }
            if (attribute == "characterX") {
                return std::to_string(this->characterX);
            }
            if (attribute == "characterY") {
                return std::to_string(this->characterY);
            }
            if (attribute == "obstacleTop") {
                return std::to_string(this->obstacleDistTop);
            }
            if (attribute == "obstacleRight") {
                return std::to_string(this->obstacleDistRight);
            }
            if (attribute == "obstacleBottom") {
                return std::to_string(this->obstacleDistBottom);
            }
            if (attribute == "obstacleLeft") {
                return std::to_string(this->obstacleDistLeft);
            }
            return "";
        }
};

// Entity class extends sf::sprite to contain update functions and more kinematic information
class Entity : public sf::Sprite, public Kinematics {
    private:
        // Attributes
        Engine *engine;
        std::vector<WeightedBehavior> behaviors;

        std::vector<Breadcrumb> breadcrumbs;
        float breadcrumbTimer = 0;
        int currentBreadcrumb = 0;

        AbstractDMTree *decisionMakingTree = nullptr;
        std::string currentAction = "No action selected";

        bool invisible = false;

        // Helper methods
        void nextAccelerations();

    public:
        Entity();
        Entity(sf::Vector2f position);

        // Set the engine the entity is in
        void setEngine(Engine *engine);

        // Set the current movement behaviors
        void setBehaviors(std::vector<WeightedBehavior> behaviors);

        // Set a decision tree for the entity
        void setDecisionMakingTree(AbstractDMTree *tree);

        // Set the invisible status
        void setInvisibility(bool invisibility);

        // Get the invisibility status
        bool isInvisible();

        // Set the current action of the entity
        void setCurrentAction(std::string action);

        // Return a packet of state information to the recording file
        StatePacket getStatePacket(int timestamp);

        // Get various movement information from the entity
        Kinematics getKinematics();
        Target getTarget();

        // Update the entity's movement information
        void update(sf::Time dt);

        // Draw a breadcrumb at the entity's location
        void drawBreadcrumb(float dt);

        // Perform a decision from the entity's decision tree
        void decide(EnvironmentParameters *environment);

};

#endif