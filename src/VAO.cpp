#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &ID);
}

void VAO::LinkVBO(VBO& VBO, GLuint layout)
{
    VBO.Bind();
    glVertexAttribPointer(layout, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(layout);
    VBO.Unbind();
}

void VAO::Bind()
{
    glBindVertexArray(ID); // makes the vao the current object

}

void VAO::Unbind()
{
    glBindVertexArray(0); // unbinds VAO
}

void VAO::Delete()
{
    glDeleteVertexArrays(1, &ID);
}