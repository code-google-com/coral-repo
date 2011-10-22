#include "ProcessSimulation.h"

using namespace coral;

bool _processScheduled = false;
ProcessSimulation *_globalProcessSimulation = 0;

ProcessCleanerAttribute::ProcessCleanerAttribute(const std::string &name, Node *parent): Attribute(name, parent){
	setClassName("ProcessCleanerAttribute");
}

void ProcessCleanerAttribute::onDirtied(){
	ProcessSimulation *parentNode = (ProcessSimulation*)parent();
	parentNode->scheduleProcess();
}

ProcessSimulation::ProcessSimulation(const std::string &name, Node *parent): 
	Node(name, parent){
	setClassName("ProcessSimulation");
	
	if(_globalProcessSimulation){
		setIsInvalid(true, "Only one ProcessSimulation node is allowed.");
		return;
	}
	
	_process = new PassThroughAttribute("process", this);
	_cleaner = new ProcessCleanerAttribute("_cleaner", this);
	
	addInputAttribute(_process);
	addOutputAttribute(_cleaner);
	
	setAttributeAffect(_process, _cleaner);
	
	setAttributeAllowedSpecialization(_process, "process");
	
	_globalProcessSimulation = this;
}

void ProcessSimulation::deleteIt(){
	Node::deleteIt();
	
	_globalProcessSimulation = 0;
}

void ProcessSimulation::process(Attribute *attribute){
	_processScheduled = false;
	
	if(_globalProcessSimulation){
		std::vector<Attribute*> attrs = _globalProcessSimulation->inputAttributes();
		for(int i = 0; i < attrs.size(); ++i){
			attrs[i]->value();
		}
	}
}

void ProcessSimulation::scheduleProcess(){
	if(!_processScheduled){
		_processScheduled = true;
		Attribute::queueDirtyingDoneCallback(&ProcessSimulation::process);
	}
}


