// array represents an array class with special functionality of storing characters
#ifndef ARRAY
#define ARRAY

#include <vector>
#include "../../entity/entity.hpp"

class Array : public std::vector<Entity> {
    public:
        std::vector<Entity*> findClosest(Entity position, long unsigned int n);
};

#endif