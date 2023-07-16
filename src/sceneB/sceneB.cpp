#include <SFML/Graphics.hpp>
#include <string>
#include "sceneB.hpp"
#include "../entity/entity.hpp"
#include "../engine/engine.hpp"
#include "../mouse/mouse.hpp"
#include "../steering/steering.hpp"
#include "../steering/behaviors.hpp"
#include "../utils/graph/graph.hpp"
#include "../utils/graph/vertex.hpp"
#include "../environment/environment.hpp"
#include "../utils/tree/behavior-tree/behavior-tree.hpp"

/* Create conditional or action nodes for the behavior tree */

// Determine if the monster is colliding with the character
class IsMonsterColliding : public BehaviorTreeNode {
    public:
        // Return true if the entity is touching the character
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            sf::Vector2f monsterPos = environment->character->getTarget().position;
            sf::FloatRect monsterBox = environment->character->getGlobalBounds();

            sf::Vector2f characterPos = environment->playableCharacter->getTarget().position;
            sf::FloatRect characterBox = environment->playableCharacter->getGlobalBounds();

            return monsterBox.contains(characterPos) || characterBox.contains(monsterPos) ? &BOOL_TRUE : &BOOL_FALSE;
        }
};

// Make the character invisible
class MakeCharacterInvisible : public BehaviorTreeNode {
    public:
        // Make the character invisible and return true
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            environment->playableCharacter->setInvisibility(true);
            environment->gameState->mouse->resetLastClicked();
            environment->playableCharacter->setPosition(sf::Vector2f(40, 200));
            environment->gameState->setStateVariable("invisible", &BOOL_TRUE);

            return &BOOL_TRUE;
        }
};

// Determine if the character is invisible
class IsCharacterInvisible : public BehaviorTreeNode {
    public:
        // Return true if the character is invisible
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);
            return (*environment->stateVariables)["invisible"];
        }
};

// Make the monster spin
class MakeMonsterSpin : public BehaviorTreeNode {
    private:
        int timestep = 0;

        std::vector<WeightedBehavior> behaviors;
        ConstantVelocityMatch constantVelocityMatch = ConstantVelocityMatch(0, 0.01f);
        WeightedBehavior spinning;

    public:
        MakeMonsterSpin() {
            this->spinning.weight = 1;
            this->spinning.behavior = [this](Target character) mutable -> Accelerations {
                Params params;
                params.character = character;
                params.target.angularVelocity = 100.0f;

                RotationMatch matcher = RotationMatch(0.1f);
                return matcher.find(params);
            };

            this->constantVelocityMatch.weight = 1;

            this->behaviors.push_back(this->spinning);
            this->behaviors.push_back(this->constantVelocityMatch);
        }

        // Make the monster spin and return true after one full rotation
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            environment->character->setBehaviors(this->behaviors);

            this->timestep++;

            if (this->timestep >= 8) {
                this->timestep = 0;
                return &BOOL_TRUE;
            }

            return &BOOL_FALSE;
        }
};

// Reset the character
class MakeCharacterReset : public BehaviorTreeNode {
    public:
        // Make the character go back to the start and turn back visible
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            environment->playableCharacter->setInvisibility(false);
            environment->gameState->setStateVariable("invisible", &BOOL_FALSE);
            environment->gameState->setStateVariable("was_spinning", &BOOL_TRUE);

            return &BOOL_TRUE;
        }
};

// Determine if the character is in the hostile zone
class IsCharacterInHostileZone : public BehaviorTreeNode {
    public:
        sf::Vector2f position = sf::Vector2f(120, 0);
        sf::Vector2f size = sf::Vector2f(520, 480);

        // Return true if the character is in the hostile zone
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            sf::Vector2f c = environment->playableCharacter->getTarget().position;

            bool inX = position.x <= c.x && c.x <= position.x + size.x;
            bool inY = position.y <= c.y && c.y <= position.y + size.y;

            return inX && inY ? &BOOL_TRUE : &BOOL_FALSE;
        }
};

// Pathfind to the character
class PathfindToCharacter : public BehaviorTreeNode {
    public:
        Engine *engine;
        
        // Objects relating to pathfinding
        Arrive arrive = Arrive(5.0f, 10.0f, 50.0f, 0.3f);
        EuclideanHeuristic<int> heuristic = EuclideanHeuristic<int>();
        AlignToVelocity align = AlignToVelocity(5.0f, 10.0f, 0.1f, Align(8.0f, 30.0f, 180.0f, 0.01f));
        PathfindToPosition *pathfindToCharacter;

        PathfindToCharacter(Engine *engine) {
            this->engine = engine;

            this->pathfindToCharacter = new PathfindToPosition(this->engine, &this->arrive, &this->heuristic, 0.2, sf::Vector2f(0, 0));
            this->pathfindToCharacter->weight = 1;

            align.weight = 1;
        }

        ~PathfindToCharacter() {
            delete this->pathfindToCharacter;
        }

        // Pathfind to the character
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            sf::Vector2f futurePosition = environment->playableCharacter->getPosition() + environment->playableCharacter->getTarget().linearVelocity * 0.8f;
            this->pathfindToCharacter->reset(futurePosition);

            std::vector<WeightedBehavior> behaviors;
            behaviors.push_back(*this->pathfindToCharacter);
            behaviors.push_back(this->align);

            // Update pathfind position to character
            environment->character->setBehaviors(behaviors);

            return &BOOL_TRUE;
        }
};

// Pathfind to the next patrol point
class PathfindToPatrolPoint : public BehaviorTreeNode {
    public:
        Engine *engine;

        // Pathfinding objects
        Arrive arrive = Arrive(5.0f, 10.0f, 40.0f, 0.3f);
        EuclideanHeuristic<int> heuristic = EuclideanHeuristic<int>();
        AlignToVelocity align = AlignToVelocity(5.0f, 10.0f, 0.1f, Align(8.0f, 30.0f, 180.0f, 0.01f));
        PathfindToMultiplePosition *pathfind;
        std::vector<WeightedBehavior> behaviors;

        PathfindToPatrolPoint(Engine *engine, std::vector<sf::Vector2f*> *patrolPoints) {
            this->engine = engine;

            this->pathfind = new PathfindToMultiplePosition(this->engine, &this->arrive, &this->heuristic, 0.2, patrolPoints);
            this->pathfind->weight = 1;

            this->align.weight = 1;

            this->behaviors.push_back(*this->pathfind);
            this->behaviors.push_back(this->align);
        }

        ~PathfindToPatrolPoint() {
            delete this->pathfind;
        }

        // Pathfind to the next patrol point
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            if (((Boolean*) environment->gameState->getStateVariable("was_spinning"))->is()) {
                this->engine->setStateVariable("was_spinning", &BOOL_FALSE);
                this->pathfind->reset();
            }

            environment->character->setBehaviors(this->behaviors);

            return &BOOL_TRUE;
        }
};

// Pathfind to a random point
class PathfindToRandomPoint : public BehaviorTreeNode {
    public:
        Engine *engine;

        // Pathfinding objects
        Arrive arrive = Arrive(5.0f, 10.0f, 40.0f, 0.3f);
        EuclideanHeuristic<int> heuristic = EuclideanHeuristic<int>();
        AlignToVelocity align = AlignToVelocity(5.0f, 10.0f, 0.1f, Align(8.0f, 30.0f, 180.0f, 0.01f));
        PathfindToRandomPosition *pathfind;
        std::vector<WeightedBehavior> behaviors;

        PathfindToRandomPoint(Engine *engine, std::vector<sf::Vector2f*> *patrolPoints) {
            this->engine = engine;

            this->pathfind = new PathfindToRandomPosition(this->engine, &this->arrive, &this->heuristic, 0.2, patrolPoints);
            this->pathfind->weight = 1;

            this->align.weight = 1;

            this->behaviors.push_back(*this->pathfind);
            this->behaviors.push_back(this->align);
        }

        ~PathfindToRandomPoint() {
            delete this->pathfind;
        }

        // Pathfind to the next patrol point
        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            if (((Boolean*) environment->gameState->getStateVariable("was_reset"))->is()) {
                environment->gameState->setStateVariable("was_reset", &BOOL_FALSE);
                this->pathfind->reset();
            }

            environment->character->setBehaviors(this->behaviors);

            return &BOOL_TRUE;
        }
};

// Set the character's behaviors to an empty list
class MakeCharacterDoNothing : public BehaviorTreeNode {
    private:
        ConstantVelocityMatch constantVelocityMatch = ConstantVelocityMatch(0, 0.01f);
        ConstantRotationMatch constantRotationMatch = ConstantRotationMatch(0, 0.01f);
        std::vector<WeightedBehavior> behaviors;

    public:
        MakeCharacterDoNothing() {
            this->constantVelocityMatch.weight = 1;
            this->constantRotationMatch.weight = 1;
            this->behaviors.push_back(this->constantVelocityMatch);
            this->behaviors.push_back(this->constantRotationMatch);
        }

        void *run(EnvironmentParameters *environment) {
            environment->character->setCurrentAction(this->name);

            environment->character->setBehaviors(this->behaviors);
            environment->gameState->setStateVariable("was_reset", &BOOL_TRUE);
            return &BOOL_TRUE;
        }
};

// Constructor for SceneB, which sets up the scenes entities
SceneB::SceneB() {
    srand(time(NULL));

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
    settings.timePerDecision = sf::seconds(0.2);

    // Setup the mouse capture
    Mouse mouse = Mouse(settings.timePerFrame, settings.timePerFrame.asSeconds() * 15);
    this->mouse = &mouse;

    // Create the game engine
    Engine engine = Engine("Game AI Demo", &settings);
    this->engine = &engine;
    this->engine->setMouse(&mouse);
    this->engine->setStateVariable("was_spinning", &BOOL_FALSE);
    this->engine->setStateVariable("was_reset", &BOOL_FALSE);
    this->engine->setStateVariable("invisible", &BOOL_FALSE);

    // Setup the behavior tree for the monster
    sf::Vector2f pos1 = sf::Vector2f(550, 440);
    sf::Vector2f pos2 = sf::Vector2f(150, 40);
    sf::Vector2f pos3 = sf::Vector2f(200, 400);
    sf::Vector2f pos4 = sf::Vector2f(300, 200);
    sf::Vector2f pos5 = sf::Vector2f(600, 260);
    sf::Vector2f pos6 = sf::Vector2f(550, 50);
    std::vector<sf::Vector2f*> patrolPoints;
    patrolPoints.push_back(&pos1);
    patrolPoints.push_back(&pos2);
    patrolPoints.push_back(&pos3);
    patrolPoints.push_back(&pos4);
    patrolPoints.push_back(&pos5);
    patrolPoints.push_back(&pos6);

    // Setup all action/condition nodes
    IsMonsterColliding isMonsterColliding;
    isMonsterColliding.name = "Checking if monster is colliding";

    MakeCharacterInvisible makeCharacterInvisible;
    makeCharacterInvisible.name = "Making character invisible";

    IsCharacterInvisible isCharacterInvisible;
    isCharacterInvisible.name = "Checking if character is invisible";

    InverterDecorator isCharacterNotInvisible = InverterDecorator(&isCharacterInvisible);
    isCharacterNotInvisible.name = "Checking if character is not invisible";

    MakeMonsterSpin makeMonsterSpin = MakeMonsterSpin();
    makeMonsterSpin.name = "Making monster spin";

    RepeatDecorator makeMonsterSpinMultiple = RepeatDecorator(&makeMonsterSpin, 3);
    makeMonsterSpinMultiple.name = "Making monster spin multiple times";

    MakeCharacterReset makeCharacterReset;
    makeCharacterReset.name = "Making character reset";

    IsCharacterInHostileZone isCharacterInHostileZone;
    isCharacterInHostileZone.name = "Checking if character is in hostile zone";

    PathfindToCharacter pathfindToCharacter = PathfindToCharacter(&engine);
    pathfindToCharacter.name = "Pathfinding to character";

    PathfindToPatrolPoint pathfindToPatrolPoint = PathfindToPatrolPoint(&engine, &patrolPoints);
    pathfindToPatrolPoint.name = "Pathfinding to patrol point";

    // Make interior nodes
    SequenceNode onCollisionInvisible;
    SequenceNode onCollisionSpinning;
    SequenceNode onChase;
    SequenceNode onPatrol;
    SelectorNode root;

    // Setup the behavior tree
    BehaviorTree monsterTree;
    Vertex<AbstractDMNode*> *rootNode = monsterTree.addRoot(&root);

    // Root node children
    Vertex<AbstractDMNode*> *onCollisionInvisibleNode = monsterTree.addNode(&onCollisionInvisible, rootNode, true); 
    Vertex<AbstractDMNode*> *onCollisionSpinningNode = monsterTree.addNode(&onCollisionSpinning, rootNode, true); 
    Vertex<AbstractDMNode*> *onChaseNode = monsterTree.addNode(&onChase, rootNode, true); 
    Vertex<AbstractDMNode*> *onPatrolNode = monsterTree.addNode(&onPatrol, rootNode, true); 

    // On collision invisible children
    monsterTree.addNode(&isCharacterNotInvisible, onCollisionInvisibleNode, true);
    monsterTree.addNode(&isMonsterColliding, onCollisionInvisibleNode, true);
    monsterTree.addNode(&makeCharacterInvisible, onCollisionInvisibleNode, true);

    // On collision spinning children
    monsterTree.addNode(&isCharacterInvisible, onCollisionSpinningNode, true);
    monsterTree.addNode(&makeMonsterSpinMultiple, onCollisionSpinningNode, true);
    monsterTree.addNode(&makeCharacterReset, onCollisionSpinningNode, true);

    // On chase children
    monsterTree.addNode(&isCharacterNotInvisible, onChaseNode, true);
    monsterTree.addNode(&isCharacterInHostileZone, onChaseNode, true);
    monsterTree.addNode(&pathfindToCharacter, onChaseNode, true);

    // On patrol children
    monsterTree.addNode(&isCharacterNotInvisible, onPatrolNode, true);
    monsterTree.addNode(&pathfindToPatrolPoint, onPatrolNode, true);

    // Create behaviors for the character
    sf::Vector2f pos01 = sf::Vector2f(40, 40);
    sf::Vector2f pos02 = sf::Vector2f(40, 41);
    sf::Vector2f pos03 = sf::Vector2f(40, 42);
    sf::Vector2f pos06 = sf::Vector2f(40, 400);
    sf::Vector2f pos07 = sf::Vector2f(40, 401);
    sf::Vector2f pos08 = sf::Vector2f(40, 402);
    std::vector<sf::Vector2f*> findPoints;
    findPoints.push_back(&pos1);
    findPoints.push_back(&pos5);
    findPoints.push_back(&pos01);
    findPoints.push_back(&pos02);
    findPoints.push_back(&pos03);
    findPoints.push_back(&pos06);
    findPoints.push_back(&pos07);
    findPoints.push_back(&pos08);

    PathfindToRandomPoint pathfindToRandomPoint = PathfindToRandomPoint(&engine, &findPoints);
    pathfindToRandomPoint.name = "Pathfinding to random point";

    MakeCharacterDoNothing makeCharacterDoNothing;
    makeCharacterDoNothing.name = "Make character do nothing";

    SelectorNode rootCharacter;
    SequenceNode pathfindCharacter;

    // Setup the character's behavior tree
    BehaviorTree characterTree;
    Vertex<AbstractDMNode*> *rootCharacterNode = characterTree.addRoot(&rootCharacter);

    // Add children to the root selector node
    Vertex<AbstractDMNode*> *pathfindNode = characterTree.addNode(&pathfindCharacter, rootCharacterNode, true);
    characterTree.addNode(&makeCharacterDoNothing, rootCharacterNode, true);

    // Add children to the pathfind sequence node
    characterTree.addNode(&isCharacterNotInvisible, pathfindNode, true);
    characterTree.addNode(&pathfindToRandomPoint, pathfindNode, true);

    // Load the sprite assets
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./assets/boid.png")) {
        exit(EXIT_FAILURE);
    }
    boidTexture.setSmooth(true);

    sf::Texture monsterTexture;
    if (!monsterTexture.loadFromFile("./assets/boid-monster.png")) {
        exit(EXIT_FAILURE);
    }
    monsterTexture.setSmooth(true);

    sf::Texture obstacleTexture;
    if (!obstacleTexture.loadFromFile("./assets/square.png")) {
        exit(EXIT_FAILURE);
    }
    obstacleTexture.setRepeated(true);

    // Create the character
    Entity character = Entity(sf::Vector2f(40, 200));
    character.setTexture(boidTexture);
    character.scale(sf::Vector2f(0.3, 0.3));
    character.setOrigin(character.getLocalBounds().width / 2, character.getLocalBounds().height / 2);
    character.setDecisionMakingTree(&characterTree);
    engine.newEntity(&character);
    engine.setPlayableCharacter(&character);

    // Create the monster
    Entity monster = Entity(sf::Vector2f(400, 400));
    monster.setTexture(monsterTexture);
    monster.scale(sf::Vector2f(0.3, 0.3));
    monster.setOrigin(monster.getLocalBounds().width / 2, monster.getLocalBounds().height / 2);
    monster.setDecisionMakingTree(&monsterTree);
    engine.newEntity(&monster);
    //engine.addRecorder("./assets/monster-states.csv", &monster);

    // Create walls and obstacles
    for (int row = 0; row < settings.yTiles / 2 - 2; row++) {
        for (int col = 4; col < 6; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = settings.yTiles / 2 + 2; row < settings.yTiles; row++) {
        for (int col = 4; col < 6; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 13; row < 15; row++) {
        for (int col = 18; col < 22; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 10; row < 12; row++) {
        for (int col = 26; col < 28; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 16; row < 22; row++) {
        for (int col = 12; col < 14; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 20; row < 22; row++) {
        for (int col = 25; col < 31; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 6; row < 8; row++) {
        for (int col = 20; col < 22; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 11; row < 13; row++) {
        for (int col = 9; col < 11; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 4; row < 6; row++) {
        for (int col = 9; col < 15; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 8; row < 10; row++) {
        for (int col = 26; col < 31; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    for (int row = 4; row < 7; row++) {
        for (int col = 29; col < 31; col++) {
            GridObstacle *o = new GridObstacle(Grid(row, col));
            o->setTexture(obstacleTexture);
            o->scale(sf::Vector2f(640 / settings.xTiles / 10, 480 / settings.yTiles / 10));
            engine.newObstacle(o);
        }
    }

    // Start the engine
    this->engine->start();
}

