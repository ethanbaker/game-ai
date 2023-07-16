#include <SFML/Graphics.hpp>
#include <string>
#include "sceneA.hpp"
#include "../entity/entity.hpp"
#include "../engine/engine.hpp"
#include "../mouse/mouse.hpp"
#include "../steering/steering.hpp"
#include "../steering/behaviors.hpp"
#include "../utils/graph/graph.hpp"
#include "../utils/graph/vertex.hpp"
#include "../environment/environment.hpp"
#include "../utils/tree/decision-tree/decision-tree.hpp"

// Decision nodes for decision trees
class IsAtBoundary : public Decision {
    public:
        void *run(EnvironmentParameters *environment) {
            sf::Vector2f pos = environment->character->getTarget().position;

            bool closeToHorizontal = pos.x >= 610 || pos.x <= 10;
            bool closeToVertical = pos.y >= 470 || pos.y <= 10;

            return closeToHorizontal || closeToVertical ? this->getTruthNode()->run(environment) : this->getFalseNode()->run(environment);
        }
};

class IsAtMaze : public Decision {
    public:
        sf::Vector2f start = sf::Vector2f(290, 0);
        sf::Vector2f size = sf::Vector2f(350, 480);

        void *run(EnvironmentParameters *environment) {
            sf::Vector2f pos = environment->character->getTarget().position;

            bool inX = start.x <= pos.x && pos.x <= start.x + size.x;
            bool inY = start.y <= pos.y && pos.y <= start.y + size.y;

            return inX && inY ? this->getTruthNode()->run(environment) : this->getFalseNode()->run(environment);
        }
};

class IsAtMazeEnd : public Decision {
    public:
        sf::Vector2f start = sf::Vector2f(580, 420);

        void *run(EnvironmentParameters *environment) {
            sf::Vector2f pos = environment->character->getTarget().position;

            bool inX = start.x <= pos.x;
            bool inY = start.y <= pos.y;

            return inX && inY ? this->getTruthNode()->run(environment) : this->getFalseNode()->run(environment);
        }
};

// Constructor for SceneA, which sets up the scenes entities
SceneA::SceneA() {
    srand(time(NULL));

    // String representation of the maze
    std::vector<std::string> mazeString;

    mazeString.push_back("  xxxxxxxxxxxxx");
    mazeString.push_back("x     x xxx   x");
    mazeString.push_back("x           x x");
    mazeString.push_back("xxxxx xxxxxxx x");
    mazeString.push_back("x     x       x");
    mazeString.push_back("x xxx x xxx x x");
    mazeString.push_back("x   x x     x x");
    mazeString.push_back("xxxxxxx xxxxxxx");
    mazeString.push_back("x       x     x");
    mazeString.push_back("x x x xxxxx x x");
    mazeString.push_back("x x x   x   x x");
    mazeString.push_back("x xxxxxxx xxx x");
    mazeString.push_back("x       x x   x");
    mazeString.push_back("x x x xxxxxxx x");
    mazeString.push_back("x x x         x");
    mazeString.push_back("xxx x x  xxxx x");
    mazeString.push_back("x x x xxxx  x x");
    mazeString.push_back("x xxxxx  x  x x");
    mazeString.push_back("x             x");
    mazeString.push_back("x xxxxxxxxxxxxx");
    mazeString.push_back("x   x   x   x x");
    mazeString.push_back("xxx x x x      ");
    mazeString.push_back("x     x        ");
    mazeString.push_back("xxxxxxxxxxx    ");

    // Settings for the game engine
    Settings settings = Settings();
    settings.background = sf::Color::White;
    settings.fps = 100;
    settings.timePerFrame = sf::seconds(1.0f / 100);
    settings.maxLinearVelocity = 100;
    settings.maxAngularVelocity = 100;
    settings.maxLinearAcceleration = 1000;
    settings.maxAngularAcceleration = 1000;
    settings.width = 640;
    settings.height = 480;
    settings.xTiles = 32;
    settings.yTiles = 24;
    settings.breadcrumbInterval = 20.0f;
    settings.breadcrumbsPerEntity = 20;
    settings.breadcrumbRadius = 5.0f;
    settings.timePerDecision = sf::seconds(1);

    // Setup the mouse capture
    Mouse mouse = Mouse(settings.timePerFrame, settings.timePerFrame.asSeconds() * 15);
    this->mouse = &mouse;

    // Create the game engine
    Engine engine = Engine("Game AI Demo", &settings);
    this->engine = &engine;
    this->engine->setMouse(&mouse);

    // Setup decision nodes
    IsAtBoundary isAtBoundary;
    IsAtMazeEnd isAtMazeEnd;
    IsAtMaze isAtMaze;

    // Align behavior for all actions
    AlignToVelocity align = AlignToVelocity(5.0f, 10.0f, 0.1f, Align(8.0f, 30.0f, 180.0f, 0.01f));
    align.weight = 1;

    // Pathfinding behaviors for moving through maze
    Arrive arrive = Arrive(5.0f, 10.0f, 40.0f, 0.3f);
    EuclideanHeuristic<int> heuristic = EuclideanHeuristic<int>();

    sf::Vector2f endPoint = isAtMazeEnd.start + sf::Vector2f(20, 20);
    PathfindToPositionOld pathfindToEnd = PathfindToPositionOld(&engine, &arrive, &heuristic, 0.2, endPoint);
    pathfindToEnd.weight = 1;

    // Spinning behaviors for when character reaches end
    WeightedBehavior spinning;
    spinning.weight = 1;
    spinning.behavior = [this](Target character) mutable -> Accelerations {
        Params params;
        params.character = character;
        params.target.angularVelocity = 100.0f;

        RotationMatch matcher = RotationMatch(0.1f);
        return matcher.find(params);
    };

    ConstantVelocityMatch constantVelocityMatch = ConstantVelocityMatch(0, 0.01f);
    constantVelocityMatch.weight = 1;

    // Flee behavior for flee towards center action
    WeightedBehavior seek;
    seek.weight = 1;
    seek.behavior = [this](Target character) mutable -> Accelerations {
        Params params;
        params.character = character;
        params.target.position = sf::Vector2f(320, 240);

        Seek matcher = Seek(100.0, 0.3);
        return matcher.find(params);
    };

    // Wander behavior for wander action
    Wander wander = Wander(0, 300.0f, 50.0f, Arrive(20.0f, 75.0f, 200.0f, 0.5f), sf::seconds(4.0f), &settings, isAtMaze.start.x + 100, 480.0);
    wander.weight = 1;

    // Setup behavior lists
    std::vector<WeightedBehavior> pathfindToEndBehaviors;
    pathfindToEndBehaviors.push_back(align);
    pathfindToEndBehaviors.push_back(pathfindToEnd);

    std::vector<WeightedBehavior> spinningBehaviors;
    spinningBehaviors.push_back(spinning);
    spinningBehaviors.push_back(constantVelocityMatch);

    std::vector<WeightedBehavior> wanderBehaviors;
    wanderBehaviors.push_back(align);
    wanderBehaviors.push_back(wander);

    std::vector<WeightedBehavior> seekBehaviors;
    seekBehaviors.push_back(align);
    seekBehaviors.push_back(seek);

    // Setup actions in the decision tree
    Action pathfindToEndAction;
    pathfindToEndAction.behaviors = pathfindToEndBehaviors;
    pathfindToEndAction.name = "pathfinding to end";

    Action spinningAction;
    spinningAction.behaviors = spinningBehaviors;
    spinningAction.name = "spinning";

    Action wanderAction;
    wanderAction.behaviors = wanderBehaviors;
    wanderAction.name = "wandering";

    Action seekAction;
    seekAction.behaviors = seekBehaviors;
    seekAction.name = "seeking to center";

    DecisionTree tree;
    Vertex<AbstractDMNode*> *isAtMazeEndNode = tree.addRoot(&isAtMazeEnd);

    tree.insertNode(&spinningAction, isAtMazeEndNode, true);
    Vertex<AbstractDMNode*> *isAtMazeNode = tree.insertNode(&isAtMaze, isAtMazeEndNode, false);

    tree.insertNode(&pathfindToEndAction, isAtMazeNode, true);
    Vertex<AbstractDMNode*> *atGameBoundaryNode = tree.insertNode(&isAtBoundary, isAtMazeNode, false);

    tree.insertNode(&seekAction, atGameBoundaryNode, true);
    tree.insertNode(&wanderAction, atGameBoundaryNode, false);

    // Load the sprite assets
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./assets/boid.png")) {
        exit(EXIT_FAILURE);
    }
    boidTexture.setSmooth(true);

    sf::Texture obstacleTexture;
    if (!obstacleTexture.loadFromFile("./assets/square.png")) {
        exit(EXIT_FAILURE);
    }
    obstacleTexture.setRepeated(true);

    // Create the character
    Entity character = Entity(sf::Vector2f(25, 25));
    character.setTexture(boidTexture);
    character.scale(sf::Vector2f(0.3, 0.3));
    character.setOrigin(character.getLocalBounds().width / 2, character.getLocalBounds().height / 2);
    character.setDecisionMakingTree(&tree);
    engine.newEntity(&character);

    // Create a maze
    for (std::vector<std::string>::size_type row = 0; row < mazeString.size(); row++) {
        for (std::vector<std::string>::size_type col = 0; col < mazeString.at(0).size(); col++) {
            if (mazeString.at(row).at(col) == 'x') {
                GridObstacle *o = new GridObstacle(Grid((int) row, (int) col + (32 - 15)));
                o->setTexture(obstacleTexture);
                o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
                engine.newObstacle(o);
            }
        }
    }

    // Start the engine
    this->engine->start();
}

