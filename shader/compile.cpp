#include <GL/glew.h>  // for GLint, GL_COMPILE_STATUS, glAttachShader, glCompileShader, glCreate...
#include <cstdlib>   // for exit, NULL, EXIT_FAILURE
#include <fstream>    // for operator<<, basic_ostream, endl, ifstream, ostream, basic_ostream::...
#include <iostream>   // for cerr
#include <sstream>
#include <string>     // for string, char_traits, operator<<

static inline std::string ReadFile(const std::string &path) {
  std::ifstream file(path, std::ifstream::in);
  if (!file) {
    std::cerr << "Shader unable to open '" << path << "'." << std::endl;
    exit(EXIT_FAILURE);
  }
  std::stringstream shader_stream;
  shader_stream << file.rdbuf();
  file.close();
  return shader_stream.str();
}

GLuint CompileAndLinkVertexFragmentShaderProgram(const std::string &vertex_shader_path,
                                                 const std::string &fragment_shader_path) {
  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  GLint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  const std::string vshader_code = ReadFile(vertex_shader_path);
  const char* vshader_code_ptr = vshader_code.c_str();
  glShaderSource(vertex_shader, 1, &vshader_code_ptr, NULL);
  glCompileShader(vertex_shader);
  // check for shader compile errors
  GLint success;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(EXIT_FAILURE);
  }

  // fragment shader
  GLint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const std::string fshader_code = ReadFile(fragment_shader_path);
  const char* fshader_code_ptr = fshader_code.c_str();
  glShaderSource(fragment_shader, 1, &fshader_code_ptr, NULL);
  glCompileShader(fragment_shader);
  // check for shader compile errors
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(EXIT_FAILURE);
  }
  // link shaders
  GLint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  // check for linking errors
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    exit(EXIT_FAILURE);
  }
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

