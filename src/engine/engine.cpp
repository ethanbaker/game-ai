#include <SFML/Graphics.hpp>
#include <string>
#include <limits>
#include <algorithm>
#include <iostream>
#include "engine.hpp"
#include "../mouse/mouse.hpp"
#include "../utils/vmath/vmath.hpp"
#include "../utils/graph/graph.hpp"
#include "../utils/algorithm/algorithm.hpp"
#include "../utils/algorithm/heuristic.hpp"

// Default constructor for an engine
Engine::Engine(std::string title, Settings *settings) : sf::RenderWindow(sf::VideoMode(settings->width, settings->height), title), environment(settings->xTiles, settings->yTiles, settings->width, settings->height) {
    this->settings = settings;
}

float mouseX;
float mouseY;

// Handle updates sent from the window
void Engine::handleEvents() {
    // Keep listenting for new events
    sf::Event event;
    while (this->pollEvent(event)) {
        switch(event.type) {
            // Close the window on 'Closed' events
            case sf::Event::Closed:
                this->close();
                break;

            // Update the position of the mouse
            case sf::Event::MouseMoved:
                mouseX = event.mouseMove.x;
                mouseY = event.mouseMove.y;
                break;

            // Update where the mouse last clicked
            case sf::Event::MouseButtonPressed:
                this->mouse->updateClick(event.mouseButton.x, event.mouseButton.y);
                break;

            case sf::Event::MouseLeft:
                this->mouse->setOnScreen(false);
                break;

            case sf::Event::MouseEntered:
                this->mouse->setOnScreen(true);
                break;

            // Ignore other commands
            default:
                continue;
        }
    }
}

// Handle updating entities in the window
void Engine::update(sf::Time dt) {
    // Calculate time for the behavior update clock
    this->timeSinceLastBehaviorUpdate += this->behaviorUpdateClk.restart();
    bool shouldUpdate = this->timeSinceLastBehaviorUpdate >= this->settings->timePerDecision;

    // Update all of the entities in the scene
    for (long unsigned int i = 0; i < this->entities.size(); i++) {
        Entity *entity = entities.at(i);
        entity->update(dt);

        if (shouldUpdate) {
            EnvironmentParameters environment;
            environment.gameState = this;
            environment.settings = this->settings;
            environment.character = entity;
            environment.playableCharacter = this->playerCharacter;
            environment.stateVariables = &this->stateVariables;

            entity->decide(&environment);
        }

        // Teleport entities back to the screen smoothly if they go off an edge
        float x = entity->getPosition().x;
        float y = entity->getPosition().y;
        float width = entity->getGlobalBounds().width / 2;
        float height = entity->getGlobalBounds().height / 2;

        float maxWidth = this->getSize().x;
        float maxHeight = this->getSize().y;

        if (x + width < 0) {
            x = maxWidth + width;
        } else if (x - width > maxWidth) {
            x = -width;
        }

        if (y + height < 0) {
            y = maxHeight + height;
        } else if (y - height > maxHeight) {
            y = -height;
        }

        entity->setPosition(x, y);
    }


    if (shouldUpdate) {
        this->timeSinceLastBehaviorUpdate = sf::Time::Zero;

        // Write a new line to the recordings
        for (long unsigned int i = 0; i < this->recordings.size(); i++) {
            Recording *recording = this->recordings.at(i);
            recording->writePacket(this->timestamp);
        }
        this->timestamp++;
    }
}

// Render the next update cycle of the game
void Engine::render() {
    // Update the mouse handler class
    this->mouse->update(mouseX, mouseY);

    this->clear(this->settings->background);

    // Draw all obstacles
    for (Obstacle* o : *this->environment.getObstacles()) {
        this->draw(*o);
    }

    // Draw all breadcrumbs
    for (Array::size_type i = 0; i < this->entities.size(); i++) {
        if (!this->entities[i]->isInvisible()) {
            this->entities[i]->drawBreadcrumb(this->settings->timePerFrame.asSeconds());
        }
    }

    // Draw all entities
    for (Array::size_type i = 0; i < this->entities.size(); i++) {
        if (!this->entities[i]->isInvisible()) {
            this->draw(*this->entities[i]);
        }
    }

    this->display();
}

// Create a new entity to be rendered in the game
void Engine::newEntity(Entity *entity) {
    // Add the entity to the entities list
    entity->setEngine(this);
    entities.push_back(entity);
}

// Create a new obstacle in the game
void Engine::newObstacle(GridObstacle *obstacle) {
    this->environment.addObstacle(obstacle);
}

// Add a new recorder for an entity
void Engine::addRecorder(std::string filepath, Entity* entity) {
    Recording *recorder = new Recording(filepath, entity);
    this->recordings.push_back(recorder);
}

// Setup a mouse for the engine
void Engine::setMouse(Mouse *mouse) {
    this->mouse = mouse;
}

// Setup a playable character in the game
void Engine::setPlayableCharacter(Entity *entity) {
    this->playerCharacter = entity;
}

// Get the playable character in the game
Entity *Engine::getPlayableCharacter() {
    return this->playerCharacter; 
}

// Set a state variable
void Engine::setStateVariable(std::string key, void* value) {
    this->stateVariables[key] = value;
}

// Get a state variable
void *Engine::getStateVariable(std::string key) {
    return this->stateVariables[key];
}

// Get the current timestamp
int Engine::getTimestamp() {
    return this->timestamp;
}

// Start the game
void Engine::start() {
    // Create a clock to enforce a given frame rate
    sf::Clock clk;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (this->isOpen()) {
        this->handleEvents();

        // Add the time change since the last iteration
        sf::Time dt = clk.restart();
        timeSinceLastUpdate += dt;

        // If enough time has passed for a new frame to be drawn, update it
        while (timeSinceLastUpdate > this->settings->timePerFrame) {
            timeSinceLastUpdate -= this->settings->timePerFrame;

            this->update(this->settings->timePerFrame);
        }

        this->render();
    }
}

// Helper struct for sorting closest entities
struct EntityDistance {
    Entity entity;
    float distance;
};

// Get the closest entities to a given target
std::vector<Entity> Engine::getClosestEntities(long unsigned int n, Target entity) {
    std::vector<EntityDistance> entitiesByDistance;

    // Add a first, sentinel value
    EntityDistance first;
    first.distance = std::numeric_limits<int>::max();
    entitiesByDistance.push_back(first);

    // Loop for each entity and find the distance between them
    for (long unsigned int i = 0; i < this->entities.size(); i++) {
        EntityDistance d;
        d.entity = *this->entities.at(i);
        d.distance = Vmath::length(entity.position - d.entity.getPosition());

        // Don't count distances of 0
        if (d.distance == 0) {
            continue;
        }

        // Add the entity into a sorted list
        for (long unsigned int j = 0; j < entitiesByDistance.size(); j++) {
            if (entitiesByDistance.at(j).distance > d.distance) {
                entitiesByDistance.insert(entitiesByDistance.begin() + j, d);
                break;
            }
        } 
    }

    // Return the closest n entities
    std::vector<Entity> closest;
    for (Array::size_type i = 0; i < std::min(n, (long unsigned int) entitiesByDistance.size() - 1); i++) {
        closest.push_back(entitiesByDistance.at(i).entity);
    }

    return closest;
}

// Get all entities (unordered) in a given radius
std::vector<Entity> Engine::getEntitiesInRadius(float n, Target entity) {
    std::vector<Entity> closest;

    // Loop for each entity and find the distance between them
    float distance;
    for (Array::size_type i = 0; i < this->entities.size(); i++) {
        distance = Vmath::length(entity.position - this->entities[i]->getPosition());

        if (distance < n) {
            closest.push_back(*this->entities[i]);
        }
    }

    return closest;
}

// Pathfind from a given position in the game environment to the next
std::vector<Edge<int>*> Engine::pathfind(sf::Vector2f currentPosition, sf::Vector2f goalPosition, Heuristic<Grid<int>, int> *heuristic) {
    // Quantize the start and end positions
    Vertex<Grid<int>> *startVertex = this->environment.quantize(currentPosition);
    Vertex<Grid<int>> *endVertex = this->environment.quantize(goalPosition);

    if (startVertex == nullptr) {
        startVertex = this->environment.quantize(sf::Vector2f(320, 240));
    }
    if (endVertex == nullptr) {
        endVertex = this->environment.quantize(sf::Vector2f(320, 240));
    }

    // Find the shortest path
    std::vector<Edge<int>*> path;
    bool success = Algorithm<Grid<int>, int>::astar(&path, this->environment.getGraph(), startVertex, endVertex, heuristic);

    if (!success) {
        return std::vector<Edge<int>*>();
    }

    return path;
}

// Get the environment
GridEnvironment *Engine::getEnvironment() {
    return &this->environment;
}

// Find the nearest obstacle in a given direction
float Engine::nearestObstacle(sf::Vector2f position, Direction direction) {
    // Find the changes in width/height
    float dx = 0;
    float dy = 0;
    float dd = 0;

    switch (direction) {
        case Direction::Top:
            dy = -1;
            dd = this->settings->height / this->settings->yTiles;
            break;

        case Direction::Right:
            dx = 1;
            dd = this->settings->width / this->settings->xTiles;
            break;

        case Direction::Bottom:
            dy = 1;
            dd = this->settings->height / this->settings->yTiles;
            break;

        case Direction::Left:
            dx = -1;
            dd = this->settings->width / this->settings->xTiles;
            break;
    }

    // Parametrize the current position and look for obstacles in a given direction
    Vertex<Grid<int>> *currentVertex = this->environment.quantize(position);

    int row = currentVertex->getElement().row;
    int col = currentVertex->getElement().column;

    float distance = 0;
    while (row >= 0 && row < this->settings->yTiles && col >= 0 && col < this->settings->xTiles) {
        row += dx;
        col += dy;
        distance += dd;

        if (this->environment.isObstacle(row, col)) {
            break;
        }
    }

    return distance;
}