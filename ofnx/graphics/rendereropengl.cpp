/*
MIT License

Copyright (c) 2026 Alys_Elica

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "rendereropengl.h"

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/gl.h"
#include "ofnx/tools/log.h"

namespace ofnx::graphics {

/* PRIVATE */
class RendererOpenGL::Impl {
    friend class RendererOpenGL;

public:
    GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc);

private:
    GLuint m_textureVr = 0;
    GLuint m_textureFrame = 0;
    GLuint m_shaderVr = 0;
    GLuint m_shaderFrame = 0;
    GLuint m_vaoVr;
    GLuint m_vboVr;
    GLuint m_eboVr;
    GLuint m_vaoFrame;
    GLuint m_vboFrame;
    GLuint m_eboFrame;
};

GLuint RendererOpenGL::Impl::createShaderProgram(const char* vertexSrc, const char* fragmentSrc)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

const char* fragmentShader = R"(
#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D tex;
void main() {
    FragColor = texture(tex, vTexCoord);
}
)";
const char* vertexShaderVr = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
uniform mat4 view;
uniform mat4 projection;
void main() {
    vTexCoord = aTexCoord;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";
const char* vertexShaderFrame = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

glm::vec3 CUBE_VERTICES[26] = {
    glm::vec3 { -1, -1, -1 },
    glm::vec3 { 1, -1, -1 },
    glm::vec3 { 1, 1, -1 },
    glm::vec3 { -1, 1, -1 },
    glm::vec3 { -1, -1, 1 },
    glm::vec3 { 1, -1, 1 },
    glm::vec3 { 1, 1, 1 },
    glm::vec3 { -1, 1, 1 },
    glm::vec3 { 0, -1, -1 },
    glm::vec3 { 1, 0, -1 },
    glm::vec3 { 0, 1, -1 },
    glm::vec3 { -1, 0, -1 },
    glm::vec3 { 0, 0, -1 },
    glm::vec3 { 1, -1, 0 },
    glm::vec3 { 1, 0, 1 },
    glm::vec3 { 1, 1, 0 },
    glm::vec3 { 1, 0, 0 },
    glm::vec3 { 0, -1, 1 },
    glm::vec3 { 0, 1, 1 },
    glm::vec3 { -1, 0, 1 },
    glm::vec3 { 0, 0, 1 },
    glm::vec3 { -1, -1, 0 },
    glm::vec3 { -1, 1, 0 },
    glm::vec3 { -1, 0, 0 },
    glm::vec3 { 0, -1, 0 },
    glm::vec3 { 0, 1, 0 },
};

// Older version
int SUBFACE_INDICES_VR1[24][4] = {
    // Bottom
    0x3, 0xA, 0xC, 0xB,
    0xA, 0x2, 0x9, 0xC,
    0xC, 0x9, 0x1, 0x8,
    0xB, 0xC, 0x8, 0x0,

    // Front
    0x4, 0x13, 0x17, 0x15,
    0x13, 0x7, 0x16, 0x17,
    0x17, 0x16, 0x3, 0xB,
    0x15, 0x17, 0xB, 0x0,

    // Top
    0x4, 0x11, 0x14, 0x13,
    0x11, 0x5, 0xE, 0x14,
    0x14, 0xE, 0x6, 0x12,
    0x13, 0x14, 0x12, 0x7,

    // Back
    0x6, 0xE, 0x10, 0xF,
    0xE, 0x5, 0xD, 0x10,
    0x10, 0xD, 0x1, 0x9,
    0xF, 0x10, 0x9, 0x2,

    // Right
    0x7, 0x12, 0x19, 0x16,
    0x12, 0x6, 0xF, 0x19,
    0x19, 0xF, 0x2, 0xA,
    0x16, 0x19, 0xA, 0x3,

    // Left
    0x5, 0x11, 0x18, 0xD,
    0x11, 0x4, 0x15, 0x18,
    0x18, 0x15, 0x0, 0x8,
    0xD, 0x18, 0x8, 0x1
};

// Newer version
int SUBFACE_INDICES_VR2[24][4] = {
    // Left
    0x5, 0x11, 0x18, 0xD,
    0x11, 0x4, 0x15, 0x18,
    0x18, 0x15, 0x0, 0x8,
    0xD, 0x18, 0x8, 0x1,

    // Front
    0x4, 0x13, 0x17, 0x15,
    0x13, 0x7, 0x16, 0x17,
    0x17, 0x16, 0x3, 0xB,
    0x15, 0x17, 0xB, 0x0,

    // Back
    0x6, 0xE, 0x10, 0xF,
    0xE, 0x5, 0xD, 0x10,
    0x10, 0xD, 0x1, 0x9,
    0xF, 0x10, 0x9, 0x2,

    // Right
    0x7, 0x12, 0x19, 0x16,
    0x12, 0x6, 0xF, 0x19,
    0x19, 0xF, 0x2, 0xA,
    0x16, 0x19, 0xA, 0x3,

    // Top
    0x6, 0x12, 0x14, 0xE,
    0x12, 0x7, 0x13, 0x14,
    0x14, 0x13, 0x4, 0x11,
    0xE, 0x14, 0x11, 0x5,

    // Bottom
    0x1, 0x8, 0xC, 0x9,
    0x8, 0x0, 0xB, 0xC,
    0xC, 0xB, 0x3, 0xA,
    0x9, 0xC, 0xA, 0x2
};

/* PUBLIC */
RendererOpenGL::RendererOpenGL()
{
    d_ptr = new Impl;
}

RendererOpenGL::~RendererOpenGL()
{
    delete d_ptr;
}

bool RendererOpenGL::init(int width, int height, bool isNewVr, oglLoadFunc oglFct)
{
    if (!gladLoadGL((GLADloadfunc)oglFct)) {
        LOG_CRITICAL("Failed to load GLAD");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    /* VR init */
    std::vector<float> vertices;
    for (int i = 0; i < 24; ++i) {
        float tex_v0 = (i * 256.0f) / 6144.0f;
        float tex_v1 = ((i + 1) * 256.0f) / 6144.0f;

        for (int j = 0; j < 4; ++j) {
            glm::vec3 v;
            if (isNewVr) {
                v = CUBE_VERTICES[SUBFACE_INDICES_VR2[i][j]];
            } else {
                v = CUBE_VERTICES[SUBFACE_INDICES_VR1[i][j]];
            }
            float u = (j == 1 || j == 2) ? 1.0f : 0.0f;
            float vTex = (j == 2 || j == 3) ? tex_v1 : tex_v0;
            vertices.insert(vertices.end(), { v.x, v.y, v.z, u, vTex });
        }
    }

    std::vector<unsigned int> indices;
    for (int i = 0; i < 24; ++i) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }

    glGenVertexArrays(1, &d_ptr->m_vaoVr);
    glBindVertexArray(d_ptr->m_vaoVr);

    glGenBuffers(1, &d_ptr->m_vboVr);
    glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_vboVr);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &d_ptr->m_eboVr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ptr->m_eboVr);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glGenTextures(1, &d_ptr->m_textureVr);
    glBindTexture(GL_TEXTURE_2D, d_ptr->m_textureVr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 6144, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    d_ptr->m_shaderVr = d_ptr->createShaderProgram(vertexShaderVr, fragmentShader);

    /* Frame init */
    float quadVertices[] = {
        -1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &d_ptr->m_vaoFrame);
    glBindVertexArray(d_ptr->m_vaoFrame);

    glGenBuffers(1, &d_ptr->m_vboFrame);
    glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_vboFrame);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &d_ptr->m_eboFrame);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ptr->m_eboFrame);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glGenTextures(1, &d_ptr->m_textureFrame);
    glBindTexture(GL_TEXTURE_2D, d_ptr->m_textureFrame);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    d_ptr->m_shaderFrame = d_ptr->createShaderProgram(vertexShaderFrame, fragmentShader);

    return true;
}

void RendererOpenGL::deinit()
{
    if (d_ptr->m_shaderVr) {
        glDeleteProgram(d_ptr->m_shaderVr);
        d_ptr->m_shaderVr = 0;
    }
    if (d_ptr->m_shaderFrame) {
        glDeleteProgram(d_ptr->m_shaderFrame);
        d_ptr->m_shaderFrame = 0;
    }
    if (d_ptr->m_textureVr) {
        glDeleteTextures(1, &d_ptr->m_textureVr);
        d_ptr->m_textureVr = 0;
    }
    if (d_ptr->m_textureFrame) {
        glDeleteTextures(1, &d_ptr->m_textureFrame);
        d_ptr->m_textureFrame = 0;
    }

    glDeleteVertexArrays(1, &d_ptr->m_vaoVr);
    glDeleteBuffers(1, &d_ptr->m_vboVr);
    glDeleteBuffers(1, &d_ptr->m_eboVr);

    glDeleteVertexArrays(1, &d_ptr->m_vaoFrame);
    glDeleteBuffers(1, &d_ptr->m_vboFrame);
    glDeleteBuffers(1, &d_ptr->m_eboFrame);
}

void RendererOpenGL::updateVr(unsigned short* vr)
{
    glBindTexture(GL_TEXTURE_2D, d_ptr->m_textureVr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 6144, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, vr);
}

void RendererOpenGL::updateFrame(unsigned short* frame)
{
    glBindTexture(GL_TEXTURE_2D, d_ptr->m_textureFrame);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frame);
}

void RendererOpenGL::renderVr(int width, int height, float yaw, float pitch, float roll, float fov)
{
    // Update camera
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view, glm::radians(roll), glm::vec3(0, 1, 0));
    view = glm::rotate(view, -glm::radians(pitch), glm::vec3(1, 0, 0));
    view = glm::rotate(view, glm::radians(yaw), glm::vec3(0, 0, 1));

    glm::mat4 projection = glm::perspective(fov, (float)width / (float)height, 0.1f, 100.0f);

    // Render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Send to shader
    GLuint shaderProgram = d_ptr->m_shaderVr;
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);

    glBindVertexArray(d_ptr->m_vaoVr);
    glDrawElements(GL_TRIANGLES, 144, GL_UNSIGNED_INT, 0);
}

void RendererOpenGL::renderFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(d_ptr->m_shaderFrame);
    glBindVertexArray(d_ptr->m_vaoFrame);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, d_ptr->m_textureFrame);
    glUniform1i(glGetUniformLocation(d_ptr->m_shaderFrame, "tex"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

} // namespace ofnx::graphics
