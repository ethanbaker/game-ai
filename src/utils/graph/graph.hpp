// Graph represents an adjacency list graph implemtation for world building
#ifndef ALGRAPH
#define ALGRAPH

#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <random>
#include "vertex.hpp"

// Vertex base (to be implemented later)
template <typename V>
class Vertex {
    private:
        V element; // Encapsulated element
    
    public:
        // Constructor for a vertex which encapsulates an element
        Vertex(V element) {
            this->element = element;
        }

        // Get the encapsulated element in the vertex
        V getElement() {
            return this->element;
        }

        // Set the encapsulated element in the vertex
        void setElement(V element) {
            this->element = element;
        }

        virtual ~Vertex() {};
};

// Edge base (to be implemented later)
template <typename E>
class Edge {
    private:
        E element;                                        // Encapsulated element

    public:
        // Default constructor for an edge which takes a value to encapsulate and two associated vertices
        Edge(E element) {
            this->element = element;
        }

        // Get the encapsulated element in the edge
        E getElement() {
            return this->element;
        }

        // Set the encapsulated element in the edge
        void setElement(E element) {
            this->element = element;
        }

        virtual ~Edge() {}
};

// Vertex representation for an adjacency list graph
template <typename V, typename E>
class ALVertex : public Vertex<V> {
    private:
        std::vector<Edge<E>*> outgoing; // Outgoing edges
        std::vector<Edge<E>*> incoming; // Incoming edges

    public:
        // Constructor for an ALVertex which encapsulates an element and sets directed status
        ALVertex(V element, bool directed) : Vertex<V>(element) {
            this->outgoing = std::vector<Edge<E>*>();
            if (directed) {
                this->incoming = std::vector<Edge<E>*>();
            } else {
                this->incoming = this->outgoing;
            }
        }

        // Get the outgoing list of edges for the vertex
        std::vector<Edge<E>*> *getOutgoing() {
            return &this->outgoing;
        }

        // Get the incoming list of edges for the vertex
        std::vector<Edge<E>*> *getIncoming() {
            return &this->incoming;
        }

        // Add an outgoing edge to the vertex
        void addOutgoing(Edge<E> *edge) {
            this->outgoing.push_back(edge);
        }

        // Add an incoming edge to the vertex
        void addIncoming(Edge<E> *edge) {
            this->incoming.push_back(edge);
        }
};

// Edge representation for an adjacency list graph
template <typename V, typename E>
class ALEdge : public Edge<E> {
    private:
        std::array<Vertex<V>*, 2> endpoints;                      // Vertex endpoints of the edge

    public:
        // Constructor for an ALEdge which encapsulates an element and represents a connection between vertices
        ALEdge(E element, Vertex<V> *v1, Vertex<V> *v2) : Edge<E>(element) {
            this->endpoints[0] = v1;
            this->endpoints[1] = v2;
        }

        // Get the vertex endpoints from the edge
        std::array<Vertex<V>*, 2> getEndpoints() {
            return this->endpoints;
        }
};

// Graph class represents an adjacency list
template <typename V, typename E>
class AdjacencyListGraph {
    private:
        bool directed;
        std::vector<Vertex<V>*> vertexList;
        std::vector<Edge<E>*> edgeList;

    public:
        // Default constructor for an adjacency list graph
        AdjacencyListGraph() : AdjacencyListGraph<V, E>(false) {}

        // Constructor for an adjaceny list graph that contains a directed status
        AdjacencyListGraph(bool directed) {
            this->directed = directed;
            this->vertexList = std::vector<Vertex<V>*>();
            this->edgeList = std::vector<Edge<E>*>();
        }

        // Return the number of vertices in the graph
        int numVertices() {
            return this->vertexList.size();
        }

        // Return the list of vertices in the graph
        std::vector<Vertex<V>*> vertices() {
            return this->vertexList;
        }

        // Return the number of edges in the graph
        int numEdges() {
            return this->edgeList.size();
        }

        // Return the list of edges in the graph
        std::vector<Edge<E>*> edges() {
            return this->edgeList;
        }

        // Return a list of outgoing edges associated with a vertex
        std::vector<Edge<E>*> *outgoingEdges(Vertex<V> *v) {
            ALVertex<V, E> *vertex = validateALVertex(v);

            return vertex->getOutgoing();
        }

        // Return a list of incoming edges associated with a vertex
        std::vector<Edge<E>*> *incomingEdges(Vertex<V> *v) {
            ALVertex<V, E> *vertex = validateALVertex(v);

            return vertex->getIncoming();
        }

        // Return an edge associated with two vertices (if it exists)
        Edge<E> *getEdge(Vertex<V> *v1, Vertex<V> *v2) {
            ALVertex<V, E> *vertex1 = validateALVertex(v1);
            ALVertex<V, E> *vertex2 = validateALVertex(v2);

            // Loop through each edge in the graph
            for (Edge<E> *e : this->edgeList) {
                ALEdge<V, E> *edge = validateALEdge(e);

                std::array<Vertex<V>*, 2> ends = edge->getEndpoints();

                if (this->directed && ends[1] == vertex1 && ends[0] == vertex2) {
                    return edge;
                }
                if (ends[0] == vertex1 && ends[1] == vertex2) {
                    return edge;
                }
            }

            return nullptr;
        }

        // Return the number of outgoing edges from a vertex
        int outDegree(Vertex<V> *v) {
            ALVertex<V, E> *vertex = validateALVertex(v);
            
            return vertex->getOutgoing()->size();
        }

        // Return the number of incoming edges from a vertex
        int inDegree(Vertex<V> *v) {
            ALVertex<V, E> *vertex = validateALVertex(v);

            return vertex->getIncoming()->size();
        }

        // Insert a new vertex into the graph
        Vertex<V> *insertVertex(V vertexData) {
            ALVertex<V, E> *vertex = new ALVertex<V, E>(vertexData, this->directed);

            this->vertexList.push_back(vertex);

            return vertex;
        }

        // Insert a new edge into the graph
        Edge<E> *insertEdge(Vertex<V> *org, Vertex<V> *dest, E edgeData) {
            ALVertex<V, E> *origin = validateALVertex(org);
            ALVertex<V, E> *destination = validateALVertex(dest);

            ALEdge<V, E> *edge = new ALEdge<V, E>(edgeData, origin, destination);

            this->edgeList.push_back(edge);
            origin->addOutgoing(edge);
            destination->addIncoming(edge);

            return edge;
        }

        // Remove a vertex from the graph
        void removeVertex(Vertex<V> *vertex) {
            // Find the edges to remove
            for (Edge<E> *e : this->edges()) {
                if (vertex == this->endVertices(e)[0] || vertex == this->endVertices(e)[1]) {
                    this->removeEdge(e);
                }
            }

            // Delete the vertex from the vertex list
            for (typename std::vector<Vertex<V> *>::iterator it = this->vertexList.begin(); it != this->vertexList.end(); it++) {
                if (*it == vertex) {
                    this->vertexList.erase(it);
                    break;
                }
            }

            delete vertex;
        }

        // Remove an edge from the graph
        void removeEdge(Edge<E> *e) {
            ALEdge<V, E> *edge = validateALEdge(e);

            // Get the vertex endpoints
            std::array<Vertex<V>*, 2> ends = edge->getEndpoints();
            ALVertex<V, E> *origin = validateALVertex(ends[0]);
            ALVertex<V, E> *destination = validateALVertex(ends[1]);


            // Remove the edge from the endpoints
            for (typename std::vector<Edge<E> *>::iterator it = origin->getOutgoing()->begin(); it != origin->getOutgoing()->end(); it++) {
                if (*it == edge) {
                    origin->getOutgoing()->erase(it);
                    break;
                }
            }
            for (typename std::vector<Edge<E> *>::iterator it = destination->getIncoming()->begin(); it != destination->getIncoming()->end(); it++) {
                if (*it == edge) {
                    destination->getIncoming()->erase(it);
                    break;
                }
            }

            // Remove the edge from the edge list
            for (typename std::vector<Edge<E> *>::iterator it = this->edgeList.begin(); it != this->edgeList.end(); it++) {
                if (*it == edge) {
                    this->edgeList.erase(it);
                    break;
                } 
            }

            delete e;
        }

        // Find the opposite vertex from a given edge and vertex
        Vertex<V> *opposite(Vertex<V> *vertex, Edge<E> *e) {
            ALEdge<V, E> *edge = validateALEdge(e);

            std::array<Vertex<V>*, 2> ends = edge->getEndpoints();

            if (ends[0] == vertex) {
                return ends[1];
            }
            if (ends[1] == vertex) {
                return ends[0];
            }

            return nullptr;
        }

        // Get the endpoints of a given edge
        std::array<Vertex<V>*, 2> endVertices(Edge<E> *e) {
            ALEdge<V, E> *edge = validateALEdge(e);

            return edge->getEndpoints();
        }

        // Return true if the graph contains this vertex
        bool containsVertex(Vertex<V> *v) {
            try {
                validateALVertex(v);
            } catch (const std::exception) {
                return false;
            }
            return true;
        }

        // Return true if the graph contains the edge
        bool containsEdge(Edge<E> *e) {
            try {
                validateALEdge(e);
            } catch (const std::exception) {
                return false;
            }
            return true;
        }

        // Helper method to validate vertices
        ALVertex<V, E> *validateALVertex(Vertex<V> *v) {
            try {
                return dynamic_cast<ALVertex<V, E>*>(v);
            } catch (const std::bad_cast& e) {
                throw std::invalid_argument("Edge is not valid");
            }
        }

        // Helper method to validate edges
        ALEdge<V, E> *validateALEdge(Edge<E> *e) {
            try {
                return dynamic_cast<ALEdge<V, E>*>(e);
            } catch (const std::bad_cast& e) {
                throw std::invalid_argument("Edge is not valid");
            }
        }
};

/* Util Functions */

// Convert a graph into a csv file
template <typename V, typename E>
void writeGraphToCSV(AdjacencyListGraph<V, E> graph, std::string filename) {
    std::ofstream file;
    file.open(filename);

    // Write the header of the CSV file
    file << "origin,destination,element\n";

    // Write each edge
    for (Edge<E> *e : graph.edges()) {
        file << graph.endVertices(e)[0]->getElement() << "," << graph.endVertices(e)[1]->getElement() << "," << e->getElement() << "\n";
    }

    file.close();
}

// Convert a grid graph into a csv file
template <typename V>
void writeGridGraphToCSV(AdjacencyListGraph<Grid<V>, V> graph, std::string filename) {
    std::ofstream file;
    file.open(filename);

    // Write the header of the CSV file
    file << "origin-row,origin-col,destination-row,destination-col,element\n";

    // Write each edge
    for (Edge<V> *e : graph.edges()) {
        file << graph.endVertices(e)[0]->getElement().row << "," << graph.endVertices(e)[0]->getElement().column << "," << graph.endVertices(e)[1]->getElement().row << "," << graph.endVertices(e)[1]->getElement().column << "," << e->getElement() << "\n";
    }

    file.close();
}

// Read a graph from a csv file
template <typename V, typename E>
void readGraphFromCSV(AdjacencyListGraph<V, E> *graph, std::string filename) {
    // Read in the file
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    // Map to represent vertices we have already added
    std::map<std::string, Vertex<V>*> vertexMap;

    // Parse line by line
    std::string line;
    std::string buf;
    while (std::getline(file, line)) {
        std::string origin;
        std::string dest;
        std::string weight;

        std::istringstream stream;
        stream.str(line);

        // Parse column by column
        int col = 0;
        while (std::getline(stream, buf, ',')) {
            if (col == 0) {
                origin.append(buf);
            } else if (col == 1) {
                dest.append(buf);
            } else if (col == 2) {
                weight.append(buf);
            }
            col++;
        }

        // Don't consume the header row
        if (origin == "origin") {
            continue;
        }

        // If vertices have not been added already, add them
        if (vertexMap.find(origin) == vertexMap.end()) {
            vertexMap[origin] = graph->insertVertex(origin);
        }
        if (vertexMap.find(dest) == vertexMap.end()) {
            vertexMap[dest] = graph->insertVertex(dest);
        }

        // Add a new edge between the vertices
        graph->insertEdge(vertexMap[origin], vertexMap[dest], stoi(weight));
    }
}

// Read a grid graph from a csv file
template <typename E>
void readGridGraphFromCSV(AdjacencyListGraph<Grid<E>, E> *graph, std::string filename) {
    // Read in the file
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    // Map to represent vertices we have already added
    std::map<Grid<E>, Vertex<Grid<E>>*> vertexMap;

    // Parse line by line
    std::string line;
    std::string buf;
    while (std::getline(file, line)) {
        std::string originRow;
        std::string originCol;
        std::string destRow;
        std::string destCol;
        std::string weight;

        std::istringstream stream;
        stream.str(line);

        // Parse column by column
        int col = 0;
        while (std::getline(stream, buf, ',')) {
            if (col == 0) {
                originRow.append(buf);
            } else if (col == 1) {
                originCol.append(buf);
            } else if (col == 2) {
                destRow.append(buf);
            } else if (col == 3) {
                destCol.append(buf);
            } else if (col == 4) {
                weight.append(buf);
            }
            col++;
        }

        // Don't consume the header row
        if (originRow == "origin-row") {
            continue;
        }

        Grid<E> originGrid;
        originGrid.row = stoi(originRow);
        originGrid.column = stoi(originCol);

        Grid<E> destGrid;
        destGrid.row = stoi(destRow);
        destGrid.column = stoi(destCol);

        // If vertices have not been added already, add them
        if (vertexMap.find(originGrid) == vertexMap.end()) {
            vertexMap[originGrid] = graph->insertVertex(originGrid);
        }
        if (vertexMap.find(destGrid) == vertexMap.end()) {
            vertexMap[destGrid] = graph->insertVertex(destGrid);
        }

        // Add a new edge between the vertices
        graph->insertEdge(vertexMap[originGrid], vertexMap[destGrid], stoi(weight));
    }
}

// Generate a random graph according to given parameters
void generateRandomGraph(AdjacencyListGraph<std::string, std::string> *graph, int verticesCount, int minOutgoingEdges, int minIncomingEdges, int maxWeight);

// Generate a random grid graph according to given parameters
void generateRandomGridGraph(AdjacencyListGraph<Grid<int>, int> *graph, int minOutgoingEdges, int minIncomingEdges, int maxWeight, int rows, int cols);

#endif