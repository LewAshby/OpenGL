#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "ShaderHandler.h"
#include "VertexArray.h"
#include "Texture.h"

const int dimension = 3;

struct FileData
{
	unsigned int ncols;
	unsigned int nrows;
	float cellsize;
	std::vector<float> altitudes;
	int NoDataValue;

};

static std::vector<float> calculateVertices(const unsigned int rows, const unsigned int columns, const unsigned int dimention, const std::vector<float> z)
{
	std::vector<float> positions;
	float min = *min_element(z.begin(), z.end());
	float max = *max_element(z.begin(), z.end());

	for (unsigned int i = 0; i < columns; i++)
	{
		for (unsigned int j = 0; j < rows; j++)
		{
			positions.push_back(float(i) / float(rows));
			positions.push_back(float(j) / float(columns));
			if (dimention == 3)
				positions.push_back((z[i+j]-min)/(max-min));
			positions.push_back(float(i) / float(rows));
			positions.push_back(float(j) / float(columns));
		}
	}

	return positions;
}

static std::vector<unsigned int> calculatePositions(const unsigned int rows, const unsigned int columns)
{
	std::vector<unsigned int> positions;

	for (unsigned int i = 0; i < columns - 1; i++)
	{
		for (unsigned int j = 0; j < rows - 1; j++)
		{

			positions.push_back(i * rows + j);
			positions.push_back(i * rows + j + 1);
			positions.push_back((i + 1) * rows + j + 1);
			positions.push_back((i + 1) * rows + j + 1);
			positions.push_back((i + 1) * rows + j);
			positions.push_back(i * rows + j);
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

static FileData readFile(std::string path)
{
	std::ifstream infile(path);
	FileData data;
	std::string line, l1, l2, temp;
	int flag = 0;
	
	while (std::getline(infile, line))
	{
		if (flag == 1)
		{
			std::stringstream ss(line);
			while (ss >> temp)
				data.altitudes.push_back(std::stof(temp));
		}
		else
		{
			std::stringstream ss(line);
			ss >> l1 >> l2;
			if (l1 == "ncols")
				data.ncols = std::stoi(l2);
			if (l1 == "nrows")
				data.nrows = std::stoi(l2);
			if (l1 == "cellsize")
				data.cellsize = std::stof(l2);
			if (l1 == "NODATA_value")
			{
				data.NoDataValue = std::stoi(l2);
				flag = 1;
			}
		}
	}
	return data;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;

	{

		FileData ZData = readFile("resources/test.dat");
		std::vector<float> positions = calculateVertices(ZData.nrows, ZData.ncols, dimension, ZData.altitudes);
		std::vector<unsigned int> indices = calculatePositions(ZData.nrows, ZData.ncols);

		std::cout << std::endl;
		std::cout << "Rows: " << ZData.nrows  << std::endl;
		std::cout << "Columns: " << ZData.ncols << std::endl;
		std::cout << std::endl;
		//print(positions, indices, ZData.nrows, ZData.ncols);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		VertexArray va;
		VertexBuffer vb(positions.data(), positions.size() * sizeof(float));

		VertexBufferLayout layout;
		layout.Push<float>(dimension);
		layout.Push<float>(2);
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices.data(), indices.size() * sizeof(unsigned int));

		ShaderHandler shader("resources/shaders/Basic.shader");
		shader.Bind();
		shader.setUniform4f("u_Color", 0.0f, 1.0f, 0.0f, 1.0f);

		Texture texture("resources/textures/texture.png");
		texture.Bind();
		shader.setUniform1i("u_Texture", 0);

		va.Unbind();
		vb.Unbind();
		ib.Unbind();
		shader.Unbind();

		Renderer renderer;

		float r = 0.0f;
		float increment = 0.5f;
		/* Loop until the user closes the window */

		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			renderer.Clear();

			shader.Bind();
			shader.setUniform4f("u_Color", r, 1.0f, 0.0f, 1.0f);

			renderer.Draw(va, ib, shader);

			/*if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;*/

			r += increment;

			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

	}
    glfwTerminate();
    return 0;
}