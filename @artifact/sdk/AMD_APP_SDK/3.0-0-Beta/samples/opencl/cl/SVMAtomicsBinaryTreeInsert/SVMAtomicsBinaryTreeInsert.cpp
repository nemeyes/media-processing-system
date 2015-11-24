/**********************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/


#include "SVMAtomicsBinaryTreeInsert.hpp"
#include "SVMAtomicsBinaryTreeInsert_Host.hpp"

int SVMAtomicsBinaryTreeInsert::setupSVMBinaryTree()
{
  //Ensure that there is atleast 1 node to start with
  if (init_tree_insert < 1)
	init_tree_insert = 1;

  if (num_insert > 125000000)
	num_insert = 125000000;

  //Num of nodes to insert on host and device
  host_nodes = (size_t)((double)num_insert * ((float)hostCompPercent / 100));
  device_nodes = num_insert - host_nodes;

  total_nodes = num_insert + init_tree_insert;

  return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::setupCL(void)
{
  cl_int status = 0;
  cl_device_type dType;
  
  if(sampleArgs->deviceType.compare("cpu") == 0)
    {
      dType = CL_DEVICE_TYPE_CPU;
    }
  else //deviceType = "gpu"
    {
      dType = CL_DEVICE_TYPE_GPU;
      if(sampleArgs->isThereGPU() == false)
        {
	  std::cout << "GPU not found. Falling back to CPU device" << std::endl;
	  dType = CL_DEVICE_TYPE_CPU;
        }
    }
  
  // Get platform
  cl_platform_id platform = NULL;
  int retValue = getPlatform(platform, sampleArgs->platformId,
			     sampleArgs->isPlatformEnabled());
  CHECK_ERROR(retValue, SDK_SUCCESS, "getPlatform() failed");
  
  // Display available devices.
  retValue = displayDevices(platform, dType);
  CHECK_ERROR(retValue, SDK_SUCCESS, "displayDevices() failed");
    
  // If we could find our platform, use it. Otherwise use just available 
  // platform.
  cl_context_properties cps[3] =
    {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)platform,
      0
    };
  
  context = clCreateContextFromType(
				    cps,
				    dType,
				    NULL,
				    NULL,
				    &status);
  CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed.");
  
  status = getDevices(context, &devices, sampleArgs->deviceId,
		      sampleArgs->isDeviceIdEnabled());
  CHECK_ERROR(status, SDK_SUCCESS, "getDevices() failed");
  
  //Set device info of given cl_device_id
  status = deviceInfo.setDeviceInfo(devices[sampleArgs->deviceId]);
  CHECK_ERROR(status, SDK_SUCCESS, "SDKDeviceInfo::setDeviceInfo() failed");
 

  //check 2.x compatibility
  bool check2_x = deviceInfo.checkOpenCL2_XCompatibility();

  if (!check2_x)
  {
	OPENCL_EXPECTED_ERROR("Unsupported device! Required CL_DEVICE_OPENCL_C_VERSION 2.0 or higher");
  }

  //check SVM capabilities Finegrain and Atomics

  if (!(deviceInfo.svmcaps & CL_DEVICE_SVM_ATOMICS))
  {
	OPENCL_EXPECTED_ERROR("Unsupported device! Device does not support SVM Atomics");
  }


  // Create command queue
  cl_queue_properties prop[] = {0};
  commandQueue = clCreateCommandQueueWithProperties(
                    context,
		    devices[sampleArgs->deviceId],
		    prop,
		    &status);
  CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");

  // create a CL program using the kernel source
  buildProgramData buildData;
  buildData.kernelName = std::string("SVMAtomicsBinaryTreeInsert_Kernels.cl");
  buildData.devices = devices;
  buildData.deviceId = sampleArgs->deviceId;
  buildData.flagsStr = std::string("");
  
  if(sampleArgs->isLoadBinaryEnabled())
    {
      buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
    }

  if(sampleArgs->isComplierFlagsSpecified())
    {
      buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
    }

  retValue = buildOpenCLProgram(program, context, buildData);
  CHECK_ERROR(retValue, SDK_SUCCESS, "buildOpenCLProgram() failed");


  // get a kernel object handle for a kernel with the given name
  binTreeInsert_kernel = clCreateKernel(program, "binTreeInsert", &status);
  CHECK_OPENCL_ERROR(status, "clCreateKernel::sample_kernel failed.");


  // initialize any device/SVM memory here.
  int flags = CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS;
  svmTreeBuf = (node *) clSVMAlloc(context,
			  flags,
			  total_nodes*sizeof(node),
			  0);
  
  if(NULL == svmTreeBuf)
    retValue = SDK_FAILURE;

  CHECK_ERROR(retValue, SDK_SUCCESS, "clSVMAlloc(svmTreeBuf) failed.");  

  return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::genBinaryImage()
{
    bifData binaryData;
    binaryData.kernelName = std::string("SVMAtomicsBinaryTreeInsert_Kernels.cl");
    binaryData.flagsStr = std::string("");
    if(sampleArgs->isComplierFlagsSpecified())
    {
      binaryData.flagsFileName = std::string(sampleArgs->flags.c_str());
    }
    binaryData.binaryName = std::string(sampleArgs->dumpBinary.c_str());
    int status = generateBinaryImage(binaryData);
    return status;
}

int SVMAtomicsBinaryTreeInsert::runCLKernels(void)
{
    cl_int status;

    if (device_nodes > 0)
    {
	    status =  kernelInfo.setKernelWorkGroupInfo(binTreeInsert_kernel,
		      devices[sampleArgs->deviceId]);
	    CHECK_ERROR(status, SDK_SUCCESS, "setKErnelWorkGroupInfo() failed");

	    size_t globalThreads = device_nodes;
	    size_t localThreads  = kernelInfo.kernelWorkGroupSize;

	    size_t deviceStartNode = init_tree_insert + host_nodes;

		// Set appropriate arguments to the kernel
	    status = clSetKernelArgSVMPointer(binTreeInsert_kernel,
						  0,
						  (void *)(svmTreeBuf));
	    CHECK_OPENCL_ERROR(status, "clSetKernelArgSVMPointer(svmTreeBuf) failed.");

	    status = clSetKernelArgSVMPointer(binTreeInsert_kernel,
					      1,
					      (void *)(svmTreeBuf + deviceStartNode));
	    CHECK_OPENCL_ERROR(status,"clSetKernelArgSVMPointer(svmTreeBuf + deviceStartNode) failed.");

	    status = clSetKernelArg(binTreeInsert_kernel, 2, sizeof(size_t), &device_nodes);
	    CHECK_OPENCL_ERROR(status,"clSetKernelArg(device_nodes) failed.");

	    // Enqueue a kernel run call
	    status = clEnqueueNDRangeKernel(
		         commandQueue,
		         binTreeInsert_kernel,
		         1,
		         NULL,
		         &globalThreads,
		         &localThreads,
		         0,
		         NULL,
		         NULL);
	    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	    status = clFlush(commandQueue);
	    CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
    }
    if (host_nodes > 0)
    {
#pragma omp parallel for
		for (long k = 0; k < host_nodes; k++)
    	{
			insertNode(&(currNode[(size_t)k]), &svmRoot);
		}
    }

    if (device_nodes > 0)
    {
    	status = clFinish(commandQueue);
    	CHECK_OPENCL_ERROR(status, "clFinish failed.(commandQueue)");
    }    
    return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::initialize()
{
  // Call base class Initialize to get default configuration
  if(sampleArgs->initialize() != SDK_SUCCESS)
    {
      return SDK_FAILURE;
    }
  
  Option* new_option = new Option;
  CHECK_ALLOCATION(new_option, "Memory allocation error. (new_option)");
  
  new_option->_sVersion = "s";
  new_option->_lVersion = "seed";
  new_option->_description = "Seed to random number generator";
  new_option->_type = CA_ARG_INT;
  new_option->_value = &localSeed;
  sampleArgs->AddOption(new_option);
  
  new_option->_sVersion = "n";
  new_option->_lVersion = "numins";
  new_option->_description = "Number of nodes to insert(< 125000000)";
  new_option->_type = CA_ARG_LONG;
  new_option->_value = &num_insert;
  sampleArgs->AddOption(new_option);

  new_option->_sVersion = "in";
  new_option->_lVersion = "initnodes";
  new_option->_description = "Number of initial nodes (> 1)";
  new_option->_type = CA_ARG_LONG;
  new_option->_value = &init_tree_insert;
  sampleArgs->AddOption(new_option);

  new_option->_sVersion = "hp";
  new_option->_lVersion = "host";
  new_option->_description = "Percentage of nodes to be inserted on host (between 0 and 100)";
  new_option->_type = CA_ARG_INT;
  new_option->_value = &hostCompPercent;
  sampleArgs->AddOption(new_option);

  new_option->_sVersion = "po";
  new_option->_lVersion = "prorder";
  new_option->_description = "Print order of the tree nodes";
  new_option->_type = CA_NO_ARGUMENT;
  new_option->_value = &printTreeOrder;
  sampleArgs->AddOption(new_option);

  delete new_option;
  
  return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::setup()
{
  int retStatus;

  if(setupSVMBinaryTree() != SDK_SUCCESS)
  {
     return SDK_FAILURE;
  }
  
  int timer = sampleTimer->createTimer();
  sampleTimer->resetTimer(timer);
  sampleTimer->startTimer(timer);
  
  retStatus = setupCL();
  if (retStatus != SDK_SUCCESS)
  {
     return retStatus;
  }
  
  sampleTimer->stopTimer(timer);
  setupTime = (cl_double)sampleTimer->readTimer(timer);

  return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::run()
{
    int status = 0;
	
    //create the initial binary tree with init_tree_insert nodes
    status = cpuCreateBinaryTree();
    CHECK_ERROR(status, SDK_SUCCESS, "cpuCreateBinaryTree() failed.");
	
    //Advance the current node after initial insert
    currNode = svmRoot + init_tree_insert;

    /* if voice is not deliberately muzzled, shout parameters */
    if(!sampleArgs->quiet)
    {
	std::cout << "--------------------------------------------------";
	std::cout << "-----------------------" << std::endl;
	std::cout << "Inserting " << num_insert << " nodes in  a Binary Tree having ";
	std::cout << init_tree_insert << " Nodes..." << std::endl;
	std::cout << "--------------------------------------------------";
	std::cout << "-----------------------" << std::endl;
    }  

    int timer = sampleTimer->createTimer();
    sampleTimer->resetTimer(timer);
    sampleTimer->startTimer(timer);

    // Arguments are set and execution call is enqueued on command buffer
    if(runCLKernels() != SDK_SUCCESS)
    {
	return SDK_FAILURE;
    }
    
    sampleTimer->stopTimer(timer);
    execTime = (double)(sampleTimer->readTimer(timer));

    if(!sampleArgs->quiet)
    {
	std::cout << "Nodes inserted on host   = " << host_nodes << std::endl;
	std::cout << "Nodes inserted on device = " << device_nodes << std::endl;
    }

    if (printTreeOrder)
	recursiveInOrder(svmRoot);

    return SDK_SUCCESS;
}

size_t SVMAtomicsBinaryTreeInsert::count_nodes(node* root)
{
    size_t count = 0;
    if (root)
	count = 1;

    if (root->left)
	count += count_nodes(root->left);

    if (root->right)
	count += count_nodes(root->right);

    return count;
}

int SVMAtomicsBinaryTreeInsert::verifyResults()
{
  int status = SDK_SUCCESS;
  if(sampleArgs->verify)
    {
      size_t actualNodes = count_nodes(svmTreeBuf);
      std::cout << "Actual Nodes (including the initial nodes) = " << actualNodes << std::endl;

      if (actualNodes == total_nodes)      
      {
	  std::cout << "Passed!\n" << std::endl;
      }
      else
      {
	  std::cout << "Failed\n" << std::endl;
      }
    }
  return status;
}

void SVMAtomicsBinaryTreeInsert::printStats()
{
    if(sampleArgs->timing)
    {
        std::string strArray[3] =
        {
            "Setup Time(sec)",
            "Avg. Node Insert time (sec)",
	    "Nodes inserted/sec"
        };
        std::string stats[3];
        double avgExecTime = execTime;
	double nodesPerSec   = (double)total_nodes/avgExecTime;

        stats[0] = toString(setupTime, std::dec);
        stats[1] = toString(avgExecTime, std::dec);
        stats[2] = toString(nodesPerSec, std::dec);

        printStatistics(strArray, stats, 3);
    }
}

int SVMAtomicsBinaryTreeInsert::cleanup()
{
    // Releases OpenCL resources (Context, Memory etc.)
    cl_int status = 0;

    clSVMFree(context,svmTreeBuf);

    status = clReleaseKernel(binTreeInsert_kernel);
    CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.(binTreeInsert_kernel)");

    status = clReleaseProgram(program);
    CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.(program)");

    status = clReleaseCommandQueue(commandQueue);
    CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.(commandQueue)");

    status = clReleaseContext(context);
    CHECK_OPENCL_ERROR(status, "clReleaseContext failed.(context)");

    return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::cpuCreateBinaryTree()
{
  node*    root;

  //Initialize the node elements
  initialize_nodes(svmTreeBuf, total_nodes, localSeed);

  //Make tree with given initial nodes - init_tree_insert
  root   = cpuMakeBinaryTree(init_tree_insert, svmTreeBuf);
  
  /* set the root */
  svmRoot = root;

  return SDK_SUCCESS;
}

int SVMAtomicsBinaryTreeInsert::recursiveInOrder(node* leaf)
{
  if(NULL != leaf)
    {
      recursiveInOrder(leaf->left);
      std::cout << leaf->value << ", ";
      recursiveInOrder(leaf->right);
    }

  return SDK_SUCCESS;
}

int main(int argc, char * argv[])
{
    SVMAtomicsBinaryTreeInsert clSVMBinaryTree;
	int retStatus;

    // Initialize
    if(clSVMBinaryTree.initialize() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    if(clSVMBinaryTree.sampleArgs->parseCommandLine(argc, argv) != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    if(clSVMBinaryTree.sampleArgs->isDumpBinaryEnabled())
    {
        //GenBinaryImage
        return clSVMBinaryTree.genBinaryImage();
    }

    // Setup
	retStatus = clSVMBinaryTree.setup();
    if(retStatus != SDK_SUCCESS)
    {
        return retStatus;
    }

    // Run
    if(clSVMBinaryTree.run() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // VerifyResults
    if(clSVMBinaryTree.verifyResults() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Cleanup
    if (clSVMBinaryTree.cleanup() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    clSVMBinaryTree.printStats();
    return SDK_SUCCESS;
}
