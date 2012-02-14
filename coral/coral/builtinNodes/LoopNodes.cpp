#include "LoopNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"
#include "../src/NetworkManager.h"
#include "../src/AttributeAccessor.h"
#include "../src/stringUtils.h"

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
	int currentElement = _index->value()->intValueAt(0);
	loopStep(currentElement);
}

ForLoopNode::ForLoopNode(const std::string &name, Node *parent): 
Node(name, parent),
_selectedOperation(0){
	setClassName("ForLoop");
	setAllowDynamicAttributes(true);
	
	_array = new NumericAttribute("array", this);
	_currentElement = new NumericAttribute("_currentElement", this);
	_currentIndex = new NumericAttribute("_currentIndex", this);
	_out = new PassThroughAttribute("out", this);
	
	addInputAttribute(_array);
	addInputAttribute(_currentElement);
	addInputAttribute(_currentIndex);
	addOutputAttribute(_out);
	
	setAttributeAffect(_array, _out);

	setAttributeAllowedSpecialization(_currentIndex, "Int");

	std::vector<std::string> arraySpec;
	arraySpec.push_back("IntArray");
	arraySpec.push_back("FloatArray");
	arraySpec.push_back("Vec3Array");
	arraySpec.push_back("Col4Array");
	arraySpec.push_back("Matrix44Array");

	setAttributeAllowedSpecializations(_array, arraySpec);

	std::vector<std::string> elementSpec;
	elementSpec.push_back("Int");
	elementSpec.push_back("Float");
	elementSpec.push_back("Vec3");
	elementSpec.push_back("Col4");
	elementSpec.push_back("Matrix44");

	setAttributeAllowedSpecializations(_currentElement, elementSpec);

	addAttributeSpecializationLink(_currentElement, _array);
}

void ForLoopNode::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = specializationA[i] + "Array";
		}
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = stringUtils::strip(specializationB[i], "Array");
		}
	}
}

void ForLoopNode::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &ForLoopNode::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &ForLoopNode::updateFloat;
			}
			else if(type == Numeric::numericTypeVec3Array){
				_selectedOperation = &ForLoopNode::updateVec3;
			}
			else if(type == Numeric::numericTypeCol4Array){
				_selectedOperation = &ForLoopNode::updateCol4;
			}
			else if(type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &ForLoopNode::updateMatrix44;
			}
		}
	}
}

void ForLoopNode::addDynamicAttribute(Attribute *attribute){
	Node::addDynamicAttribute(attribute);
	
	if(attribute->isOutput()){
		setAttributeAffect(_array, attribute);
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

void ForLoopNode::updateInt(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain){
	const std::vector<int> &arrayValues = array->intValues();
	for(int i = 0; i < arrayValues.size(); ++i){
		currentElement->setIntValueAt(0, arrayValues[i]);
		currentIndex->setIntValueAt(0, i);

		subClean(subCleanChain);
	}
}

void ForLoopNode::updateFloat(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain){
	const std::vector<float> &arrayValues = array->floatValues();
	for(int i = 0; i < arrayValues.size(); ++i){
		currentElement->setFloatValueAt(0, arrayValues[i]);
		currentIndex->setIntValueAt(0, i);

		subClean(subCleanChain);
	}
}

void ForLoopNode::updateVec3(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain){
	const std::vector<Imath::V3f> &arrayValues = array->vec3Values();
	for(int i = 0; i < arrayValues.size(); ++i){
		currentElement->setVec3ValueAt(0, arrayValues[i]);
		currentIndex->setIntValueAt(0, i);

		subClean(subCleanChain);
	}
}

void ForLoopNode::updateCol4(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain){
	const std::vector<Imath::Color4f> &arrayValues = array->col4Values();
	for(int i = 0; i < arrayValues.size(); ++i){
		currentElement->setCol4ValueAt(0, arrayValues[i]);
		currentIndex->setIntValueAt(0, i);

		subClean(subCleanChain);
	}
}

void ForLoopNode::updateMatrix44(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain){
	const std::vector<Imath::M44f> &arrayValues = array->matrix44Values();
	for(int i = 0; i < arrayValues.size(); ++i){
		currentElement->setMatrix44ValueAt(0, arrayValues[i]);
		currentIndex->setIntValueAt(0, i);

		subClean(subCleanChain);
	}
}

void ForLoopNode::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *currentElement = _currentElement->outValue();
		Numeric *currentIndex = _currentIndex->outValue();
		Numeric *array = _array->value();

		int loopRangeSize = array->size();
		
		std::map<int, std::vector<Attribute*> > subCleanChain;
		getSubCleanChain(attribute, subCleanChain);
		
		std::vector<LoopIteratorNode*> loopOperators;
		collectLoopOperators(loopOperators);
		int loopOperatorsSize = loopOperators.size();
		
		for(int i = 0; i < loopOperatorsSize; ++i){
			loopOperators[i]->loopStart(loopRangeSize);
		}
		
		(this->*_selectedOperation)(array, currentElement, currentIndex, subCleanChain);
		
		for(int i = 0; i < loopOperatorsSize; ++i){
			loopOperators[i]->loopEnd();
		}
	}
}


