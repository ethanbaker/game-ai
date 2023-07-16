#include "environment.hpp"
#include "../engine/engine.hpp"

/* GridObstacle Class */

// Constructor to create a new GridObstacle
GridObstacle::GridObstacle(Grid<int> gridLocation) {
    this->gridLocation = gridLocation;
}

// Get the grid location this obstacle represents
Grid<int> GridObstacle::getGridLocation() {
    return this->gridLocation;
}

/* GridEnvironment Class */

// Generate a new Grid Environment that is split into a given number of tiles
GridEnvironment::GridEnvironment(int xTiles, int yTiles, int width, int height) {
    // Get information about each tile in the grid
    this->xTiles = xTiles;
    this->yTiles = yTiles;
    this->width = width;
    this->height = height;

    this->tileWidth = width / xTiles;
    this->tileHeight = height / yTiles;

    // Create a graph representing all vertices in the grid
    this->setGraph(AdjacencyListGraph<Grid<int>, int>(true));
    for (int row = 0; row < yTiles; row++) {
        for (int col = 0; col < xTiles; col++) {
            this->getGraph()->insertVertex(Grid(row, col));
        }
    }

    // Add edges between vertices in the grid
    for (Vertex<Grid<int>> *v : this->getGraph()->vertices()) {
        Grid<int> grid = v->getElement();
        for (Vertex<Grid<int>> *connecting : this->getGraph()->vertices()) {
            Grid<int> connectingGrid = connecting->getElement();
            if ((grid.row - 1 == connectingGrid.row || grid.row + 1 == connectingGrid.row) && grid.column == connectingGrid.column) {
                this->getGraph()->insertEdge(v, connecting, 1);
            } else if ((grid.column - 1 == connectingGrid.column || grid.column + 1 == connectingGrid.column) && grid.row == connectingGrid.row) {
                this->getGraph()->insertEdge(v, connecting, 1);
            }
        } 
    }
}

// Quantize a given environment position to a vertex on the graph (specifically for grid environments)
Vertex<Grid<int>> *GridEnvironment::quantize(sf::Vector2f position) {
    // Get the row and column of the position
    int column = position.x / this->tileWidth;
    int row = position.y / this->tileHeight;

    // Find the associated vertex
    Grid<int> element;
    for (Vertex<Grid<int>> *v : this->getGraph()->vertices()) {
        element = v->getElement();
        if (element.row == row && element.column == column) {
            return v;
        }
    }

    return nullptr;
}

// Localize a given vertex to an environment position (specifically for grid environments)
sf::Vector2f GridEnvironment::localize(Vertex<Grid<int>> *vertex) {
    sf::Vector2f position;
    Grid<int> element = vertex->getElement();

    // Set the position to the center of the tile
    position.x = element.column * this->tileWidth + this->tileWidth / 2;
    position.y = element.row * this->tileHeight + this->tileHeight / 2;

    return position;
}

// Add a grid obstacle to the environment
void GridEnvironment::addObstacle(GridObstacle *gridObstacle) {
    this->getObstacles()->push_back(gridObstacle);
    gridObstacle->setPosition(gridObstacle->getGridLocation().column * (this->width / this->xTiles), gridObstacle->getGridLocation().row * (this->height / this->yTiles));

    // Remove the vertex that overlaps with the graph
    Grid<int> element;
    for (Vertex<Grid<int>> *v : this->getGraph()->vertices()) {
        element = v->getElement();
        if (element.row == gridObstacle->getGridLocation().row && element.column == gridObstacle->getGridLocation().column) {
            this->getGraph()->removeVertex(v);
            break;
        }
    }
}

// Determine if a given grid element is an obstacle
bool GridEnvironment::isObstacle(int row, int col) {
    Grid<int> element;
    for (Vertex<Grid<int>> *v : this->getGraph()->vertices()) {
        element = v->getElement();
        if (element.row == row && element.column == col) {
            return false;
        }
    }

    return true;
}

// Localize a given vertex endpoint
sf::Vector2f GridEnvironment::localizeEndpoint(Edge<int> *edge, int index) {
    std::array<Vertex<Grid<int>>*, 2> edges = this->getGraph()->endVertices(edge);

    if (edges.size() != 2) {
        return sf::Vector2f(-1, -1);
    }

    return this->localize(edges[index]);
}