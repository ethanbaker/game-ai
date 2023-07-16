#include <iostream>
#include "entity.hpp"
#include "../engine/engine.hpp"
#include "../utils/kinematics/kinematics.hpp"
#include "../utils/vmath/vmath.hpp"

/** Entity Class */

Entity::Entity() : sf::Sprite() {
    this->setPosition(-100, -100);
}

Entity::Entity(sf::Vector2f position) : sf::Sprite() {
    this->setPosition(position);
}

void Entity::setEngine(Engine* engine) {
    this->engine = engine;
    Breadcrumb::initializeBreadcrumbs(&this->breadcrumbs, this->engine, this->getPosition(), this->engine->settings->breadcrumbRadius, this->engine->settings->breadcrumbsPerEntity);
}

void Entity::setBehaviors(std::vector<WeightedBehavior> behaviors) {
    this->behaviors = behaviors;
}

void Entity::setDecisionMakingTree(AbstractDMTree *tree) {
    this->decisionMakingTree = tree;
}

void Entity::setInvisibility(bool invisibility) {
    this->invisible = invisibility;
}

bool Entity::isInvisible() {
    return this->invisible;
}

void Entity::setCurrentAction(std::string action) {
    this->currentAction = action;
}

StatePacket Entity::getStatePacket(int timestamp) {
    StatePacket packet;

    packet.timestamp = timestamp;
    packet.action = this->currentAction;
    if (this->engine->getEnvironment()->quantize(this->getPosition()) != nullptr) {
        packet.x = this->engine->getEnvironment()->quantize(this->getPosition())->getElement().column;
        packet.y = this->engine->getEnvironment()->quantize(this->getPosition())->getElement().row;
        packet.obstacleDistTop = this->engine->nearestObstacle(this->getPosition(), Direction::Top);
        packet.obstacleDistRight = this->engine->nearestObstacle(this->getPosition(), Direction::Right);
        packet.obstacleDistBottom = this->engine->nearestObstacle(this->getPosition(), Direction::Bottom);
        packet.obstacleDistLeft = this->engine->nearestObstacle(this->getPosition(), Direction::Left);
    }
    if (this->engine->getEnvironment()->quantize(this->engine->getPlayableCharacter()->getPosition()) != nullptr) {
        packet.characterX = this->engine->getEnvironment()->quantize(this->engine->getPlayableCharacter()->getPosition())->getElement().column;
        packet.characterY = this->engine->getEnvironment()->quantize(this->engine->getPlayableCharacter()->getPosition())->getElement().row;
    }

    return packet;
}

Kinematics Entity::getKinematics() {
    Kinematics k;

    k.linearVelocity = this->linearVelocity;
    k.linearAcceleration = this->linearAcceleration;
    k.angularVelocity = this->angularVelocity;
    k.angularAcceleration = this->angularAcceleration;

    return k;
}

Target Entity::getTarget() {
    Target t;

    t.position = this->getPosition();
    t.orientation = this->getRotation();

    t.linearVelocity = this->linearVelocity;
    t.linearAcceleration = this->linearAcceleration;
    t.angularVelocity = this->angularVelocity;
    t.angularAcceleration = this->angularAcceleration;

    return t;
}

void Entity::update(sf::Time dt) {
    float seconds = dt.asSeconds();

    // Update the position and orientation
    this->move(this->linearVelocity * seconds);
    this->rotate(this->angularVelocity * seconds);

    // Update and/or cap the velocities
    this->linearVelocity += this->linearAcceleration * seconds;
    this->angularVelocity += this->angularAcceleration * seconds;

    if (Vmath::length(this->linearVelocity) > this->engine->settings->maxLinearVelocity) {
        this->linearVelocity = Vmath::scale(this->linearVelocity, engine->settings->maxLinearVelocity);
    }
    if (this->angularVelocity > this->engine->settings->maxAngularVelocity) {
        this->angularVelocity = this->engine->settings->maxAngularVelocity;
    }

    // Update and/or cap the accelerations
    this->nextAccelerations();

    if (Vmath::length(this->linearAcceleration) > this->engine->settings->maxLinearAcceleration) {
        this->linearAcceleration = Vmath::scale(this->linearAcceleration, engine->settings->maxLinearAcceleration);
    }
    if (this->angularAcceleration > this->engine->settings->maxAngularAcceleration) {
        this->angularAcceleration = this->engine->settings->maxAngularAcceleration;
    }
}

void Entity::nextAccelerations() {
    // Calculated the weighted acceleration total
    Accelerations a;
    for (WeightedBehavior b : this->behaviors) {
        a = a + b.behavior(this->getTarget()) * b.weight;
    }

    this->linearAcceleration = a.linearAcceleration;
    this->angularAcceleration = a.angularAcceleration;
}

void Entity::drawBreadcrumb(float dt) {
    this->breadcrumbTimer += dt;

    // Update when the interval has been reached
    if (this->breadcrumbTimer > this->engine->settings->breadcrumbInterval) {
        this->breadcrumbs[this->currentBreadcrumb].update(this->getPosition());
        this->currentBreadcrumb = (this->currentBreadcrumb + 1) % this->breadcrumbs.size();
        this->breadcrumbTimer = 0;
    }

    // Draw all breadcrumbs
    for (std::vector<Breadcrumb>::size_type i = 0; i < this->breadcrumbs.size(); i++) {
        this->breadcrumbs[i].draw();
    }
}

void Entity::decide(EnvironmentParameters *environment) {
    if (this->decisionMakingTree != nullptr) {
        //AbstractDMNode *node = (AbstractDMNode*) this->decisionMakingTree->decide(environment);
        this->decisionMakingTree->decide(environment);
        //std::cout << node->name << "\n";
    }
}