// kinematics creates helpful structs for using kinematic information
#ifndef KINEMATICS
#define KINEMATICS

#include <SFML/Graphics.hpp>

// Accelerations class holds acceleration information
struct Accelerations {
    sf::Vector2f linearAcceleration = sf::Vector2f(0, 0);
    float angularAcceleration = 0.0;

    // Overload + for addition of accelerations
    Accelerations operator + (Accelerations const &obj) {
        Accelerations res;

        res.linearAcceleration = linearAcceleration + obj.linearAcceleration;
        res.angularAcceleration = angularAcceleration + obj.angularAcceleration;

        return res;
    }

    // Overload * for scalar multiplication of weights
    Accelerations operator * (float d) {
        Accelerations res;

        res.linearAcceleration = linearAcceleration * d;
        res.angularAcceleration = angularAcceleration * d;

        return res;
    }
};

// Kinematics class holds kinematic information critical for a steering behavior
struct Kinematics : public Accelerations {
    sf::Vector2f linearVelocity = sf::Vector2f(0, 0);
    float angularVelocity = 0.0;
};

// Target represents Kinematic information with a position (normally delegated to the sprite class)
struct Target: public Kinematics {
    sf::Vector2f position = sf::Vector2f(0, 0);
    float orientation = 0.0;
};

// Params represents data passed into steering behaviors to produce required outputs
struct Params {
    // The target a steering function may utilize
    Target target;

    // The character a steering function is representing
    Target character;
};

#endif