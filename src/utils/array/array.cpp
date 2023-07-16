#include <SFML/Graphics.hpp>
#include <math.h>
#include "array.hpp"
#include "../../entity/entity.hpp"

// Helper struct to contain an entity reference and distance for calculating closest entities
struct ClosestEntry {
    Entity* element;
    float distance;

    ClosestEntry(Entity* element, float distance) {
        this->element = element;
        this->distance = distance;
    }
};

// findClosest returns the closest n items from the given position (not including positions equal to the target)
std::vector<Entity*> Array::findClosest(Entity current, long unsigned int n) {
    // The sorted list of entities
    std::vector<ClosestEntry> closest;

    // Loop through each element and sort by distances
    sf::Vector2f currentPos = current.getPosition();
    sf::Vector2f elementPos;
    float distance;
    for (Entity element : *this) {
        // Get the current element and calculate the distance
        elementPos = element.getPosition();
        distance = sqrt( pow((elementPos.x - currentPos.x), 2) + pow((elementPos.y - currentPos.y), 2) );

        // Catch to not include entities directly on the target
        if (distance == 0) {
            continue;
        }

        // Otherwise, add it in a sorted position to closest
        for (std::vector<ClosestEntry>::iterator it = closest.begin(); it != closest.end(); ++it) {
            if (it->distance > distance) {
                closest.insert(it, ClosestEntry(&element, distance));
            }
        }
    }

    // Get a list of the first n entries
    std::vector<Entity*> entries;

    for (std::vector<Entity*>::size_type i = 0; i < n; i++) {
        entries.push_back(closest.at(i).element);
    }

    return entries;
}