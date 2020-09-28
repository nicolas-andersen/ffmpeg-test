#include <stdio.h>

#include "glad/glad.h"

#include "opengl.h"

//
// shitty code but it werkz
//

//
unsigned int shader_program;
unsigned int buffer;
unsigned int vertex_array;
unsigned int texture;
int texture_width;
int texture_height;

// shaders
void compile_shader(const char *shader_source, unsigned int shader)
{
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    int ret = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        printf("gl_compile_shader: shader did not compile");
    }
}

// initialize our opengl texture display
void gl_init(int width, int height)
{
    texture_width = width;
    texture_height = height;

    const char *vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "out vec2 texture_cord;\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "texture_cord = vec2((aPos.x + 1.0) * 0.5, (aPos.y + 1.0) * -0.5);\n"
    "}\0";

    unsigned int vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    compile_shader(vertex_shader_source, vertex_shader);

    // fragment shader
    const char *fragment_shader_source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 texture_cord;\n"
    "uniform sampler2D color;\n"
    "void main()\n"
    "{\n"
        "FragColor = texture(color, texture_cord);\n"
    "}\0";

    unsigned int fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    compile_shader(fragment_shader_source, fragment_shader);

    //shader program
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // verts for a 2D rect
    float verts[] = {
        -1.f, -1.f,
        -1.f, 1.f,
        1.f, -1.f,
        1.f, 1.f
    };

    glGenBuffers(1, &buffer);
    glGenVertexArrays(1, &vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBindVertexArray(vertex_array);

    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // unbind buffers again
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // setup texture
   
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);
}

// width and height should always be the same
void gl_update_texture(unsigned char *buffer)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
}

// run this every frame
void gl_new_frame()
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vertex_array);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void gl_destroy()
{
    glDeleteBuffers(1, &buffer);
    glDeleteProgram(shader_program);
    glDeleteVertexArrays(1, &vertex_array);
}
