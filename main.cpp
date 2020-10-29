#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <string>
#include <eigen3/Eigen/Dense>

//#define GLEW_STATIC
#include <GL/glew.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "bspline.hpp"

struct GlobalState {
  Camera camera;
  double cursor_drag_previous_xpos;
  double cursor_drag_previous_ypos;
  bool cursor_dragging;
};
GlobalState global_state;

// Shader sources
const GLchar* vertexSource = R"glsl(
    #version 150 core
    in vec2 position;
    in vec3 color;
    in vec2 texcoord;
    out vec3 Color;
    out vec2 Texcoord;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 proj;
    void main()
    {
        Color = color;
        Texcoord = texcoord;
        gl_Position = proj * view * model * vec4(position, 0.0, 1.0);
    }
)glsl";
const GLchar* fragmentSource = R"glsl(
    #version 150 core
    in vec3 Color;
    in vec2 Texcoord;
    out vec4 outColor;
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;
    void main()
    {
        outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5);
    }
)glsl";

static void KeyCallback(GLFWwindow* window,
                        int key,
                        int scancode __attribute__((unused)),
                        int action,
                        int mods __attribute__((unused))) {
  fprintf(stderr, "Key press! %d\n", key);
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void CursorPositionCallback(GLFWwindow* window __attribute__((unused)),
                                   double xpos,
                                   double ypos) {
  if (global_state.cursor_dragging) {
    global_state.camera.Drag(static_cast<float>(xpos - global_state.cursor_drag_previous_xpos),
                             static_cast<float>(ypos - global_state.cursor_drag_previous_ypos));
    global_state.cursor_drag_previous_xpos = xpos;
    global_state.cursor_drag_previous_ypos = ypos;
  }
}

static void MouseButtonCallback(GLFWwindow* window,
                                int button,
                                int action,
                                int mods __attribute__((unused))) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // emable drag state
    global_state.cursor_dragging = true;

    // set previous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    global_state.cursor_drag_previous_xpos = xpos;
    global_state.cursor_drag_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    // disable drag state
    global_state.cursor_dragging = false;

    // initialize previous position for determinism
    global_state.cursor_drag_previous_xpos = 0;
    global_state.cursor_drag_previous_ypos = 0;
  }

  //fprintf(stderr, "Mouse button pressed: %d %d (%.1f, %.1f)\n", button, action, xpos, ypos);
}

static void ScrollCallback(GLFWwindow* window __attribute__((unused)),
                           double xoffset __attribute__((unused)),
                           double yoffset) {
  global_state.camera.Scroll(static_cast<float>(yoffset));
}

static void ErrorCallback(int error, const char* description)
{
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

static GLFWwindow* OpenglSetup() {
  glfwSetErrorCallback(ErrorCallback);

  // Load GLFW and Create a Window
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize glfw");
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  GLFWwindow* const window = glfwCreateWindow(800, 600, "basketball", nullptr, nullptr);
  // Check for Valid Context
  if (window == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }


  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, CursorPositionCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetScrollCallback(window, ScrollCallback);

  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  glewInit();

  //gladLoadGL();
  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  glfwSwapInterval(1);

  return window;
}

int main(int argc, char * argv[]) {
  Eigen::Matrix<glm::dvec3, 6, 4> ps;
  ps <<
    glm::dvec3(1.0, 0,  1.0), glm::dvec3(1.0, 3,  1.0), glm::dvec3(1.0, 4,  0.0), glm::dvec3(1.0, 6,  1.0),
    glm::dvec3(1.5, 0,  1.0), glm::dvec3(1.5, 3,  1.0), glm::dvec3(1.5, 4,  1.0), glm::dvec3(1.5, 6,  0.0),
    glm::dvec3(2.0, 0, -1.0), glm::dvec3(2.0, 3, -2.0), glm::dvec3(2.0, 4, -1.0), glm::dvec3(2.0, 6, -1.0),
    glm::dvec3(3.0, 0,  0.0), glm::dvec3(3.0, 3,  0.0), glm::dvec3(3.0, 4,  1.0), glm::dvec3(3.0, 7,  0.0),
    glm::dvec3(4.0, 0,  1.0), glm::dvec3(4.0, 3,  4.0), glm::dvec3(4.0, 4,  1.0), glm::dvec3(4.0, 6,  1.0),
    glm::dvec3(5.0, 0,  0.0), glm::dvec3(5.0, 3,  0.0), glm::dvec3(5.0, 4,  0.0), glm::dvec3(5.0, 6,  0.0);

  const Eigen::Matrix<glm::dvec3, 20, 30> interpolated_ps = CubicBSplineSurface<20, 30, 6, 4>(ps);

  // Parse args
  if (argc != 2) {
    fprintf(stderr, "need one argument: court.jpg location\n");
    exit(EXIT_FAILURE);
  }
  const std::string image_path = argv[1];

  // Boilerplate
  GLFWwindow * const window = OpenglSetup();

  // Create Vertex Array Object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create a Vertex Buffer Object and copy the vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);

  GLfloat vertices[] = {
  //  Position      Color             Texcoords
    -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
    -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create an element array
  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  // Create and compile the vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);

  // Create and compile the fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);

  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // Specify the layout of the vertex data
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

  GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

  // Load textures
  GLuint textures[2];
  glGenTextures(2, textures);

  int width, height;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  unsigned char* image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
  //unsigned char* image = SOIL_load_image(image_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
  if (image == nullptr) {
    fprintf(stderr, "Can't load image %s: %s\n", image_path.c_str(), SOIL_last_result());
    exit(EXIT_FAILURE);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  SOIL_free_image_data(image);
  glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures[1]);
      image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
      SOIL_free_image_data(image);
  glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLint uniModel = glGetUniformLocation(shaderProgram, "model");

  // Set up projection
  GLint uniView = glGetUniformLocation(shaderProgram, "view");

  const float min_clip = 1e-3f;
  const float max_clip = 1e4f;
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, min_clip, max_clip);
  GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
  glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

  auto t_start = std::chrono::high_resolution_clock::now();
  while (glfwWindowShouldClose(window) == false) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate transformation
    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

    // Model transformation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(
      model,
      time * glm::radians(0.1f * 180.0f),
      glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    // Camera transformation
    glm::mat4 view = glm::lookAt(
      global_state.camera.Eye(),
      global_state.camera.Center(),
      glm::vec3(0.0f, 0.0f, -1.0f)
    );
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    // Draw a rectangle from the 2 triangles using 6 indices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Swap buffers
    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glDeleteTextures(2, textures);

  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);

  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);

  glDeleteVertexArrays(1, &vao);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
