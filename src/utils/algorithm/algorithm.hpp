/**
 * Algorithm holds various algorithms that can be utilized in various ways
 * 
 * Dijkstra's algorithm is adapted from "AI For Games" by Ian Millington
 */
#ifndef ALGORITHM
#define ALGORITHM

#include <queue>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "../graph/graph.hpp"
#include "./heuristic.hpp"

// VertexRecord is used to keep track of information associated with each vertex
template <typename V, typename E>
struct VertexRecord {
    Vertex<V> *vertex;
    Edge<E> *edge;
    E cost;      // Represents estimatedCost in A*
    E costSoFar; // Only used in A*

    // Compare VertexRecords by looking at their underlying cost
    bool operator < (const VertexRecord<V, E> r) const {
        return this->cost < r.cost;
    }
    bool operator > (const VertexRecord<V, E> r) const {
        return this->cost > r.cost;
    }
};

// AdaptableSearchableVector extends vector functionality to find and update path costs
template <typename V, typename E>
class AdaptableSearchableVector : public std::vector<VertexRecord<V, E>> {
    public:
        // Find a given element (returns the iterator of that element, or the end of the vector if the element cannot be found)
        typename std::vector<VertexRecord<V, E>>::iterator find(Vertex<V> *item) {
            for (typename std::vector<VertexRecord<V, E>>::size_type i = 0; i < this->size(); i++) {
                if (this->at(i).vertex == item) {
                    return this->begin() + i;
                }
            }

            return this->end();
        }
};

// Algorithm class which contains static methods for algorithms
template <typename V, typename E>
class Algorithm {
    public:
        // Dijkstra's Algorithm, which finds the shortest path between two vertices in a given graph
        static bool dijkstras(std::vector<Edge<E>*> *path, AdjacencyListGraph<V, E> *graph, Vertex<V> *startVertex, Vertex<V> *endVertex) {
            // Initialize the open and closed lists
            AdaptableSearchableVector<V, E> openList;
            AdaptableSearchableVector<V, E> closedList;

            // Initialize the record for the start node
            VertexRecord<V, E> start;
            start.vertex = startVertex;
            start.cost = 0;
            openList.push_back(start);

            // Iterate through processing each vertex
            VertexRecord<V, E> current;
            while (openList.size() > 0) {
                // Get the smallest element in the open list
                std::pop_heap(openList.begin(), openList.end(), std::greater<VertexRecord<V, E>>{});
                current = openList.back();
                openList.pop_back();

                // If the current vertex is the end vertex, break
                if (current.vertex == endVertex) {
                    break;
                }

                // Loop through each edge in the vertex
                for (Edge<E> *e : *graph->outgoingEdges(current.vertex)) {
                    // Get the cost estimate for each new vertex
                    Vertex<V> *opposite = graph->opposite(current.vertex, e);
                    E newCost = current.cost + e->getElement();

                    if (closedList.find(opposite) != closedList.end()) {
                        // Skip if the vertex is closed
                        continue;
                    } else if (openList.find(opposite) != openList.end()) {
                        // The vertex is open, so check if we've found a better path
                        VertexRecord<V, E> record = (*openList.find(opposite));
                        record.cost = std::min<E>(record.cost, newCost);
                    } else {
                        // We have an unvisited vertex, so record it
                        VertexRecord<V, E> record;
                        record.vertex = opposite;
                        record.cost = newCost;
                        record.edge = e;

                        openList.push_back(record);
                    }
                }
                
                // Move the current vertex to the closed list
                closedList.push_back(current);

                // Make sure the open list is a heap
                std::make_heap(openList.begin(), openList.end(), std::greater<VertexRecord<V, E>>{});
            }

            // Make sure we've reached the goal vertex
            if (current.vertex != endVertex) {
                return false;
            }
            
            // Compile a list of edges we took to get to this path
            while (current.vertex != startVertex) {
                path->push_back(current.edge);
                current = (*closedList.find(graph->opposite(current.vertex, current.edge)));
            }

            // Return the reversed path
            std::reverse(std::begin(*path), std::end(*path));
            return true;
        }

        // A* algorithm, which uses dijkstra's algorithm plus a heuristic
        static bool astar(std::vector<Edge<E>*> *path, AdjacencyListGraph<V, E> *graph, Vertex<V> *startVertex, Vertex<V> *endVertex, Heuristic<V, E> *heuristic) {
            // Initialize the open and closed lists
            AdaptableSearchableVector<V, E> openList;
            AdaptableSearchableVector<V, E> closedList;

            // Initialize the record for the start node
            VertexRecord<V, E> start;
            start.vertex = startVertex;
            start.costSoFar = 0;
            start.cost = heuristic->estimate(startVertex, endVertex);
            openList.push_back(start);

            // Iterate through processing each vertex
            VertexRecord<V, E> current;
            while (openList.size() > 0) {
                // Get the smallest element in the open list
                std::pop_heap(openList.begin(), openList.end(), std::greater<VertexRecord<V, E>>{});
                current = openList.back();
                openList.pop_back();

                // If the current vertex is the end vertex, break
                if (current.vertex == endVertex) {
                    break;
                }

                // Loop through each edge in the vertex
                for (Edge<E> *e : *graph->outgoingEdges(current.vertex)) {
                    // Get the cost estimate for each new vertex
                    Vertex<V> *opposite = graph->opposite(current.vertex, e);
                    E newCost = current.costSoFar + e->getElement();

                    if (closedList.find(opposite) != closedList.end()) {
                        // Either skip or remove the vertex from the closed list
                        VertexRecord<V, E> record = (*closedList.find(opposite));

                        // If we didn't find a shorter route, skip
                        if (record.costSoFar <= newCost) {
                            continue;
                        }

                        // Otherwise, remove it from the closed list and add it back to the open list
                        closedList.erase(closedList.find(opposite));

                        record.cost = newCost + (record.cost - record.costSoFar);
                        record.costSoFar = newCost;
                        record.edge = e;
                    } else if (openList.find(opposite) != openList.end()) {
                        // The vertex is open, so check if we've found a better path
                        VertexRecord<V, E> record = (*openList.find(opposite));

                        // Continue if we didn't find a better path
                        if (record.costSoFar <= newCost) {
                            continue;
                        }

                        // Update the costs and heuristic
                        record.cost = newCost + (record.cost - record.costSoFar);
                        record.costSoFar = newCost;
                        record.edge = e;
                    } else {
                        // We have an unvisited vertex, so record it
                        VertexRecord<V, E> record;

                        record.vertex = opposite;
                        record.edge = e;
                        record.costSoFar = newCost;
                        record.cost = newCost + heuristic->estimate(opposite, endVertex);

                        openList.push_back(record);
                    }
                }
                
                // Move the current vertex to the closed list
                closedList.push_back(current);

                // Make sure the open list is a heap
                std::make_heap(openList.begin(), openList.end(), std::greater<VertexRecord<V, E>>{});
            }

            // Make sure we've reached the goal vertex
            if (current.vertex != endVertex) {
                return false;
            }
            
            // Compile a list of edges we took to get to this path
            while (current.vertex != startVertex) {
                path->push_back(current.edge);
                current = (*closedList.find(graph->opposite(current.vertex, current.edge)));
            }

            // Return the reversed path
            std::reverse(std::begin(*path), std::end(*path));
            return true;
        }
};

#endif