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

#include "NetworkManager.h"
#include "Node.h"
#include "Attribute.h"
#include "ErrorObject.h"
#include "containerUtils.h"

using namespace coral;

int NetworkManager::_nextAvailableId = 0;
std::map<int, Object *> NetworkManager::_objectsById;

int NetworkManager::useNextAvailableId(){
	_nextAvailableId += 1;
	
	return _nextAvailableId;
}

void NetworkManager::storeObject(int id, Object *object){
	_objectsById[id] = object;
}

void NetworkManager::removeObject(int id){
	std::map<int, Object *>::iterator it = _objectsById.find(id);
	if(it != _objectsById.end()){
		_objectsById.erase(it);
	}
}

int NetworkManager::objectCount(){
	return (int)_objectsById.size();
}

Object *NetworkManager::findObjectById(int id){
	Object *foundObject = 0;
	
	std::map<int, Object *>::iterator it = _objectsById.find(id);
	if(it != _objectsById.end()){
		foundObject = _objectsById[id];
	}
	
	return foundObject;
}

bool NetworkManager::isCycle(Attribute *attribute, Attribute *input){

	if(attribute && input){
		bool inputNeedsRecursion = false;
		bool affectedByNeedsRecursion = false;

		if(attribute->_isInput && attribute->_input){
			if(attribute->_input == input)
				return true;
			else
				inputNeedsRecursion = true;
		}
		// check attribute->affectedBy() too before starting an input recursion
		else if(attribute->_isOutput && attribute->_affectedBy.empty() == false){
			if(attribute->isAffectedBy(input))
				return true;
			else
				affectedByNeedsRecursion = true;
		}

		if(inputNeedsRecursion)
			if(isCycle(attribute->_input, input))
				return true;

		if(affectedByNeedsRecursion){
			for(unsigned int u = 0; u < attribute->_affectedBy.size(); ++u){
				if(isCycle(attribute->_affectedBy[u], input)){
					return true;
				}
			}
		}
	}

	return false;
}

bool NetworkManager::allowConnection(Attribute *sourceAttribute, Attribute *destinationAttribute, ErrorObject *errorObject){
	if(!sourceAttribute){
		errorObject->setMessage(sourceAttribute->fullName() + " is not a valid object");

		return false;
	}

	if(!destinationAttribute){
		errorObject->setMessage(destinationAttribute->fullName() + " is is not a valid object");

		return false;
	}

	if(sourceAttribute == destinationAttribute){
		errorObject->setMessage(sourceAttribute->fullName() + " can't be connected to itself");

		return false;
	}

	if(sourceAttribute->parent() == 0 || containerUtils::elementInContainer(sourceAttribute, sourceAttribute->parent()->attributes()) == false){
		errorObject->setMessage("no parent node found for " + sourceAttribute->fullName());

		return false;
	}
	
	if(destinationAttribute->parent() == 0 || containerUtils::elementInContainer(destinationAttribute, destinationAttribute->parent()->attributes()) == false){
		errorObject->setMessage("no parent node found for " + destinationAttribute->fullName());

		return false;
	}

	if(destinationAttribute->_input == sourceAttribute){
		std::string message(sourceAttribute->fullName() + " already connected to ");
		errorObject->setMessage(message + destinationAttribute->fullName());

		return false;
	}
	
	if(destinationAttribute->_isOutput == true && destinationAttribute->_passThrough == false){
		errorObject->setMessage(destinationAttribute->fullName() + " is output");

		return false;
	}

	if(isCycle(sourceAttribute, destinationAttribute)){	// connection/affect
		std::string message("cycle : p");
		errorObject->setMessage(message);

		return false;
	}
	
	if(!sourceAttribute->allowConnectionTo(destinationAttribute)){
		std::string message("types don't match : /");
		errorObject->setMessage(message);
		
		return false;
	}


	return true;
}

bool NetworkManager::connect(Attribute *sourceAttribute, Attribute *destinationAttribute, ErrorObject *errorObject){
	bool success = false;
		
	if(!errorObject){
		errorObject = new ErrorObject();
	}
	
	errorObject->addReference();
	
	if(allowConnection(sourceAttribute, destinationAttribute, errorObject)){
		success = sourceAttribute->connectTo(destinationAttribute, errorObject);
	}

	errorObject->removeReference();
	
	return success;
}

