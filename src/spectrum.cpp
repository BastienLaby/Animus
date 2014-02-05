#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <time.h>
#include <sstream>

#include <vector>

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "GL/glfw.h"
#include "stb/stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imguiRenderGL3.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "ShaderTools.hpp"
#include "IMGUITools.hpp"
#include "SoundTools.hpp"
#include "LightManager.hpp"
#include "CameraManager.hpp"

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#if DEBUG_PRINT == 0
#define debug_print(FORMAT, ...) ((void)0)
#else
#ifdef _MSC_VER
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __func__, __FILE__, __LINE__, __VA_ARGS__)
#endif
#endif



int main( int argc, char **argv )
{

    //
    // INITIALISATION
    //

    int width = 1920, height = 1080;
    float widthf = (float) width, heightf = (float) height;
    double t;
    float fps = 0.f;

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    // Open a window and create its OpenGL context
    if( !glfwOpenWindow( width, height, 0,0,0,0, 24, 0, GLFW_FULLSCREEN ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );

        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    glfwEnable( GLFW_MOUSE_CURSOR );
    glfwSetWindowTitle( "Animus" );

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
          /* Problem: glewInit failed, something is seriously wrong. */
          fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
          exit( EXIT_FAILURE );
    }

    // Ensure we can capture the escape key being pressed below
    glfwEnable( GLFW_STICKY_KEYS );

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );

    //
    // Init imgui
    //

    init_imgui();
    GUIStates guiStates;
    init_gui_states(guiStates);

    //
    // Init Cameras
    //

    CameraManager cammg;
    int idCam1 = cammg.createCamera();
    int idCam2 = cammg.createCamera();
    int idCam3 = cammg.createCamera();
    cammg.switchTo(idCam1);

    //
    // Init textures
    //

    // Load images and upload textures
    GLuint textures[3];
    glGenTextures(3, textures);
    int x, y, comp;

    unsigned char * diffuse = stbi_load("textures/spnza_bricks_a_diff.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char * spec = stbi_load("textures/spnza_bricks_a_spec.tga", &x, &y, &comp, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y, 0, GL_RED, GL_UNSIGNED_BYTE, spec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //
    // Try to load and compile shader
    //

    // Gbuffer

    int status;
    ShaderGLSL gbuffer_shader;
    const char * shaderFileGBuffer = "src/spectrum_gbuffer.glsl";
    status = load_shader_from_file(gbuffer_shader, shaderFileGBuffer, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileGBuffer);
        exit( EXIT_FAILURE );
    }    

    GLuint gbuffer_projectionLocation = glGetUniformLocation(gbuffer_shader.program, "Projection");
    GLuint gbuffer_viewLocation = glGetUniformLocation(gbuffer_shader.program, "View");
    GLuint gbuffer_objectLocation = glGetUniformLocation(gbuffer_shader.program, "Object");
    GLuint gbuffer_timeLocation = glGetUniformLocation(gbuffer_shader.program, "Time");
    GLuint gbuffer_diffuseLocation = glGetUniformLocation(gbuffer_shader.program, "Diffuse");
    GLuint gbuffer_specLocation = glGetUniformLocation(gbuffer_shader.program, "Spec");
    GLuint gbuffer_spectrumOffsetLocation = glGetUniformLocation(gbuffer_shader.program, "SpectrumOffset");
    GLuint gbuffer_renderLightModel = glGetUniformLocation(gbuffer_shader.program, "RenderLightModel");

    // Blit shader

    ShaderGLSL blit_shader;
    const char * shaderFileBlit = "src/2_blit.glsl";
    status = load_shader_from_file(blit_shader, shaderFileBlit, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileBlit);
        exit( EXIT_FAILURE );
    }    

    GLuint blit_tex1Location = glGetUniformLocation(blit_shader.program, "Texture1");

    // Pointlight buffer

    ShaderGLSL pointlight_shader;
    const char * shaderPointLighting = "src/pointlight.glsl";
    status = load_shader_from_file(pointlight_shader, shaderPointLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderPointLighting);
        exit( EXIT_FAILURE );
    }    

    GLuint pointlight_materialLocation = glGetUniformLocation(pointlight_shader.program, "Material");
    GLuint pointlight_normalLocation = glGetUniformLocation(pointlight_shader.program, "Normal");
    GLuint pointlight_depthLocation = glGetUniformLocation(pointlight_shader.program, "Depth");
    GLuint pointlight_timeLocation = glGetUniformLocation(pointlight_shader.program, "Time");
    GLuint pointlight_inverseViewProjectionLocation = glGetUniformLocation(pointlight_shader.program, "InverseViewProjection");
    GLuint pointlight_cameraPositionLocation = glGetUniformLocation(pointlight_shader.program, "CameraPosition");
    GLuint pointlight_lightPositionLocation = glGetUniformLocation(pointlight_shader.program, "LightPosition");
    GLuint pointlight_lightDiffuseColorLocation = glGetUniformLocation(pointlight_shader.program, "LightDiffuseColor");
    GLuint pointlight_lightSpecularColorLocation = glGetUniformLocation(pointlight_shader.program, "LightSpecularColor");
    GLuint pointlight_lightIntensityLocation = glGetUniformLocation(pointlight_shader.program, "LightIntensity");
    GLuint pointlight_projectionLocation = glGetUniformLocation(pointlight_shader.program, "Projection");
    
    // Directionnal buffer

    ShaderGLSL dirlight_shader;
    const char * shaderDirLighting = "src/dirlight.glsl";
    status = load_shader_from_file(dirlight_shader, shaderDirLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderDirLighting);
        exit( EXIT_FAILURE );
    }    

    GLuint dirlight_materialLocation = glGetUniformLocation(dirlight_shader.program, "Material");
    GLuint dirlight_normalLocation = glGetUniformLocation(dirlight_shader.program, "Normal");
    GLuint dirlight_depthLocation = glGetUniformLocation(dirlight_shader.program, "Depth");
    GLuint dirlight_timeLocation = glGetUniformLocation(dirlight_shader.program, "Time");
    GLuint dirlight_inverseViewProjectionLocation = glGetUniformLocation(dirlight_shader.program, "InverseViewProjection");
    GLuint dirlight_cameraPositionLocation = glGetUniformLocation(dirlight_shader.program, "CameraPosition");
    GLuint dirlight_lightDirectionLocation = glGetUniformLocation(dirlight_shader.program, "LightDirection");
    GLuint dirlight_lightDiffuseColorLocation = glGetUniformLocation(dirlight_shader.program, "LightDiffuseColor");
    GLuint dirlight_lightSpecularColorLocation = glGetUniformLocation(dirlight_shader.program, "LightSpecularColor");
    GLuint dirlight_lightIntensityLocation = glGetUniformLocation(dirlight_shader.program, "LightIntensity");
    GLuint dirlight_projectionLocation = glGetUniformLocation(dirlight_shader.program, "Projection");

    // Spotlight

    ShaderGLSL spotlight_shader;
    const char * shaderSpotLighting = "src/spotlight.glsl";
    status = load_shader_from_file(spotlight_shader, shaderSpotLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderSpotLighting);
        exit( EXIT_FAILURE );
    }    

    GLuint spotlight_materialLocation = glGetUniformLocation(spotlight_shader.program, "Material");
    GLuint spotlight_normalLocation = glGetUniformLocation(spotlight_shader.program, "Normal");
    GLuint spotlight_depthLocation = glGetUniformLocation(spotlight_shader.program, "Depth");
    GLuint spotlight_timeLocation = glGetUniformLocation(spotlight_shader.program, "Time");
    GLuint spotlight_inverseViewProjectionLocation = glGetUniformLocation(spotlight_shader.program, "InverseViewProjection");
    GLuint spotlight_cameraPositionLocation = glGetUniformLocation(spotlight_shader.program, "CameraPosition");
    GLuint spotlight_lightPositionLocation = glGetUniformLocation(spotlight_shader.program, "LightPosition");
    GLuint spotlight_lightDirectionLocation = glGetUniformLocation(spotlight_shader.program, "LightDirection");
    GLuint spotlight_lightDiffuseColorLocation = glGetUniformLocation(spotlight_shader.program, "LightDiffuseColor");
    GLuint spotlight_lightSpecularColorLocation = glGetUniformLocation(spotlight_shader.program, "LightSpecularColor");
    GLuint spotlight_lightIntensityLocation = glGetUniformLocation(spotlight_shader.program, "LightIntensity");
    GLuint spotlight_lightExternalAngleLocation = glGetUniformLocation(spotlight_shader.program, "LightExternalAngle");
    GLuint spotlight_lightInternalAngleLocation = glGetUniformLocation(spotlight_shader.program, "LightInternalAngle");
    GLuint spotlight_projectionLocation = glGetUniformLocation(spotlight_shader.program, "Projection");

    // Load geometry
    int   cube_triangleCount = 12;
    int   cube_triangleList[] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 19, 17, 20, 21, 22, 23, 24, 25, 26, };
    float cube_uvs[] = {0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f, 0.f, 0.f, 1.f, 1.f,  1.f, 0.f,  };
    float cube_vertices[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5 };
    float cube_normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, };
    int   quad_triangleCount = 2;
    int   quad_triangleList[] = {0, 1, 2, 2, 1, 3}; 
    float quad_vertices[] =  {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
    int   plane_triangleCount = 2;
    int   plane_triangleList[] = {0, 1, 2, 2, 1, 3}; 
    float plane_uvs[] = {0.f, 0.f, 0.f, 10.f, 10.f, 0.f, 10.f, 10.f};
    float plane_vertices[] = {-50.0, -1.0, 50.0, 50.0, -1.0, 50.0, -50.0, -1.0, -50.0, 50.0, -1.0, -50.0};
    float plane_normals[] = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};

    // Vertex Array Object
    GLuint vao[3];
    glGenVertexArrays(3, vao);

    // Vertex Buffer Objects
    GLuint vbo[12];
    glGenBuffers(12, vbo);

    // Cube
    glBindVertexArray(vao[0]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_triangleList), cube_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_uvs), cube_uvs, GL_STATIC_DRAW);

    // Plane
    glBindVertexArray(vao[1]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_triangleList), plane_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_normals), plane_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_uvs), plane_uvs, GL_STATIC_DRAW);

    // Quad
    glBindVertexArray(vao[2]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_triangleList), quad_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    // Unbind everything. Potentially illegal on some implementations
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Init frame buffers
    GLuint gbufferFbo;
    GLuint gbufferTextures[3];
    GLuint gbufferDrawBuffers[2];
    glGenTextures(3, gbufferTextures);

    // Create color texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create normal texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create depth texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create Framebuffer Object
    glGenFramebuffers(1, &gbufferFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);

    // Attach textures to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, gbufferTextures[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, gbufferTextures[1], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbufferTextures[2], 0);
    gbufferDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    gbufferDrawBuffers[1] = GL_COLOR_ATTACHMENT1;
    // Note : la profondeur se remplit automatiquement.

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Error on building framebuffer\n");
        exit( EXIT_FAILURE );
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Light data
    srand (time(NULL));
    LightManager lightManager;

    // Animation Data
    int spectrumStepLoop = 0;

    // GUI Data
    bool showDeferrefTextures = false;
    int showUI = true;

    //
    // SOUND
    //

    SoundManager soundManager;
    unsigned int idSound1 = soundManager.createSound("sounds/sound1.wav");
    soundManager.playSound(idSound1);
    unsigned int maxSpectrumSize = 1024;
    unsigned int currentSpectrumSize = maxSpectrumSize;

    int spaceKeyState = glfwGetKey(GLFW_KEY_SPACE);

    do
    {

        t = glfwGetTime();

        //
        // Handle User inputs
        //

        // Mouse states
        int leftButton = glfwGetMouseButton( GLFW_MOUSE_BUTTON_LEFT );
        int rightButton = glfwGetMouseButton( GLFW_MOUSE_BUTTON_RIGHT );
        int middleButton = glfwGetMouseButton( GLFW_MOUSE_BUTTON_MIDDLE );

        if( leftButton == GLFW_PRESS )
            guiStates.turnLock = true;
        else
            guiStates.turnLock = false;

        if( rightButton == GLFW_PRESS )
            guiStates.zoomLock = true;
        else
            guiStates.zoomLock = false;

        if( middleButton == GLFW_PRESS )
            guiStates.panLock = true;
        else
            guiStates.panLock = false;

        int spacePressed = glfwGetKey(GLFW_KEY_SPACE);
        if(spacePressed!= spaceKeyState)
        {
            showUI = !showUI;
        }

        // Camera movements
        int altPressed = glfwGetKey(GLFW_KEY_LSHIFT);
        if (!altPressed && (leftButton == GLFW_PRESS || rightButton == GLFW_PRESS || middleButton == GLFW_PRESS))
        {
            int x; int y;
            glfwGetMousePos(&x, &y);
            guiStates.lockPositionX = x;
            guiStates.lockPositionY = y;
        }
        if (altPressed == GLFW_PRESS)
        {
            int mousex; int mousey;
            glfwGetMousePos(&mousex, &mousey);
            int diffLockPositionX = mousex - guiStates.lockPositionX;
            int diffLockPositionY = mousey - guiStates.lockPositionY;
            if (guiStates.zoomLock)
            {
                float zoomDir = 0.0;
                if (diffLockPositionX > 0)
                    zoomDir = -1.f;
                else if (diffLockPositionX < 0 )
                    zoomDir = 1.f;
                cammg.zoom(zoomDir * GUIStates::MOUSE_ZOOM_SPEED);
            }
            else if (guiStates.turnLock)
            {
                cammg.turn(diffLockPositionY * GUIStates::MOUSE_TURN_SPEED, diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

            }
            else if (guiStates.panLock)
            {
                cammg.pan(diffLockPositionX * GUIStates::MOUSE_PAN_SPEED, diffLockPositionY * GUIStates::MOUSE_PAN_SPEED);
            }
            guiStates.lockPositionX = mousex;
            guiStates.lockPositionY = mousey;
        }
  
        //
        // Get camera matrices
        //

        glm::mat4 projection = glm::perspective(45.0f, widthf / heightf, 0.1f, 1000.f); 
        glm::mat4 worldToView = glm::lookAt(cammg.getEye(), cammg.getOrigin(), cammg.getUp());
        glm::mat4 objectToWorld;
        glm::mat4 worldToScreen = projection * worldToView;
        glm::mat4 screenToWorld = glm::transpose(glm::inverse(worldToScreen));

        //
        // Render into framebuffer
        //

        glViewport( 0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(gbuffer_shader.program);
        
        glUniformMatrix4fv(gbuffer_projectionLocation, 1, 0, glm::value_ptr(projection));
        glUniformMatrix4fv(gbuffer_viewLocation, 1, 0, glm::value_ptr(worldToView));
        glUniformMatrix4fv(gbuffer_objectLocation, 1, 0, glm::value_ptr(objectToWorld));
        glUniform1i(gbuffer_diffuseLocation, 0);
        glUniform1i(gbuffer_specLocation, 1);
        glUniform1f(gbuffer_timeLocation, t);

        // Bind personnal buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activer la liste des buffers dans lesquels dessiner
        glDrawBuffers(2, gbufferDrawBuffers);
        
        // Rendu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        glBindVertexArray(vao[0]);
        
        if(spectrumStepLoop < 3) { spectrumStepLoop++; }
        else { spectrumStepLoop = 0; }
        float spectrum[maxSpectrumSize];
        soundManager.getSpectrum(idSound1, spectrum, maxSpectrumSize); // Fill the max-size spectrum
        float reduceSpectrum[256]; // To display a spectrum thiner than the max-size spectrum
        int bar = 0;    
        for(unsigned int i = 0; i < 256; i++)
        {
            reduceSpectrum[i] = spectrum[i]*2000;
            int barHeight = (int)reduceSpectrum[i];
            if(barHeight < 1)
                barHeight = 1;                
            glUniform1i(gbuffer_spectrumOffsetLocation, bar);
            bar++;
            glDrawElementsInstanced(GL_TRIANGLES, cube_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, barHeight);    
        }

        //glBindVertexArray(vao[1]);
        //glDrawElements(GL_TRIANGLES, plane_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        // Draw lights representations
        glBindVertexArray(vao[0]);
        glUniform1i(gbuffer_renderLightModel, 1);
        for(size_t i = 0; i < lightManager.getNumPointLight(); i++)
        {
            glUniformMatrix4fv(gbuffer_objectLocation, 1, 0, glm::value_ptr(glm::translate(objectToWorld, lightManager.getPLPosition(i))));
            glDrawElements(GL_TRIANGLES, cube_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }
        glUniform1i(gbuffer_renderLightModel, 0);

         // Debind personnal buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //
        // Rendu de la scène dans un quad plein écran pour chaque lumière
        //

        glViewport( 0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);    
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);

        glBindVertexArray(vao[2]);

        // Pointlights

        glUseProgram(pointlight_shader.program);

        glUniformMatrix4fv(pointlight_projectionLocation, 1, 0, glm::value_ptr(projection));
        glUniform1i(pointlight_materialLocation, 0);
        glUniform1i(pointlight_normalLocation, 1);
        glUniform1i(pointlight_depthLocation, 2);
        glUniform3fv(pointlight_cameraPositionLocation, 1, glm::value_ptr(cammg.getEye()));
        glUniformMatrix4fv(pointlight_inverseViewProjectionLocation, 1, 0, glm::value_ptr(screenToWorld));
        glUniform1f(pointlight_timeLocation, t);

        for (unsigned int i = 0; i < lightManager.getNumPointLight(); ++i)
        {
            glm::vec3 pos = lightManager.getPLPosition(i);
            glm::vec3 diff = lightManager.getPLDiffuse(i);
            glm::vec3 spec = lightManager.getPLSpec(i);
            float lightPosition[3] = {pos.x, pos.y, pos.z};
            float lightDiffuseColor[3] = {diff.r, diff.g, diff.b};
            float lightSpecColor[3] = {spec.r, spec.g, spec.b};
            glUniform3fv(pointlight_lightPositionLocation, 1, lightPosition);
            glUniform3fv(pointlight_lightDiffuseColorLocation, 1, lightDiffuseColor);
            glUniform3fv(pointlight_lightSpecularColorLocation, 1, lightSpecColor);
            glUniform1f(pointlight_lightIntensityLocation, lightManager.getPLIntensity(i));
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }

        // DirLights

        glUseProgram(dirlight_shader.program);
        glUniformMatrix4fv(dirlight_projectionLocation, 1, 0, glm::value_ptr(projection));
        glUniform1i(dirlight_materialLocation, 0);
        glUniform1i(dirlight_normalLocation, 1);
        glUniform1i(dirlight_depthLocation, 2);
        glUniform3fv(dirlight_cameraPositionLocation, 1, glm::value_ptr(cammg.getEye()));
        glUniformMatrix4fv(dirlight_inverseViewProjectionLocation, 1, 0, glm::value_ptr(screenToWorld));
        glUniform1f(dirlight_timeLocation, t);

        for (unsigned int i = 0; i < lightManager.getNumDirLight(); ++i)
        {
            glm::vec3 dir = lightManager.getDLDirection(i);
            glm::vec3 diff = lightManager.getDLDiffuse(i);
            glm::vec3 spec = lightManager.getDLSpec(i);
            float lightDirection[3] = {dir.x, dir.y, dir.z};
            float lightDiffuseColor[3] = {diff.r, diff.g, diff.b};
            float lightSpecColor[3] = {spec.r, spec.g, spec.b};
            glUniform3fv(dirlight_lightDirectionLocation, 1, lightDirection);
            glUniform3fv(dirlight_lightDiffuseColorLocation, 1, lightDiffuseColor);
            glUniform3fv(dirlight_lightSpecularColorLocation, 1, lightSpecColor);
            glUniform1f(dirlight_lightIntensityLocation, lightManager.getDLIntensity(i));
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }

        // SpotLights

        glUseProgram(spotlight_shader.program);
        glUniformMatrix4fv(spotlight_projectionLocation, 1, 0, glm::value_ptr(projection));
        glUniform1i(spotlight_materialLocation, 0);
        glUniform1i(spotlight_normalLocation, 1);
        glUniform1i(spotlight_depthLocation, 2);
        glUniform3fv(spotlight_cameraPositionLocation, 1, glm::value_ptr(cammg.getEye()));
        glUniformMatrix4fv(spotlight_inverseViewProjectionLocation, 1, 0, glm::value_ptr(screenToWorld));
        glUniform1f(spotlight_timeLocation, t);

        for (unsigned int i = 0; i < lightManager.getNumSpotLight(); ++i)
        {
            glm::vec3 pos = lightManager.getSPLPosition(i);
            glm::vec3 dir = lightManager.getSPLDirection(i);
            glm::vec3 diff = lightManager.getSPLDiffuse(i);
            glm::vec3 spec = lightManager.getSPLSpec(i);
            float lightPosition[3] = {pos.x, pos.y, pos.z};
            float lightDirection[3] = {dir.x, dir.y, dir.z};
            float lightDiffuseColor[3] = {diff.r, diff.g, diff.b};
            float lightSpecColor[3] = {spec.r, spec.g, spec.b};
            glUniform3fv(spotlight_lightPositionLocation, 1, lightPosition);
            glUniform3fv(spotlight_lightDirectionLocation, 1, lightDirection);
            glUniform3fv(spotlight_lightDiffuseColorLocation, 1, lightDiffuseColor);
            glUniform3fv(spotlight_lightSpecularColorLocation, 1, lightSpecColor);
            glUniform1f(spotlight_lightIntensityLocation, lightManager.getSPLIntensity(i));
            glUniform1f(spotlight_lightExternalAngleLocation, lightManager.getSPLExternalAngle(i));
            glUniform1f(spotlight_lightInternalAngleLocation, lightManager.getSPLInternalAngle(i));
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }

        glDisable(GL_BLEND);

        //
        // Dessin des textures remplies du FrameBuffer
        //

        if(showDeferrefTextures)
        {
            glDisable(GL_DEPTH_TEST);
            glUseProgram(blit_shader.program);        
            glUniform1i(blit_tex1Location, 0); // Le shader utilisera la texture bindée sur l'unité de texture 0
            glActiveTexture(GL_TEXTURE0);      // On active le bonne unité de texture. Les futurs bind se feront dessus. Donc le shader prendra la texture actuellement bindée.
            // Quad 1/3
            glViewport(0, 0, width/3, height/4);
            glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
            glBindVertexArray(vao[2]); // Pour dessiner le quad
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
            // Quad 2/3
            glViewport(width/3, 0, width/3, height/4);
            glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
            // Quad 3/3
            glViewport(2*width/3, 0, width/3, height/4);
            glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }

#if 1

        if(showUI)
        {
            // Draw UI
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glViewport(0, 0, width, height);

            unsigned char mbut = 0;
            int mscroll = 0;
            int mousex; int mousey;
            glfwGetMousePos(&mousex, &mousey);
            mousey = height - mousey;

            if(leftButton == GLFW_PRESS)
                mbut |= IMGUI_MBUT_LEFT;

            imguiBeginFrame(mousex, mousey, mbut, mscroll);
            int logScroll = 0;
            char lineBuffer[512];
            imguiBeginScrollArea("Animation", 0, height/4, 200, 3*height/4, &logScroll);
            sprintf(lineBuffer, "FPS %f", fps);
            imguiLabel(lineBuffer);

            imguiSeparatorLine();

            int dfs = imguiButton("Show Deferred Shading");
            if(dfs) { showDeferrefTextures = !showDeferrefTextures; }

            imguiSeparatorLine();

            int buttonCamera1 = imguiButton("Camera1");
            int buttonCamera2 = imguiButton("Camera2");
            int buttonCamera3 = imguiButton("Camera3");
            if(buttonCamera1) {
                cammg.switchTo(idCam1);
            } 
            if(buttonCamera2) {
                cammg.switchTo(idCam2);
            }
            if(buttonCamera3) {
                cammg.switchTo(idCam3);
            }
                

            imguiSeparatorLine();

            sprintf(lineBuffer, "Spectrum range : %d", 44100/2);
            imguiLabel(lineBuffer);
            sprintf(lineBuffer, "One bar = %1.1f Hz", ((44100.f/2.f)/currentSpectrumSize));
            imguiLabel(lineBuffer);

            imguiEndScrollArea();

            imguiBeginScrollArea("Lights", 200, height/4, 200, 3*height/4, &logScroll);

            sprintf(lineBuffer, "%d PointLights", lightManager.getNumPointLight());
            imguiLabel(lineBuffer);
            int button_addPL = imguiButton("Add PointLight");
            if(button_addPL)
            {
                unsigned int nbL = lightManager.getNumPointLight();
                srand(time(NULL));
                lightManager.addPointLight( glm::vec3(0, 15, 5),
                                            glm::vec3(cos(nbL), sin(nbL), 1),
                                            glm::vec3(1, 1, 1),
                                            5.f);
            }

            for (unsigned int i = 0; i < lightManager.getNumPointLight(); ++i)
            {
                std::ostringstream ss;
                ss << (i+1);
                std::string s(ss.str());
                int toggle = imguiCollapse("Point Light", s.c_str(), lightManager.getPLCollapse(i));
                if(lightManager.getPLCollapse(i))
                {
                    imguiIndent();
                        float intens = lightManager.getPLIntensity(i);
                        imguiSlider("Intensity", &intens, 0, 10, 0.1); lightManager.setPLIntensity(i, intens);
                        imguiLabel("Position :");
                        imguiIndent();
                            glm::vec3 pos = lightManager.getPLPosition(i);
                            imguiSlider("x", &pos.x, -30, 30, 0.1);
                            imguiSlider("y", &pos.y, -30, 30, 0.1);
                            imguiSlider("z", &pos.z, -30, 30, 0.1);
                            lightManager.setPLPosition(i, pos);
                        imguiUnindent();
                        imguiLabel("Diffuse :");
                        imguiIndent();
                            glm::vec3 diff = lightManager.getPLDiffuse(i);
                            imguiSlider("r", &diff.x, 0, 1, 0.01);
                            imguiSlider("g", &diff.y, 0, 1, 0.01);
                            imguiSlider("b", &diff.z, 0, 1, 0.01);
                            lightManager.setPLDiffuse(i, diff);
                        imguiUnindent();
                        imguiLabel("Specular :");
                        imguiIndent();
                            glm::vec3 spec = lightManager.getPLSpec(i);
                            imguiSlider("r", &spec.x, 0, 1, 0.01);
                            imguiSlider("g", &spec.y, 0, 1, 0.01);
                            imguiSlider("b", &spec.z, 0, 1, 0.01);
                            lightManager.setPLSpec(i, spec);
                        imguiUnindent();
                        int removeLight = imguiButton("Remove"); 
                        if(removeLight)
                            lightManager.removePointLight(i);
                    imguiUnindent();
                }
                if(toggle) { lightManager.setPLCollapse(i, !lightManager.getPLCollapse(i)); }
            }

            imguiSeparatorLine();

            int button_addDL = imguiButton("Add DirLight");
            if(button_addDL)
            {
                unsigned int nbL = lightManager.getNumDirLight();
                lightManager.addDirLight( glm::vec3(0.5, -0.5, -0.5),
                                            glm::vec3(1, 1, 1),
                                            glm::vec3(1, 1, 0.5),
                                            0.6f);
            }

            for (unsigned int i = 0; i < lightManager.getNumDirLight(); ++i)
            {
                std::ostringstream ss;
                ss << (i+1);
                std::string s(ss.str());
                int toggle = imguiCollapse("Dir Light", s.c_str(), lightManager.getDLCollapse(i));
                if(lightManager.getDLCollapse(i))
                {
                    imguiIndent();
                        float intens = lightManager.getDLIntensity(i);
                        imguiSlider("Intensity", &intens, 0, 10, 0.1); lightManager.setDLIntensity(i, intens);
                        imguiLabel("Direction :");
                        imguiIndent();
                            glm::vec3 dir = lightManager.getDLDirection(i);
                            imguiSlider("x", &dir.x, -30, 30, 0.1);
                            imguiSlider("y", &dir.y, -30, 30, 0.1);
                            imguiSlider("z", &dir.z, -30, 30, 0.1);
                            lightManager.setDLDirection(i, dir);
                        imguiUnindent();
                        imguiLabel("Diffuse :");
                        imguiIndent();
                            glm::vec3 diff = lightManager.getDLDiffuse(i);
                            imguiSlider("r", &diff.x, 0, 1, 0.01);
                            imguiSlider("g", &diff.y, 0, 1, 0.01);
                            imguiSlider("b", &diff.z, 0, 1, 0.01);
                            lightManager.setDLDiffuse(i, diff);
                        imguiUnindent();
                        imguiLabel("Specular :");
                        imguiIndent();
                            glm::vec3 spec = lightManager.getDLSpec(i);
                            imguiSlider("r", &spec.x, 0, 1, 0.01);
                            imguiSlider("g", &spec.y, 0, 1, 0.01);
                            imguiSlider("b", &spec.z, 0, 1, 0.01);
                            lightManager.setDLSpec(i, spec);
                        imguiUnindent();
                        int removeLight = imguiButton("Remove"); 
                        if(removeLight)
                            lightManager.removeDirLight(i);
                    imguiUnindent();
                }
                if(toggle) { lightManager.setDLCollapse(i, !lightManager.getDLCollapse(i)); }
            }

            imguiSeparatorLine();

            int button_addSPL = imguiButton("Add SpotLight");
            if(button_addSPL)
            {
                unsigned int nbL = lightManager.getNumSpotLight();
                lightManager.addSpotLight( glm::vec3(5, 5, 5),
                                            glm::vec3(-1, -1, -1),
                                            glm::vec3(1, 1, 0),
                                            glm::vec3(1, 0, 1),
                                            2.f,
                                            1.f,
                                            1.f);
            }

            for (unsigned int i = 0; i < lightManager.getNumSpotLight(); ++i)
            {
                std::ostringstream ss;
                ss << (i+1);
                std::string s(ss.str());
                int toggle = imguiCollapse("Spot Light", s.c_str(), lightManager.getSPLCollapse(i));
                if(lightManager.getSPLCollapse(i))
                {
                    imguiIndent();
                        float intens = lightManager.getSPLIntensity(i);
                        imguiSlider("Intensity", &intens, 0, 10, 0.1); lightManager.setSPLIntensity(i, intens);
                        imguiLabel("Position :");
                        imguiIndent();
                            glm::vec3 pos = lightManager.getSPLPosition(i);
                            imguiSlider("x", &pos.x, -30, 30, 0.1);
                            imguiSlider("y", &pos.y, -30, 30, 0.1);
                            imguiSlider("z", &pos.z, -30, 30, 0.1);
                            lightManager.setSPLPosition(i, pos);
                        imguiUnindent();
                        imguiLabel("Direction :");
                        imguiIndent();
                            glm::vec3 dir = lightManager.getSPLDirection(i);
                            imguiSlider("x", &dir.x, -1, 1, 0.01);
                            imguiSlider("y", &dir.y, -1, 1, 0.01);
                            imguiSlider("z", &dir.z, -1, 1, 0.01);
                            lightManager.setSPLDirection(i, dir);
                        imguiUnindent();
                        imguiLabel("Diffuse :");
                        imguiIndent();
                            glm::vec3 diff = lightManager.getSPLDiffuse(i);
                            imguiSlider("r", &diff.x, 0, 1, 0.01);
                            imguiSlider("g", &diff.y, 0, 1, 0.01);
                            imguiSlider("b", &diff.z, 0, 1, 0.01);
                            lightManager.setSPLDiffuse(i, diff);
                        imguiUnindent();
                        imguiLabel("Specular :");
                        imguiIndent();
                            glm::vec3 spec = lightManager.getSPLSpec(i);
                            imguiSlider("r", &spec.x, 0, 1, 0.01);
                            imguiSlider("g", &spec.y, 0, 1, 0.01);
                            imguiSlider("b", &spec.z, 0, 1, 0.01);
                            lightManager.setSPLSpec(i, spec);
                        imguiUnindent();
                        float extangle = lightManager.getSPLExternalAngle(i);
                        imguiSlider("External angle", &extangle, 0, 2, 0.01); lightManager.setSPLExternalAngle(i, extangle);
                        float intangle = lightManager.getSPLInternalAngle(i);
                        imguiSlider("Internal angle", &intangle, 0, 2, 0.01); lightManager.setSPLInternalAngle(i, intangle);
                        int removeLight = imguiButton("Remove"); 
                        if(removeLight)
                            lightManager.removeSpotLight(i);
                    imguiUnindent();
                }
                if(toggle) { lightManager.setSPLCollapse(i, !lightManager.getSPLCollapse(i)); }
            }

            imguiEndScrollArea();

            imguiEndFrame();
            
            imguiRenderGLDraw(width, height); 

            glDisable(GL_BLEND);
        }
        
#endif
        
        // Check for errors
        GLenum err = glGetError();
        if(err != GL_NO_ERROR)
        {
            fprintf(stderr, "OpenGL Error : %s\n", gluErrorString(err));
            
        }

        // Swap buffers
        glfwSwapBuffers();
        double newTime = glfwGetTime();
        fps = 1.f/ (newTime - t);
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
           glfwGetWindowParam( GLFW_OPENED ) );

    // Clean UI
    imguiRenderGLDestroy();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    exit( EXIT_SUCCESS );
}
