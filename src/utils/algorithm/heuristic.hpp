#ifndef HEURISTIC
#define HEURISTIC

#include <cmath>
#include "../graph/graph.hpp"
#include "../graph/vertex.hpp"

// Base heuristic class that all specific heuristics implement
template <typename V, typename E>
class Heuristic {
    public:
        virtual E estimate(Vertex<V> *current, Vertex<V> *end) = 0;
};

// ManhattanHeuristic class represents the manhattan distance between vertices in the grid system
template <typename E>
class ManhattanHeuristic : public Heuristic<Grid<E>, E> {
    public:
        E estimate(Vertex<Grid<E>> *current, Vertex<Grid<E>> *end) {
            return std::abs(end->getElement().row - current->getElement().row) + std::abs(end->getElement().column - current->getElement().column);
        }
};

// EuclideanHeuristic class represents the euclidean distance between vertices in the grid system
template <typename E>
class EuclideanHeuristic : public Heuristic<Grid<E>, E> {
    public:
        E estimate(Vertex<Grid<E>> *current, Vertex<Grid<E>> *end) {
            return std::sqrt(std::pow(end->getElement().row - current->getElement().row, 2) + std::pow(end->getElement().column - current->getElement().column, 2));
        }
};

// EuclideanSquaredHeuristic class represents the euclidean distance squred between vertices in the grid system (in order to get rid of costly square root)
template <typename E>
class EuclideanSquaredHeuristic : public Heuristic<Grid<E>, E> {
    public:
        E estimate(Vertex<Grid<E>> *current, Vertex<Grid<E>> *end) {
            return std::pow(end->getElement().row - current->getElement().row, 2) + std::pow(end->getElement().column - current->getElement().column, 2);
        }
};

#endif