#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#include <vector>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "ShaderHandler.h"
#include "VertexArray.h"

const int dimension = 3;


static std::vector<float> calculateVertices(const unsigned int rows, const unsigned int columns, const unsigned int dimention)
{
	std::vector<float> positions;
	
	for (unsigned int i = 0; i < columns; i++)
	{
		for (unsigned int j = 0; j < rows; j++)
		{
			positions.push_back(float(i) / float(rows));
			positions.push_back(float(j) / float(columns));
			if (dimention == 3)
				positions.push_back(0);
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
	for (int i = 0; i < positions.size(); i += 3)
	{
		std::cout << std::fixed;
		std::cout << std::setprecision(6);
		std::cout << float(positions[i]) << "f, ";
		std::cout << float(positions[i + 1]) << "f, ";
		std::cout << float(positions[i + 2]) << "f, ";
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

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
		/*float positions[] = {

			0.000000f, 0.000000f, 0.000000f,
			0.000000f, 0.333333f, 0.000000f,
			0.000000f, 0.666667f, 0.000000f,
			0.333333f, 0.000000f, 0.000000f,
			0.333333f, 0.333333f, 0.000000f,
			0.333333f, 0.666667f, 0.000000f,
			0.666667f, 0.000000f, 0.000000f,
			0.666667f, 0.333333f, 0.000000f,
			0.666667f, 0.666667f, 0.000000f,
		};

		unsigned int indices[] = {

			0, 1, 4,
			4, 3, 0,
			1, 2, 5,
			5, 4, 1,
			3, 4, 7,
			7, 6, 3,
			4, 5, 8,
			8, 7, 4,

		};*/

		const unsigned int rows = 50;
		const unsigned int columns = 50;
		std::vector<float> positions = calculateVertices(rows, columns, dimension);
		std::vector<unsigned int> indices = calculatePositions(rows, columns);

		std::cout << std::endl;
		std::cout << "Rows: " << rows  << std::endl;
		std::cout << "Columns: " << columns << std::endl;
		std::cout << std::endl;
		//print(positions, indices, rows, columns);
		

		VertexArray va;
		VertexBuffer vb(positions.data(), positions.size() * sizeof(float));

		VertexBufferLayout layout;
		layout.Push<float>(dimension);
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices.data(), indices.size() * sizeof(unsigned int));

		ShaderHandler shader("resources/shaders/Basic.shader");
		shader.Bind();
		shader.setUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

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
			shader.setUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

			renderer.Draw(va, ib, shader);

			if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;

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