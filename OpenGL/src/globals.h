#pragma once

#define __CL_ENABLE_EXCEPTIONS

#include "cl.hpp"
#include<vector>
#include "glm/glm.hpp"
#include "camera.h"

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

const int dimension = 3;

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

int nrows;
int ncols;
int cellsize;
float NoDataValue;

float xllcorner = 0;
float yllcorner = 0;

float lavaMax = 0.0;
int steps = 4000;

std::vector<float> positions;
std::vector<unsigned int> indices;

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

float cameraX;
float cameraY;
float cameraZ;
Camera camera;

float lightX;
float lightY;
float lightZ;
glm::vec3 lightPos;

bool flag = false;
bool parallel = false;

cl::Context context;
cl::Program outflow_computation;
cl::Program mass_balance;
cl::Program outflow_reset;

cl::Buffer hB;
cl::Buffer SoB;
cl::Buffer zB;

cl::CommandQueue queue;