#ifndef ENVIRONMENT
#define ENVIRONMENT

#include <SFML/Graphics.hpp>
#include <map>
#include "../utils/graph/graph.hpp"
#include "../utils/kinematics/kinematics.hpp"

class Engine;
class Entity;
class Settings;

// EnvironmentParameters represents knowledge and state information passed into decision making algorithms
class EnvironmentParameters {
    public:
        // State information:
        Engine *gameState;
        Entity *character;
        Entity *playableCharacter;
        std::map<std::string, void*> *stateVariables;

        // Knowledge information
        Settings *settings;
};

/* Obstacle Classes */

// Obstacle class represents a renderable obstacle in the game environment
class Obstacle : public sf::Sprite {
    public:
        Obstacle() : sf::Sprite() {}
};

// GridObstacle class represents an obstacle that can be easily encoded in a grid environment system
class GridObstacle : public Obstacle {
    private:
        Grid<int> gridLocation;

    public:
        GridObstacle(Grid<int> gridLocation);

        Grid<int> getGridLocation();
};

/* Environment Classes */

// Environment class provides a way to represent a game environment with a specific graph representation
template <typename V, typename E>
class Environment {
    private:
        // The encapsulated graph that represents the game environment
        AdjacencyListGraph<V, E> graph;

        // Obstacles inside of the game environment
        std::vector<Obstacle*> obstacles;


    public:
        // Create a new environment with reference to the engine
        Environment() {}

        // Quantize a given environment position to a vertex on the graph
        virtual Vertex<V> *quantize(sf::Vector2f position) = 0;

        // Localize a given vertex to an environment position
        virtual sf::Vector2f localize(Vertex<V> *vertex) = 0;

        // Getters
        AdjacencyListGraph<V, E> *getGraph() {
            return &this->graph;
        }
        std::vector<Obstacle*> *getObstacles() {
            return &this->obstacles;
        }

        // Setters
        void setGraph(AdjacencyListGraph<V, E> graph) {
            this->graph = graph;
        }
};


// GridEnvironment class represents an environment coded as a grid system
class GridEnvironment : public Environment<Grid<int>, int> {
    private:
        float tileWidth;
        float tileHeight;

        int xTiles;
        int yTiles;

        int height;
        int width;

    public:
        GridEnvironment(int xTiles, int yTiles, int width, int height);

        Vertex<Grid<int>> *quantize(sf::Vector2f position);
        sf::Vector2f localize(Vertex<Grid<int>> *vertex);

        bool isObstacle(int row, int col);
        void addObstacle(GridObstacle *gridObstacle);
        sf::Vector2f localizeEndpoint(Edge<int> *edge, int index);
};

#endif