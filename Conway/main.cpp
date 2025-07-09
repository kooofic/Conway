#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include <vector>

const int numberOfSeparators = 80;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform vec2 trans;"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x + trans.x, aPos.y + trans.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\0";

static bool mapGrid[numberOfSeparators][numberOfSeparators] = { false };
static bool simulationRunning = false;

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        simulationRunning = !simulationRunning;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (simulationRunning) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);
		int x = (int)( (xpos / 1200.0f) * numberOfSeparators );
		int y = (int)( ( ( 1200 - ypos ) / 1200.0f) * numberOfSeparators );
		mapGrid[y][x] = !mapGrid[y][x];


    }
}

unsigned createShaderProgram(const char* vertexShaderCode, const char* fragmentShaderCode)
{
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        throw -1;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        throw -1;
    }

    long shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
        throw -1;
    }

	return shaderProgram;
}

// There will be a single window, so we can handle the initialization in a single function.
GLFWwindow* init()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        std::cout << "Couldn't initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwSetErrorCallback(error_callback);

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1200, 1200, "Conways", NULL, NULL);
    if (!window)
    {
        std::cout << "Couldn't create window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSwapInterval(1);
    glViewport(0, 0, 1200, 1200);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return window;
}

struct vec2
{
    float x = 0.0f;
	float y = 0.0f;
};

class Grid
{
    std::vector<float> vertices;
    unsigned int vbo;
    unsigned int vao;



    Grid() = delete;

public:

    Grid(int gridSize)
    {
        for (int i = 0; i < numberOfSeparators; i++)
        {
            float x = -1.0f + (2.0f / numberOfSeparators) * i;

            this->vertices.push_back(x);
            this->vertices.push_back(1.0f);
            this->vertices.push_back(0.0f);
            this->vertices.push_back(x);
            this->vertices.push_back(-1.0f);
            this->vertices.push_back(0.0f);

            this->vertices.push_back(1.0f);
            this->vertices.push_back(x);
            this->vertices.push_back(0.0f);
            this->vertices.push_back(-1.0f);
            this->vertices.push_back(x);
            this->vertices.push_back(0.0f);
        }
        
		this->vertices.shrink_to_fit();

        glGenBuffers(1, &this->vbo);
        glGenVertexArrays(1, &this->vao);

        glBindVertexArray(this->vao);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &this->vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    //I have a single shader, so I won't need to bind it.
    void Draw(int translationLocation)
    {
        glUniform2f(translationLocation, 0.0f, 0.0f);

        glBindVertexArray(this->vao);
        glDrawArrays(GL_LINES, 0, (int)(vertices.size() / 3));
        glBindVertexArray(0);
    }
};


class Squares
{
    float vertices[12];

    std::vector<vec2> translations;

    unsigned int indices[6] = {
        0, 1, 2,
        1, 2, 3 
    };

    unsigned int vbo;
    unsigned int vao;
    unsigned int ebo;


	Squares() = delete;
public:

    Squares(float size)
    {
        vertices[0]= 0.0f;
        vertices[1]= 0.0f;
        vertices[2]= 0.0f;

        vertices[3]= 0.0f + size;
        vertices[4]= 0.0f;
        vertices[5]= 0.0f;

        vertices[6]= 0.0f;
        vertices[7]= 0.0f + size;
        vertices[8]= 0.0f;

        vertices[9] = 0.0f + size;
        vertices[10]= 0.0f + size;
        vertices[11]= 0.0f;


        glGenBuffers(1,         &this->vbo);
        glGenVertexArrays(1,    &this->vao);
        glGenBuffers(1,         &this->ebo);

        glBindVertexArray(this->vao);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), this->vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void clearTranslations()
    {
        this->translations.clear();
	};

    void addTranslation(float x, float y)
    {
        vec2 translation;
        translation.x = x;
        translation.y = y;
        this->translations.push_back(translation);
    };

    void addTranslation(vec2 translation)
    {
        this->translations.push_back(translation);
	};

    //I have a single shader, so I won't need to bind it.
    void Draw(int translationLocation)
    {
        if (translations.size() == 0) return;


        glBindVertexArray(this->vao);
        for (int i = 0; i < translations.size(); i++)
        {
            glUniform2f(translationLocation, translations[i].x, translations[i].y);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
    }
};

std::vector<float> vertices;

int main(void)
{
	GLFWwindow* window = init();

    if (!window) return -1;

    unsigned int shaderProgram;

    try
    {
        shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    }
    catch (int e)
    {
        return e;
    }

	Grid grid(numberOfSeparators);
	Squares square(2.0f/numberOfSeparators);


    const double fpsLimit = 1.0 / 15.0;
    double lastUpdateTime = 0;  // number of seconds since the last loop
    double lastFrameTime = 0;   // number of seconds since the last frame

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {

        double now = glfwGetTime();
        double deltaTime = now - lastUpdateTime;

        if ((now - lastFrameTime) >= fpsLimit)
        {
            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);



            //Conways Game of Life logic
            if (simulationRunning)
            {
                bool newMapGrid[numberOfSeparators][numberOfSeparators] = { false };
                for (int i = 0; i < numberOfSeparators; i++)
                {
                    for (int j = 0; j < numberOfSeparators; j++)
                    {
                        int aliveNeighbours = 0;

                        //Check 8 neighbourhood
                        for (int x = -1; x <= 1; x++)
                        {
                            if (i + x < 0 || i + x >= numberOfSeparators) continue;

                            for (int y = -1; y <= 1; y++)
                            {
                                if (j + y < 0 || j + y >= numberOfSeparators) continue;
                                if (x == 0 && y == 0) continue;

                                if (mapGrid[i + x][j + y]) aliveNeighbours++;
                            }
                        }

                        if (mapGrid[i][j] && (aliveNeighbours < 2 || aliveNeighbours > 3)) newMapGrid[i][j] = false;
                        else if (!mapGrid[i][j] && aliveNeighbours == 3) newMapGrid[i][j] = true;
                        else
                        {
							newMapGrid[i][j] = mapGrid[i][j];
                        }
                    }
                }

                for(int i = 0; i < numberOfSeparators; i++)
                {
                    for (int j = 0; j < numberOfSeparators; j++)
                    {
                        mapGrid[i][j] = newMapGrid[i][j];
                    }
				}
            }

            //Ugly, but it works.
            square.clearTranslations();
            for (int i = 0; i < numberOfSeparators; i++)
            {
                for (int j = 0; j < numberOfSeparators; j++)
                {
                    if (mapGrid[i][j])
                    {
                        float x = -1.0f + (2.0f / numberOfSeparators) * j;
                        float y = -1.0f + (2.0f / numberOfSeparators) * i;
                        square.addTranslation(x, y);
                    }
                }
            }

            glUseProgram(shaderProgram);

            int translationLocation = glGetUniformLocation(shaderProgram, "trans");

            grid.Draw(translationLocation);
            square.Draw(translationLocation);


            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();

            lastFrameTime = now;
        }

        lastUpdateTime = now;

    }

    glfwTerminate();
    return 0;
}

