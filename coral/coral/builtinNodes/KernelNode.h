
#ifndef CORAL_KERNELNODE_H
#define CORAL_KERNELNODE_H

#include "../src/Node.h"
#include "../src/NumericAttribute.h"
#include "../src/StringAttribute.h"

#define __CL_ENABLE_EXCEPTIONS

#include <map>
#include <vector>
#include <CL/cl.hpp>

namespace coral{

class KernelNode: public Node{
public:
	KernelNode(const std::string &name, Node *parent);
	void addDynamicAttribute(Attribute *attribute);
	void attributeDirtied(Attribute *attribute);
	void update(Attribute *attribute);
	void attributeSpecializationChanged(Attribute *attribute);

	std::string buildInfo();
	void buildKernelSource();

private:
	void initCL();
	void cacheBufferReadWrite(Attribute *attribute);
	
	StringAttribute *_kernelSource;

	cl::Context _context;
	cl::CommandQueue _queue;
	cl::Kernel _kernel;
	std::vector<cl::Device> _devices;
	std::map<int, void(*)(Numeric *, cl::Buffer &, int, int, cl::Kernel &, cl::Context &, cl::CommandQueue &, cl::Event &)> _writeBuffer;
	std::map<int, void(*)(Numeric *, cl::Buffer &, int, cl::CommandQueue &, cl::Event &)> _readBuffer;
	bool _kernelReady;
	std::string _buildMessage;
};

}

#endif