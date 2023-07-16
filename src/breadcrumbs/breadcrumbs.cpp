#include <SFML/Graphics.hpp>
#include <iostream>
#include "breadcrumbs.hpp"

// Default constructor of a breadcrumb that sets a given position
Breadcrumb::Breadcrumb(sf::RenderWindow *window, float radius) {
    this->window = window;
    this->radius = radius;

    this->setRadius(this->radius);
    this->setFillColor(sf::Color::Cyan);
    this->setPosition(-this->radius * 2, -this->radius * 2);
    this->setOrigin(this->getLocalBounds().width / 2, this->getLocalBounds().height / 2);
}

// Drop the breadcrumb to a certain position
void Breadcrumb::update(sf::Vector2f position) {
    this->setPosition(position);
}

void Breadcrumb::draw() {
    this->window->draw(*this);
}