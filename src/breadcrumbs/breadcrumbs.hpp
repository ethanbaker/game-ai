#ifndef BREADCRUMBS
#define BREADCRUMBS

#include <SFML/Graphics.hpp>

// Breadcrumb represents a drawable breadcrumb on the screen
class Breadcrumb : public sf::CircleShape {
    private:
        // The radius of the breadcrumb
        float radius;

        // The window associated with the breadcrumb
        sf::RenderWindow *window;

    public:
        Breadcrumb(sf::RenderWindow *window, float radius);
        void update(sf::Vector2f position);
        void draw();

        // Initialize a list of breadcrumbs
        static void initializeBreadcrumbs(std::vector<Breadcrumb> *breadcrumbs, sf::RenderWindow *w, sf::Vector2f pos, float radius, long unsigned int n) {
            for (std::vector<Breadcrumb>::size_type i = 0; i < n; i++) {
                Breadcrumb b = Breadcrumb(w, radius);
                b.update(pos);
                breadcrumbs->push_back(b);
            }
        }

};

#endif