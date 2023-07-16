#include "graph.hpp"

// Generate a random graph with a given number of vertices and minimum outgoing and incoming edges per vertex
void generateRandomGraph(AdjacencyListGraph<std::string, std::string> *graph, int verticesCount, int minOutgoingEdges, int minIncomingEdges, int maxWeight) {
    // Create a bunch of vertices
    std::vector<Vertex<std::string>*> vertices;
    for (int i = 0; i < verticesCount; i++) {
        if (i % 1000 == 0) {
            std::cout << "Creating vertex " << i << "\n";
        }
        vertices.push_back(graph->insertVertex(std::to_string(i)));
    }

    // Create incoming and outgoing edges for each vertex
    int count = 0;
    for (Vertex<std::string> *v : graph->vertices()) {
        if (count % 1000 == 0) {
            std::cout << "Creating edges for vertex " << count << "\n";
        }
        count++;

        for (int i = 0; i < minOutgoingEdges - graph->outDegree(v); i++) {
            Vertex<std::string> *rv = vertices.at(std::rand() % vertices.size());
            std::string rw = std::to_string(std::rand() % maxWeight);
            
            // Only accept new edges or edges where v is not the source
            if ((graph->getEdge(v, rv) == nullptr || graph->endVertices(graph->getEdge(v, rv))[1] == v) && v != rv) {
                graph->insertEdge(v, rv, rw);
            } else {
                i--;
            }
        }

        for (int i = 0; i < minIncomingEdges - graph->inDegree(v); i++) {
            Vertex<std::string> *rv = vertices.at(std::rand() % vertices.size());
            std::string rw = std::to_string(std::rand() % maxWeight);
            
            // Only accept new edges or edges where v is not the destination
            if ((graph->getEdge(v, rv) == nullptr || graph->endVertices(graph->getEdge(v, rv))[0] == v) && v != rv) {
                graph->insertEdge(rv, v, rw);
            } else {
                i--;
            }
        }
    }
}

// Generate a random grid graph according to given parameters
void generateRandomGridGraph(AdjacencyListGraph<Grid<int>, int> *graph, int minOutgoingEdges, int minIncomingEdges, int maxWeight, int rows, int cols) {
    // Create a bunch of vertices
    std::vector<Vertex<Grid<int>>*> vertices;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if ((row * rows + col) % 1000 == 0) {
                std::cout << "Creating vertex " << (row * rows + col) << "\n";
            }

            Grid<int> g;
            g.row = row;
            g.column = col;
            vertices.push_back(graph->insertVertex(g));
        }
    }

    // Create incoming and outgoing edges for each vertex
    int count = 0;
    for (Vertex<Grid<int>> *v : graph->vertices()) {
        if (count % 1000 == 0) {
            std::cout << "Creating edges for vertex " << count << "\n";
        }
        count++;

        for (int i = 0; i < minOutgoingEdges - graph->outDegree(v); i++) {
            Vertex<Grid<int>> *rv = vertices.at(std::rand() % vertices.size());
            int rw = std::rand() % maxWeight;
            
            // Only accept new edges or edges where v is not the source
            if ((graph->getEdge(v, rv) == nullptr || graph->endVertices(graph->getEdge(v, rv))[1] == v) && v != rv) {
                graph->insertEdge(v, rv, rw);
            } else {
                i--;
            }
        }

        for (int i = 0; i < minIncomingEdges - graph->inDegree(v); i++) {
            Vertex<Grid<int>> *rv = vertices.at(std::rand() % vertices.size());
            int rw = std::rand() % maxWeight;
            
            // Only accept new edges or edges where v is not the destination
            if ((graph->getEdge(v, rv) == nullptr || graph->endVertices(graph->getEdge(v, rv))[0] == v) && v != rv) {
                graph->insertEdge(rv, v, rw);
            } else {
                i--;
            }
        }
    }
}