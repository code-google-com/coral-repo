
#include "KernelNode.h"

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
}

KernelNode::KernelNode(const std::string &name, Node *parent): 
Node(name, parent),
_kernelReady(false){
	setAllowDynamicAttributes(true);

	_indexRange = new NumericAttribute("indexRange", this);
	_kernelSource = new StringAttribute("_kernelSource", this);

	addInputAttribute(_indexRange);
	addInputAttribute(_kernelSource);

	catchAttributeDirtied(_kernelSource);
}

void KernelNode::attributeDirtied(Attribute *attribute){
	
}

void KernelNode::addDynamicAttribute(Attribute *attribute){
	Node::addDynamicAttribute(attribute);
	
	if(attribute->isOutput()){
		std::vector<Attribute*> inputAttrs = inputAttributes();
		for(int i = 0; i < inputAttrs.size(); ++i){
			setAttributeAffect(inputAttrs[i], attribute);
		}
	}
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

void KernelNode::buildKernelSource(){
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
        std::cerr << "OpenCL error: " << er.what() << " " << oclErrorString(er.err()) << std::endl;
    }

    try{
    	err = program.build(_devices);
    }
    catch(cl::Error er){
    	std::cerr << "OpenCL error: " << oclErrorString(er.err()) << std::endl;
    	std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(_devices[0]) << std::endl;
    	return;
    }

    try{
        _kernel = cl::Kernel(program, "main", &err);
        if(err != 0){
        	_kernelReady = true;
        }
    }
    catch (cl::Error er) {
        std::cerr << "OpenCL error: " << er.what() << " " << oclErrorString(er.err()) << std::endl;
    }
}

void KernelNode::update(Attribute *attribute){
	if(_kernelReady){

	}
}

 //    std::vector<float> aValues;
 //    aValues.push_back(1.0);

 //    std::vector<float> bValues;
 //    bValues.push_back(2.0);

 //    std::vector<float> cValues;
 //    cValues.push_back(0.0);

 //    size_t array_size = sizeof(float) * aValues.size();

 //    cl::Buffer cl_a = cl::Buffer(_context, CL_MEM_READ_ONLY, array_size, NULL, &err);
 //    cl::Buffer cl_b = cl::Buffer(_context, CL_MEM_READ_ONLY, array_size, NULL, &err);
 //    cl::Buffer cl_c = cl::Buffer(_context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);

 //    cl::Event event;
 //    err = _queue.enqueueWriteBuffer(cl_a, CL_TRUE, 0, array_size, &aValues[0], NULL, &event);
 //    err = _queue.enqueueWriteBuffer(cl_b, CL_TRUE, 0, array_size, &bValues[0], NULL, &event);
 //    err = _queue.enqueueWriteBuffer(cl_c, CL_TRUE, 0, array_size, &cValues[0], NULL, &event);

 //    err = kernel.setArg(0, cl_a);
 //    err = kernel.setArg(1, cl_b);
 //    err = kernel.setArg(2, cl_c);

 //    _queue.finish();

 //    /// run kernel
 //    err = _queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(aValues.size()), cl::NullRange, NULL, &event); 
 //    _queue.finish();

 //    std::vector<float> res(1);
 //    err = _queue.enqueueReadBuffer(cl_c, CL_TRUE, 0, array_size, &res[0], NULL, &event);

 //    for(int i = 0; i < res.size(); ++i){
 //    	std::cout << res[i] << std::endl;
 //    }
//}
