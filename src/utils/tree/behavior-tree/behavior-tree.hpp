#ifndef BEHAVIOR_TREE
#define BEHAVIOR_TREE

#include <vector>
#include "../../../environment/environment.hpp"
#include "../tree.hpp"

class BehaviorTree;

// Boolean helper class allows for easy pointer functionality
class Boolean {
    private:
        bool value = false;    

    public:
        // Setup the value of the boolean
        Boolean(bool value);

        // Return the encapsulated value
        bool is();

};

static Boolean BOOL_TRUE = Boolean(true);
static Boolean BOOL_FALSE = Boolean(false);

// Abstract BehaviorTreeNode class represents a node in a behavior tree
class BehaviorTreeNode : public AbstractDMNode {
    private:
        // Behavior tree this node belongs to
        BehaviorTree *tree = nullptr;

        // Vertex this node is encapsulated by
        Vertex<AbstractDMNode*> *vertex = nullptr;

    public:
        // Set the tree reference for this node
        void setTree(BehaviorTree *tree);

        // Get the tree reference for this node
        BehaviorTree *getTree();

        // Set the vertex this node is encapsulated by
        void setVertex(Vertex<AbstractDMNode*> *vertex);

        // Get the vertex this node is encapsulated by
        Vertex<AbstractDMNode*> *getVertex();

        // Run the given node and return whether it was successful
        virtual void *run(EnvironmentParameters *environment) = 0;
};

/* Specialized Behavior Tree Nodes */

// SelectorNode class represents a selector node in a behavior tree
class SelectorNode : public BehaviorTreeNode {
    public:
        // Stop and return true for the first child that returns true. Otherwise, return false
        void *run(EnvironmentParameters *environment);
};

// SequenceNode class represents a sequence node in a behavior tree
class SequenceNode : public BehaviorTreeNode {
    public:
        // Stop and return false for the first child that returns false. Otherwise, return true
        void *run(EnvironmentParameters *environment);
};

// RandomNode class represents a random node in a behavior tree
class RandomNode : public BehaviorTreeNode {
    public:
        // Choose a random child to run and return that result
        void *run(EnvironmentParameters *environment);
};

/* Decorator Nodes */

// Abstract Decorator class represents a decorator node in a behavior tree
class Decorator : public BehaviorTreeNode {
    private:
        // The child the decorator represents
        BehaviorTreeNode *c;

    public:
        // Default constructor for a decorator, which initializes its child
        Decorator(BehaviorTreeNode *child);

        // Get the child this decorator encapsulates
        BehaviorTreeNode *child();

        // Modify the output of the child in some way in the run method
        virtual void *run(EnvironmentParameters *environment) = 0;
};

// LimitDecorator class represents a decorator that limits the amount of times a child can be run
class LimitDecorator : public Decorator {
    private:
        int limit = 0;
        int count = 0;

    public:
        // Default constructor for a LimitDecorator
        LimitDecorator(BehaviorTreeNode *child, int limit, int count);

        // Only run the child for a given count
        void *run(EnvironmentParameters *environment);
};

// RepeatUntilDecorator repeats a child until it returns a given condition
class RepeatUntilDecorator : public Decorator {
    private:
        bool condition = false;

    public:
        // Default constructor for a RepeatUntilDecorator
        RepeatUntilDecorator(BehaviorTreeNode *child, bool condition);

        // Run the child until that condition is true
        void *run(EnvironmentParameters *environment);
};

// RepeatUntilDecorator repeats a child until it returns a given condition
class RepeatDecorator : public Decorator {
    private:
        int max = 0;
        int count = 0;

    public:
        // Default constructor for a RepeatUntilDecorator
        RepeatDecorator(BehaviorTreeNode *child, int max);

        // Run the child until that condition is true
        void *run(EnvironmentParameters *environment);
};


// InverterDecorator returns the inverted result from its child
class InverterDecorator : public Decorator {
    public:
        // Default constructor for an InverterDecorator
        InverterDecorator(BehaviorTreeNode *child);

        // Return the inverse condition from the child
        void *run(EnvironmentParameters *environment);
};

/* BehaviorTree Class */
class BehaviorTree : public AbstractDMTree {
    public:
        // Make a decision
        void *decide(EnvironmentParameters *environment);

        // Add a root node to the behavior tree
        Vertex<AbstractDMNode*> *addRoot(BehaviorTreeNode* node);

        // Add a node to the behavior tree
        Vertex<AbstractDMNode*> *addNode(BehaviorTreeNode* node, Vertex<AbstractDMNode*> *parent, bool edgeData);

        // Get children for an associated node
        std::vector<Vertex<AbstractDMNode*>*> getChildren(Vertex<AbstractDMNode*> *node);

        // Validate a given node to be a behavior tree node
        BehaviorTreeNode *validateNode(AbstractDMNode *node);
};


#endif