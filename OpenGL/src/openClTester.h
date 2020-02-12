#pragma once


#include "globals.h"
#include "flow.h"
#include "openClExecutor.h"

#include "err_code.h"
#include "util.hpp"

#include <ctime>

void serialAlgorithm(double* z, double* sThickness, double* So[], Neighborhood& neighborhood)
{
	time_t startTest = time(0);
    double sumOfKernels1ExecutionTimes = 0, sumOfKernels2ExecutionTimes = 0, sumOfKernels3ExecutionTimes = 0;
    for (int i = steps; i > 0; i--) {

        time_t startTestKernel1 = time(0);
        for (int i = 1; i < nrows - 2; i++)
            for (int j = 1; j < nrows - 2; j++)
                if (z[i * ncols + j] != NoDataValue)
                    kernel1_outflow_computation(ncols, i, j, z, sThickness, So, dumping_factor, neighborhood);
        sumOfKernels1ExecutionTimes += time(0) - startTestKernel1;

        time_t startTestKernel2 = time(0);
        for (int i = 1; i < nrows - 2; i++)
            for (int j = 1; j < nrows - 2; j++)
                if (z[i * ncols + j] != NoDataValue)
                    kernel2_mass_balance(ncols, i, j, sThickness, So, neighborhood);
        sumOfKernels2ExecutionTimes += time(0) - startTestKernel2;


        time_t startTestKernel3 = time(0);
        for (int i = 1; i < nrows - 2; i++)
            for (int j = 1; j < nrows - 2; j++)
                if (z[i * ncols + j] != NoDataValue)
                    kernel3_outflow_reset(ncols, i, j, So, neighborhood);
        sumOfKernels3ExecutionTimes += time(0) - startTestKernel3;
    }
    std::cout << "\tExecuted " << steps << " steps in " << time(0) - startTest << " seconds" << std::endl;
    std::cout << "\t\tKernel 1 avg time: " << sumOfKernels1ExecutionTimes / steps << " seconds" << std::endl;
    std::cout << "\t\tKernel 2 avg time: " << sumOfKernels2ExecutionTimes / steps << " seconds" << std::endl;
    std::cout << "\t\tKernel 3 avg time: " << sumOfKernels3ExecutionTimes / steps << " seconds" << std::endl;
}

void parallelAlgorithm()
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

		double sumOfKernels1ExecutionTimes = 0, sumOfKernels2ExecutionTimes = 0, sumOfKernels3ExecutionTimes = 0, sumOfCopiesTime = 0;
		time_t startTest = time(0);

		for (int n = steps; n > 0; n--)
		{
			cl::Buffer hB = cl::Buffer(context, H.begin(), H.end(), CL_MEM_READ_WRITE, true);
			cl::Buffer SoB = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(double) * SoSize);
			cl::Buffer zB = cl::Buffer(context, Z.begin(), Z.end(), CL_MEM_READ_ONLY, true);

			cl::CommandQueue queue(context);

			queue.enqueueWriteBuffer(zB, CL_TRUE, 0, sizeof(double) * Z.size(), &Z[0]);
			queue.enqueueWriteBuffer(hB, CL_TRUE, 0, sizeof(double) * H.size(), &H[0]);

			time_t startTestKernel1 = time(0);
			kernel1(
				cl::EnqueueArgs(queue, cl::NDRange(nrows - 2, ncols - 2)),
				nrows,
				ncols,
				0.75,
				VON_NEUMANN_NEIGHBORS,
				NoDataValue,
				zB,
				hB,
				SoB
			);
			sumOfKernels1ExecutionTimes += time(0) - startTestKernel1;

			queue.enqueueReadBuffer(SoB, CL_TRUE, 0, sizeof(double) * SoSize, &lSo[0]);

			time_t startTestKernel2 = time(0);
			kernel2(
				cl::EnqueueArgs(queue, cl::NDRange(nrows - 2, ncols - 2)),
				nrows,
				ncols,
				VON_NEUMANN_NEIGHBORS,
				NoDataValue,
				zB,
				hB,
				SoB
			);
			sumOfKernels2ExecutionTimes += time(0) - startTestKernel2;

			queue.enqueueReadBuffer(hB, CL_TRUE, 0, sizeof(double) * H.size(), &H[0]);

			time_t startTestKernel3 = time(0);
			kernel3(
				cl::EnqueueArgs(queue, cl::NDRange(nrows - 2, ncols - 2)),
				nrows,
				ncols,
				VON_NEUMANN_NEIGHBORS,
				NoDataValue,
				zB,
				SoB
			);
			sumOfKernels3ExecutionTimes += time(0) - startTestKernel3;

			queue.finish();

			time_t copyTime = time(0);

			cl::copy(queue, SoB, lSo.begin(), lSo.end());

			int lSoIndex = 0;
			for (int i = 0; i < VON_NEUMANN_NEIGHBORS; i++) {
				for (int j = 0; j < 5; j++) {
					if (i == 0)
						lSoIndex++;
					else
						So[i][j] = lSo[lSoIndex++];
				}
			}
			sumOfCopiesTime += time(0) - copyTime;
		}
		std::cout << "\tExecuted " << steps << " steps in " << time(0) - startTest << " seconds" << std::endl;
		std::cout << "\t\tKernel 1 avg time: " << sumOfKernels1ExecutionTimes / steps << " seconds" << std::endl;
		std::cout << "\t\tKernel 2 avg time: " << sumOfKernels2ExecutionTimes / steps << " seconds" << std::endl;
		std::cout << "\t\tKernel 3 avg time: " << sumOfKernels3ExecutionTimes / steps << " seconds" << std::endl;
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

void test(double* z, double* sThickness, double* So[], Neighborhood& neighborhood)
{
	std::cout << "Serial execution algorithm" << std::endl;
	serialAlgorithm(z, sThickness, So, neighborhood);

	std::cout << "Parallel execution algorithm" << std::endl;
	parallelAlgorithm();
}