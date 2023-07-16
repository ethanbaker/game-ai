// Helper functions to deal with vector arithmetic
#ifndef VMATH
#define VMATH

#include <SFML/Graphics.hpp>
#include <math.h>
#include <time.h>
#include <random>
#include <iostream>

class Vmath {
    public:

        // Calculate the length of a vector
        static float length(sf::Vector2f v) {
            return sqrt(v.x * v.x + v.y * v.y);
        }

        // Normalize a vector to length 1
        static sf::Vector2f normalize(sf::Vector2f v) {
            return v / Vmath::length(v);
        }

        // Scale a vector to a given length
        static sf::Vector2f scale(sf::Vector2f v, float length) {
            return Vmath::normalize(v) * length;
        }

        // Normalize a range of rotation
        static float mapOrientationToRange(float o) {
            // Get the orientation in a range of (-2PI, 2PI)
            float orientation = fmod(o, 360);

            // Return the orientaiton if it is between (-PI, PI)
            if (abs(orientation) <= 180) {
                return orientation;
            } else if (orientation > 180) {
                return orientation - 360;
            } else {
                return orientation + 360;
            }
        }

        // Find a random binomial number
        static int randomBinomial(int n) {
            return rand() % n - rand() % n;
        }

        // Convert an orientation to a vector
        static sf::Vector2f orientationToVector(float angle) {
            angle *= M_PI / 180;
            return sf::Vector2f(cos(angle), sin(angle));
        }

        // Get the dot product of two vectors
        static float dotProduct(sf::Vector2f u, sf::Vector2f v) {
            return u.x * v.x + u.y * v.y;
        }

        // Get a random number between a range
        static int randomRange(int low, int high) {
            return low + rand() % (high - low + 1);
        }
};

#endif