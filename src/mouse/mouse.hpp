// mouse extends the sf::Mouse class for extra functionality
#ifndef MOUSE
#define MOUSE

#include <SFML/Graphics.hpp>
#include "../utils/kinematics/kinematics.hpp"

// Mouse class represents the user's mouse with various information associated with it
class Mouse {
    private:
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f lastClicked;
        sf::Time timePerFrame;
        float updateInterval;
        float updateTimer = 0;

        bool onScreen = false;
        bool wasOff = true;

    public:
        Mouse(sf::Time timePerFrame, float updateInterval);

        sf::Vector2f getPosition();
        sf::Vector2f getVelocity();
        sf::Vector2f getLastClicked();
        Target getTarget();
        Target getLastClickedTarget();
        void resetLastClicked();

        void setOnScreen(bool onScreen);        

        void update(float x, float y);
        void updateClick(float x, float y);
};

#endif