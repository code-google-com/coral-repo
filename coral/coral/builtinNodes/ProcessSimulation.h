#ifndef CORAL_PROCESSNODE_H
#define CORAL_PROCESSNODE_H

#include "../src/Node.h"
#include "../src/PassThroughAttribute.h"
#include "../src/Attribute.h"

namespace coral{

class ProcessCleanerAttribute: public Attribute{
public:
	ProcessCleanerAttribute(const std::string &name, Node *parent);
	void onDirtied();
};

class ProcessSimulation: public Node{
public:
	ProcessSimulation(const std::string &name, Node *parent);
	void scheduleProcess();
	void deleteIt();
	
private:
	PassThroughAttribute *_process;
	ProcessCleanerAttribute *_cleaner;
	
	static void process(Attribute *attribute);
};

}

#endif
