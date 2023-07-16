#ifndef TREE
#define TREE

#include "../graph/graph.hpp"
#include "../../environment/environment.hpp"

// Tree class represents a tree representation extended from an adjacency list graph
template <typename V, typename E>
class Tree : private AdjacencyListGraph<V, E> {
    private:
        Vertex<V> *root = nullptr; // The root of the tree

        // Helper function to build a string representation
        void toStringHelper(std::string *str, std::string indent, Vertex<V> *root) {
            if (root == nullptr) {
                return;
            }

            *str += indent + root->getElement()->name + "\n";

            for (Vertex<V> *child : this->children(root)) {
                this->toStringHelper(str, indent + " ", child);
            }
        }

    public:
        // Default constructor for Tree
        Tree() : AdjacencyListGraph<V, E>(true) {}

        // Return the parent of a given node
        Vertex<V> *parent(Vertex<V> *vertex) {
            return this->opposite(vertex, this->incomingEdges(vertex)->at(0));
        }

        // Return a list of children for a given node
        std::vector<Vertex<V> *> children(Vertex<V> *vertex) {
            std::vector<Vertex<V> *> children;
            if (vertex == nullptr) {
                return children;
            }

            for (Edge<E> *e : *this->outgoingEdges(vertex)) {
                children.push_back(this->opposite(vertex, e));
            }

            return children;
        }

        // Get the number of children for a given node
        int numChildren(Vertex<V> *vertex) {
            return this->outDegree(vertex);
        }

        // Returns true if the node has one or more children
        bool isInternal(Vertex<V> *vertex) {
            return this->numChildren(vertex) > 0;
        }

        // Returns true if the node has zero children
        bool isLeaf(Vertex<V> *vertex) {
            return this->numChildren(vertex) == 0;
        }

        // Returns true if the node is the root of the tree
        bool isRoot(Vertex<V> *vertex) {
            return this->root == vertex;
        }

        // Get the root of the tree
        Vertex<V> *getRoot() {
            return this->root;
        }

        // Get the size of the tree
        int size() {
            return this->numVertices();
        }

        // Returns true if the tree is empty
        bool isEmpty() {
            return this->size() == 0;
        }

        // Set the root of the tree
        Vertex<V> *addRoot(V rootData) {
            Vertex<V> *oldRoot = this->root;
            this->root = this->insertVertex(rootData);

            // If the old root didn't exist, do nothing
            if (oldRoot == nullptr) {
                return this->root;
            }

            // Otherwise, add all edges of the old root to the new root
            if (this->incomingEdges(oldRoot) != nullptr) {
                for (Edge<E> *e : *this->incomingEdges(oldRoot)) {
                    this->insertEdge(this->root, this->endVertices(e)[1], e->getElement());
                }
            }

            // Delete the old root
            this->removeVertex(oldRoot);

            return this->root;
        }

        // Get an edge that might exist between two nodes
        Edge<E> *getEdge(Vertex<V> *vertex1, Vertex<V> *vertex2) {
            for (Edge<E> *e : this->outgoingEdges(vertex1)) {
                if (this->opposite(vertex1, e) == vertex2) {
                    return e;
                }
            }

            for (Edge<E> *e : this->incomingEdges(vertex1)) {
                if (this->opposite(vertex1, e) == vertex2) {
                    return e;
                }
            }
        }

        // Set an edge between two nodes to a given value
        Edge<E> *setEdge(Vertex<V> *vertex1, Vertex<V> *vertex2, E edgeData) {
            Edge<E> *edge = this->getEdge(vertex1, vertex2);

            edge->setElement(edgeData);
        }

        // Insert a new node into the tree
        Vertex<V> *insertNode(V vertexData, E edgeData, Vertex<V> *parent) {
            Vertex<V> *vertex = this->insertVertex(vertexData);
            this->insertEdge(parent, vertex, edgeData);

            return vertex;
        }

        // Return the tree as a string representation
        std::string toString() {
            std::string output = "[";
            this->toStringHelper(&output, "", this->getRoot());
            output += "]";

            return output;
        }
};

// AbstractDMNode represents an abstract node in a decision making tree
class AbstractDMNode {
    public:
        // Name of the node, used for testing purposes
        std::string name;

        // Run the node and return a pointer related to the outcome of the run
        virtual void *run(EnvironmentParameters *environment) = 0;
};

// AbstractDMTree represents an abstract decision making tree
class AbstractDMTree : public Tree<AbstractDMNode*, bool> {
    public:
        // Run the decision making tree and perform an output
        virtual void *decide(EnvironmentParameters *parameters) = 0;
};

#endif