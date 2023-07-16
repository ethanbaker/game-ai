#ifndef DECISION_TREE_LEARNING
#define DECISION_TREE_LEARNING

#include <vector>
#include "../../entity/entity.hpp"

// Read in a list of state packets from a given CSV file
void readStatePacketsFromCSV(std::string filepath, std::vector<StatePacket> *packets);

// Calculate the entropy of a given list of packets
float entropyOfList(std::vector<StatePacket> packets);

// Calculate the entropy of a map of lists
float entropyOfSet(std::map<std::string, std::vector<StatePacket>> set, int packetCount);

// Split a list by attribute
void splitByAttribute(std::map<std::string, std::vector<StatePacket>> *set, std::vector<StatePacket> *packets, std::string attribute);

// Make a decision tree based on our given packet information
void makeDecisionTree(std::vector<StatePacket> packets, std::vector<std::string> attributes, Vertex<AbstractDMNode*> *node, DecisionTree *tree);

#endif