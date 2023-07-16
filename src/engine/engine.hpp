#ifndef ENGINE
#define ENGINE

#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include "../entity/entity.hpp"
#include "../utils/array/array.hpp"
#include "../mouse/mouse.hpp"
#include "../utils/graph/graph.hpp"
#include "../environment/environment.hpp"
#include "../utils/algorithm/heuristic.hpp"

// Settings struct helps to hold game settings
struct Settings {
    public:
        sf::Color background; // Color of the game background

        int fps; // Frames per second
        sf::Time timePerFrame; // Time per frame

        float maxLinearVelocity; // Maximum linear velocity
        float maxAngularVelocity; // Maximum angular velocity
        float maxLinearAcceleration; // Maximum linear acceleration
        float maxAngularAcceleration; // Maximum angular acceleration

        int width; // Width of the game
        int height; // Height of the game

        float mouseInterval = 0;

        int breadcrumbsPerEntity = 0;
        float breadcrumbInterval = 0;
        float breadcrumbRadius = 10.0f;

        int xTiles;
        int yTiles;

        sf::Time timePerDecision; // Time per decision make
};

// Direction represents one of the four cardinal 2D directions
enum Direction {
    Top,
    Right,
    Bottom,
    Left
};

// Recording represents a given entity that should have their state information recorded to a file
class Recording {
    public:
        std::ofstream file;
        Entity *entity;

        Recording(std::string filepath, Entity *entity) {
            this->entity = entity;

            this->file.open(filepath);
            this->file << "timestamp,action,x,y,char_x,char_y,obstacle_top,obstacle_right,obstacle_bottom,obstacle_left\n";
        }

        void writePacket(int timestamp) {
            StatePacket packet = this->entity->getStatePacket(timestamp);
            this->file << timestamp << ",";
            this->file << packet.action << ",";
            this->file << packet.x << ",";
            this->file << packet.y << ",";
            this->file << packet.characterX << ",";
            this->file << packet.characterY << ",";
            this->file << packet.obstacleDistTop << ",";
            this->file << packet.obstacleDistRight << ",";
            this->file << packet.obstacleDistBottom << ",";
            this->file << packet.obstacleDistLeft << "\n";
        }
};

// Engine represents the physics engine of a scene
class Engine : public sf::RenderWindow {
    private:
        // List of entities in the scene
        std::vector<Entity*> entities;

        // List of file recorders
        std::vector<Recording*> recordings;

        // The player character
        Entity *playerCharacter;

        // Game environment the engine is running on
        GridEnvironment environment;

        // Variables of the game state
        std::map<std::string, void*> stateVariables;

        // Timekeeping variables for updating behaviors
        sf::Clock behaviorUpdateClk;
        sf::Time timeSinceLastBehaviorUpdate = sf::Time::Zero;

        // Timekeeping incremental varaible
        int timestamp = 0;

        // Methods to handle running game
        void handleEvents();
        void update(sf::Time dt);
        void render();

    public:
        Engine(std::string title, Settings *settings);
        Settings *settings;
        Mouse *mouse;

        // Setup methods
        void setMouse(Mouse *mouse);
        void setPlayableCharacter(Entity *entity);
        Entity *getPlayableCharacter();
        void setStateVariable(std::string key, void* value);
        void *getStateVariable(std::string key);
        int getTimestamp();
        void newEntity(Entity *entity);
        void newObstacle(GridObstacle *obstacle);
        void addRecorder(std::string filepath, Entity* entity);
        void start();

        // Methods entities can call
        std::vector<Entity> getClosestEntities(long unsigned int n, Target entity);
        std::vector<Entity> getEntitiesInRadius(float n, Target entity);
        std::vector<Edge<int>*> pathfind(sf::Vector2f currentPosition, sf::Vector2f goalPosition, Heuristic<Grid<int>, int> *heuristic);
        GridEnvironment *getEnvironment();
        float nearestObstacle(sf::Vector2f position, Direction direction);
};


#endif