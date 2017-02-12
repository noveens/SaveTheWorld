#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
    float x;
    float y;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

void glutBitmapCharacter(void *font, int character);
void glRasterPos2f(float x, float y);

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
				VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
					0,                  // attribute 0. Vertices
					3,                  // size (x,y,z)
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
					);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
					1,                  // attribute 1. Color
					3,                  // size (r,g,b)
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
					);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


/***********************
 * Global Declarations *
 ***********************/


float x_global_one = 0.0, x_global_two = 0.0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

int FLAG = 0;
int fl1 = 0;
int fl2 = 0;
int fl3 = 0;
int fl4 = 0;
int fl22 = 0;
int time_fl4 = 0;
int done = 1;
int mouse_press = 0;
int mouse_press_right = 0;
int a_good_name = 0;
double prevMouse_x = 0;
double prevMouse_y = 0;
float angle = 0;
float y_global_lazer = 0.0;
double mouse_x,mouse_y;
double laser_time = 0.5;
int laser_time_dup = 5;
int laser_time_flag = 0;
int score = 0;
int score_flag = 1;
int out_of_names = 0;
int teemppp[100][10];
int faster = 0;
double maxCoord = 100, xCap = 0, yCap = 0;

VAO *battery_semi[1000];
VAO *ScoreRect;
VAO *battery_out;
VAO *trap1, *trap2;
VAO *lazer_rect, *lazer_circle[400], *lazer_end;
set < pair< VAO *, int > > to_fall_blocks;
set < pair< pair< VAO *, float >, int > > fell_blocks, fade, fade_temp;
set < VAO * > faded_t;
set < pair< VAO *, int> > mirror;
set < pair< VAO *, vector<float> > > lazer;
set < VAO *> score_rect;
set < VAO *> time_rect;
set < VAO *> template_rect;
set < VAO *> testing;
map < char, int * > map_alpha;


/**************************
 * Customizable functions *
 **************************/

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset==-1)
		maxCoord+=5;
	else if(yoffset==1)
		maxCoord-=5;
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) 
    {
        switch (key) 
        {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_D:
                fl1 = 0;
                break;
            case GLFW_KEY_A:
                fl1 = 0;
                break;
            case GLFW_KEY_LEFT:
                fl2 = 0;
                break;
            case GLFW_KEY_RIGHT:
                fl2 = 0;
                break;
            case GLFW_KEY_UP:
                fl22 = 0;
                break;
            case GLFW_KEY_DOWN:
                fl22 = 0;
                break;
            case GLFW_KEY_W:
                fl3 = 0;
                break;
            case GLFW_KEY_S:
                fl3 = 0;
                break;
            case GLFW_KEY_SPACE:
                fl4 = 0;
                break;
            default:
                break;
        }
    }

    if (action == GLFW_PRESS) 
    {
        switch (key) 
        {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_D:
                fl1 = 2;
                break;
            case GLFW_KEY_A:
                fl1 = 1;
                break;
            case GLFW_KEY_RIGHT:
                fl2 = 2;
                break;
            case GLFW_KEY_LEFT:
                fl2 = 1;
                break;
             case GLFW_KEY_UP:
                fl22 = 2;
                break;
            case GLFW_KEY_DOWN:
                fl22 = 1;
                break;
            case GLFW_KEY_W:
                fl3 = 1;
                break;
            case GLFW_KEY_S:
                fl3 = 2;
                break;
            case GLFW_KEY_SPACE:
                fl4 = 1;
                break;
            case GLFW_KEY_M:
                faster++;
                break;
            case GLFW_KEY_N:
                faster--;
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) 
    {
        case 'Q':
            break;
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) 
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {
                mouse_press = 0;
            }
            else if (action == GLFW_PRESS) 
            {
                mouse_press = 1;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
        	if (action == GLFW_RELEASE)
            {
                mouse_press_right = 0;
            }
            else if (action == GLFW_PRESS) 
            {
                mouse_press_right = 1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-maxCoord + xCap, maxCoord + xCap + 80.0, -maxCoord + yCap, maxCoord + yCap, (double)0.1, (double)500.0);
}

void CreateMirror (int x, int y, int aangle)
{
	const GLfloat vertex_buffer_data [] = {
		x, y, 0, // vertex 1
		x + 2, y, 0, // vertex 2
		x + 2, y - 30, 0, // vertex 3

		x + 2, y - 30, 0, // vertex 2
		x, y - 30, 0, // vertex 3
		x, y, 0,  // vertex 4

		x + 2, y, 0,
		x + 3, y, 0,
		x + 3, y - 30, 0,

		x + 2, y, 0,
		x + 2, y - 30, 0,
		x + 3, y - 30
	};

	const GLfloat color_buffer_data [] = {
		0.53, 0.8, 0.98, // color 1
		0.53, 0.8, 0.98, // color 2
		0.53, 0.8, 0.98, // color 3

		0.53, 0.8, 0.98, // color 3
		0.53, 0.8, 0.98, // color 4
		0.53, 0.8, 0.98, // color 1

		0.3, 0.3, 0.3, // color 1
		0.3, 0.3, 0.3, // color 2
		0.3, 0.3, 0.3, // color 3

		0.3, 0.3, 0.3, // color 3
		0.3, 0.3, 0.3, // color 4
		0.3, 0.3, 0.3  // color 1
	};

	VAO * temp1 = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
	temp1->x = x;
	temp1->y = y;

	mirror.insert(make_pair(temp1, aangle));

}

void CreateBlock (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int ty)
{
	const GLfloat vertex_buffer_data [] = {
		x1, y1, 0, // vertex 1
		x2, y2, 0, // vertex 2
		x3, y3, 0, // vertex 3

		x2, y2, 0, // vertex 2
		x3, y3, 0, // vertex 3
		x4, y4, 0  // vertex 4
	};

	if(ty == 1)
	{
		const GLfloat color_buffer_data [] = {
			0.3, 0.3, 0.3, // color 1
			0.3, 0.3, 0.3, // color 2
			0.3, 0.3, 0.3, // color 3

			0.3, 0.3, 0.3, // color 3
			0.3, 0.3, 0.3, // color 4
			0.3, 0.3, 0.3  // color 1
		};

        VAO * temp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
        temp->x = x1;
        temp->y = y1;

		to_fall_blocks.insert(make_pair(temp, ty));
	}
	else if(ty == 2)
	{
		const GLfloat color_buffer_data [] = {
			0.5, 0.5, 0.5, // color 1
			0.5, 0.5, 0.5, // color 2
			0.5, 0.5, 0.5, // color 5

			0.5, 0.5, 0.5, // color 5
			0.5, 0.5, 0.5, // color 4
			0.5, 0.5, 0.5  // color 1
		};

        VAO * temp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
        temp->x = x1;
        temp->y = y1;

		to_fall_blocks.insert(make_pair(temp, ty));
	}

	else if(ty == -1)
	{
		const GLfloat color_buffer_data [] = {
			0.5, 0.5, 0.5, // color 1
			0.5, 0.5, 0.5, // color 2
			0.5, 0.5, 0.5, // color 5

			0.5, 0.5, 0.5, // color 5
			0.5, 0.5, 0.5, // color 4
			0.5, 0.5, 0.5  // color 1
		};

        VAO * temp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
        temp->x = x1;
        temp->y = y1;

		to_fall_blocks.insert(make_pair(temp, ty));
	}

	else
	{
		const GLfloat color_buffer_data [] = {
			1.0, 0.0, 0.0, // color 1
			1.0, 0.0, 0.0, // color 2
			1.0, 0.0, 0.0, // color 0

			1.0, 0.0, 0.0, // color 0
			1.0, 0.0, 0.0, // color 4
			1.0, 0.0, 0.0  // color 1
		};

        VAO * temp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
        temp->x = x1;
        temp->y = y1;

		to_fall_blocks.insert(make_pair(temp, ty));	
	}
}

void create_test(float x, float y, float col)
{
	const GLfloat vertex_buffer_data [] = {
        x, y , 0, // vertex 1
        x+1, y , 0, // vertex 2
        x+1, y - 1, 0, // vertex 3

        x, y , 0, // vertex 2
        x, y - 1, 0, // vertex 3
        x + 1, y - 1, 0  // vertex 4
    };

    const GLfloat color_buffer_data [] = {
        col, col, col, // color 1
        col, col, col, // color 2
        col, col, col, // color 3

        col, col, col, // color 3
        col, col, col, // color 4
        col, col, col // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    testing.insert(create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL));
}

void create_faded_block (float x, float y, float col)
{
    const GLfloat vertex_buffer_data [] = {
        x, y , 0, // vertex 1
        x+3, y , 0, // vertex 2
        x+3, y - 15, 0, // vertex 3

        x+3, y - 15 , 0, // vertex 2
        x, y - 15, 0, // vertex 3
        x, y, 0  // vertex 4
    };

    const GLfloat color_buffer_data [] = {
        col, col, col, // color 1
        col, col, col, // color 2
        col, col, col, // color 3

        col, col, col, // color 3
        col, col, col, // color 4
        col, col, col // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    faded_t.insert(create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL));
}

void CreateScore ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		100, 100 , 0, // vertex 1
		180, 100 , 0, // vertex 2
		180, -100, 0, // vertex 3

		180, -100 , 0, // vertex 2
		100, -100, 0, // vertex 3
		100, 100, 0  // vertex 4
	};

	const GLfloat color_buffer_data [] = {
		0.227, 0.153, 0.323, // color 1
		0.227, 0.153, 0.323, // color 2
		0.227, 0.153, 0.323, // color 1

		0.227, 0.153, 0.323, // color 1
		0.227, 0.153, 0.323, // color 4
		0.227, 0.153, 0.323  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	ScoreRect = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void CreateTrap1 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-50, -80 , 0, // vertex 1
		-10, -80 , 0, // vertex 2
		-40, -100, 0, // vertex 3

		-10, -80 , 0, // vertex 2
		-40, -100, 0, // vertex 3
		-20, -100, 0  // vertex 4
	};

	const GLfloat color_buffer_data [] = {
		0.3, 0.3, 0.3, // color 1
		0.3, 0.3, 0.3, // color 2
		0.3, 0.3, 0.3, // color 3

		0.3, 0.3, 0.3, // color 3
		0.3, 0.3, 0.3, // color 4
		0.3, 0.3, 0.3  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	trap1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void create_batery_out()
{
	const GLfloat vertex_buffer_data [] = {
		120, -30 , 0, // vertex 1
		155, -30 , 0, // vertex 2
		155, -40, 0, // vertex 3
		120, -30 , 0, // vertex 2
		120, -40, 0, // vertex 3
		155, -40, 0,  // vertex 4

		120, -30, 0,
		120, -25, 0,
		155, -25, 0,
		155, -25, 0,
		155, -30, 0,
		120, -30, 0,

		155, -30, 0,
		160, -30, 0,
		160, -40, 0,
		155, -40, 0,
		160, -40, 0,
		155, -30, 0,

		120, -40, 0,
		120, -45, 0,
		155, -45, 0,
		155, -45, 0,
		155, -40, 0,
		120, -40, 0,

		120, -30, 0,
		115, -30, 0,
		115, -40, 0,
		115, -40, 0,
		120, -40, 0,
		120, -30, 0,

		160, -33, 0,
		163, -33, 0,
		163, -37, 0,
		163, -37, 0,
		160, -37, 0,
		160, -33, 0
	};

	const GLfloat color_buffer_data [] = {
		0.7, 0.7, 0.7, // color 1
		0.7, 0.7, 0.7, // color 2
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 4
		0.7, 0.7, 0.7,  // color 1

		0.7, 0.7, 0.7, // color 1
		0.7, 0.7, 0.7, // color 2
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 4
		0.7, 0.7, 0.7,  // color 1

		0.7, 0.7, 0.7, // color 1
		0.7, 0.7, 0.7, // color 2
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 4
		0.7, 0.7, 0.7,  // color 1

		0.7, 0.7, 0.7, // color 1
		0.7, 0.7, 0.7, // color 2
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 4
		0.7, 0.7, 0.7,  // color 1
		
		0.7, 0.7, 0.7, // color 1
		0.7, 0.7, 0.7, // color 2
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 7
		0.7, 0.7, 0.7, // color 4
		0.7, 0.7, 0.7,  // color 1

		0.95, 0.95, 0.95, // color 1
		0.95, 0.95, 0.95, // color 2
		0.95, 0.95, 0.95, // color 95
		0.95, 0.95, 0.95, // color 95
		0.95, 0.95, 0.95, // color 4
		0.95, 0.95, 0.95  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	battery_out = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

	for(int i=270;i<=360;i++)
	{
		const GLfloat vertex_buffer_data [] = {
			120.0, -30 , 0, // vertex 1
			120.0 + 5 * sin((float)i*M_PI/180.0f),  -30.0 + 5.0 * cos((float)i*M_PI/180.0f) , 0, // vertex 2
			120.0 + 5 * sin((float)(i+1)*M_PI/180.0f),  -30.0 + 5.0 * cos((float)(i+1)*M_PI/180.0f) , 0, // vertex 3
		};

		const GLfloat color_buffer_data [] = {
			0.7, 0.7, 0.7, // color 1
			0.7, 0.7, 0.7, // color 2
			0.7, 0.7, 0.7, // color 3
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		battery_semi[i] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	for(int i=0;i<=90;i++)
	{
		const GLfloat vertex_buffer_data [] = {
			155.0, -30 , 0, // vertex 1
			155.0 + 5 * sin((float)i*M_PI/180.0f),  -30.0 + 5.0 * cos((float)i*M_PI/180.0f) , 0, // vertex 2
			155.0 + 5 * sin((float)(i+1)*M_PI/180.0f),  -30.0 + 5.0 * cos((float)(i+1)*M_PI/180.0f) , 0, // vertex 3
		};

		const GLfloat color_buffer_data [] = {
			0.7, 0.7, 0.7, // color 1
			0.7, 0.7, 0.7, // color 2
			0.7, 0.7, 0.7, // color 3
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		battery_semi[i] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	for(int i=90;i<=180;i++)
	{
		const GLfloat vertex_buffer_data [] = {
			155.0, -40 , 0, // vertex 1
			155.0 + 5 * sin((float)i*M_PI/180.0f),  -40.0 + 5.0 * cos((float)i*M_PI/180.0f) , 0, // vertex 2
			155.0 + 5 * sin((float)(i+1)*M_PI/180.0f),  -40.0 + 5.0 * cos((float)(i+1)*M_PI/180.0f) , 0, // vertex 3
		};

		const GLfloat color_buffer_data [] = {
			0.7, 0.7, 0.7, // color 1
			0.7, 0.7, 0.7, // color 2
			0.7, 0.7, 0.7, // color 3
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		battery_semi[i] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	for(int i=180;i<=270;i++)
	{
		const GLfloat vertex_buffer_data [] = {
			120.0, -40 , 0, // vertex 1
			120.0 + 5 * sin((float)i*M_PI/180.0f),  -40.0 + 5.0 * cos((float)i*M_PI/180.0f) , 0, // vertex 2
			120.0 + 5 * sin((float)(i+1)*M_PI/180.0f),  -40.0 + 5.0 * cos((float)(i+1)*M_PI/180.0f) , 0, // vertex 3
		};

		const GLfloat color_buffer_data [] = {
			0.7, 0.7, 0.7, // color 1
			0.7, 0.7, 0.7, // color 2
			0.7, 0.7, 0.7, // color 3
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		battery_semi[i] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	}
}

void CreateLazerBeam (float w, float x, float y, float ang, float a, float b, int st)
{
	const GLfloat vertex_buffer_data4 [] = {
		x, y , 0, // vertex 1
		w, y , 0, // vertex 2
		w, y-0.8, 0, // vertex 3

		x, y , 0, // vertex 2
		w, y-0.8, 0, // vertex 3
		x, y-0.8, 0  // vertex 4
	};

	const GLfloat color_buffer_data4 [] = {
		1, 0, 0, // color 1
		1, 0, 0, // color 2
		1, 0, 0, // color 5

		1, 0, 0, // color 5
		1, 0, 0, // color 4
		1, 0, 0  // color 1
	};

	vector <float> tem;
	if(st == 0)
	{
		tem.push_back(-95);
		tem.push_back(y_global_lazer);
		tem.push_back(ang);
		tem.push_back(a);
		tem.push_back(b);
		tem.push_back(w);
	}
	else
	{
		tem.push_back(x);
		tem.push_back(y);
		tem.push_back(ang);
		tem.push_back(a);
		tem.push_back(b);
		tem.push_back((w-x) * cos(ang * (M_PI / 180.0)));
	}

	if(FLAG == 0)
	lazer.clear();

	// create3DObject creates and returns a handle to a VAO that can be used later
	lazer.insert(make_pair(create3DObject(GL_TRIANGLES, 6, vertex_buffer_data4, color_buffer_data4, GL_FILL), tem));
}

void CreateLazer ()
{
	const GLfloat vertex_buffer_data2 [] = {
		-94, 1.4 , 0, // vertex 1
		-90, 1.4 , 0, // vertex 2
		-90, -1.4, 0, // vertex 3

		-94, 1.4 , 0, // vertex 2
		-90, -1.4, 0, // vertex 3
		-94, -1.4, 0  // vertex 4
	};

	const GLfloat color_buffer_data2 [] = {
		0.3, 0.3, 0.3, // color 1
		0.3, 0.3, 0.3, // color 2
		0.3, 0.3, 0.3, // color 3

		0.3, 0.3, 0.3, // color 3
		0.3, 0.3, 0.3, // color 4
		0.3, 0.3, 0.3  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	lazer_end = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);

	const GLfloat vertex_buffer_data1 [] = {
		-100, 8 , 0, // vertex 1
		-95,  8 , 0, // vertex 2
		-95, -8 , 0, // vertex 3

		-100,  8 , 0, // vertex 2
		-95, -8 , 0, // vertex 3
		-100, -8, 0  // vertex 4
	};

	const GLfloat color_buffer_data1 [] = {
		0.5, 0.5, 0.5, // color 1
		0.5, 0.5, 0.5, // color 2
		0.5, 0.5, 0.5, // color 5

		0.5, 0.5, 0.5, // color 5
		0.5, 0.5, 0.5, // color 4
		0.5, 0.5, 0.5  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	lazer_rect = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);

	for(int i=0;i<=360;i++)
	{
		const GLfloat vertex_buffer_data [] = {
			-95.0, 0 , 0, // vertex 1
			-95.0 + 4 * sin((float)i*M_PI/180.0f),  4.0 * cos((float)i*M_PI/180.0f) , 0, // vertex 2
			-95.0 + 4 * sin((float)(i+1)*M_PI/180.0f),  4.0 * cos((float)(i+1)*M_PI/180.0f) , 0, // vertex 3
		};

		const GLfloat color_buffer_data [] = {
			0.3, 0.3, 0.3, // color 1
			0.3, 0.3, 0.3, // color 2
			0.3, 0.3, 0.3, // color 3
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		lazer_circle[i] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

}

void CreateTrap2 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		50, -80 , 0, // vertex 1
		10, -80 , 0, // vertex 2
		40, -100, 0, // vertex 3

		10, -80 , 0, // vertex 2
		40, -100, 0, // vertex 3
		20, -100, 0  // vertex 4
	};

	const GLfloat color_buffer_data [] = {
		0.5, 0.5, 0.5, // color 1
		0.5, 0.5, 0.5, // color 2
		0.5, 0.5, 0.5, // color 3

		0.5, 0.5, 0.5, // color 3
		0.5, 0.5, 0.5, // color 4
		0.5, 0.5, 0.5  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	trap2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
int ttt = 0;

void CreateScoreRect (float xx, float yy, int ty, int ttyy)
{
	if(ty == 0)
	{
		// GL3 accepts only Triangles. Quads are not supported
		const GLfloat vertex_buffer_data [] = {
			xx, yy , 0, // vertex 1
			xx + 5.0, yy , 0, // vertex 2
			xx + 5.0, yy - 2.0, 0, // vertex 3

			xx, yy , 0, // vertex 2
			xx, yy - 2.0, 0, // vertex 3
			xx + 5.0, yy - 2.0, 0  // vertex 4
		};

		const GLfloat color_buffer_data [] = {
			1.0, 1.0, 1.0, // color 1
			1.0, 1.0, 1.0, // color 2
			1.0, 1.0, 1.0, // color 3

			1.0, 1.0, 1.0, // color 3
			1.0, 1.0, 1.0, // color 4
			1.0, 1.0, 1.0  // color 1
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		VAO * tempp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
		//score_rect.insert(tempp);
		if(ttyy == 0) score_rect.insert(tempp);
		if(ttyy == 1) time_rect.insert(tempp);
		if(ttyy == 2) template_rect.insert(tempp);
	}

	else
	{
		// GL3 accepts only Triangles. Quads are not supported
		const GLfloat vertex_buffer_data [] = {
			xx, yy , 0, // vertex 1
			xx + 2.0, yy , 0, // vertex 2
			xx + 2.0, yy - 5.0, 0, // vertex 3

			xx, yy , 0, // vertex 2
			xx, yy - 5.0, 0, // vertex 3
			xx + 2.0, yy - 5.0, 0  // vertex 4
		};

		const GLfloat color_buffer_data [] = {
			1.0, 1.0, 1.0, // color 1
			1.0, 1.0, 1.0, // color 2
			1.0, 1.0, 1.0, // color 3

			1.0, 1.0, 1.0, // color 3
			1.0, 1.0, 1.0, // color 4
			1.0, 1.0, 1.0  // color 1
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		VAO * tempp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
		//score_rect.insert(tempp);
		if(ttyy == 0) score_rect.insert(tempp);
		if(ttyy == 1) time_rect.insert(tempp);
		if(ttyy == 2) template_rect.insert(tempp);
	}
}

void create_batery_in(int timet)
{
	const GLfloat vertex_buffer_data [] = {
		118, -28, 0,
		118 + (39 * (float)timet / 5.0), -28, 0,
		118 + (39 * (float)timet / 5.0), -42, 0,
		118 + (39 * (float)timet / 5.0), -42, 0,
		118, -42, 0,
		118, -28, 0
	};

	const GLfloat color_buffer_data [] = {
		1.0, 0.0, 0.0, // color 1
		1.0, 0.0, 0.0, // color 2
		1.0, 0.0, 0.0, // color 3

		1.0, 0.0, 0.0, // color 3
		1.0, 0.0, 0.0, // color 4
		1.0, 0.0, 0.0  // color 1
	};

	VAO * te = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	time_rect.insert(te);
}

void draw_dot(float xx, float yy, int ttyy)
{
	// GL3 accepts only Triangles. Quads are not supported
		const GLfloat vertex_buffer_data [] = {
			xx, yy , 0, // vertex 1
			xx + 2.0, yy , 0, // vertex 2
			xx + 2.0, yy - 2.0, 0, // vertex 3

			xx, yy , 0, // vertex 2
			xx, yy - 2.0, 0, // vertex 3
			xx + 2.0, yy - 2.0, 0  // vertex 4
		};

		const GLfloat color_buffer_data [] = {
			1.0, 1.0, 1.0, // color 1
			1.0, 1.0, 1.0, // color 2
			1.0, 1.0, 1.0, // color 3

			1.0, 1.0, 1.0, // color 3
			1.0, 1.0, 1.0, // color 4
			1.0, 1.0, 1.0  // color 1
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		VAO * tempp = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
		if(ttyy == 0) score_rect.insert(tempp);
		if(ttyy == 1) time_rect.insert(tempp);
		if(ttyy == 2) template_rect.insert(tempp);
}

void draw_alpha (float xx, float yy, int * arr, int ttyy)
{
	if(arr[0] == 1)
	{
		CreateScoreRect(xx, yy, 0, ttyy);
	}
	if(arr[1] == 1)
	{
		CreateScoreRect(xx, yy - 3.0, 1, ttyy);
	}
	if(arr[2] == 1)
	{
		CreateScoreRect(xx + 3.0, yy - 3.0, 1, ttyy);
	}
	if(arr[3] == 1)
	{
		CreateScoreRect(xx, yy - 9.0, 0, ttyy);
	}
	if(arr[4] == 1)
	{
		CreateScoreRect(xx, yy - 12.0, 1, ttyy);
	}
	if(arr[5] == 1)
	{
		CreateScoreRect(xx + 3.0, yy - 12.0, 1, ttyy);
	}
	if(arr[6] == 1)
	{
		CreateScoreRect(xx, yy - 18.0, 0, ttyy);
	}
}

void printMessage(float xx, float yy,float zz, string st, int ttyy)
{
	//get the length of the string to display
	int len = (int) st.size();

	//loop to display character by character
	for (int i = 0; i < len; i++) 
	{
		float x_coor = xx + (7*i);
		float y_coor = yy;
		if(st[i] == '.')
		{
			draw_dot(x_coor + 1, y_coor - 18, ttyy);
		}
		else draw_alpha(x_coor, y_coor, map_alpha[st[i]], ttyy);
	}
};

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	glm::mat4 translateTrap1 = glm::translate (glm::vec3(0.0f + x_global_one, 0.0f, 0.0f)); // glTranslatef
	// glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 trap1Transform = translateTrap1;
	Matrices.model *= trap1Transform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(trap1);

	// draw3DObject draws the VAO given to it using current MVP matrix
	// draw3DObject(rectangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateTrap2 = glm::translate (glm::vec3(0.0f + x_global_two, 0.0f, 0.0f));        // glTranslatef
	// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateTrap2;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(trap2);

    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateTrap3 = glm::translate (glm::vec3(0.0f, 0.0f + y_global_lazer, 0.0f));
	Matrices.model *= translateTrap3;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(lazer_rect);
    

    Matrices.model = glm::mat4(1.0f);
    
    glm::mat4 translateLazerRect = glm::translate (glm::vec3(0.0f - y_global_lazer * sin(angle * (M_PI / 180.0)), 
                                                    y_global_lazer * cos(angle * (M_PI / 180.0)),
                                                    0.0f));        // glTranslatef

    glm::mat4 translateLaser_origin = glm::translate (glm::vec3(95, -1.0 * y_global_lazer, 0));
    
    glm::mat4 translateLaser_back = glm::translate (glm::vec3(-95.0, 1.0 * y_global_lazer, 0));

    glm::mat4 rotateLaser = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
    
    Matrices.model *= translateLazerRect * translateLaser_back * rotateLaser * translateLaser_origin;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(lazer_end);
	for(int i=0;i<=360;i++) draw3DObject(lazer_circle[i]);

	if(fl4 == 1 && time_fl4 == 1)
	{
		for(auto i : lazer)
		{
			Matrices.model = glm::mat4(1.0f);
	    
		    glm::mat4 translateLazerRect1 = glm::translate (glm::vec3(i.second[3], i.second[4], 0.0f));// glTranslatef

		    glm::mat4 translateLaser_origin1 = glm::translate (glm::vec3(-1 * i.second[0], -1 * i.second[1], 0));
		    
		    glm::mat4 translateLaser_back1 = glm::translate (glm::vec3(i.second[0], i.second[1], 0));

		    glm::mat4 rotateLaser1 = glm::rotate((float)(i.second[2]*M_PI/180.0f), glm::vec3(0,0,1));
		    
		    Matrices.model *= translateLazerRect1 * translateLaser_back1 * rotateLaser1 * translateLaser_origin1;
		    MVP = VP * Matrices.model;
		    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		    //printf("%d\n", i.second);
		    // draw3DObject draws the VAO given to it using current MVP matrix
			draw3DObject(i.first);
		}
	}

	for(auto i : mirror)
	{
		Matrices.model = glm::mat4(1.0f);
	    
	    glm::mat4 translateLazerRect1 = glm::translate (glm::vec3(0.0f, 0.0f, 0.0f));// glTranslatef

	    glm::mat4 translateLaser_origin1 = glm::translate (glm::vec3(-1 * i.first->x, -1.0 * i.first->y, 0));
	    
	    glm::mat4 translateLaser_back1 = glm::translate (glm::vec3(i.first->x, 1.0 * i.first->y, 0));

	    glm::mat4 rotateLaser1 = glm::rotate((float)(i.second*M_PI/180.0f), glm::vec3(0,0,1));
	    
	    Matrices.model *= translateLazerRect1 * translateLaser_back1 * rotateLaser1 * translateLaser_origin1;
	    MVP = VP * Matrices.model;
	    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	    //printf("%d\n", i.second);
	    // draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(i.first);
	}

    fade_temp.clear();
    faded_t.clear();

    for(auto i : fade)
    {
        if(i.second == 0)
        {
            if(i.second < 98)
                fade_temp.insert(make_pair(i.first, 32));
            fade.erase(fade.find(i));
        }
        else if(i.second == 1)
        {
            if(i.second < 98)
                fade_temp.insert(make_pair(i.first, 52));
            fade.erase(fade.find(i));
        }
        else
        {
            if(i.second < 1790)
            {
                create_faded_block(i.first.first->x, i.first.first->y - i.first.second, (float)i.second / 2000.0);

                fade_temp.insert(make_pair(i.first, i.second + 1));
            }
            fade.erase(fade.find(i));   
        }
    }

    for(auto i : faded_t)
    {   
        Matrices.model = glm::mat4(1.0f);

        glm::mat4 translateTrap2 = glm::translate (glm::vec3(0.0f, 0.0f, 0.0f));        // glTranslatef
        // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= translateTrap2;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(i);
    }

    for(auto i : fade_temp)
    {
        fade.insert(i);
    }

	set< pair< pair< VAO *, float >, int> > fell_blocks_temp;

	for(auto i : fell_blocks)
	{
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateTrap2 = glm::translate (glm::vec3(0.0f, 0.0f - i.first.second, 0.0f));        // glTranslatef
		// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= translateTrap2;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(i.first.first);
		fell_blocks_temp.insert(make_pair(make_pair(i.first.first, i.first.second + 0.008 + (faster * (0.002) )), i.second));
	}

	fell_blocks.clear();
	for(auto i : fell_blocks_temp) 
	{
		if(i.first.second < 165)
        {
			fell_blocks.insert(i);
        }

        else
        {
        	// doomed

        	float x_block = i.first.first->x;
        	int type_block = i.second;

        	if(x_block > 10 + x_global_two && x_block < 50 + x_global_two && i.first.second >= 165 && i.first.second <= 166)
        	{
        		printf("under trap 2, type = %d\n", type_block);
        		if(type_block == 2) score ++, score_flag = 0;
        		else if(type_block == 3) score -= 5, score_flag = 0;
        	}

        	else if(x_block < -10 + x_global_one && x_block > -50 + x_global_one && i.first.second >= 165 && i.first.second <= 166)
        	{
        		printf("under trap 1, type = %d\n", type_block);
        		if(type_block == 1) score ++, score_flag = 0;
        		else if(type_block == 3) score -= 5, score_flag = 0;
        	}

        	else
        	{
        		if(i.first.second < 200) fell_blocks.insert(i);
        		else
        		{
        			if(type_block == 3) score -= 5, score_flag = 0;
        		}
        	}
        }
	}

	for(auto i : to_fall_blocks)
	{
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateTrap2 = glm::translate (glm::vec3(0.0f, 0.0f, 0.0f));        // glTranslatef
		// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= translateTrap2;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);



		// draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(i.first);
		fell_blocks.insert(make_pair(make_pair(i.first, 0.0), i.second));
		to_fall_blocks.erase(to_fall_blocks.find(i));
	}

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateTrap22 = glm::translate (glm::vec3(0.0f, 0.0f, 0.0f));        // glTranslatef
	// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateTrap22;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(ScoreRect);

	string msg = "";
	string msg2 = "";
	int ttemp = score;
	double temp2 = laser_time;

	int temp3 = temp2 * 10;
	int temp5 = temp3;

	if(temp3 == 0) msg2 += '0';
	while(temp3 > 0)
	{
		char temp4 = (temp3 % 10) + '0';
		msg2 += temp4;
		temp3 /= 10;
	}

	int neg = 0;
	if(score == 0) msg += '0';
	if(score < 0)
	{
		neg = 1;
		ttemp *= -1;
	}
	while(ttemp > 0)
	{
		char aab = (ttemp % 10) + '0';
		msg += aab;
		ttemp /= 10;
	}
	// printf("%d %d\n", score_flag, laser_time_flag);
	// printf("size = %d, score_flag = %d\n", score_rect.size(), score_flag);
	if(score_flag == 0 || out_of_names == 0)
	{
		if(neg == 1) msg += '-';
		reverse(msg.begin(), msg.end());
		score_rect.clear();
		printMessage(158.0, 70.0, 0.0, msg, 0);
	}
	if(laser_time_flag == 1 || out_of_names == 0)
	{
		reverse(msg2.begin(), msg2.end());
		time_rect.clear();
		//printMessage(147.0, -25.0, 0.0, msg2, 1);
		create_batery_in((int)(laser_time * 10));
	}

	//printf("template size = %d\n", template_rect.size());
	for(auto i : template_rect)
	{
		Matrices.model = glm::mat4(1.0f);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(i);
	}
	//printf("score size = %d\n", score_rect.size());
	for(auto i : score_rect)
	{
		Matrices.model = glm::mat4(1.0f);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(i);
	}
	//printf("time size = %d\n", time_rect.size());
	for(auto i : testing)
	{
		Matrices.model = glm::mat4(1.0f);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(i);
	}

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(battery_out);
	for(int i=0;i<=360;i++) draw3DObject(battery_semi[i]);
	for(auto i : time_rect)
	{
		Matrices.model = glm::mat4(1.0f);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(i);
	}
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		// exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		// exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, scroll_callback);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	// Generate the VAO, VBOs, vertices data & copy into the array buffer
	CreateTrap1();
	CreateTrap2();
	CreateLazer();
	CreateScore();
	create_batery_out();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.9f, 0.9f, 0.9f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int flagg = 0;

int make_alpha_map()
{
	teemppp[0][0] = 1; teemppp[0][1] = 1; teemppp[0][3] = 1; teemppp[0][5] = 1; teemppp[0][6] = 1;
	map_alpha['S'] = teemppp[0];
	map_alpha['s'] = teemppp[0];
	map_alpha['5'] = teemppp[0];

	teemppp[1][0] = 1; teemppp[1][1] = 1; teemppp[1][4] = 1; teemppp[1][6] = 1;
	map_alpha['C'] = teemppp[1];
	map_alpha['c'] = teemppp[1];

	teemppp[2][0] = 1; teemppp[2][1] = 1; teemppp[2][2] = 1; teemppp[2][4] = 1; teemppp[2][5] = 1; teemppp[2][6] = 1;
	map_alpha['O'] = teemppp[2];
	map_alpha['o'] = teemppp[2];
	map_alpha['0'] = teemppp[2];

	teemppp[3][0] = 1; teemppp[3][1] = 1; teemppp[3][2] = 1; teemppp[3][3] = 1; teemppp[3][4] = 1; teemppp[3][5] = 1;
	map_alpha['R'] = teemppp[3];
	map_alpha['r'] = teemppp[3];

	teemppp[4][0] = 1; teemppp[4][1] = 1; teemppp[4][3] = 1; teemppp[4][4] = 1; teemppp[4][6] = 1;
	map_alpha['E'] = teemppp[4];
	map_alpha['e'] = teemppp[4];

	teemppp[5][2] = 1; teemppp[5][5] = 1;
	map_alpha['1'] = teemppp[5];

	teemppp[6][0] = 1; teemppp[6][2] = 1; teemppp[6][3] = 1; teemppp[6][4] = 1; teemppp[6][6] = 1;
	map_alpha['2'] = teemppp[6];

	teemppp[7][0] = 1; teemppp[7][2] = 1; teemppp[7][3] = 1; teemppp[7][5] = 1; teemppp[7][6] = 1;
	map_alpha['3'] = teemppp[7];

	teemppp[8][1] = 1; teemppp[8][3] = 1; teemppp[8][2] = 1; teemppp[8][5] = 1;
	map_alpha['4'] = teemppp[8];

	teemppp[9][0] = 1; teemppp[9][1] = 1; teemppp[9][3] = 1; teemppp[9][5] = 1; teemppp[9][6] = 1; teemppp[9][4] = 1;
	map_alpha['6'] = teemppp[9];

	teemppp[10][0] = 1; teemppp[10][2] = 1; teemppp[10][5] = 1;
	map_alpha['7'] = teemppp[10];

	teemppp[11][0] = 1; teemppp[11][1] = 1; teemppp[11][3] = 1; teemppp[11][5] = 1; teemppp[11][6] = 1; teemppp[11][4] = 1; teemppp[11][2] = 1;
	map_alpha['8'] = teemppp[11];

	teemppp[12][0] = 1; teemppp[12][1] = 1; teemppp[12][3] = 1; teemppp[12][5] = 1; teemppp[12][6] = 1; teemppp[12][2] = 1;
	map_alpha['9'] = teemppp[12];

	teemppp[14][0] = 0;
	map_alpha[' '] = teemppp[14];

	teemppp[15][3] = 1;
	map_alpha['-'] = teemppp[15];

	teemppp[16][8] = 1;
	map_alpha['.'] = teemppp[16];

	return(0);
}

int epic_random(int a, int b)
{
	std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(a, b); // distribution in range [1, 6]

    //std::cout << "as " << dist6(rng) << std::endl;
	return dist6(rng);
}

int main (int argc, char** argv)
{
	make_alpha_map();
	int width = 1500;
	int height = 1000;

	GLFWwindow* window = initGLFW(width, height);
	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time, save_time = glfwGetTime();

	// get number of mirrors:
	
	int num_mirr = epic_random(1, 2);
	for(int i=0;i<num_mirr;i++)
	{
		int mirror_angle = epic_random(0, 70);

		int x = epic_random(-70, 70);

		int y = epic_random(-40 + 40 * i,  30 * i);
		
		CreateMirror(x ,y, mirror_angle);
	}

    double lasttime = glfwGetTime();
    double lasttime2 = glfwGetTime();

    int space_pressed = 0;

    string som = "Score ";
    printMessage(115.0, 70.0, 0.0, som, 2);

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) 
    {
    	reshapeWindow(window, width, height);

    	maxCoord = min(maxCoord, 160.0);
    	maxCoord = max(maxCoord, 40.0);

    	faster = max(faster, -3);
    	faster = min(faster, 5);

    	laser_time_flag = 0;
    	score_flag = 1;
    	//printf("score = %d\n", score);
    	laser_time = max(laser_time, 0.0);
        if(mouse_press)
        {
            glfwGetCursorPos(window, &mouse_x, &mouse_y);

            int ff1 = 0, ff2 = 0;

            // printf("%lf %lf\n", mouse_x * (14.0 /75.0), mouse_y);

            if( (mouse_x * (14.0 / 75.0) >= 50 + x_global_one && 
            	mouse_x * (14.0 / 75.0) <= 90 + x_global_one) ) ff1 = 1;

            if( (mouse_x * (14.0 / 75.0) >= 110 + x_global_two && 
            	mouse_x * (14.0 / 75.0) <= 150 + x_global_two) ) ff2 = 1;

            if(mouse_y >= 900 && (ff1 == 1 || ff2 == 1))
            {
            	//printf("yes\n");

            	if(ff1 == 1)
            	{
            		if(fabs(prevMouse_x - mouse_x) >= 20.0) prevMouse_x = 0.0;
            		//printf("%lf\n", prevMouse_x);
            		if(prevMouse_x != 0.0) x_global_one += (mouse_x - prevMouse_x) * (14.0 / 75.0);
            		prevMouse_x = mouse_x;
            	}

            	else
            	{
            		if(fabs(prevMouse_x - mouse_x) >= 20.0) prevMouse_x = 0.0;
            		//printf("%lf\n", prevMouse_x);
            		if(prevMouse_x != 0.0) x_global_two += (mouse_x - prevMouse_x) * (14.0 / 75.0);
            		prevMouse_x = mouse_x;
            	}
            }

            else if(prevMouse_y != mouse_y || prevMouse_x != mouse_x)
            {
                if(prevMouse_x != mouse_x)
                    prevMouse_x = mouse_x;

                if(prevMouse_y != mouse_y)
                    prevMouse_y = mouse_y;
                
                if(mouse_x == 0)
                    angle = 90;
                
                else
                {
                    angle = atan(abs(500.0 - mouse_y) / abs(mouse_x)); 
                    angle *= 180/M_PI;
                    if(mouse_y > 500) angle = -1*angle;
                    if(mouse_x < 0 && mouse_y < 500) angle = 90;
                    if(mouse_x <=0 && mouse_y >= 500) angle = -90;
                }
            }
        }

        float se = 250.0;
        FLAG = 0;

        double currtime = glfwGetTime();
        double currtime2 = glfwGetTime();

        laser_time_flag = 0;

        if(currtime2 - lasttime2 >= 0.1)
        {
        	if(fl4 == 1 && laser_time != 0.0)
        	{
        		laser_time -= 0.1;
        		laser_time_flag = 1;
        	}

        	else if(fl4 == 0 && laser_time != 0.5)
        	{
        		laser_time += 0.1;
        		laser_time_flag = 1;
        	}

        	lasttime2 = currtime2;
        }

        laser_time = min(laser_time, 0.5);
        laser_time = max(laser_time, 0.0);

        time_fl4 = 0;

        // detect collision
        if(fl4)
        {
        	if(laser_time > 0.0)
        	{
        		time_fl4 = 1;
        		set < pair< pair< VAO *, float >, int > > fell_blocks_temp2;

	            for(auto i : mirror)
	            {
	            	double x_left = i.first->x;
	                double x_right = x_left + 30.0 * (sin(i.second * (M_PI / 180.0)));

	                double y_top = i.first->y;
	                double y_low = y_top - (30.0 * (cos(i.second * (M_PI / 180.0))));

	                //create_test(x_left, y_top, 0.1);
	                //create_test(x_right, y_low, 0.1);
	                //create_test(-95.0, y_global_lazer, 0.7);
	                
	                double c = y_global_lazer + (tan(angle * (M_PI / 180.0)) * 95.0);

	                double a1 = (tan(angle * (M_PI / 180.0)) * x_left) - y_top + c;
	                double a2 = (tan(angle * (M_PI / 180.0)) * x_right) - y_low + c;

	                if(a1 * a2 < 0.0) 
	                {
	                	double m1;
	                	if(i.second != 0) m1 = tan((90.0 + i.second) * (M_PI/180.0));
	                	else m1 = 1000000;

	                	double m2 = tan(angle * (M_PI / 180.0));

	                	double c1 = y_low - (m1 * x_right);
	                	double c2 = c;

	                	double x_int = (c2 - c1) / (m1 - m2);
	                	double y_int = (m1 * x_int) + c1;

	                	lazer.clear();

	                	se = ( (x_int + 95) / cos(angle * (M_PI / 180.0)) ) - 95 ;

	                	//create_test(x_int, y_int, 0.5);

	                	CreateLazerBeam(500.0, x_int, y_int, (2.0 * (90+i.second)) - angle, 0.0, 0.0, 1);
	                	FLAG = 1;
	                }
	            }

	            CreateLazerBeam(se, -90.0, 0.5, angle, -1 * y_global_lazer * sin(angle * (M_PI / 180.0)), 
	        	y_global_lazer * cos(angle * (M_PI / 180.0)), 0);
	        
	        	//FLAG = 0;

	            for(auto i : fell_blocks)
	            {
	            	for(auto j : lazer)
	            	{
		                float x_left = i.first.first->x;
		                float x_right = x_left + 3.0;

		                float y_top = 100.0 - i.first.second;
		                float y_low = y_top - 15.0;

		                //testing.clear();
		                //create_test(x_left, y_top, 0.1);
		                //create_test(x_right, y_low, 0.1);

		                // create_test(j.second[0], j.second[1], 0.1);

		                // printf("x is %lf y is %lf angle is %lf\n", j.second[0], j.second[1], j.second[2] - 180.0);
		                

		                double pass = j.second[2];
		                if(pass >= 180.0) pass -= 180.0;

		                //printf("pass = %lf, x = %lf\n", pass, j.second[0]);
		                double c = j.second[1] - (tan((pass) * (M_PI / 180.0)) * j.second[0]);

		               	//create_test(j.second[0], j.second[1], 0.7);
		                //create_test(j.second[0] + 10 * cos(pass * (M_PI / 180.0)), j.second[1] + 10 * sin(pass * (M_PI / 180.0)), 0.7);
		                //create_test(j.second[0] + 20 * cos(pass * (M_PI / 180.0)), j.second[1] + 20 * sin(pass * (M_PI / 180.0)), 0.7);

		                double a1 = (tan(angle * (M_PI / 180.0)) * x_left) - y_top + c;
		                double a2 = (tan(angle * (M_PI / 180.0)) * x_right) - y_low + c;

		                double ang1 = atan( (y_top - j.second[1]) / (x_left - j.second[0]) ) * (180.0 / M_PI);
		                double ang2 = atan( (y_low - j.second[1]) / (x_right - j.second[0]) ) * (180.0 / M_PI);

		                //printf("ang1 = %lf, ang2 = %lf, pass = %lf\n", ang1, ang2, pass);
		                
		                int some_flag = 0;

		                if(x_left >= j.second[0] && x_left <= j.second[5]) some_flag = 1;

		                if( ((ang1 >= pass && ang2 <= pass ) || ( ang1 <= pass && ang2 >= pass )) && some_flag == 1) 
		                {
			                //printf("start = %lf, end = %lf, x_left = %lf, end2 = %lf, size = %d\n",j.second[0] ,j.second[5], x_left, j.second[6], lazer.size());
		                    fade.insert(i);
		                    fell_blocks_temp2.insert(i);
		                    if(i.second == 3) 
		                    {
		                    	score += 2;
		                    	score_flag = 0;
		                    }
		                }
		            }
	            }

	            for(auto i : fell_blocks_temp2)
	            {
	            	fell_blocks.erase(fell_blocks.find(i));
	            }
	        }
        }

        if(se == 250.0)
        {
	        CreateLazerBeam(se, -90.0, 0.5, angle, -1 * y_global_lazer * sin(angle * (M_PI / 180.0)), 
	        	y_global_lazer * cos(angle * (M_PI / 180.0)), 0);
	        FLAG = 0;
	    }

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		current_time = glfwGetTime(); // Time in seconds

		if ((current_time - save_time) >= 2 - (faster * 0.2)) 
        {
			save_time = current_time;
			int num_to_fall = epic_random(1, 2);

			for(int i=0;i<num_to_fall;i++)
			{
				int xcoor = -80 + ((200 / num_to_fall) * i) + epic_random(0, 200 / num_to_fall);
				xcoor = min(xcoor, 80);
				int typee = epic_random(1, 3);
				CreateBlock(
							(float)xcoor, 100.0, 
							(float)(xcoor+3), 100.0, 
							(float)xcoor, 85.0, 
							(float)(xcoor+3), 85.0, typee
                );
			}
		}

		if ((current_time - last_update_time) >= 0.5) 
		{
			last_update_time = current_time;
		}

		if(fl1 == 1) x_global_one -= 0.02;
		if(fl1 == 2) x_global_one += 0.02;

		if(fl2 == 1) 
		{
			if(mouse_press_right == 0) x_global_two -= 0.02;
			else xCap += 0.2;
		}
		if(fl2 == 2) 
		{
			if(mouse_press_right == 0) x_global_two += 0.02;
			else xCap -= 0.2;
		}
		if(fl22 == 1)
		{
			yCap += 0.2;
		}
		if(fl22 == 2)
		{
			yCap -= 0.2;
		}

		if(fl3 == 1) y_global_lazer += 0.02;
		if(fl3 == 2) y_global_lazer -= 0.02;

		if(x_global_one >= 112.2) x_global_one = 112.2;
		if(x_global_one <= -50.2) x_global_one = -50.2;

		if(x_global_two >= 50.2) x_global_two = 50.2;
		if(x_global_two <= -111.2) x_global_two = -111.2;

		if(y_global_lazer >= 101.9) y_global_lazer = 101.9;
		if(y_global_lazer <= -67.9) y_global_lazer = -67.9;

		out_of_names = 1;
	}

	glfwTerminate();
}
