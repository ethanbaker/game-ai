#include <vector>
#include "behavior-tree.hpp"
#include "../../../environment/environment.hpp"

/* Boolean Class */

Boolean::Boolean(bool value) {
    this->value = value;
}

bool Boolean::is() {
    return this->value;
}

/* BehaviorTreeNode Class */

void BehaviorTreeNode::setTree(BehaviorTree *tree) {
    this->tree = tree;
}

BehaviorTree *BehaviorTreeNode::getTree() {
    return this->tree;
}

void BehaviorTreeNode::setVertex(Vertex<AbstractDMNode*> *vertex) {
    this->vertex = vertex;
}

Vertex<AbstractDMNode*> *BehaviorTreeNode::getVertex() {
    return this->vertex;
}

/* SelectorNode Class */

void *SelectorNode::run(EnvironmentParameters *environment) {
    std::vector<Vertex<AbstractDMNode*>*> children = this->getTree()->getChildren(BehaviorTreeNode::getVertex());

    // Make sure the node has at least one child
    if (children.size() == 0) {
        return &BOOL_FALSE;
    }

    // Loop through the children
    for (Vertex<AbstractDMNode*> *child : children) {
        AbstractDMNode *node = child->getElement();

        if (((Boolean*) node->run(environment))->is()) {
            return &BOOL_TRUE;
        }
    }

    return &BOOL_FALSE;
}

/* SequenceNode Class */

void *SequenceNode::run(EnvironmentParameters *environment) {
    std::vector<Vertex<AbstractDMNode*>*> children = this->getTree()->getChildren(BehaviorTreeNode::getVertex());

    // Make sure the node has at least one child
    if (children.size() == 0) {
        return &BOOL_FALSE;
    }

    // Loop through the children
    for (Vertex<AbstractDMNode*> *child : children) {
        AbstractDMNode *node = child->getElement();

        if (!((Boolean*) node->run(environment))->is()) {
            return &BOOL_FALSE;
        }
    }

    return &BOOL_TRUE;
}

/* RandomNode Class */

void *RandomNode::run(EnvironmentParameters *environment) {
    std::vector<Vertex<AbstractDMNode*>*> children = this->getTree()->getChildren(BehaviorTreeNode::getVertex());

    // Make sure the node has at least one child
    if (children.size() == 0) {
        return &BOOL_FALSE;
    }

    int index = rand() % children.size();
    return children.at(index)->getElement()->run(environment);
}

/* Decorator Class */

Decorator::Decorator(BehaviorTreeNode *child) {
    this->c = child;
}

BehaviorTreeNode *Decorator::child() {
    return this->c;
}

/* LimitDecorator Class */

LimitDecorator::LimitDecorator(BehaviorTreeNode *child, int limit, int count) : Decorator(child) {
    this->limit = limit;
    this->count = count;
}

void *LimitDecorator::run(EnvironmentParameters *environment) {
    if (count >= limit) {
        return &BOOL_FALSE;
    }

    count++;
    return this->child()->run(environment);
}

/* RepeatUntilDecorator Class */

RepeatUntilDecorator::RepeatUntilDecorator(BehaviorTreeNode *child, bool condition) : Decorator(child) {
    this->condition = condition;
}

void *RepeatUntilDecorator::run(EnvironmentParameters *environment) {
    while (((Boolean*) this->child()->run(environment))->is() != condition);

    return &BOOL_TRUE;
}

/* RepeatDecorator Class */

RepeatDecorator::RepeatDecorator(BehaviorTreeNode *child, int max) : Decorator(child) {
    this->max = max;
}

void *RepeatDecorator::run(EnvironmentParameters *environment) {
    if (this->count < this->max) {
        if (((Boolean*) this->child()->run(environment))->is()) {
            this->count++;
        }
    } else {
        this->count = 0;
    }

    return this->count >= this->max ? &BOOL_TRUE : &BOOL_FALSE;
}

/* InverterDecorator Class */

InverterDecorator::InverterDecorator(BehaviorTreeNode *child) : Decorator(child) {};

void *InverterDecorator::run(EnvironmentParameters *environment) {
    return ((Boolean*) this->child()->run(environment))->is() ? &BOOL_FALSE : &BOOL_TRUE;
}

/* BehaviorTree Class */

void *BehaviorTree::decide(EnvironmentParameters *environment) {
    BehaviorTreeNode *root = this->validateNode(Tree::getRoot()->getElement());

    return root->run(environment);
}

Vertex<AbstractDMNode*> *BehaviorTree::addRoot(BehaviorTreeNode *node) {
    Vertex<AbstractDMNode*> *vertex = Tree::addRoot(node);
    node->setVertex(vertex);
    node->setTree(this);

    return vertex;
}

Vertex<AbstractDMNode*> *BehaviorTree::addNode(BehaviorTreeNode *node, Vertex<AbstractDMNode*> *parent, bool edgeData) {
    Vertex<AbstractDMNode*> *vertex = Tree::insertNode(node, edgeData, parent);
    node->setVertex(vertex);
    node->setTree(this);

    return vertex;
}

std::vector<Vertex<AbstractDMNode*>*> BehaviorTree::getChildren(Vertex<AbstractDMNode*> *node) {
    return Tree::children(node);
}

BehaviorTreeNode *BehaviorTree::validateNode(AbstractDMNode *node) {
    try {
        return dynamic_cast<BehaviorTreeNode*>(node);
    } catch (const std::bad_cast& e) {
        throw std::invalid_argument("Node is not valid");
    }
}