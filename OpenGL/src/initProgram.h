#pragma once

#include <iostream>
#include "globals.h"
#include "VertexCreation.h"
#include "openClTester.h"

void initSo(int r, int c, double* M[])
{
	SoSize = nrows * ncols * VON_NEUMANN_NEIGHBORS;
	lSo.resize(SoSize);
	M[0] = NULL;
	for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
	{
		M[n] = (double*)malloc(sizeof(double) * r * c);

		for (int i = 0; i < r; i++)
			for (int j = 0; j < c; j++)
				set(M[n], c, i, j, 0.0);
	}
}

int initProgram(int argc, char** argv)
{
	if (argc != 6)
	{
		std::cout << "\nThe application have to be executed as follows:\n./OpenGL {surfaceZ values path} {surface lava values path} {NumberOfSteps} {test|no} {parallel|no}\n";
		std::cout << "\nUse key 1 and 2 to change from/to polygon mode.";
		std::cout << "\nUse key 3 and 4 to start/stop simulation.\n.";
		return -1;
	}

	ZData = readFile(argv[1]);
	LData = readFile(argv[2]);
	try
	{
		initSo(nrows, ncols, So);
	}
	catch (const std::exception&)
	{
		std::cout << "Wrong file" << std::endl;
		exit(0);
	}

	steps = atoi(argv[3]);

	parallel = strcmp(argv[5], "parallel") == 0;
	if (parallel)
		std::cout << "Running in parallel" << std::endl << std::endl;
	else
		std::cout << "Running in serial" << std::endl << std::endl;

	std::vector<double> Z1(ZData.values.begin(), ZData.values.end());
	std::vector<double> H1(LData.values.begin(), LData.values.end());

	Z.assign(Z1.begin(), Z1.end());
	H.assign(H1.begin(), H1.end());

	if (strcmp(argv[4], "test") == 0) {
		std::cout << "Test with data:\n\tRows: " << nrows << "\n\tColumns: " << ncols << std::endl << std::endl;
		test(Z.data(), H.data(), So, neighborhood);
		return -1;
	}

	std::cout << "Data init and loaded:\n\tRows: " << nrows << "\n\tColumns: " << ncols << std::endl << std::endl;
		
	cameraX = (nrows - 1) * cellsize / 2;
	cameraY = sqrt(nrows * ncols) * cellsize * 1.5;
	cameraZ = -(nrows * cellsize / 2);

	lightX = ncols / 2 * cellsize;
	lightY = sqrt(nrows * ncols) * cellsize * 1.5;
	lightZ = -(nrows / 2 * cellsize);

	camera = Camera(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -40.0f);
	lightPos = glm::vec3(lightX, lightY, lightZ);

	return 0;
}