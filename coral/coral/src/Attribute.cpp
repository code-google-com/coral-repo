// <license>
// Copyright (C) 2011 Andrea Interguglielmi, All rights reserved.
// This file is part of the coral repository downloaded from http://code.google.com/p/coral-repo.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
// 
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// </license>

#include <tbb/parallel_for.h>
#include <tbb/mutex.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <map>
#include <vector>
#include <assert.h>

#include "Attribute.h"
#include "Node.h"
#include "NetworkManager.h"
#include "Value.h"
#include "containerUtils.h"
#include "Command.h"
#include "ErrorObject.h"
#include "stringUtils.h"
#include "attributeParallelAlgos.h"

using namespace coral;

void(*Attribute::_connectToCallback)(Attribute *self, Attribute *other) = 0;
void(*Attribute::_disconnectInputCallback)(Attribute *self) = 0;
void(*Attribute::_disconnectOutputCallback)(Attribute *self, Attribute *other) = 0;
void(*Attribute::_deleteItCallback)(Attribute *self) = 0;
void(*Attribute::_specializationCallBack)(Attribute *self) = 0;
void(*Attribute::_valueChangedCallback)(Attribute *self) = 0;
std::vector<void(*)(Attribute *)> _dirtyingDoneCallbackQueue;
bool _cleaningLocked = false;
std::string _cleaningAttributeName;

namespace {
	bool parentNodeNotInParallelSlice(Attribute *attribute, const std::vector<Attribute*> &parallelSlice){
		Node *parentNode = attribute->parent();
		for(int i = 0; i < parallelSlice.size(); ++i){
			if(parallelSlice[i]->parent() == parentNode){
				return false;
			}
		}
	
		return true;
	}

	std::vector<std::string> intersectedSpecialization(const std::vector<std::string> &specialization1, const std::vector<std::string> &specialization2){
		std::vector<std::string> newSpecialization;
	
		for(int i = 0; i < specialization1.size(); ++i){
			if(containerUtils::elementInContainer(specialization1[i], specialization2)){
				newSpecialization.push_back(specialization1[i]);
			}
		}
	
		return newSpecialization;
	}

	bool sameSpecialization(const std::vector<std::string> &specialization1, const std::vector<std::string> &specialization2){
		for(int i = 0; i < specialization1.size(); ++i){
			if(!containerUtils::elementInContainer(specialization1[i], specialization2)){
				return false;
			}
		}
	
		for(int i = 0; i < specialization2.size(); ++i){
			if(!containerUtils::elementInContainer(specialization2[i], specialization1)){
				return false;
			}
		}
	
		return true;
	}
	
	tbb::mutex _globalMutex;
}

Attribute::Attribute(const std::string &name, Node *parent):
	NestedObject(name, parent),
	_value(0),
	_inputValue(0),
	_input(0),
	_isClean(false),
	_isOutput(false),
	_isInput(false),
	_passThrough(false),
	_valueObserved(0),
	_computeTimeSeconds(0),
	_computeTimeMilliseconds(0){
}

Attribute::~Attribute(){
	if(!isDeleted()){
		setIsDeleted(true);
		deleteIt();
	}
}

void Attribute::setValueObserved(bool value){
	_valueObserved = value;
}

void Attribute::setPassThrough(bool value){
	_passThrough = value;
}

bool Attribute::isPassThrough(){
	return _passThrough;
}

Node* Attribute::parent(){
	return (Node*)parentObject();
}

void Attribute::setParent(Node *parent){
	setParentObject(parent);
}

void Attribute::removeAffectFrom(Attribute *attribute){
	if(containerUtils::elementInContainer(attribute, _affectedBy)){
		containerUtils::eraseElementInContainer(attribute, _affectedBy);
	}
}

void Attribute::removeAffect(Attribute *attribute){
	if(containerUtils::elementInContainer(attribute, _affect)){
		containerUtils::eraseElementInContainer(attribute, _affect);
		
		if(attribute->isAffectedBy(this)){
			attribute->removeAffectFrom(this);
		}
	}
}

bool Attribute::isConnectedTo(Attribute *attribute){
	return attribute && containerUtils::elementInContainer(attribute, _outputs);
}

void Attribute::resetInputValuesInChain(){
	_inputValue = _value;
	
	if(_input){
		if(_input->_inputValue){
			_inputValue = _input->_inputValue;
		}
	}
	
	if(_passThrough){
		for(int i = 0; i < _outputs.size(); ++i){
			_outputs[i]->resetInputValuesInChain();
		}
	}
}

void Attribute::disconnectInput(){
	if(_input){
		Attribute *oldInput = _input;
		_input = 0;
		_inputValue = _value;
		
		if(oldInput->isConnectedTo(this)){
			oldInput->disconnectOutput(this);
		}
		
		if(!isDeleted()){
			ErrorObject *error = new ErrorObject();
			
			resetInputValuesInChain();
			cacheEvaluationChain();
		
			updateBranchSpecializations(error);
			dirty();
			
			if(parent()){
				parent()->attributeConnectionChanged(this);
			}
			
			if(_disconnectInputCallback){
				_disconnectInputCallback(this);
			}
			
			delete error;
		}
	}
}

void Attribute::disconnectOutput(Attribute *attribute){
	if(this->isConnectedTo(attribute)){		
		containerUtils::eraseElementInContainer(attribute, _outputs);
		
		if(attribute->input() == this){
			attribute->disconnectInput();
		}
		
		if(!isDeleted()){
			ErrorObject *error = new ErrorObject();
			
			cacheEvaluationChain();
			updateBranchSpecializations(error);
			
			if(parent()){
				parent()->attributeConnectionChanged(this);
			}
			
			delete error;
		}
	}
}

bool Attribute::isClean(){
	return _isClean;
}

void Attribute::setIsClean(bool value){
	_isClean = value;
}

void Attribute::setIsOutput(bool value){
	_isOutput = value;

	_isInput = !_isOutput;
}

void Attribute::setIsInput(bool value){
	_isInput = value;

	_isOutput = !_isInput;
}

Attribute *Attribute::input(){
	return _input;
}

bool Attribute::isAffectedBy(Attribute *attribute){
	return containerUtils::elementInContainer(attribute, _affectedBy);
}

std::vector<Attribute*> Attribute::affectedBy(){
	return _affectedBy;
}

std::vector<Attribute*> Attribute::affecting(){
	return _affect;
}

void Attribute::addAffectedFrom(Attribute *attribute){
	if(attribute){
		if(containerUtils::elementInContainer(attribute, _affectedBy) == false){
			_affectedBy.push_back(attribute);
		}
	}
}

void Attribute::addAffect(Attribute *attribute){
	if(attribute){
		if(containerUtils::elementInContainer(attribute, _affect) == false){
			_affect.push_back(attribute);
			
			if(attribute->isAffectedBy(this) == false){
				attribute->addAffectedFrom(this);
			}
			
			cacheEvaluationChain();
		}
	}
}

void Attribute::setValuePtr(Value *value){
	if(_value){
		_value->removeReference();
	}
	
	_value = value;
	_value->addReference();
	_inputValue = _value;
}

void Attribute::valueChanged(){
	dirty();
}

Value *Attribute::value(){
	clean();
	
	return _inputValue;
}

Value *Attribute::outValue(){
	return _value;
}

void Attribute::clean(){
	if(!_cleaningLocked){
		if(_isClean == false){
			_cleaningLocked = true;
			_cleaningAttributeName = fullName();
			
			boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::universal_time();
	
			for(int i = 0; i < _cleanChain.size(); ++i){
				std::vector<Attribute*> *inputAttributes = &_cleanChain[i][0];
				std::vector<Attribute*> *outputAttributes = &_cleanChain[i][1];
			 	
				for(int i = 0; i < inputAttributes->size(); ++i){
					inputAttributes->at(i)->_isClean = true;
				}
				
				#ifdef CORAL_PARALLEL_TBB
					tbb::parallel_for(tbb::blocked_range<size_t>(0, outputAttributes->size()), attribute_parallelClean(outputAttributes));
				#else
					for(int i = 0; i < outputAttributes->size(); ++i){
						outputAttributes->at(i)->cleanSelf();
					}
				#endif
			}
	
			boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::universal_time();
			_computeTimeSeconds = boost::posix_time::time_period(startTime, endTime).length().total_seconds();
			_computeTimeMilliseconds = boost::posix_time::time_period(startTime, endTime).length().total_milliseconds() % 1000;
	
			_cleaningLocked = false;
			_cleaningAttributeName = "";
		}
	}
}

void Attribute::cleanSelf(){
	if(_isClean == false){
		Node *parentNode = parent();
		if(parentNode){
			if(parentNode->updateEnabled()){
				parentNode->doUpdate(this);
			}
		}
		
		_isClean = true;
	}
}

void Attribute::dirty(){
	if(!_cleaningLocked){
		if(_isClean){
			for(int i = 0; i < _dirtyChain.size(); ++i){
				Attribute* attr = _dirtyChain[i];
				attr->_isClean = false;
				attr->onDirtied();
			
				if(attr->_valueObserved){
					if(attr->_valueChangedCallback && !attr->isDeleted()){
						attr->_valueChangedCallback(attr);
					}
				}
			}
		
			processDirtyingDoneCallbackQueue();
		}
	}
}

void Attribute::processDirtyingDoneCallbackQueue(){
	for(int i = 0; i < _dirtyingDoneCallbackQueue.size(); ++i){
		_dirtyingDoneCallbackQueue[i](this);
	}
	
	_dirtyingDoneCallbackQueue.clear();
}

void Attribute::queueDirtyingDoneCallback(void(*callback)(Attribute *)){
	#ifdef CORAL_PARALLEL_TBB
		tbb::mutex::scoped_lock(_globalMutex);
	#endif
	
	_dirtyingDoneCallbackQueue.push_back(callback);
}

void Attribute::setInput(Attribute *attribute){
	if(attribute){
		if(_input != attribute){
			if(_input){
				disconnectInput();
			}
		
			_input = attribute;
			_inputValue = _value;
			
			cacheEvaluationChain();
			resetInputValuesInChain();
			
			dirty();
		}
	}
}

bool Attribute::connectTo(Attribute *attribute, ErrorObject *errorObject){
	_outputs.push_back(attribute);
	attribute->setInput(this);
	
	bool success = updateBranchSpecializations(errorObject);
	
	if(parent()){
		parent()->attributeConnectionChanged(this);
	}
	
	if(attribute->parent()){
		attribute->parent()->attributeConnectionChanged(attribute);
	}
	
	if(_connectToCallback && !isDeleted())
		_connectToCallback(this, attribute);
	
	return success;
}

void Attribute::forceSpecializationUpdate(){
	ErrorObject *error = new ErrorObject();
	updateBranchSpecializations(error);
	delete error;
}

std::vector<Attribute*> Attribute::outputs(){
	return _outputs;
}

void Attribute::deleteIt(){
	if(_deleteItCallback && !isDeleted()){
		_deleteItCallback(this);
	}
	
	if(parent()){
		parent()->removeAttribute(this);
	}
	
	if(_input){
		disconnectInput();
	}
	
	std::vector<Attribute*> oldAffect = _affect;
	for(std::vector<Attribute*>::iterator itAffect = oldAffect.begin(); itAffect != oldAffect.end(); ++itAffect){
		removeAffect(*itAffect);
	}
	
	std::vector<Attribute*> oldAffectFrom = _affectedBy;
	for(std::vector<Attribute*>::iterator itAffect = oldAffectFrom.begin(); itAffect != oldAffectFrom.end(); ++itAffect){
		(*itAffect)->removeAffect(this);
	}
	
	std::vector<Attribute*> oldOutputs = _outputs;
	for(std::vector<Attribute*>::iterator itOutput = oldOutputs.begin(); itOutput != oldOutputs.end(); ++itOutput){
		disconnectOutput(*itOutput);
	}
	
	std::vector<SpecializationLink*> oldSpecializationLinks = _specializationLinks;
	for(std::vector<SpecializationLink*>::iterator itSpecializationLink = oldSpecializationLinks.begin(); itSpecializationLink != oldSpecializationLinks.end(); ++itSpecializationLink){
		removeSpecializationLink(*itSpecializationLink);
	}
	
	if(_value){
		_value->removeReference();
		_value = 0;
	}
	
	setIsDeleted(true);
}

void Attribute::onDirtied(){
}

bool Attribute::isInput(){
	return _isInput;
}

bool Attribute::isOutput(){
	return _isOutput;
}

std::string Attribute::asScript(){
	std::string script;
	
	if(_value && _input == 0 && _affectedBy.size() == 0){
		std::string valueSaveScript = _value->asString();
		if(valueSaveScript.empty() == false){
			Command setAttributeValueCmd;
			setAttributeValueCmd.setName("SetAttributeValue");
			setAttributeValueCmd.setArgString("attribute", fullName());
			setAttributeValueCmd.setArgString("value", valueSaveScript);
			script += setAttributeValueCmd.asScript() += "\n";
		}
	}
	
	return script;
}

bool Attribute::specializationContainedOne(const std::vector<std::string> &specialization1, const std::vector<std::string> &specialization2){
	for(int i = 0; i < specialization1.size(); ++i){
		if(containerUtils::elementInContainer(specialization1[i], specialization2)){
			return true;
		}
	}
	
	return false;
}

std::vector<std::string> Attribute::allowedSpecialization(){
	return _allowedSpecialization;
}

Attribute *Attribute::findStrongerSpecializerInBranch(Attribute *attribute, bool linked, std::vector<Attribute*> &processedAttributes, std::map<int, std::vector<std::string> > &specializationMap){
	Attribute *strongerSpecializer = 0;

	if(attribute != this){
		if(attribute->isPassThrough() && specializationMap[attribute->id()].size() == 0){
			strongerSpecializer = this;
		}
		else if(specializationContainedOne(specializationMap[id()], specializationMap[attribute->id()]) && linked == false){
			if(specializationMap[id()].size() <= specializationMap[attribute->id()].size() && !sameSpecialization(specializationMap[id()], specializationMap[attribute->id()])){
		        strongerSpecializer = this;
			}
		}
	}
	
	if(containerUtils::elementInContainer(this, processedAttributes) == false){
		processedAttributes.push_back(this);
		
		for(int i = 0; i < _specializationLinks.size(); ++i){
			Attribute *attributeA = _specializationLinks[i]->attributeA;
			Attribute *attributeB = _specializationLinks[i]->attributeB;
			
			if(attributeA != this){
				Attribute *newStrongerSpecializer = attributeA->findStrongerSpecializerInBranch(this, true, processedAttributes, specializationMap);
				if(newStrongerSpecializer){
					strongerSpecializer = newStrongerSpecializer;
				}
			}
			
			if(attributeB != this){
				Attribute *newStrongerSpecializer = attributeB->findStrongerSpecializerInBranch(this, true, processedAttributes, specializationMap);
				if(newStrongerSpecializer){
					strongerSpecializer = newStrongerSpecializer;
				}
			}
		}
		
		if(_input){
			Attribute *newStrongerSpecializer = _input->findStrongerSpecializerInBranch(this, false, processedAttributes, specializationMap);
			if(newStrongerSpecializer){
				strongerSpecializer = newStrongerSpecializer;
			}
		}

		for(int i = 0; i < _outputs.size(); ++i){
			Attribute *newStrongerSpecializer = _outputs[i]->findStrongerSpecializerInBranch(this, false, processedAttributes, specializationMap);
			if(newStrongerSpecializer){
				strongerSpecializer = newStrongerSpecializer;
			}
		}
	}
	
	return strongerSpecializer;
}

void Attribute::initSpecializationMap(std::map<int, std::vector<std::string> > &specializationMap){
	std::map<int, std::vector<std::string> >::iterator it = specializationMap.find(id());
	
	if(it == specializationMap.end()){
		if(_specializationOverride.size() == 1){
			specializationMap[id()] = _specializationOverride;
		}
		else{
			specializationMap[id()] = _allowedSpecialization;
		}
		
		for(int i = 0; i < _specializationLinks.size(); ++i){
			Attribute *attributeA = _specializationLinks[i]->attributeA;
			Attribute *attributeB = _specializationLinks[i]->attributeB;

			attributeA->initSpecializationMap(specializationMap);
			attributeB->initSpecializationMap(specializationMap);
		}

		if(_input){
			_input->initSpecializationMap(specializationMap);
		}

		for(int i = 0; i < _outputs.size(); ++i){
			_outputs[i]->initSpecializationMap(specializationMap);
		}
	}
}

void Attribute::updateAllSpceializationLinks(std::vector<Attribute*> &processedAttributes, std::map<int, std::vector<std::string> > &specializationMap){
	if(containerUtils::elementInContainer(this, processedAttributes) == false){
		processedAttributes.push_back(this);
		
		for(int i = 0; i < _specializationLinks.size(); ++i){
			Attribute *attributeA = _specializationLinks[i]->attributeA;
			Attribute *attributeB = _specializationLinks[i]->attributeB;
			
			Node *parentNode = parent();
			if(parentNode){
				parentNode->updateSpecializationLink(attributeA, attributeB, specializationMap[attributeA->id()], specializationMap[attributeB->id()]);
			}
			
			attributeA->updateAllSpceializationLinks(processedAttributes, specializationMap);
			attributeB->updateAllSpceializationLinks(processedAttributes, specializationMap);
		}
		
		if(_input){
			_input->updateAllSpceializationLinks(processedAttributes, specializationMap);
		}

		for(int i = 0; i < _outputs.size(); ++i){
			_outputs[i]->updateAllSpceializationLinks(processedAttributes, specializationMap);
		}
	}
}

Attribute* Attribute::findSpecializerInBranch(std::map<int, std::vector<std::string> > &specializationMap){
	std::vector<Attribute*> processedAttributes;
	updateAllSpceializationLinks(processedAttributes, specializationMap);
	processedAttributes.clear();
	Attribute *specializer = findStrongerSpecializerInBranch(this, false, processedAttributes, specializationMap);
	
	return specializer;
}

bool Attribute::updateBranchSpecializations(ErrorObject *errorObject){
	bool success = true;
	std::map<int, std::vector<std::string> > specializationMap;
	
	// reset specializations
	initSpecializationMap(specializationMap);
	
	// specialize untill nothing left to be specialized
	Attribute* specializer = findSpecializerInBranch(specializationMap);
	
	int maxRecursion = 100;
	int i = 0;
	for(; i < maxRecursion; ++i){
		if(specializer){
			std::vector<Attribute*> processedAttributes;
			bool allowed = specializer->updateSpecialization(specializationMap[specializer->id()], specializationMap, processedAttributes, errorObject);
			if(!allowed){
				success = false;
				break;
			}
			
			specializer = findSpecializerInBranch(specializationMap);
		}
		else{
			break;
		}
	}
	
	if(i == maxRecursion){
		errorObject->setMessage("Maximum recursion reached during specialization.");
		success = false;
	}
	
	// set collected specializations
	std::map<int, std::vector<std::string> >::iterator specializationMapIt = specializationMap.begin();
	for(; specializationMapIt != specializationMap.end(); ++specializationMapIt){
		int id = specializationMapIt->first;
		std::vector<std::string> newSpec = specializationMapIt->second;
	
		Object *object = NetworkManager::findObjectById(id);
		if(object){
			Attribute *attribute = (Attribute*)object;
			attribute->setSpecialization(newSpec);
		}
	}
	
	return success;
}

void Attribute::onSettingSpecialization(const std::vector<std::string> &specialization){
}

void Attribute::setSpecialization(const std::vector<std::string> &specialization){
	if(specialization != _specialization && !isDeleted()){
		_specialization = specialization;
		
		onSettingSpecialization(specialization);
		
		Node *parentNode = parent();
		if(parentNode){
			parentNode->attributeSpecializationChanged(this);
		}
		
		if(_specializationCallBack ){
			_specializationCallBack(this);
		}
		
		dirty();
	}
}

std::vector<std::string> Attribute::specialization(){
	return _specialization;
}

bool Attribute::updateSpecialization(const std::vector<std::string> &newSpecialization, std::map<int, std::vector<std::string> > &specializationMap, std::vector<Attribute*> &processedAttributes, ErrorObject *errorObject){
	if(_passThrough && specializationMap[id()].size() == 0){
		specializationMap[id()] = newSpecialization;
	}
	else if(specializationContainedOne(newSpecialization, specializationMap[id()])){
		if(newSpecialization != specializationMap[id()]){
			specializationMap[id()] = intersectedSpecialization(newSpecialization, specializationMap[id()]);
		}
	}
	
	if(containerUtils::elementInContainer(this, processedAttributes)){
		return true;
	}
	
	processedAttributes.push_back(this);
	
	if(_input){
		bool allow = _input->updateSpecialization(specializationMap[id()], specializationMap, processedAttributes, errorObject);
		if(!allow){
			return false;
		}
	}
	
	for(int i = 0; i < _outputs.size(); ++i){
		bool allow = _outputs[i]->updateSpecialization(specializationMap[id()], specializationMap, processedAttributes, errorObject);
		if(!allow){
			return false;
		}
	}
	
	Node *parentNode = parent();
	if(parentNode){
		for(int i = 0; i < _specializationLinks.size(); ++i){
			Attribute *attributeA = _specializationLinks[i]->attributeA;
			Attribute *attributeB = _specializationLinks[i]->attributeB;
			
			std::vector<std::string> specializationA = specializationMap[attributeA->id()];
			std::vector<std::string> specializationB = specializationMap[attributeB->id()];
			
			parentNode->updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
			
			if(attributeA != this){
				bool allow = attributeA->updateSpecialization(specializationA, specializationMap, processedAttributes, errorObject);
				if(!allow){
					return false;
				}
			}
			
			if(attributeB != this){
				bool allow = attributeB->updateSpecialization(specializationB, specializationMap, processedAttributes, errorObject);
				if(!allow){
					return false;
				}
			}
		}
	}
	
	return true;
}

bool Attribute::allowConnectionTo(Attribute *attribute){
	bool allow = false;
	
	if((isPassThrough() || attribute->isPassThrough()) && (specialization().size() == 0 || attribute->specialization().size() == 0)){
		allow = true;
	}
	else if(specialization().size() > 0 && attribute->specialization().size() > 0){
		allow = specializationContainedOne(_specialization, attribute->_specialization);
	}
	else{
		allow = classNames() == attribute->classNames();
	}
	
	return allow;
}

void Attribute::setAllowedSpecialization(const std::vector<std::string> &specialization){
	_allowedSpecialization = specialization;
	_specialization = specialization;
	onSettingSpecialization(specialization);
}

void Attribute::linkSpecializationTo(Attribute *attribute){
	SpecializationLink *specializationLink = new SpecializationLink();
	specializationLink->attributeA = this;
	specializationLink->attributeB = attribute;
	
	_specializationLinks.push_back(specializationLink);
	attribute->_specializationLinks.push_back(specializationLink);
}

void Attribute::removeSpecializationLink(SpecializationLink *specializationLink){
	containerUtils::eraseElementInContainer(specializationLink, specializationLink->attributeA->_specializationLinks);
	containerUtils::eraseElementInContainer(specializationLink, specializationLink->attributeB->_specializationLinks);
	delete specializationLink;
}

std::string Attribute::debugInfo(){
	std::string info;
	
	info += "attribute: " + fullName() + "\n";
	info += "isClean: " + stringUtils::boolToString(_isClean) + "\n";
	info += "last cleaning took: secs:" + stringUtils::intToString(_computeTimeSeconds) + ", millisecs: " + stringUtils::intToString(_computeTimeMilliseconds) + "\n";
	
	// info += "clean chain: ";
	// 	for(int i = 0; i < _cleanChain.size(); ++i){
	// 		std::vector<Attribute*> *inputAttributes = &_cleanChain[i][0];
	// 		std::vector<Attribute*> *outputAttributes = &_cleanChain[i][1];
	// 		
	// 		info += "{";
	// 		if(inputAttributes->size()){
	// 			info += "inputs:[";
	// 			for(int j = 0; j < inputAttributes->size(); ++j){
	// 				info += inputAttributes->at(j)->fullName() + ",";
	// 			}
	// 			info += "],";
	// 		}
	// 		if(outputAttributes->size()){
	// 			info += "outputs:[";
	// 			for(int j = 0; j < outputAttributes->size(); ++j){
	// 				info += outputAttributes->at(j)->fullName() + ",";
	// 			}
	// 			info += "]";
	// 		}
	// 		info += "}";
	// 		
	// 		if(i < _cleanChain.size() - 1){
	// 			info += " --> ";
	// 		}
	// 	}
	// 	info += "\n";
	// 	
	// 	info += "dirty chain: ";
	// 	for(int i = 0; i < _dirtyChain.size(); ++i){
	// 		info += _dirtyChain[i]->fullName();
	// 		
	// 		if(i < _dirtyChain.size() - 1){
	// 			info += " --> ";
	// 		}
	// 	}
	// 	info += "\n";
	
	return info;
}

void Attribute::cacheDirtyChainUpstream(std::vector<Attribute*> &dirtyChain, std::vector<Attribute*> &processedAttributes){
	for(int i = 0; i < _affect.size(); ++i){
		Attribute *attr = _affect[i];
		attr->cacheDirtyChainUpstream(dirtyChain, processedAttributes);
	}

	for(int i = 0; i < _outputs.size(); ++i){
		Attribute *attr = _outputs[i];
		attr->cacheDirtyChainUpstream(dirtyChain, processedAttributes);
	}
	
	if(containerUtils::elementInContainer(this, dirtyChain) == false){
		dirtyChain.push_back(this);
	}
	
	if(containerUtils::elementInContainer(this, processedAttributes) == false){
		_dirtyChain.clear();
		processedAttributes.push_back(this);
		cacheDirtyChainUpstream(_dirtyChain, processedAttributes);
	}
}

void Attribute::cacheDirtyChainDownstream(){
	if(_input == 0 && _affectedBy.size() == 0){
		_dirtyChain.clear();
		std::vector<Attribute*> processedAttributes;
		processedAttributes.push_back(this);
		cacheDirtyChainUpstream(_dirtyChain, processedAttributes);
	}
	else{
		if(_input){
			_input->cacheDirtyChainDownstream();
		}
		
		for(int i = 0; i < _affectedBy.size(); ++i){
			_affectedBy[i]->cacheDirtyChainDownstream();
		}
	}
}

void Attribute::cacheCleanChainDownstream(std::vector<Attribute*> &cleanChain, std::vector<Attribute*> &processedAttributes){
	if(_input){
		_input->cacheCleanChainDownstream(cleanChain, processedAttributes);
	}
	
	for(int i = 0; i < _affectedBy.size(); ++i){
		_affectedBy[i]->cacheCleanChainDownstream(cleanChain, processedAttributes);
	}
	
	if(containerUtils::elementInContainer(this, cleanChain) == false){
		cleanChain.push_back(this);
	}
	
	if(containerUtils::elementInContainer(this, processedAttributes) == false){
		std::vector<Attribute*> newCleanChain;
		processedAttributes.push_back(this);
		cacheCleanChainDownstream(newCleanChain, processedAttributes);
		cacheParallelCleanChain(cleanChain);
	}
}

bool Attribute::isLastAttributeInChain(Attribute* attribute, const std::vector<Attribute*> &chain){
	for(int i = 0; i < chain.size(); ++i){
		Attribute *attr = chain[i];
		for(int j = 0; j < attr->_affect.size(); ++j){
			if(attr->_affect[j] == attribute){
				return false;
			}
		}
		
		for(int j = 0; j < attr->_outputs.size(); ++j){
			if(attr->_outputs[j] == attribute){
				return false;
			}
		}
	}
	
	return true;
}

void Attribute::cacheParallelCleanChain(const std::vector<Attribute*> &cleanChain){
	std::vector<Attribute*> cleanChainCopy = cleanChain;
	_cleanChain.clear();
	
	std::vector<std::vector<Attribute*> > parallelSlice(2);
	while(cleanChainCopy.size() > 0){
		std::vector<Attribute*> inputParallelSlice;
		std::vector<Attribute*> outputParallelSlice;
		
		std::vector<Attribute*> localCleanChainCopy = cleanChainCopy;
		
		for(int i = 0; i < localCleanChainCopy.size(); ++i){
			Attribute *attr = localCleanChainCopy[i];
			if(isLastAttributeInChain(attr, localCleanChainCopy)){
				if(attr->_isOutput){
					if(parentNodeNotInParallelSlice(attr, outputParallelSlice)){ // avoid having the same node with multiple outputs compute on different cores at the same time (it'd crash)
						outputParallelSlice.push_back(attr);
						containerUtils::eraseElementInContainer(attr, cleanChainCopy);
					}
				}
				else{
					inputParallelSlice.push_back(attr);
					containerUtils::eraseElementInContainer(attr, cleanChainCopy);
				}
			}
		}
		
		parallelSlice[0] = inputParallelSlice;
		parallelSlice[1] = outputParallelSlice;
		_cleanChain.push_back(parallelSlice);
	}
}

void Attribute::cacheCleanChainUpstream(){
	if(_outputs.size() == 0 && _affect.size() == 0 && !_passThrough){
		std::vector<Attribute*> cleanChain;
		std::vector<Attribute*> processedAttributes;
		
		cacheCleanChainDownstream(cleanChain, processedAttributes);
		cacheParallelCleanChain(cleanChain);
	}
	else{
		for(int i = 0; i < _outputs.size(); ++i){
			_outputs[i]->cacheCleanChainUpstream();
		}
	
		for(int i = 0; i < _affect.size(); ++i){
			_affect[i]->cacheCleanChainUpstream();
		}
	}
}

void Attribute::cacheEvaluationChain(){
	cacheDirtyChainDownstream();
	cacheCleanChainUpstream();
}

std::vector<Attribute*> Attribute::specializationLinkedTo(){
	std::vector<Attribute*> attrs;
	for(int i = 0; i < _specializationLinks.size(); ++i){
		if(_specializationLinks[i]->attributeB != this){
			attrs.push_back(_specializationLinks[i]->attributeB);
		}
	}
	
	return attrs;
}

std::vector<Attribute*> Attribute::specializationLinkedBy(){
	std::vector<Attribute*> attrs;
	for(int i = 0; i < _specializationLinks.size(); ++i){
		if(_specializationLinks[i]->attributeB == this){
			attrs.push_back(_specializationLinks[i]->attributeB);
		}
	}
	
	return attrs;
}

Attribute *Attribute::findInputNotPass(Attribute *attribute){
	Attribute *input = 0;
	if(attribute->_input){
		input = findInputNotPass(attribute->_input);
	}

	if(input == 0 && attribute != this && !attribute->_passThrough){
		input = attribute;
	}
	
	return input;
}

Attribute *Attribute::inputSource(){
	Attribute * source = findInputNotPass(this);
	
	return source;
}

void Attribute::setSpecializationOverride(const std::string &specialization){
	std::vector<std::string> spec(1);
	spec[0] = specialization;
	
	_specializationOverride = spec;
	setSpecialization(_specializationOverride);
}

void Attribute::removeSpecializationOverride(){
	_specializationOverride.clear();
	setSpecialization(_allowedSpecialization);
}


