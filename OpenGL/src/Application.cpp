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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// lighting
//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightPos(0.5f, 0.0f, 2.0f);



const int dimension = 3;

struct FileData
{
	unsigned int ncols;
	unsigned int nrows;
	float cellsize;
	std::vector<float> values;
	int NoDataValue;

};

static std::vector<float> calculateVertices(const unsigned int rows, const unsigned int columns, const unsigned int dimention, 
	const std::vector<float> z, float offset, const std::vector<float> lava)
{
	std::vector<float> positions;
	float min = *min_element(z.begin(), z.end());
	float max = *max_element(z.begin(), z.end());

	int k = 0;
	float r = 0.4f;
	float g = 0.4f;
	float b = 0.4f;
	float a = 1.0f;

	for (unsigned int i = 0; i < rows; i++)
	{
		for (unsigned int j = 0; j < columns; j++)
		{
			positions.push_back(float(j) / float(columns));		// x-point coordinate
			positions.push_back(float(i) / float(rows));		// y-point coordinate
			if (dimention == 3) 
				positions.push_back((z[k]-min) / (max - min));	// z-point coordinate

			
			if (lava[k] > 0)
				r = 1.0f;
			else
				r = 0.4f;
			positions.push_back(r);								// r - color
			positions.push_back(g);								// g - color	
			positions.push_back(b);								// b - color
			positions.push_back(a);								// a - color

			k++;

			positions.push_back(float(j) / float(columns));		// x-texture coordinate
			positions.push_back(float(i) / float(rows));		// y-texture coordinate

			positions.push_back(((double)rand() / (RAND_MAX)));
			positions.push_back(((double)rand() / (RAND_MAX)));
			positions.push_back(((double)rand() / (RAND_MAX)));
		}
	}

	return positions;
}

static std::vector<unsigned int> calculatePositions(const unsigned int rows, const unsigned int columns)
{
	std::vector<unsigned int> positions;

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

	std::ofstream myfile;
	myfile.open("indices.txt");
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
	myfile.close();
}

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
				//data.nrows = data.ncols;
			if (l1 == "cellsize")
				data.cellsize = std::stof(l2);
			if (l1 == "NODATA_value")
			{
				data.NoDataValue = std::stoi(l2);
				flag = 1;
			}
		}
	}

	for (int i = alt_temp.size()-1; i >= 0; i--)
		data.values.push_back(alt_temp[i]);

	return data;
}

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

		FileData ZData = readFile("resources/altitudes.dat");
		FileData LData = readFile("resources/lava.dat");
		FileData TData = readFile("resources/temperature.dat");
		for (int i = 0; i < ZData.values.size(); i++)
			ZData.values[i] += LData.values[i];
		std::vector<float> positions = calculateVertices(ZData.nrows, ZData.ncols, dimension, ZData.values, ZData.cellsize, LData.values);
		std::vector<unsigned int> indices = calculatePositions(ZData.nrows, ZData.ncols);

		std::cout << std::endl;
		std::cout << "Rows: " << ZData.nrows  << std::endl;
		std::cout << "Columns: " << ZData.ncols << std::endl;
		std::cout << std::endl;
		//print(positions, indices, ZData.nrows, ZData.ncols);

		////// blend for transparency-alpha channels //////
		/*GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));*/
		////// blend for transparency-alpha channels //////

		float vertices[] = {
			 0.0f,  0.0f, -0.5f,  0.0f,  0.0f, -1.0f,
			 1.0f,  0.0f, -0.5f,  0.0f,  0.0f, -1.0f,
			 1.0f,  1.0f, -0.5f,  0.0f,  0.0f, -1.0f,
			 1.0f,  1.0f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.0f,  1.0f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.0f,  0.0f, -0.5f,  0.0f,  0.0f, -1.0f,

			 0.0f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
			 1.0f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
			 1.0f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f,
			 1.0f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f,
			 0.0f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f,
			 0.0f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f,

			 0.0f,  1.0f,  0.5f, -1.0f,  0.0f,  0.0f,
			 0.0f,  1.0f, -0.5f, -1.0f,  0.0f,  0.0f,
			 0.0f,  0.0f, -0.5f, -1.0f,  0.0f,  0.0f,
			 0.0f,  0.0f, -0.5f, -1.0f,  0.0f,  0.0f,
			 0.0f,  0.0f,  0.5f, -1.0f,  0.0f,  0.0f,
			 0.0f,  1.0f,  0.5f, -1.0f,  0.0f,  0.0f,

			 1.0f,  1.0f,  0.5f,  1.0f,  0.0f,  0.0f,
			 1.0f,  1.0f, -0.5f,  1.0f,  0.0f,  0.0f,
			 1.0f,  0.0f, -0.5f,  1.0f,  0.0f,  0.0f,
			 1.0f,  0.0f, -0.5f,  1.0f,  0.0f,  0.0f,
			 1.0f,  0.0f,  0.5f,  1.0f,  0.0f,  0.0f,
			 1.0f,  1.0f,  0.5f,  1.0f,  0.0f,  0.0f,

			 0.0f,  0.0f, -0.5f,  0.0f, -1.0f,  0.0f,
			 1.0f,  0.0f, -0.5f,  0.0f, -1.0f,  0.0f,
			 1.0f,  0.0f,  0.5f,  0.0f, -1.0f,  0.0f,
			 1.0f,  0.0f,  0.5f,  0.0f, -1.0f,  0.0f,
			 0.0f,  0.f,  0.5f,  0.0f, -1.0f,  0.0f,
			 0.0f,  0.0f, -0.5f,  0.0f, -1.0f,  0.0f,

			 0.0f,  1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
			 1.0f,  1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
			 1.0f,  1.0f,  0.5f,  0.0f,  1.0f,  0.0f,
			 1.0f,  1.0f,  0.5f,  0.0f,  1.0f,  0.0f,
			 0.0f,  1.0f,  0.5f,  0.0f,  1.0f,  0.0f,
			 0.0f,  1.0f, -0.5f,  0.0f,  1.0f,  0.0f
		};
		unsigned int VBO, cubeVAO;
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(cubeVAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
		unsigned int lightVAO;
		glGenVertexArrays(1, &lightVAO);
		glBindVertexArray(lightVAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// note that we update the lamp's position attribute's stride to reflect the updated buffer data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		VertexArray va;
		VertexBuffer vb(positions.data(), positions.size() * sizeof(float));

		VertexBufferLayout layout;
		layout.Push<float>(dimension);
		layout.Push<float>(4);
		layout.Push<float>(2);
		layout.Push<float>(3);
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices.data(), indices.size() * sizeof(unsigned int));

		ShaderHandler shader("resources/shaders/Basic.shader");
		ShaderHandler light_shader("resources/shaders/Light.shader");
		shader.Bind();
		//shader.setUniform4f("u_Color", 0.4f, 0.4f, 0.4f, 1.0f);

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
			shader.setUniformVec3("lightColor", 1.0f, 1.0f, 1.0f);
			shader.setUniformVec3("lightPos", lightPos);
			shader.setUniformVec3("viewPos", camera.Position);

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			shader.setUniformMat4f("projection", projection);
			shader.setUniformMat4f("view", view);

			// world transformation
			glm::mat4 model = glm::mat4(1.0f);
			shader.setUniformMat4f("model", model);


			//shader.Bind();

			// pass projection matrix to shader (note that in this case it could change every frame)
			shader.setUniformMat4f("projection", projection);

			// camera/view transformation
			shader.setUniformMat4f("view", view);


			// also draw the lamp object
			light_shader.Bind();
			light_shader.setUniformMat4f("projection", projection);
			light_shader.setUniformMat4f("view", view);
			model = glm::mat4(1.0f);
			model = glm::translate(model, lightPos);
			model = glm::scale(model, glm::vec3(0.3f)); // a smaller cube
			light_shader.setUniformMat4f("model", model);

			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			

			renderer.Draw(va, ib, shader);

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
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
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