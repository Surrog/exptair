/* 
 * File:   Ai.cpp
 * Author: ancelf
 * 
 * Created on January 13, 2011, 2:46 PM
 */

#include <map>

#include "Ai.h"
#include "Node.h"
#include "InputFileParser.hpp"

bool Ai::loadPath(const std::string& filePath) {
    InputFileParser parser;
    return parser.parseFile(filePath, *this);
}

void Ai::addNode(Node* no) {
    _nodeList[no->getLetter()] = no;
}

Node* Ai::getNode(char letter) {
    NodeCont::iterator it = _nodeList.find(letter);

    if (it != _nodeList.end()) {
        return it->second;
    }
    return 0;
}

void Ai::forward()  {
    std::vector<char> nodeTrue;
    std::vector<char> nodeFalse;
    std::vector<char> nodeUnde;

    NodeCont::iterator it = _nodeList.begin();
    NodeCont::iterator ite = _nodeList.end();

    while (it != ite) {
        xbool result = (it->second)->forward();
        char letter = it->second->getLetter();
        if (result == xtrue) {
            nodeTrue.push_back(letter);
        } else if (result == xfalse) {
            nodeFalse.push_back(letter);
        } else {
            nodeUnde.push_back(letter);
        }

        ++it;
    }
}

void Ai::backward() {
    
}