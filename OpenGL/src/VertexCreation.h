#pragma once

#include "globals.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

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

static FileData readFile(std::string path)
{
	std::ifstream infileAlt(path);
	FileData data;
	std::vector<float> alt_temp;
	std::string line, l1, l2, temp;
	int flag = 0;

	while (std::getline(infileAlt, line))
	{
		if (flag == 1)
		{
			std::stringstream ss(line);
			while (ss >> temp)
				data.values.push_back(std::stof(temp));
			for (int i = data.values.size() - 1; i >= 0; i--)
				alt_temp.push_back(data.values[i]);
			data.values.clear();
		}
		else
		{
			std::stringstream ss(line);
			ss >> l1 >> l2;
			if (l1 == "ncols")
				data.ncols = std::stoi(l2);
			if (l1 == "nrows")
				data.nrows = std::stoi(l2);
			if (l1 == "xllcorner")
				data.xllcorner = 0.0f;
			if (l1 == "yllcorner")
				data.yllcorner = 0.0f;
			if (l1 == "cellsize")
				data.cellsize = std::stof(l2);
			if (l1 == "NODATA_value")
			{
				data.NoDataValue = std::stof(l2);
				flag = 1;
			}
		}
	}
	for (int i = alt_temp.size() - 1; i >= 0; i--)
		data.values.push_back(alt_temp[i]);

	nrows = data.nrows;
	ncols = data.ncols;
	cellsize = data.cellsize;
	NoDataValue = data.NoDataValue;

	return data;
}

static std::vector<float> calculateVertices(const unsigned int rows, const unsigned int columns, const unsigned int dimention,
	const std::vector<float> z, float offset, const std::vector<float> lava)
{
	std::vector<float> positions;

	int k = 0;
	float r = 0.5f;
	float g = 0.5f;
	float b = 0.5f;
	float a = 1.0f;

	for (unsigned int i = 0; i < rows; i++)
	{
		for (unsigned int j = 0; j < columns; j++)
		{
			positions.push_back(i * offset);		// x-point coordinate
			if (dimention == 3)
				positions.push_back(z[k]);			// z-point coordinate
			positions.push_back(j * offset);		// y-point coordinate

			positions.push_back(lava[k]);			// lava altitude

			positions.push_back(r);								// r - color
			positions.push_back(g);								// g - color	
			positions.push_back(b);								// b - color
			positions.push_back(a);								// a - color

			positions.push_back(float(j) / float(columns));		// x-texture coordinate
			positions.push_back(float(i) / float(rows));		// y-texture coordinate

			positions.push_back(0);								// x-normal
			positions.push_back(0);								// y-normal
			positions.push_back(0);								// z-normal

			k++;
		}
	}

	return positions;
}

static std::vector<unsigned int> calculatePositions(const unsigned int rows, const unsigned int columns, const std::vector<float> z)
{
	std::vector<unsigned int> positions;

	int h = 0;

	for (unsigned int i = 0; i < rows - 1; i++)
	{
		for (unsigned int j = 0; j < columns - 1; j++)
		{
			positions.push_back(i * columns + j);
			positions.push_back(i * columns + j + 1);
			positions.push_back((i + 1) * columns + j + 1);
			positions.push_back((i + 1) * columns + j + 1);
			positions.push_back((i + 1) * columns + j);
			positions.push_back(i * columns + j);
		}
	}

	return positions;
}

static void print(std::vector<float> positions, std::vector<unsigned int> indices, int rows, int columns)
{
	std::cout << "Vertex positions: " << std::endl;
	for (int i = 0; i < positions.size(); i += 5)
	{
		std::cout << std::fixed;
		std::cout << std::setprecision(6);
		std::cout << float(positions[i]) << "f, ";
		std::cout << float(positions[i + 1]) << "f, ";
		std::cout << float(positions[i + 2]) << "f, ";
		std::cout << float(positions[i + 3]) << "f, ";
		std::cout << float(positions[i + 4]) << "f, ";
		std::cout << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Vertex indices: " << std::endl;
	for (int i = 0; i < indices.size(); i += 3)
	{
		std::cout << std::fixed;
		std::cout << std::setprecision(6);
		std::cout << indices[i] << ", ";
		std::cout << indices[i + 1] << ", ";
		std::cout << indices[i + 2] << ", ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

static void calculateNormal(std::vector<float>& vertices, std::vector<unsigned int> indices, unsigned int vSize, unsigned int offset)
{
	glm::vec3 vec1, vec2, norm;

	for (int i = 0; i < indices.size(); i += 3)
	{
		unsigned int tVert1 = indices[i];
		unsigned int tVert2 = indices[i+1];
		unsigned int tVert3 = indices[i+2];

		float tVert1_X = vertices[tVert1 * vSize];
		float tVert1_Y = vertices[tVert1 * vSize + 1];
		float tVert1_Z = vertices[tVert1 * vSize + 2];

		float tVert2_X = vertices[tVert2 * vSize];
		float tVert2_Y = vertices[tVert2 * vSize + 1];
		float tVert2_Z = vertices[tVert2 * vSize + 2];

		float tVert3_X = vertices[tVert3 * vSize];
		float tVert3_Y = vertices[tVert3 * vSize + 1];
		float tVert3_Z = vertices[tVert3 * vSize + 2];

		vec1.x = tVert1_X - tVert2_X;
		vec1.y = tVert1_Y - tVert2_Y;
		vec1.z = tVert1_Z - tVert2_Z;

		vec2.x = tVert3_X - tVert2_X;
		vec2.y = tVert3_Y - tVert2_Y;
		vec2.z = tVert3_Z - tVert2_Z;

		norm = glm::cross(vec1, vec2);

		for (int i = 0; i < 3; i++)
		{
			vertices[tVert1 * vSize + offset] = norm.x;
			vertices[tVert1 * vSize + offset + 1] = norm.y;
			vertices[tVert1 * vSize + offset + 2] = norm.z;
		}
	}
}

static void resetNormals(std::vector<float>& vertices, unsigned int n, unsigned int vSize, unsigned int offset)
{
	for (int i = 0; i < n; i++)
	{
		vertices[i * vSize + offset] = 0.0f;
		vertices[i * vSize + offset + 1] = 0.0f;
		vertices[i * vSize + offset + 2] = 0.0f;

	}
}

static void updateLava(std::vector<float>& vertices, std::vector<double> lava, unsigned int n, unsigned int vSize, unsigned int offset)
{
	for (int i = 0; i < n; i++)
		if (lava[i] > 0 && lava[i] < 15)
			vertices[i * vSize + offset] = float(lava[i]);
}