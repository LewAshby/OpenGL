#pragma once

#define __CL_ENABLE_EXCEPTIONS

#include "cl.hpp"

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

cl::Context context;
cl::Program outflow_computation;
cl::Program mass_balance;
cl::Program outflow_reset;


void run(int nrows, int ncols, int nneighbors, float NDataValue)
{
	try
	{
		context = cl::Context(DEVICE);
		outflow_computation = cl::Program(context, util::loadProgram("resources/kernels/kernel1.cl"), true);
		mass_balance = cl::Program(context, util::loadProgram("resources/kernels/kernel2.cl"), true);
		outflow_reset = cl::Program(context, util::loadProgram("resources/kernels/kernel3.cl"), true);

		cl::make_kernel<int, int, float, int, float, cl::Buffer, cl::Buffer, cl::Buffer> kernel1(outflow_computation, "kernel1");
		cl::make_kernel<int, int, float, int, cl::Buffer, cl::Buffer, cl::Buffer> kernel2(mass_balance, "kernel2");
		cl::make_kernel<int, int, float, int, cl::Buffer, cl::Buffer> kernel3(outflow_reset, "kernel3");

		cl::Buffer hB = cl::Buffer(context, H.begin(), H.end(), CL_MEM_READ_WRITE, true);
		cl::Buffer SoB = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(double) * SoSize);
		cl::Buffer zB = cl::Buffer(context, Z.begin(), Z.end(), CL_MEM_READ_ONLY, true);

		cl::CommandQueue queue(context);

		queue.enqueueWriteBuffer(zB, CL_TRUE, 0, sizeof(double) * Z.size(), &Z[0]);
		queue.enqueueWriteBuffer(hB, CL_TRUE, 0, sizeof(double) * H.size(), &H[0]);

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
		
		queue.enqueueReadBuffer(SoB, CL_TRUE, 0, sizeof(double) * SoSize, &lSo[0]);

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

		queue.enqueueReadBuffer(hB, CL_TRUE, 0, sizeof(double) * H.size(), &H[0]);

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

		cl::copy(queue, SoB, lSo.begin(), lSo.end());
		
		int lSoIndex = 0;
		for (int i = 0; i < nneighbors; i++) {
			for (int j = 0; j < 5; j++) {
				if (i == 0)
					lSoIndex++;
				else
					So[i][j] = lSo[lSoIndex++];
			}
		}
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