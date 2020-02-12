#pragma once

#include<vector>

struct FileData
{
	unsigned int ncols;
	unsigned int nrows;
	float xllcorner;
	float yllcorner;
	float cellsize;
	std::vector<float> values;
	float NoDataValue;

};

struct CellCoordinates {
	int i;
	int j;
};

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

unsigned int nrows;
unsigned int ncols;
unsigned int cellsize;
float NoDataValue;

float xllcorner = 0;
float yllcorner = 0;

float lavaMax = 0.0;
int steps = 4000;

FileData ZData;
FileData LData;

float dumping_factor = 0.75;
const int VON_NEUMANN_NEIGHBORS = 5;
double* So[VON_NEUMANN_NEIGHBORS];
int SoSize;
std::vector<double> lSo;
std::vector<double> Z;
std::vector<double> H;

using Neighborhood = CellCoordinates[VON_NEUMANN_NEIGHBORS];

Neighborhood neighborhood;
