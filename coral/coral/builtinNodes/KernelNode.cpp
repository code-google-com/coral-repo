
#include "KernelNode.h"
#include "../src/stringUtils.h"

using namespace coral;

namespace {
	std::string oclErrorString(cl_int error)
	{
	    static const char* errorString[] = {
	        "CL_SUCCESS",
	        "CL_DEVICE_NOT_FOUND",
	        "CL_DEVICE_NOT_AVAILABLE",
	        "CL_COMPILER_NOT_AVAILABLE",
	        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
	        "CL_OUT_OF_RESOURCES",
	        "CL_OUT_OF_HOST_MEMORY",
	        "CL_PROFILING_INFO_NOT_AVAILABLE",
	        "CL_MEM_COPY_OVERLAP",
	        "CL_IMAGE_FORMAT_MISMATCH",
	        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
	        "CL_BUILD_PROGRAM_FAILURE",
	        "CL_MAP_FAILURE",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "CL_INVALID_VALUE",
	        "CL_INVALID_DEVICE_TYPE",
	        "CL_INVALID_PLATFORM",
	        "CL_INVALID_DEVICE",
	        "CL_INVALID_CONTEXT",
	        "CL_INVALID_QUEUE_PROPERTIES",
	        "CL_INVALID_COMMAND_QUEUE",
	        "CL_INVALID_HOST_PTR",
	        "CL_INVALID_MEM_OBJECT",
	        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
	        "CL_INVALID_IMAGE_SIZE",
	        "CL_INVALID_SAMPLER",
	        "CL_INVALID_BINARY",
	        "CL_INVALID_BUILD_OPTIONS",
	        "CL_INVALID_PROGRAM",
	        "CL_INVALID_PROGRAM_EXECUTABLE",
	        "CL_INVALID_KERNEL_NAME",
	        "CL_INVALID_KERNEL_DEFINITION",
	        "CL_INVALID_KERNEL",
	        "CL_INVALID_ARG_INDEX",
	        "CL_INVALID_ARG_VALUE",
	        "CL_INVALID_ARG_SIZE",
	        "CL_INVALID_KERNEL_ARGS",
	        "CL_INVALID_WORK_DIMENSION",
	        "CL_INVALID_WORK_GROUP_SIZE",
	        "CL_INVALID_WORK_ITEM_SIZE",
	        "CL_INVALID_GLOBAL_OFFSET",
	        "CL_INVALID_EVENT_WAIT_LIST",
	        "CL_INVALID_EVENT",
	        "CL_INVALID_OPERATION",
	        "CL_INVALID_GL_OBJECT",
	        "CL_INVALID_BUFFER_SIZE",
	        "CL_INVALID_MIP_LEVEL",
	        "CL_INVALID_GLOBAL_WORK_SIZE",
	    };

	    const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

	    const int index = -error;

	    return std::string((index >= 0 && index < errorCount) ? errorString[index] : "");
	}

	int findMinorInputSize(const std::vector<Attribute*> &attrs){
		int minorSize = -1;
		for(int i = 0; i < attrs.size(); ++i){
			Attribute *attr = attrs[i];
			if(attr->isInput()){
				Numeric *num = (Numeric*)attr->value();
				int size = num->size();
				if(size < minorSize || minorSize == -1){
					minorSize = size;
				}
			}
		}
		if(minorSize == -1){
			minorSize = 0;
		}
		
		return minorSize;
	}

	typedef struct{
		float x;
		float y;
		float z;
	} CLVec3;

	void writeNullBuffer(Numeric *value, cl::Buffer &buffer, int size, int argId, cl::Kernel &kernel, cl::Context &context, cl::CommandQueue &queue, cl::Event &event){
	}

	void writeIntBuffer(Numeric *value, cl::Buffer &buffer, int size, int argId, cl::Kernel &kernel, cl::Context &context, cl::CommandQueue &queue, cl::Event &event){
		size_t bufferSize = sizeof(int) * size;
		buffer = cl::Buffer(context, CL_MEM_READ_ONLY, bufferSize, NULL);
		queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, bufferSize, &value->intValues()[0], NULL, &event);

		kernel.setArg(argId, buffer);
	}

	void writeFloatBuffer(Numeric *value, cl::Buffer &buffer, int size, int argId, cl::Kernel &kernel, cl::Context &context, cl::CommandQueue &queue, cl::Event &event){
		size_t bufferSize = sizeof(float) * size;
		buffer = cl::Buffer(context, CL_MEM_READ_ONLY, bufferSize, NULL);
		queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, bufferSize, &value->floatValues()[0], NULL, &event);

		kernel.setArg(argId, buffer);
	}

	void writeVec3Buffer(Numeric *value, cl::Buffer &buffer, int size, int argId, cl::Kernel &kernel, cl::Context &context, cl::CommandQueue &queue, cl::Event &event){
		size_t bufferSize = sizeof(CLVec3) * size;
		buffer = cl::Buffer(context, CL_MEM_READ_ONLY, bufferSize, NULL);
		queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, bufferSize, &value->vec3Values()[0], NULL, &event);

		kernel.setArg(argId, buffer);
	}

	void readNullBuffer(Numeric *value, cl::Buffer &buffer, int size, cl::CommandQueue &queue, cl::Event &event){
	}

	void readIntBuffer(Numeric *value, cl::Buffer &buffer, int size, cl::CommandQueue &queue, cl::Event &event){
		size_t bufferSize = sizeof(int) * size;
		std::vector<int> outVal(size);
		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, bufferSize, &outVal[0], NULL, &event);
		value->setIntValues(outVal);
	}

	void readFloatBuffer(Numeric *value, cl::Buffer &buffer, int size, cl::CommandQueue &queue, cl::Event &event){
		size_t bufferSize = sizeof(float) * size;
		std::vector<float> outVal(size);
		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, bufferSize, &outVal[0], NULL, &event);
		value->setFloatValues(outVal);
	}

	void readVec3Buffer(Numeric *value, cl::Buffer &buffer, int size, cl::CommandQueue &queue, cl::Event &event){
		size_t bufferSize = sizeof(CLVec3) * size;
		std::vector<float> outVal(size);
		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, bufferSize, &outVal[0], NULL, &event);
		value->setFloatValues(outVal);
	}
}

KernelNode::KernelNode(const std::string &name, Node *parent): 
Node(name, parent),
_kernelReady(false){
	setAllowDynamicAttributes(true);

	_kernelSource = new StringAttribute("_kernelSource", this);

	addInputAttribute(_kernelSource);

	catchAttributeDirtied(_kernelSource);

	initCL();
}

void KernelNode::attributeDirtied(Attribute *attribute){
	buildKernelSource();
}

void KernelNode::addDynamicAttribute(Attribute *attribute){
	Node::addDynamicAttribute(attribute);
	
	if(attribute->isOutput()){
		std::vector<Attribute*> inputAttrs = inputAttributes();
		for(int i = 0; i < inputAttrs.size(); ++i){
			setAttributeAffect(inputAttrs[i], attribute);
		}
	}

	std::vector<std::string> specs;
	specs.push_back("IntArray");
	specs.push_back("FloatArray");
	specs.push_back("Vec3Array");
	setAttributeAllowedSpecializations(attribute, specs);

	cacheBufferReadWrite(attribute);
}

void KernelNode::initCL(){
	std::vector<cl::Platform> platforms;
    cl_int err = cl::Platform::get(&platforms);
    if(err){
    	std::cerr << "OpenCL error: " << oclErrorString(err) << std::endl;
    	return;
    }

    if(platforms.size() == 0){
    	std::cerr << "OpenCL error: no platforms available." << std::endl;
        return;
    }

	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
	_context = cl::Context(CL_DEVICE_TYPE_CPU, properties);
	_devices = _context.getInfo<CL_CONTEXT_DEVICES>();

	if(_devices.size() == 0){
		std::cerr << "OpenCL error: no devices available." << std::endl;
		return;
	}

	_queue = cl::CommandQueue(_context, _devices[0], 0, &err);

	if(err){
		std::cerr << "OpenCL error creating CommandQueue: " << oclErrorString(err) << std::endl;
		return;
	}
}

std::string KernelNode::buildInfo(){
	return _buildMessage;
}

void KernelNode::buildKernelSource(){
	_buildMessage.clear();

	_kernelReady = false;
	cl_int err = 0;
	
	std::string kernelSource = _kernelSource->value()->stringValue();
	if(kernelSource.empty()){
		return;
	}

	cl::Program program;
	try{
        cl::Program::Sources source(1, std::make_pair(kernelSource.c_str(), kernelSource.size()));
        program = cl::Program(_context, source);
    }
    catch(cl::Error er){
    	_buildMessage = "OpenCL error: " + std::string(er.what()) + " " + oclErrorString(er.err());
        std::cerr << _buildMessage << std::endl;
        return;
    }

    try{
    	err = program.build(_devices);
    }
    catch(cl::Error er){
    	_buildMessage = "OpenCL error: " + oclErrorString(er.err()) + "\n" + program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(_devices[0]);
    	std::cerr << _buildMessage << std::endl;
    	return;
    }

    try{
        _kernel = cl::Kernel(program, "mainKernel", &err);
        if(!err){
        	_kernelReady = true;
        	_buildMessage = "Successfully built.";
        }
    }
    catch (cl::Error er) {
    	std::string errStr = oclErrorString(er.err());

    	_buildMessage = "OpenCL error: " + std::string(er.what()) + " " + errStr;

    	if(errStr == "CL_INVALID_KERNEL_NAME"){
    		_buildMessage += "\n\nKernel name should be 'mainKernel'";
    	}

        std::cerr << _buildMessage << std::endl;
        return;
    }
}

void KernelNode::cacheBufferReadWrite(Attribute *attribute){
	Numeric *val = (Numeric*)attribute->value();
	int attrId = attribute->id();
	if(val->type() == Numeric::numericTypeIntArray){
		_writeBuffer[attrId] = writeIntBuffer;
		_readBuffer[attrId] = readIntBuffer;
	}
	else if(val->type() == Numeric::numericTypeFloatArray){
		_writeBuffer[attrId] = writeFloatBuffer;
		_readBuffer[attrId] = readFloatBuffer;
	}
	else if(val->type() == Numeric::numericTypeVec3Array){
		_writeBuffer[attrId] = writeVec3Buffer;
		_readBuffer[attrId] = readVec3Buffer;
	}
	else{
		_writeBuffer[attrId] = writeNullBuffer;
		_readBuffer[attrId] = readNullBuffer;
	}
}

void KernelNode::attributeSpecializationChanged(Attribute *attribute){
	cacheBufferReadWrite(attribute);
}

void KernelNode::update(Attribute *attribute){
	if(_kernelReady){
		cl_int err = 0;

		std::vector<Attribute*> dynAttrs = dynamicAttributes();
		int size = findMinorInputSize(dynAttrs);

		if(size == 0){
			return;
		}

		cl::Event event;
		int argId = 0;
		std::map<int, cl::Buffer> buffers;
		std::map<int, Numeric*> values;
		for(int i = 0; i < dynAttrs.size(); ++i){
			Attribute *attr = dynAttrs[i];
			int attrId = attr->id();
			cl::Buffer &attrBuffer = buffers[attrId];

			Numeric *val = (Numeric*)attr->value();
			values[attrId] = val;

			_writeBuffer[attrId](val, attrBuffer, size, i, _kernel, _context, _queue, event);
		}

		try{
			err = _queue.enqueueNDRangeKernel(_kernel, cl::NullRange, cl::NDRange(size), cl::NullRange, NULL, &event);
		}
		catch(cl::Error er){
			std::cerr << "OpenCL enqueueNDRangeKernel error: " << oclErrorString(err) << std::endl;
			_queue.finish();
			return;
		}

		const std::vector<Attribute*> &outAttrs = outputAttributes();
		for(int i = 0; i < outAttrs.size(); ++i){
			Attribute *attr = outAttrs[i];
			int attrId = attr->id();

			cl::Buffer &buffer = buffers[attrId];
			Numeric *val = values[attrId];
			_readBuffer[attrId](val, buffer, size, _queue, event);
		}

		_queue.finish();
	}
}

