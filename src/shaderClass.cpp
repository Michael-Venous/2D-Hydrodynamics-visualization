#include "shaderClass.h"

std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    /*
    OpenGL shader objects must be accessed by reference.
    1. We get the reference value when we call "glCreateShader()".
    2. We pass the shader source "glShaderSource()"
    3. We compile the shader. "glCompileShader()"
    */
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // reference value to the openGL vertex shader.
    glShaderSource(vertexShader, 1, &vertexSource, NULL); // sets the shader source
    glCompileShader(vertexShader);
    // same for fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // reference value to the openGL vertex shader.
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL); // sets the shader source
    glCompileShader(fragmentShader);

    ID = glCreateProgram(); // ref to shader program. Creates program out of all the shaders.
    glAttachShader(ID, vertexShader); 
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
    glUseProgram(ID);
}

void Shader::Delete()
{
    glDeleteProgram(ID);
}

void Shader::setMat4(const std::string& name, const glm::mat4& matrix) {
    // Get the location of the uniform variable in the shader program
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
    }
    // Pass the matrix to the shader
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}