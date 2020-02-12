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
		std::cout << "\nThe application have to be executed as follows:\n./OpenGL {surfaceZPath} {surfaceHeightPath} {NumberOfSteps} {test|no} {parallel|no}\n";
		return -1;
	}

	ZData = readFile("resources/altitudes.dat");
	LData = readFile("resources/lava.dat");

	initSo(nrows, ncols, So);

	std::vector<double> Z1(ZData.values.begin(), ZData.values.end());
	std::vector<double> H1(LData.values.begin(), LData.values.end());

	Z.assign(Z1.begin(), Z1.end());
	H.assign(H1.begin(), H1.end());

	if (strcmp(argv[4], "test") == 0) {
		std::cout << "Test with data:\n\tRows: " << nrows << "\n\tColumns: " << ncols <<
			"\n\txCellconer:" << xllcorner << "\n\tyCellcorner:" << yllcorner <<
			"\n\tcellSize:" << cellsize << "\n\tnoDataValue:" << NoDataValue << std::endl << std::endl;
		test(Z.data(), H.data(), So, neighborhood);
		return -1;
	}

	std::cout << "Data init and loaded:\n\tRows: " << nrows << "\n\tColumns: " << ncols <<
		"\n\txCellconer:" << xllcorner << "\n\tyCellcorner:" << yllcorner <<
		"\n\tcellSize:" << cellsize << "\n\tnoDataValue:" << NoDataValue << std::endl << std::endl;
		

	return 0;
}