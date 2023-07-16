#include "./decision-tree.hpp"
#include "../../graph/graph.hpp"
#include "../../../environment/environment.hpp"
#include "../../../entity/entity.hpp"
#include "../../../engine/engine.hpp"

/* Action Class */

void *Action::run(EnvironmentParameters *environment) {
    environment->character->setBehaviors(this->behaviors);
    return this;
}

/* Decision Class */

void Decision::setTruthNode(DecisionTreeNode *node) {
    this->truthNode = node;
}

DecisionTreeNode *Decision::getTruthNode() {
    return this->truthNode;
}

void Decision::setFalseNode(DecisionTreeNode *node) {
    this->falseNode = node;
}

DecisionTreeNode *Decision::getFalseNode() {
    return this->falseNode;
}

/* DecisionMulti Class */

void *DecisionMulti::run(EnvironmentParameters *environment) {
    std::string value = "";
    int timestamp = environment->gameState->getTimestamp();
    StatePacket packet = environment->character->getStatePacket(timestamp);

    // Determine the value from one of the parametrized states
    if (this->testValue == "timestamp") {
        value = std::to_string(packet.timestamp);
    } else if (this->testValue == "action") {
        value = packet.action;
    } else if (this->testValue == "x") {
        value = std::to_string(packet.x);
    } else if (this->testValue == "y") {
        value = std::to_string(packet.y);
    } else if (this->testValue == "characterX") {
        value = std::to_string(packet.characterX);
    } else if (this->testValue == "characterY") {
        value = std::to_string(packet.characterY);
    } else if (this->testValue == "obstacleTop") {
        value = std::to_string(packet.obstacleDistTop);
    } else if (this->testValue == "obstacleRight") {
        value = std::to_string(packet.obstacleDistRight);
    } else if (this->testValue == "obstacleBottom") {
        value = std::to_string(packet.obstacleDistBottom);
    } else if (this->testValue == "obstacleLeft") {
        value = std::to_string(packet.obstacleDistLeft);
    }

    // If we can't find the value, return
    if (this->children.find(value) == this->children.end()) {
        return this;
    }

    // Otherwise, propagate down to one of the children
    std::vector<DecisionTreeNode*> list = this->children[value];
    DecisionTreeNode* node = list.at(rand() % list.size());

    return node->run(environment);
}

void DecisionMulti::addChildNode(DecisionTreeNode* child, std::string onValue) {
    this->children[onValue].push_back(child);
}

std::map<std::string, std::vector<DecisionTreeNode*>> DecisionMulti::getChildren() {
    return this->children;
}

void DecisionMulti::setTestValue(std::string testValue) {
    this->name = testValue;
    this->testValue = testValue;
}

/* DecisionTree Class */

void *DecisionTree::decide(EnvironmentParameters *environment) {
    if (this->getRoot() == nullptr || this->getRoot()->getElement() == nullptr) {
        return nullptr;
    }

    return this->getRoot()->getElement()->run(environment);
}

Vertex<AbstractDMNode*> *DecisionTree::addRoot(DecisionTreeNode *root) {
    return Tree::addRoot(root);
}

Vertex<AbstractDMNode*> *DecisionTree::insertNode(DecisionTreeNode *data, Vertex<AbstractDMNode*> *parent, bool value) {
    Vertex<AbstractDMNode*> *node = Tree::insertNode(data, value, parent);

    if (value) {
        validateDecisionNode(parent->getElement())->setTruthNode(data);
    } else {
        validateDecisionNode(parent->getElement())->setFalseNode(data);
    }

    return node;
}

Vertex<AbstractDMNode*> *DecisionTree::insertNode(DecisionMulti *data, Vertex<AbstractDMNode*> *parent, std::string value) {
    Vertex<AbstractDMNode*> *node = Tree::insertNode(data, false, parent);

    validateMultiNode(parent->getElement())->addChildNode(data, value);

    return node;
}

void DecisionTree::setTestValue(Vertex<AbstractDMNode*> *node, std::string value) {
    validateMultiNode(node->getElement())->setTestValue(value);
}

Decision *DecisionTree::validateDecisionNode(AbstractDMNode *v) {
    try {
        return dynamic_cast<Decision*>(v);
    } catch (const std::bad_cast& e) {
        throw std::invalid_argument("Node is not valid");
    }
}

Action *DecisionTree::validateActionNode(AbstractDMNode *v) {
    try {
        return dynamic_cast<Action*>(v);
    } catch (const std::bad_cast& e) {
        throw std::invalid_argument("Node is not valid");
    }
}

DecisionMulti *DecisionTree::validateMultiNode(AbstractDMNode *v) {
    try {
        return dynamic_cast<DecisionMulti*>(v);
    } catch (const std::bad_cast& e) {
        throw std::invalid_argument("Node is not valid");
    }
}