#include "LoopNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"
#include "../src/NetworkManager.h"
#include "../src/AttributeAccessor.h"

using namespace coral;

LoopIteratorNode::LoopIteratorNode(const std::string &name, Node *parent): 
Node(name, parent){
	_index = new NumericAttribute("index", this);
	
	addInputAttribute(_index);
	
	setAttributeAllowedSpecialization(_index, "Int");
}

NumericAttribute *LoopIteratorNode::index(){
	return _index;
}

void LoopIteratorNode::loopStart(unsigned int loopRangeSize){
}

void LoopIteratorNode::loopStep(unsigned int index){
}

void LoopIteratorNode::loopEnd(){
}

void LoopIteratorNode::update(Attribute *attribute){
	int currentIndex = _index->value()->intValueAt(0);
	loopStep(currentIndex);
}

ForLoopNode::ForLoopNode(const std::string &name, Node *parent): Node(name, parent){
	setClassName("ForLoop");
	setAllowDynamicAttributes(true);
	
	_indexRange = new NumericAttribute("indexRange", this);
	_currentIndex = new NumericAttribute("_currentIndex", this);
	_out = new PassThroughAttribute("out", this);
	
	addInputAttribute(_indexRange);
	addInputAttribute(_currentIndex);
	addOutputAttribute(_out);
	
	setAttributeAffect(_indexRange, _out);

	setAttributeAllowedSpecialization(_indexRange, "IntArray");
	setAttributeAllowedSpecialization(_currentIndex, "Int");
}

void ForLoopNode::addDynamicAttribute(Attribute *attribute){
	Node::addDynamicAttribute(attribute);
	
	if(attribute->isOutput()){
		setAttributeAffect(_indexRange, attribute);
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

void ForLoopNode::getSubCleanChain(Attribute *attribute, std::map<int, std::vector<Attribute*> > &subCleanChain){
	std::vector<Attribute*> attributes;
	NetworkManager::getUpstreamChain(attribute, attributes);
	
	int n = 0;
	for(int i = 0; i < attributes.size(); ++i){
		Attribute *attr = attributes[i];
		if(attr->isOutput()){
			Node *parentNode = attr->parent();
			if(parentNode){
				if(parentNode->parent() == this){
					subCleanChain[n].push_back(attr);
					n++;
				}
				
				// if(parentNode->isChildOf(this)){
				// 	subCleanChain[n].push_back(attr);
				// 	n++;
				// }
			}
		}
	}
}

void ForLoopNode::subClean(std::map<int, std::vector<Attribute*> > &subCleanChain){
	for(std::map<int, std::vector<Attribute*> >::iterator i = subCleanChain.begin(); i != subCleanChain.end(); ++i){
		std::vector<Attribute*> &slice = i->second;
		for(int j = 0; j < slice.size(); ++j){
			Attribute *attr = slice[j];
			attr->parent()->update(attr);
		}
	}
}

void ForLoopNode::update(Attribute *attribute){
	Numeric * currentIndex = _currentIndex->outValue();
	Numeric *indexRange = _indexRange->value();
	const std::vector<int> &idRangeValues = indexRange->intValues();
	int loopRangeSize = idRangeValues.size();
	
	std::map<int, std::vector<Attribute*> > subCleanChain;
	getSubCleanChain(attribute, subCleanChain);
	
	std::vector<LoopIteratorNode*> loopOperators;
	collectLoopOperators(loopOperators);
	int loopOperatorsSize = loopOperators.size();
	
	for(int i = 0; i < loopOperatorsSize; ++i){
		loopOperators[i]->loopStart(loopRangeSize);
	}
	
	for(int i = 0; i < loopRangeSize; ++i){
		currentIndex->setIntValueAt(0, idRangeValues[i]);
		
		subClean(subCleanChain);
	}
	
	for(int i = 0; i < loopOperatorsSize; ++i){
		loopOperators[i]->loopEnd();
	}
}


