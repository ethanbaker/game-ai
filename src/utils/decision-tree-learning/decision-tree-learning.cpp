#include "./decision-tree-learning.hpp"
#include <vector>
#include <iostream>
#include <fstream>

void readStatePacketsFromCSV(std::string filepath, std::vector<StatePacket> *packets) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return;
    }

    // Parse line by line
    std::string line;
    std::string buf;
    while (std::getline(file, line)) {
        std::string timestamp;
        std::string action;
        std::string x;
        std::string y;
        std::string charX;
        std::string charY;
        std::string obstacleTop;
        std::string obstacleRight;
        std::string obstacleBottom;
        std::string obstacleLeft;

        std::istringstream stream;
        stream.str(line);

        // Parse column by column
        int col = 0;
        while (std::getline(stream, buf, ',')) {
            if (col == 0) {
                timestamp.append(buf);
            } else if (col == 1) {
                action.append(buf);
            } else if (col == 2) {
                x.append(buf);
            } else if (col == 3) {
                y.append(buf);
            } else if (col == 4) {
                charX.append(buf);
            } else if (col == 5) {
                charY.append(buf);
            } else if (col == 6) {
                obstacleTop.append(buf);
            } else if (col == 7) {
                obstacleRight.append(buf);
            } else if (col == 8) {
                obstacleBottom.append(buf);
            } else if (col == 9) {
                obstacleLeft.append(buf);
            }
            col++;
        }

        // Don't consume the header row
        if (timestamp == "timestamp") {
            continue;
        }

        try {
        StatePacket packet;
        packet.timestamp = stof(timestamp);
        packet.action = action;
        packet.x = stof(x);
        packet.y = stof(y);
        packet.characterX = stof(charX);
        packet.characterY = stof(charY);
        packet.obstacleDistTop = stof(obstacleTop);
        packet.obstacleDistRight = stof(obstacleRight);
        packet.obstacleDistBottom = stof(obstacleBottom);
        packet.obstacleDistLeft = stof(obstacleLeft);

        packets->push_back(packet);
        } catch (std::exception) {
            continue;
        }
    }
}

float entropyOfList(std::vector<StatePacket> packets) {
    int packetCount = packets.size();

    // Return an entropy of zero if we don't have any packets
    if (packetCount == 0) {
        return 0;
    }

    // Keep a list of different actions
    std::map<std::string, int> actions;

    // Sum up all actions for all of the packets
    for (StatePacket packet : packets) {
        actions[packet.action]++;
    }

    // Get a count for all of the actions
    int actionCount = actions.size();

    // If we only have one action then we have zero entropy
    if (actionCount == 0) {
        return 0;
    }

    // Start with zero entropy
    float entropy = 0.0;
    for (auto it = actions.begin(); it != actions.end(); it++) {
        float proportion = (float) it->second / packetCount;
        entropy -= proportion * log2f(proportion);
    }

    return entropy;
}

float entropyOfSet(std::map<std::string, std::vector<StatePacket>> set, int packetCount) {
    // Start with zero entropy
    float entropy = 0;

    // Get tne entropy contribution of each list
    for (auto it = set.begin(); it != set.end(); it++) {
        std::vector<StatePacket> list = it->second;

        float proportion = (float) list.size() / packetCount;
        entropy -= proportion * entropyOfList(list);
    }

    return entropy;
}

void splitByAttribute(std::map<std::string, std::vector<StatePacket>> *set, std::vector<StatePacket> *packets, std::string attribute) {
    // Create a map of lists for each state packet
}

void makeDecisionTree(std::vector<StatePacket> packets, std::vector<std::string> attributes, Vertex<AbstractDMNode*> *node, DecisionTree *tree) {
    // Calculate the initial entropy
    float initialEntropy = entropyOfList(packets);

    // If we have no entropy, we can't divide further
    if (initialEntropy <= 0) {
        return;
    }

    // Get the packet count
    int packetCount = packets.size();

    // Hold the best split found so far
    float bestInformationGain = 0;
    std::string bestSplitAttribute = "";
    std::map<std::string, std::vector<StatePacket>> bestSet;

    // Go though each attribute
    for (std::string attribute : attributes) {
        // Perform the split
        std::map<std::string, std::vector<StatePacket>> set;
        for (StatePacket packet : packets) {
            set[packet.getValue(attribute)].push_back(packet);
        }

        // Find overall entropy and information gain
        float overallEntropy = entropyOfSet(set, packetCount);
        float informationGain = initialEntropy - overallEntropy;

        // Check if we've got the best information so far
        if (informationGain > bestInformationGain) {
            bestInformationGain = informationGain;
            bestSplitAttribute = attribute;
            bestSet = set;
        }
    }

    // Set the node's test function
    if (bestSplitAttribute == "") {
        return;
    } else if (bestSplitAttribute == "action") {
        auto it = bestSet.begin();
        std::advance(it, rand() % bestSet.size());

        tree->setTestValue(node, it->first);
        return;
    } else if (bestSet.size() > 0) {
        tree->setTestValue(node, bestSplitAttribute);
    } else {
        return;
    }


    // Remove the attribute and continue
    std::vector<std::string> newAttributes;
    for (std::string attribute : attributes) {
        if (attribute != bestSplitAttribute) {
            newAttributes.push_back(attribute);
        }
    }

    // Fill in the daugher nodes
    for (auto it = bestSet.begin(); it != bestSet.end(); it++) {
        std::vector<StatePacket> list = it->second;

        // Find the value for the attribute in the list
        std::string attributeValue = list.at(0).getValue(bestSplitAttribute);

        // Create a child node for the tree
        DecisionMulti *child = new DecisionMulti();

        // Add it to the tree
        Vertex<AbstractDMNode*> *childNode = tree->insertNode(child, node, attributeValue);

        // Recurse the algorithm
        makeDecisionTree(list, newAttributes, childNode, tree);
    }
}