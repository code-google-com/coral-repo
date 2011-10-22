#include "LoopNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"

using namespace coral;

LoopIteratorNode::LoopIteratorNode(const std::string &name, Node *parent): 
	Node(name, parent){
}

void LoopIteratorNode::loopStart(unsigned int loopRangeSize){
}

void LoopIteratorNode::loopStep(unsigned int index){
}

void LoopIteratorNode::loopEnd(){
}

void LoopIteratorNode::update(Attribute *attribute){
	ForLoopNode *parentNode = (ForLoopNode*)parent();
	if(parentNode){
		int currentIndex = ((Numeric*)parentNode->inputAttributeAt(1)->outValue())->intValueAt(0);
		loopStep(currentIndex);
	}
}

ForLoopNode::ForLoopNode(const std::string &name, Node *parent): Node(name, parent){
	setClassName("ForLoop");
	setAllowDynamicAttributes(true);
	
	_indexRange = new NumericAttribute("indexRange", this);
	_currentIndex = new NumericAttribute("_currentIndex", this);
	
	addInputAttribute(_indexRange);
	addInputAttribute(_currentIndex);
	
	setAttributeAllowedSpecialization(_indexRange, "IntArray");
	setAttributeAllowedSpecialization(_currentIndex, "Int");
}

void ForLoopNode::addDynamicAttribute(Attribute *attribute){
	Node::addDynamicAttribute(attribute);
	
	if(attribute->isOutput()){
		setAttributeAffect(_indexRange, attribute);
	}
}

void ForLoopNode::collectCleanChain(Attribute *attribute, std::vector<Attribute*> &cleanChain){
	std::vector<NestedObject*> attrParents = attribute->allParentObjects();
	if(containerUtils::elementInContainer<NestedObject*>(this, attrParents)){
		Attribute *input = attribute->input();
		if(input){
			collectCleanChain(input, cleanChain);
		}
		
		std::vector<Attribute*> affectedBy = attribute->affectedBy();
		for(int i = 0; i < affectedBy.size(); ++i){
			collectCleanChain(affectedBy[i], cleanChain);
		}
	
		if(attribute->isOutput()){
			Node *parentNode = attribute->parent();
			if(parentNode->parent() == this && parentNode->updateEnabled()){
				if(!containerUtils::elementInContainer(attribute, cleanChain)){
					cleanChain.push_back(attribute);
				}
			}
		}
	}
}

void ForLoopNode::collectLoopOperators(std::vector<LoopIteratorNode*> &loopOperators){
	std::vector<Node*> childrenNodes = nodes();
	for(int i = 0; i < childrenNodes.size(); ++i){
		Node *childNode = childrenNodes[i];
		
		LoopIteratorNode *loopOperator = dynamic_cast<LoopIteratorNode*>(childNode);
		if(loopOperator){
			loopOperators.push_back(loopOperator);
		}
	}
}

void ForLoopNode::update(Attribute *attribute){
	Numeric * currentIndex = _currentIndex->outValue();
	Numeric *indexRange = _indexRange->value();
	const std::vector<int> &idRangeValues = indexRange->intValues();
	int loopRangeSize = idRangeValues.size();
	
	std::vector<Attribute*> cleanChain;
	collectCleanChain(attribute, cleanChain);
	
	std::vector<LoopIteratorNode*> loopOperators;
	collectLoopOperators(loopOperators);
	int loopOperatorsSize = loopOperators.size();
	
	for(int i = 0; i < loopOperatorsSize; ++i){
		loopOperators[i]->loopStart(loopRangeSize);
	}
	
	for(int i = 0; i < loopRangeSize; ++i){
		currentIndex->setIntValueAt(0, idRangeValues[i]);
		
		for(int j = 0; j < cleanChain.size(); ++j){
			Attribute *outAttr = cleanChain[j];
			Node *parentNode = outAttr->parent();
			parentNode->update(outAttr);
		}
	}
	
	for(int i = 0; i < loopOperatorsSize; ++i){
		loopOperators[i]->loopEnd();
	}
}


