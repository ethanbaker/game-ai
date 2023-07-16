#include <SFML/Graphics.hpp>
#include <iostream>
#include "mouse.hpp"
#include "../utils/kinematics/kinematics.hpp"

// Default constructor for the mouse object
Mouse::Mouse(sf::Time timePerFrame, float updateInterval) {
    this->position = sf::Vector2f(-1, -1);
    this->lastClicked = sf::Vector2f(-1, -1);
    this->timePerFrame = timePerFrame;
    this->updateInterval = updateInterval;
}

// Get the position of the mouse
sf::Vector2f Mouse::getPosition() {
    return this->position;
}

// Get the velocity of the mouse
sf::Vector2f Mouse::getVelocity() {
    return this->onScreen ? this->velocity : sf::Vector2f(0, 0);
}

// Get the last clicked location of the mouse
sf::Vector2f Mouse::getLastClicked() {
    return this->lastClicked;
}

void Mouse::resetLastClicked() {
    this->lastClicked = sf::Vector2f(-1, -1);
}

// Get the mouse as a target
Target Mouse::getTarget() {
    Target t;

    t.position = this->getPosition();
    t.linearVelocity = this->getVelocity();

    return t;
}

// Get the mouse's last clicked position as a target
Target Mouse::getLastClickedTarget() {
    Target t;

    t.position = this->getLastClicked();

    return t;
}

// Update the mouse's position/velocity
void Mouse::update(float x, float y) {
    this->updateTimer += timePerFrame.asSeconds();

    if (!this->onScreen) {
        return;
    }

    if (this->updateTimer > this->updateInterval) {
        // Update the velocity (if the mouse has a previous position)
        if (!this->wasOff) {
            this->velocity = sf::Vector2f(x - this->position.x, y - this->position.y);
            this->velocity /= timePerFrame.asSeconds();
        }
        this->wasOff = false;

        // Update the position
        this->position = sf::Vector2f(x, y);

        this->updateTimer = 0;
    }
}

// Set whether or not the mouse is on screen
void Mouse::setOnScreen(bool onScreen) {
    this->onScreen = onScreen;
    if (onScreen) {
        this->wasOff = true;
    }
}

// Update the mouse's last clicked location
void Mouse::updateClick(float x, float y) {
    this->lastClicked = sf::Vector2f(x, y);
}