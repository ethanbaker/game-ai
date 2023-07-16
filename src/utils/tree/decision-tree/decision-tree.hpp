#ifndef DECISION_TREE
#define DECISION_TREE

#include "../../kinematics/kinematics.hpp"
#include "../tree.hpp"
#include "../../../environment/environment.hpp"
#include "../../../steering/steering.hpp"


// DecisionTreeNode represents a node in a decision tree
class DecisionTreeNode : public AbstractDMNode {
    public:
        // Make a decision and walk through the tree
        virtual void *run(EnvironmentParameters *environment) = 0;
};

// Action class represents a leaf of the decision tree, or an action performed on a character
class Action : public DecisionTreeNode {
    public:
        // A list of behaviors the action is associated with
        std::vector<WeightedBehavior> behaviors;

        // Make a decision and walk through the tree
        virtual void *run(EnvironmentParameters *environment);
};

// Decision class represents an interior node of the decision tree, or a conditional expression that needs to be evaluated
class Decision : public DecisionTreeNode {
    private:
        DecisionTreeNode *truthNode;
        DecisionTreeNode *falseNode;

    public:
        // Execute the decision tree node corresponding to a given decision
        virtual void *run(EnvironmentParameters *environment) = 0;

        // Set the truth descendant node of this node
        void setTruthNode(DecisionTreeNode *node);

        // Get the truth descendant node of this node
        DecisionTreeNode *getTruthNode();

        // Set the false descendant node of this node
        void setFalseNode(DecisionTreeNode *node);

        // Get the false descendant node of this node
        DecisionTreeNode *getFalseNode();
};

// DecisionMulti class represents an interior node of the decision tree that can send control flow to more than two children
// Overrride this class with a 'run' method that parses a specified attribute from the game envrionment and uses it to find the list of nodes in the children
class DecisionMulti : public DecisionTreeNode {
    private:
        std::map<std::string, std::vector<DecisionTreeNode*>> children;
        std::string testValue;

    public:
        // Execute the decision tree node corresponding to the given decision
        void *run(EnvironmentParameters *envrionment);

        // Add a child node to this node
        void addChildNode(DecisionTreeNode* child, std::string onValue);

        // Get a list of the children for this node
        std::map<std::string, std::vector<DecisionTreeNode*>> getChildren();

        // Set the test value of the node
        void setTestValue(std::string testValue);
};

// DecisionTree class represents a decision tree that can be used in a game environment
class DecisionTree : public AbstractDMTree {
    public:
        // Make a decision
        void *decide(EnvironmentParameters *environment);

        // Add a root to the tree
        Vertex<AbstractDMNode*> *addRoot(DecisionTreeNode *node);

        // Add a new node to the decision tree
        Vertex<AbstractDMNode*> *insertNode(DecisionTreeNode *data, Vertex<AbstractDMNode*> *parent, bool value);

        // Add a new node to the decision tree (for a multinode)
        Vertex<AbstractDMNode*> *insertNode(DecisionMulti *data, Vertex<AbstractDMNode*> *parent, std::string value);

        // Set a test value for a given multinode
        void setTestValue(Vertex<AbstractDMNode*> *node, std::string value);

        // Verify that a given node is a Decision Node
        Decision *validateDecisionNode(AbstractDMNode *v);

        // Verify that a given node is an Action Node
        Action *validateActionNode(AbstractDMNode *v);

        // Verify that a given node is an DecisionMulti Node
        DecisionMulti *validateMultiNode(AbstractDMNode *v);
};

#endif