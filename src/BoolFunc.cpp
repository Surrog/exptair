/* 
 * File:   BoolFunc.cpp
 * Author: ancelf
 * 
 * Created on January 13, 2011, 2:55 PM
 */

#include <sstream>

#include "BoolFunc.h"
#include "xbool.hpp"

BoolFunc::BoolFunc() : Node(), _startByNot(false), _andFlags(true) {
}

BoolFunc::BoolFunc(const BoolFunc& orig) : Node(orig), _operand(orig._operand), _operator(orig._operator), _startByNot(orig._startByNot), _andFlags(orig._andFlags) {
}

BoolFunc::~BoolFunc() {
}

BoolFunc& BoolFunc::operator =(const BoolFunc& orig) {
    Node::operator =(orig);
    _operand = orig._operand;
    _operator = orig._operator;
    return *this;
}

void BoolFunc::operator =(xbool value) {
    if (_andFlags) {
        NodeCont::iterator it = _operand.begin();
        NodeCont::iterator ite = _operand.end();
        OperatorCont::iterator itx = _operator.begin();
        OperatorCont::iterator itxe = _operator.end();

        if (it != ite) {
            if (_startByNot)
                (*it)->operator =(Not::execute(value));
            else
                (*it)->operator =(value);
            ++it;
        }

        while (it != ite && itx != itxe) {
            if ((*itx)->getCode() == ANDNOT)
                (*it)->operator =(Not::execute(value));
            else
                (*it)->operator =(value);
            ++it;
            ++itx;
        }
    } else {
        std::vector< SmartPtr<Node> > functions;
        divideInAndBoolFunc(functions);
        int good = -1;
        for (unsigned int i = 0; i < functions.size(); i++) {
            xbool tmp = functions[i]->forward();
            if (tmp == xundefined) {
                if (good == -1)
                    good = i;
                else {
                    good = -2;
                }
            } else if (tmp == xtrue) {
                good = -2;
            }
        }
        if (good >= 0)
            functions[good]->operator =(value);
    }
}

bool BoolFunc::operator<(const BoolFunc& comp) const {
    return (complexity() < comp.complexity());
}

xbool BoolFunc::forward(ClosedList* list) {
    if (!list) {
        ClosedList _list;
        return this->forward(&_list);
    }
    if (_operand.size() == 1)
        return (_operand.front())->forward(list);

    OperatorCont::iterator it = _operator.begin();
    OperatorCont::iterator ite = _operator.end();

    NodeCont::iterator itx = _operand.begin();
    NodeCont::iterator itxe = _operand.end();

    xbool result = (*itx)->forward(list);
    if (isStartByNot())
        result = Not::execute(result);
    itx++;

    while (it != ite && itx != itxe) {
        result = (*it)->execute(result, (*itx)->forward(list));
        ++it;
        ++itx;
    }
    return result;
}

xbool BoolFunc::backward(ClosedList* list) {
    if (!list) {
        ClosedList _list;
        return this->backward(&_list);
    }
    if (_operand.size() == 1)
        return (_operand.front())->backward(list);

    OperatorCont::iterator it = _operator.begin();
    OperatorCont::iterator ite = _operator.end();

    NodeCont::iterator itx = _operand.begin();
    NodeCont::iterator itxe = _operand.end();

    xbool result = (*itx)->backward(list);
    if (isStartByNot())
        result = Not::execute(result);
    itx++;

    while (it != ite && itx != itxe) {
        result = (*it)->execute(result, (*itx)->backward(list));
        ++it;
        ++itx;
    }
    return result;
}

//on evalue la difficulté a evaluer l'equation suivant les operator dedans

int BoolFunc::complexity() const {
    int result = 0;

    OperatorCont::const_iterator it = _operator.begin();
    OperatorCont::const_iterator ite = _operator.end();

    while (it != ite) {
        if (result < (*it)->complexity()) {
            result = (*it)->complexity();
        }
        ++it;
    }
    return result;
}

void BoolFunc::addOperator(Oper& op) {
    if (op.getCode() != AND && op.getCode() != ANDNOT) {
        _andFlags = false;
    }
        
    _operator.push_back(&op);
}

void BoolFunc::addOperand(SmartPtr<Node> no) {
    _operand.push_back(no);
}

void BoolFunc::addBoolFunc(BoolFunc& func) {
    if (_andFlags) {
        NodeCont::iterator it = _operand.begin();
        NodeCont::iterator ite = _operand.end();

        OperatorCont::iterator itx = _operator.begin();
        OperatorCont::iterator itxe = _operator.end();

        if (it != ite) {
            if (_startByNot)
                func.invertStartByNot();
            (*it)->addBoolFunc(func);
            if (_startByNot)
                func.invertStartByNot(); // on revient a la normale
            ++it;
        }

        while (it != ite && itx != itxe) {
            if ((*itx)->getCode() == AND) {
                (*it)->addBoolFunc(func);
            } else {
                func.invertStartByNot(); // on inverse
                (*it)->addBoolFunc(func);
                func.invertStartByNot(); // on revient a la normale
            }
            ++it;
            ++itx;
        }
        return;
    }

    std::vector< SmartPtr<Node> > functions;

    divideInAndBoolFunc(functions);
    for (unsigned int i = 0; i < functions.size(); i++) {
        BoolFunc tmpFunc(func);
        for (unsigned int o = 1; o < functions.size(); o++) {
            tmpFunc.addOperator(Singleton<AndNot>::Instance());
            tmpFunc.addDynBoolFunc(functions[(o + i) % functions.size()]);
        }
        functions[i]->addBoolFunc(tmpFunc);
    }

}

bool BoolFunc::containAPartOf(const BoolFunc& func) const {
    NodeCont::const_iterator itx = func._operand.begin();
    NodeCont::const_iterator itxe = func._operand.end();

    while (itx != itxe) {
        NodeCont::const_iterator it = _operand.begin();
        NodeCont::const_iterator ite = _operand.end();

        while (it != ite && ((*itx))->dump() != (*(*it))->dump()) {
            ++it;
        }
        if (it == ite)
            return false;
        ++itx;
    }
    return true;
}

bool BoolFunc::isStartByNot() const {
    return _startByNot;
}

void BoolFunc::setStartByNot(bool value) {
    _startByNot = value;
}

void BoolFunc::invertStartByNot() {
    _startByNot = !_startByNot;
}

void BoolFunc::addDynBoolFunc(SmartPtr<Node> func) {
    _operand.push_back(func);
}

void BoolFunc::divideInAndBoolFunc(std::vector< SmartPtr<Node> >& in) {
    NodeCont::iterator it = _operand.begin();
    NodeCont::iterator ite = _operand.end();

    OperatorCont::iterator itx = _operator.begin();
    OperatorCont::iterator itxe = _operator.end();

    BoolFunc* tmp = new BoolFunc();
    tmp->_startByNot = _startByNot;
    while (it != ite) {
        if (!tmp) {
            tmp = new BoolFunc();
            if (itx != itxe && ((*itx)->getCode() == ORNOT || (*itx)->getCode() == XORNOT))
                tmp->_startByNot = true;
            ++itx;
        }

        tmp->addOperand(*it);
        ++it;
        while (it != ite && itx != itxe && ((*itx)->getCode() == AND || (*itx)->getCode() == ANDNOT)) {
            tmp->addOperand(*it);
            tmp->addOperator(*(*itx));
            ++it;
            ++itx;
        }

        SmartPtr<Node> sPtr;
        sPtr = tmp;
        in.push_back(sPtr);
        tmp = 0;
    }
}

std::string BoolFunc::dump() const {
    NodeCont::const_iterator it = _operand.begin();
    NodeCont::const_iterator ite = _operand.end();

    OperatorCont::const_iterator itx = _operator.begin();
    OperatorCont::const_iterator itxe = _operator.end();

    std::stringstream out;

    if (it != ite) {
        out << "(";
        if (_startByNot)
            out << "NOT ";
        out << (*it)->dump();
        ++it;
    }

    while (it != ite && itx != itxe) {
        out << " " << (*itx)->getValue() << " " << (*it)->dump();
        ++it;
        ++itx;
    }

    out << ")";
    return out.str();
}