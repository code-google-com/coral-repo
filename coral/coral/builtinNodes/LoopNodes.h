#ifndef CORAL_LOOPNODES_H
#define CORAL_LOOPNODES_H

#include "../src/Node.h"
#include "../src/NumericAttribute.h"

namespace coral{

class LoopIteratorNode: public Node{
public:
	LoopIteratorNode(const std::string &name, Node *parent);
	virtual void loopStart(unsigned int loopRangeSize);
	virtual void loopStep(unsigned int index);
	virtual void loopEnd();
	void update(Attribute *attribute);
};

class ForLoopNode: public Node{
public:
	ForLoopNode(const std::string &name, Node *parent);
	void update(Attribute *attribute);
	void addDynamicAttribute(Attribute *attribute);
	
private:	
	NumericAttribute *_indexRange;
	NumericAttribute *_currentIndex;
	
	void collectCleanChain(Attribute *attribute, std::vector<Attribute*> &cleanChain);
	void collectLoopOperators(std::vector<LoopIteratorNode*> &loopOperators);
};

}

#endif
