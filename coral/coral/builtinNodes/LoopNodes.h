#ifndef CORAL_LOOPNODES_H
#define CORAL_LOOPNODES_H

#include "../src/Node.h"
#include "../src/NumericAttribute.h"
#include "../src/PassThroughAttribute.h"

namespace coral{

class LoopIteratorNode: public Node{
public:
	LoopIteratorNode(const std::string &name, Node *parent);
	NumericAttribute *index();
	virtual void loopStart(unsigned int loopRangeSize);
	virtual void loopStep(unsigned int index);
	virtual void loopEnd();
	void update(Attribute *attribute);

private:
	NumericAttribute *_index;
};

class ForLoopNode: public Node{
public:
	ForLoopNode(const std::string &name, Node *parent);
	void update(Attribute *attribute);
	void addDynamicAttribute(Attribute *attribute);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void attributeSpecializationChanged(Attribute *attribute);

private:
	NumericAttribute *_array;
	NumericAttribute *_currentElement;
	NumericAttribute *_currentIndex;
	PassThroughAttribute *_out;
	void(ForLoopNode::*_selectedOperation)(Numeric *, Numeric *, Numeric *, std::map<int, std::vector<Attribute*> > &);
	
	void collectLoopOperators(std::vector<LoopIteratorNode*> &loopOperators);
	void getSubCleanChain(Attribute *attribute, std::map<int, std::vector<Attribute*> > &subCleanChain);
	void subClean(std::map<int, std::vector<Attribute*> > &subCleanChain);
	void updateInt(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain);
	void updateFloat(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain);
	void updateVec3(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain);
	void updateCol4(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain);
	void updateMatrix44(Numeric *array, Numeric *currentElement, Numeric *currentIndex, std::map<int, std::vector<Attribute*> > &subCleanChain);
};

}

#endif
