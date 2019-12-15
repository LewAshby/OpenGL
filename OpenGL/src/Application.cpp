#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderHandler.h"


static void calculateVertices(float verticesPositions[], unsigned int rows, unsigned int columns)
{
	for (unsigned int i = 0; i < rows*columns*3; i+=3)
	{
		verticesPositions[i] = i / rows;
		verticesPositions[i + 1] = i / columns;
		verticesPositions[i + 2] = 0;	//z coordinate
	}
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
		float positions[] = {
			-0.5f, -0.5f,
			 0.5f, -0.5f,
			 0.5f, 0.5f,
			-0.5f, 0.5f

			//-0.5f, 0.5f, 0.0f,	//0
			//0.0f, 0.5f, 0.0f,	//1
			//0.5f, 0.5f, 0.0f,	//2
			//-0.5f, -0.5f, 0.0f,	//3
			//0.0f, -0.5f, 0.0f,	//4
			//0.5f, -0.5f, 0.0f,	//5	
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0,
			/*0, 3, 1,
			3, 1, 4,
			1, 4, 2,
			4, 2, 5*/
		};

		unsigned int rows = 2;
		unsigned int columns = 2;
		float testVertices[2*2*3];
		//calculateVertices(testVertices, rows, columns);
		for (float i = 0; i < rows * columns * 3; i+=3)
		{
			for (int j = 0; j < columns; j++)
			{

			}
			std::cout << float(i/3/rows) << " ";
			std::cout << float(i/3/columns) << " ";
			std::cout << "0" << " ";
			if (int(i) % 3 == 0)
				std::cout << std::endl;
		}

		unsigned int vao;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		VertexBuffer vb(positions, sizeof(positions) * 2 * sizeof(float));

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

		IndexBuffer ib(indices, sizeof(indices)/sizeof(unsigned int));

		ShaderHandler shader("resources/shaders/Basic.shader");
		shader.Bind();
		shader.setUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

		vb.Unbind();
		ib.Unbind();
		shader.Unbind();

		float r = 0.0f;
		float increment = 0.5f;
		/* Loop until the user closes the window */

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT);

			shader.Bind();
			shader.setUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

			GLCall(glBindVertexArray(vao));
			ib.Bind();

			GLCall(glDrawElements(GL_TRIANGLES, sizeof(indices)*6, GL_UNSIGNED_INT, nullptr));

			/*if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;

			r += increment;*/

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

	}
    glfwTerminate();
    return 0;
}