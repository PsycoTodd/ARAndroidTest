//
// Created by XING on 4/8/2017.
//

#include "Teapot.h"
#include "myShader.h"
#include <opencv2/opencv.hpp>
#include <myJNIHelper.h>
#include <string>

//#define LIGHT_MODEL 1

/**
 * Template for debugging purpose.
 */
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

/**
 * Try to initialize our graphics resoruce and gl resources here.
 */
Teapot::Teapot() {
    _loadSuccess = false;
    _verticesNumber = TEAPOT_VERTS.size() / 3;
    _indicesNumber = TEAPOT_INDICES.size();
    InitShaders();
    setVerts();
    setTexCoords();
    setNorms();
    setIndices();
#ifndef  LIGHT_MODEL
    LoadTexture();
#endif

    CheckGLError("Teaport::Init");
    _loadSuccess = true;
}

/**
 * Clean up the graphics resources.
 */
Teapot::~Teapot()
{

}

/**
 * Compile the opengl shader code and binding the CPU/GPU variables.
 */
void
Teapot::InitShaders(){
    // shader related setup -- loading, attribute and uniform locations
#ifdef LIGHT_MODEL
    std::string vertexShader    = "shaders/modelLight.vsh";
    std::string fragmentShader  = "shaders/modelLight.fsh";
    _shaderProgramID         = LoadShaders(vertexShader, fragmentShader);
    _vertexAttribute         = GetAttributeLocation(_shaderProgramID, "vertexPosition");
    _normalAttribute         = GetAttributeLocation(_shaderProgramID, "vertexNormal");
    _mvpLocation             = GetUniformLocation(_shaderProgramID, "mvpMat");
    _mvLocation              = GetUniformLocation(_shaderProgramID, "mvMat");
    _lightPos                = GetUniformLocation(_shaderProgramID, "lightPos");
#else
    std::string vertexShader    = "shaders/modelTextured.vsh";
    std::string fragmentShader  = "shaders/modelTextured.fsh";
    _shaderProgramID         = LoadShaders(vertexShader, fragmentShader);
    _vertexAttribute         = GetAttributeLocation(_shaderProgramID, "vertexPosition");
    _vertexUVAttribute       = GetAttributeLocation(_shaderProgramID, "vertexUV");
    _mvpLocation             = GetUniformLocation(_shaderProgramID, "mvpMat");
    _textureSamplerLocation  = GetUniformLocation(_shaderProgramID, "textureSampler");
#endif

}


/**
 * Load the teapot texture resource from the package.
 */
void
Teapot::LoadTexture() {
    std::string assetPath = "shaders/TextureTeapotBrass.png";
    std::string texFilePath;
    gHelperObject->ExtractAssetReturnFilename(assetPath.c_str(), texFilePath);
    // Since we have openCV we use it to load image.
    MyLOGI("Loading texture %s", texFilePath.c_str());
    cv::Mat texMat = cv::imread(texFilePath);
    if(!texMat.empty())
    {
        // Convert BGR to RGB.
        cv::cvtColor(texMat, texMat, CV_BGR2RGB);
        cv::flip(texMat, texMat, 0);
        // Bind the texture.
        GLuint * textureGLNames = new GLuint[1];
        glGenTextures(1, textureGLNames);
        glBindTexture(GL_TEXTURE_2D, textureGLNames[0]);
        _textureID = textureGLNames[0];
        // Set the linear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // load the OpenCV Mat into GLES
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texMat.cols,
                     texMat.rows, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     texMat.data);
        CheckGLError("Teapot::loadGLTexGen");
        delete[] textureGLNames;
        return;
    }
    else{
        MyLOGE("Cannot load the texture %s", texFilePath.c_str());
    }




}

/**
 * Render the teapot with lighting, need model view matrix also besides MVP.
 */
void
Teapot::Render(glm::mat4 *mvpMat, glm::mat4 *mvMat)
{
    if(_loadSuccess == false)
    {
        return;
    }
    glUseProgram(_shaderProgramID);
    //Set our shader variables.
    glUniformMatrix4fv(_mvpLocation, 1, GL_FALSE, (const GLfloat*) mvpMat);
    glUniformMatrix4fv(_mvLocation, 1, GL_FALSE, (const GLfloat*)mvMat);
    // Pass in the light position
    glUniform3f(_lightPos, 0.0f, 1.0f, 1.0f);
    CheckGLError("Teapot: light pos");
    // 1. Indices buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indBuffer);
    CheckGLError("Teapot: 1");

    // 2. Vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, _vBuffer);
    glEnableVertexAttribArray(_vertexAttribute);
    glVertexAttribPointer(_vertexAttribute, 3, GL_FLOAT, 0, 0, 0);
    CheckGLError("Teapot: 2");

    // 3. Normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
    glEnableVertexAttribArray(_normalAttribute);
    glVertexAttribPointer(_normalAttribute, 3, GL_FLOAT, 0, 0, 0);
    CheckGLError("Teapot: 3");

    glDrawElements(GL_TRIANGLES, _verticesNumber * 3, GL_UNSIGNED_INT, 0);
    // Release the buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/**
 * Main render function which we used to draw our teapot.
 * mvpMat is used to update the model place.
 */
void
Teapot::Render(glm::mat4 *mvpMat)
{
    // Draw the teaport with texture.
    if(_loadSuccess == false)
    {
        return;
    }
  /*  MyLOGD("HAHAHAHAHAHA:");
    MyLOGD(to_string(TEAPOT_TEX_COORDS.size()).c_str());
    MyLOGD(to_string(TEAPOT_INDICES.size()).c_str());
    MyLOGD(to_string(TEAPOT_NORMS.size()).c_str() );
    MyLOGD(to_string(TEAPOT_VERTS.size()).c_str());
    */
    // Make sure we use the correct vertex and fragment shader.
    glUseProgram(_shaderProgramID);
    //Set our shader variables.
    glUniformMatrix4fv(_mvpLocation, 1, GL_FALSE, (const GLfloat*) mvpMat);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(_textureSamplerLocation, 0);
    CheckGLError("Teapot: setTextureResource()");

    // Now we start to binding our resources.
    // 1. Binding our texture.
    glBindTexture(GL_TEXTURE_2D, _textureID);
    CheckGLError("Teapot: 1");

    // 2. Indices buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indBuffer);
    CheckGLError("Teapot: 2");

    // 3. Vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, _vBuffer);
    glEnableVertexAttribArray(_vertexAttribute);
    glVertexAttribPointer(_vertexAttribute, 3, GL_FLOAT, 0, 0, 0);
    CheckGLError("Teapot: 3");
    // 3.5 ignore normal first.
    // 4. UV
    glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
    glEnableVertexAttribArray(_vertexUVAttribute);
    glVertexAttribPointer(_vertexUVAttribute, 3, GL_FLOAT, 0, 0, 0);
    CheckGLError("Teapot: 4");


    glDrawElements(GL_TRIANGLES, _indicesNumber, GL_UNSIGNED_INT, 0);

    // Release the buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}



void
Teapot::setVerts()
{
    // Now we try to generate OpenGL buffer.
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * _verticesNumber, &TEAPOT_VERTS[0], GL_STATIC_DRAW);
    _vBuffer = buffer;

}


void
Teapot::setTexCoords()
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) *  2 * _verticesNumber, &TEAPOT_TEX_COORDS[0], GL_STATIC_DRAW);
    _uvBuffer = buffer;
}


void
Teapot::setNorms()
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) *  3 * _verticesNumber, &TEAPOT_NORMS[0], GL_STATIC_DRAW);
    _uvBuffer = buffer;
}


void
Teapot::setIndices()
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * _indicesNumber, &TEAPOT_INDICES[0], GL_STATIC_DRAW);
   _indBuffer = buffer;
}



