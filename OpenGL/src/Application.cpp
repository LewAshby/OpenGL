#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "globals.h"
#include "openClExecutor.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "ShaderHandler.h"
#include "VertexArray.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "VertexCreation.h"
#include "flow.h"
#include "openClTester.h"
#include "initProgram.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main(int argc, char** argv)
{
	if (initProgram(argc, argv) == -1)
		return 0;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lava FLow", NULL, NULL);
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
		positions = calculateVertices(nrows, ncols, dimension, ZData.values, cellsize, LData.values);
		indices = calculatePositions(nrows, ncols, ZData.values);
		calculateNormal(positions, indices, 13, 10);

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

		ShaderHandler shader("../../../OpenGL/resources/shaders/Basic.shader");
		shader.Bind();

		Texture texture("../../../OpenGL/resources/textures/texture.png");
		texture.Bind();

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

			// per-frame time logic
			// --------------------
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// input
			// -----
			processInput(window);

			/* Render here */
			renderer.Clear();

			// be sure to activate shader when setting uniforms/drawing objects
			shader.Bind();
			shader.setUniformVec3("light.position", lightPos);
			shader.setUniformVec3("viewPos", camera.Position);

			// light properties
			glm::vec3 lightColor = glm::vec3(0.9f);
			glm::vec3 diffuseColor = lightColor * glm::vec3(0.6f); // decrease the influence
			glm::vec3 ambientColor = diffuseColor * glm::vec3(0.4f); // low influence
			shader.setUniformVec3("light.ambient", ambientColor);
			shader.setUniformVec3("light.diffuse", diffuseColor);
			shader.setUniformVec3("light.specular", 1.0f, 1.0f, 1.0f);

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000000000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			shader.setUniformMat4f("projection", projection);
			shader.setUniformMat4f("view", view);

			shader.setUniform1f("lavaMax", lavaMax);

			// world transformation
			glm::mat4 model = glm::mat4(1.0f);
			shader.setUniformMat4f("model", model);

			renderer.Draw(va, ib, shader);

			va.Bind();

			if (steps > 0 && flag == true)
			{
				// lava flow
				if (parallel)
					run(nrows, ncols, VON_NEUMANN_NEIGHBORS, NoDataValue);
				else
					globalTransitionFunction(Z.data(), H.data(), So, dumping_factor, nrows, ncols, neighborhood, NoDataValue);
				
				//resetNormals(positions, nrows * ncols, 13, 10);
				//calculateNormal(positions, indices, 13, 10);				
				updateLava(positions, H, nrows * ncols, 13, 3);

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
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		flag = true;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		flag = false;
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