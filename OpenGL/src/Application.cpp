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

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "VertexCreation.h"
#include "flow.h"

const int dimension = 3;

std::vector<std::vector<double>> So;
std::vector<double*> ptrsSo;
double* SoNew[5];
Neighborhood neighborhood;

void init(std::vector<std::vector<double>>& So, unsigned int r, unsigned int c)
{
	std::vector<double> t;
	So.push_back(t);
	for (int n = 1; n < 5; n++)
	{
		So.push_back(t);
		for (int i = 0; i < r; i++)
		{
			for (int j = 0; j < c; j++)
			{
				So[n].push_back(0.0);
			}
		}
	}
	for (auto& vec : So)
	{
		ptrsSo.push_back(vec.data());
	}
}

void init1(int r, int c, double* M[])
{
	M[0] = NULL;
	for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
	{
		M[n] = (double*)malloc(sizeof(double) * r * c);

		for (int i = 0; i < r; i++)
			for (int j = 0; j < c; j++)
				set(M[n], c, i, j, 0.0);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

FileData ZData = readFile("resources/altitudes.dat");
FileData LData = readFile("resources/lava.dat");

// camera
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float camX = ZData.xllcorner + (ZData.nrows - 1) * ZData.cellsize / 2;
float camY = sqrt(ZData.nrows * ZData.ncols) * ZData.cellsize * 1.5;
float camZ = ZData.yllcorner + ZData.nrows * ZData.cellsize / 2;
Camera camera(glm::vec3(camX, camY, camZ), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -40.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// lighting
///glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
float lightX = ZData.xllcorner + ZData.ncols / 2 * ZData.cellsize;
float lightY = sqrt(ZData.nrows * ZData.ncols) * ZData.cellsize * 1.5;
float lightZ = ZData.yllcorner + ZData.nrows / 2 * ZData.cellsize;
glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);


std::vector<float> positions;
std::vector<unsigned int> indices;
std::vector<double>Z1(ZData.values.begin(), ZData.values.end());
std::vector<double>L1(LData.values.begin(), LData.values.end());


int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Flow FLow", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;

	{
		positions = calculateVertices(ZData.nrows, ZData.ncols, dimension, ZData.values, ZData.cellsize, LData.values);
		indices = calculatePositions(ZData.nrows, ZData.ncols, ZData.values);
		calculateNormal(positions, indices, 13, 10);
		//init(So, ZData.nrows, ZData.ncols);
		init1(ZData.nrows, ZData.ncols, SoNew);

		std::cout << std::endl;
		std::cout << "Rows: " << ZData.nrows << std::endl;
		std::cout << "Columns: " << ZData.ncols << std::endl;
		std::cout << std::endl;

		VertexArray va;
		VertexBuffer vb(positions.data(), positions.size() * sizeof(float));

		VertexBufferLayout layout;
		layout.Push<float>(dimension);	//dimension
		layout.Push<float>(1);			//lava altitude
		layout.Push<float>(4);			//color
		layout.Push<float>(2);			//texture
		layout.Push<float>(3);			//normal
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices.data(), indices.size() * sizeof(unsigned int));

		ShaderHandler shader("resources/shaders/Basic.shader");
		shader.Bind();

		Texture texture("resources/textures/texture.png");
		texture.Bind();

		va.Unbind();
		vb.Unbind();
		ib.Unbind();
		shader.Unbind();

		Renderer renderer;

		float r = 0.0f;
		float increment = 0.5f;
		/* Loop until the user closes the window */

		int steps = 1000;
		while (!glfwWindowShouldClose(window))
		{

			// per-frame time logic
			// --------------------
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// input
			// -----
			processInput(window);

			/* Render here */
			//renderer.Clear();

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// be sure to activate shader when setting uniforms/drawing objects
			shader.Bind();
			shader.setUniformVec3("light.position", lightPos);
			shader.setUniformVec3("viewPos", camera.Position);

			// light properties
			glm::vec3 lightColor = glm::vec3(0.5f);
			glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
			glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
			shader.setUniformVec3("light.ambient", ambientColor);
			shader.setUniformVec3("light.diffuse", diffuseColor);
			shader.setUniformVec3("light.specular", 1.0f, 1.0f, 1.0f);

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000000000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			shader.setUniformMat4f("projection", projection);
			shader.setUniformMat4f("view", view);

			// world transformation
			glm::mat4 model = glm::mat4(1.0f);
			shader.setUniformMat4f("model", model);

			renderer.Draw(va, ib, shader);

			va.Bind();

			if (steps > 0)
			{
				// lava flow
				
				globalTransitionFunction(Z1.data(), L1.data(), SoNew, 0.75, ZData.nrows, ZData.ncols, neighborhood, ZData.NoDataValue);
				resetNormals(positions, ZData.nrows * ZData.ncols, 13, 10);
				calculateNormal(positions, indices, 13, 10);				
				updateLava(positions, L1, ZData.nrows * ZData.ncols, 13, 3);

				steps--;

			}
			vb.UpdateData(positions.data(), positions.size() * sizeof(float));

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

	}
	glfwTerminate();
	return 0;
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime * 1000);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime * 1000);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime * 1000);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime * 1000);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}