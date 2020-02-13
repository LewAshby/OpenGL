#pragma once

//#include "cl.hpp"

#include "util.hpp" // utility library

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <iostream>
#include <fstream>

// pick up device type from compiler command line or from the default type
#ifndef DEVICE
#define DEVICE CL_DEVICE_TYPE_GPU
#endif

#include "err_code.h"
#include "globals.h"


void run(int nrows, int ncols, int nneighbors, float NDataValue)
{
	try
	{
		cl::make_kernel<int, int, float, int, float, cl::Buffer, cl::Buffer, cl::Buffer> kernel1(outflow_computation, "kernel1");
		cl::make_kernel<int, int, float, int, cl::Buffer, cl::Buffer, cl::Buffer> kernel2(mass_balance, "kernel2");
		cl::make_kernel<int, int, float, int, cl::Buffer, cl::Buffer> kernel3(outflow_reset, "kernel3");

		kernel1(
			cl::EnqueueArgs(queue, cl::NDRange(nrows - 1, ncols - 1)),
			nrows,
			ncols,
			0.75,
			nneighbors,
			NDataValue,
			zB,
			hB,
			SoB
		);

		kernel2(
			cl::EnqueueArgs(queue, cl::NDRange(nrows - 1, ncols - 1)),
			nrows,
			ncols,
			nneighbors,
			NDataValue,
			zB,
			hB,
			SoB
		);

		kernel3(
			cl::EnqueueArgs(queue, cl::NDRange(nrows - 1, ncols - 1)),
			nrows,
			ncols,
			nneighbors,
			NDataValue,
			zB,
			SoB
		);

		queue.finish();

		cl::copy(queue, hB, H.begin(), H.end());

	}
	catch (cl::Error err) {
		std::cout << "Exception\n";
		std::cerr
			<< "ERROR: "
			<< err.what()
			<< "("
			<< err_code(err.err())
			<< ")"
			<< std::endl;
	}
}