// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

#include <common/shader.hpp>

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "IK sample code", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	bool targetSet = false;
	float targetX = 0, targetY = 0;

	// Mid blue-grey background
	glClearColor(0.2f, 0.2f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 220.0f);
	//For mouse interaction, it is simpler to work with an orthographic projection
	glm::mat4 Projection = glm::ortho(-60.0f, 60.0f, -60.0f, 60.0f, 0.0f, 120.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, 100), // Camera is at (4,3,-3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// Our vertices. 
	// Simple four triangle representation of a bone
	static const GLfloat g_vertex_buffer_data[] = {
		 .0f, .0f, .0f,
		 .3f, .3f, .0f,
		 .3f,  0.0f, .0f,
		 .3f,  0.0f, .0f,
		 .3f,  0.3f, .0f,
		 1.0f, .0f, .0f,
		 .0f, .0f, .0f,
		 .3f, -.3f, .0f,
		 .3f,  0.0f, .0f,
		 .3f,  0.0f, .0f,
		 .3f,  -0.3f, .0f,
		 1.0f, .0f, .0f
	};

	// Color the triangles red and white.
	static const GLfloat g_color_buffer_data[] = {
		1.0f,  0.f,  0.f,
		1.0f,  0.f,  0.f,
		1.0f,  0.f,  0.f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  0.f,  0.f,
		1.0f,  0.f,  0.f,
		1.0f,  0.f,  0.f
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);


	float worldTargetX = 0, worldTargetY = 0;

	//**************************************************************************
	//**************************************************************************
	//This is the data that defines and configures the skeleton.  You can assume
	//that it starts in a horizontal line to the right, from 0,0
	//initialize the skeleton data.
	//You'll want to set the values in the theta vector using your IK algorithm
	const float lengths[] = { 7, 13.5, 11, 3.5, 4 };
	int numBones = 5;
	//The theta vector holds the joint angles for the skeleton
	//I suggest testing that you have the rotations set up properly with some hard coded test values first.
	float theta[] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
	//float theta[] = {0.9, -1.57,  1.57, 0.2, -0.30};

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





		//Detect when the user clicks to specify a target location (just taking button down)
		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if (state == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			//fprintf(stderr, "x %lf, y %lf\n", xpos, ypos);
			targetX = xpos;
			targetY = ypos;
			targetSet = true;

			//This should be the location you actually want to use in your IK algorithm
			worldTargetX = (targetX / 1024 - 0.5) * 120;
			worldTargetY = (-targetY / 768 + 0.5) * 120;
		}


		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 scale, rot;
		glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0.0f));
		glm::vec3 rotAxis = glm::vec3(0.0f, 0.0f, 1.0f);

		// Model matrix : an identity matrix (model will be at the origin)
		Model = glm::mat4(1.0f);
		MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around


		//draw the target
		if (targetSet)
		{
			scale = glm::scale(glm::mat4(), glm::vec3(1.f, 10 / 6.0f, 1.0f));
			trans = glm::translate(glm::mat4(), glm::vec3(worldTargetX - .5, worldTargetY, 0.0f));//The .5 is to center the marker on the clicked location

			Model = trans * scale;
			MVP = Projection * View * Model;


			// Use our shader
			glUseProgram(programID);

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : colors
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, 4 * 3); // 12*3 indices starting at 0 -> 12 triangles

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);


		}


		Model = glm::mat4(1.0f);
		trans = glm::translate(glm::mat4(), glm::vec3(0.0f));
		scale = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));

		for (int i = 0; i < numBones; i++)
		{

			if (i > 0)
			{
				scale = glm::scale(glm::mat4(), glm::vec3(1 / lengths[i - 1], 1.0 / 2.f, 1.0f));
				Model = Model * scale;
			}
			if (i > 0)
			{
				trans = glm::translate(glm::mat4(), glm::vec3(lengths[i - 1], 0.f, 0.0f));
			}
			//Note that this rotation is in radians
			rot = glm::rotate(theta[i], rotAxis);

			scale = glm::scale(glm::mat4(), glm::vec3(lengths[i], 2.0f, 1.0f));

			//*********************************************************************************************
			//NOTE: I've removed the rotation from the model matrix.  You need to figure out where to add it
			Model = Model * trans * scale;
			//Model = Model * trans * rot * scale;
			MVP = Projection * View * Model;


			// Use our shader
			glUseProgram(programID);

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : colors
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, 4 * 3); // 12*3 indices starting at 0 -> 12 triangles

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);



		}



		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

