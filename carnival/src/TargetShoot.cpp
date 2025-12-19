///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// Based on Song Ho Cylinder Example
// dependency: freeglut/glut
//
// AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2017-11-02
// UPDATED: 2019-12-02
///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h> // For wglSwapInterval
#endif

#define FREEGLUT_STATIC
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Matrices.h"

#include "CubeMesh.h"
#include "QuadMesh.h"
#include "SineWaveStrip.h"
#include "DuckTarget.h"
#include "Gun.h"

#include "SOIL.h"

// SDL library for when bullet hits metal target
#include <SDL3/SDL.h>

// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void moveGun(int x, int y);
void initGL();
void InitGLEW();
bool initGLSL();
int  initGLUT(int argc, char** argv);
bool initGlobalVariables();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void animationHandler(int param);
// function for initializing sounds for when duck is shot 
void initSounds();
void toPerspective();
// function for loading textures for booth and mesh (ground)
void loadTextures();
// function for shooting gun which flattens duck on hit and plays audio sound 
void shootGun(int value);

// constants
const int   SCREEN_WIDTH = 900;
const int   SCREEN_HEIGHT = 600;
const float CAMERA_DISTANCE = 24.0f;
const int   TEXT_WIDTH = 8;
const int   TEXT_HEIGHT = 13;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Vertex Shaders source for bullseye target 
const char* vsBullseyeSource = R"(
// GLSL version
#version 110

varying vec3 normal;
varying vec3 position;
varying vec3 bullsEyeCenter;

uniform vec3 targetCenter;

void main(void) {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    // all passed in eye coords 
    position = vec3(gl_ModelViewMatrix * gl_Vertex);
    normal = normalize(gl_NormalMatrix * gl_Normal);
    bullsEyeCenter = vec3(gl_ModelViewMatrix * vec4(targetCenter, 1.0));
}
)";

// Fragment Shaders source for bullseye target
const char* fsBullseyeSource = R"(
// GLSL version
#version 110

varying vec3 normal;
varying vec3 position;
varying vec3 bullsEyeCenter;

// uniforms
uniform vec4 lightPosition;             // should be in the eye space
uniform vec4 lightAmbient;              // light ambient color
uniform vec4 lightDiffuse;              // light diffuse color
uniform vec4 lightSpecular;             // light specular color
uniform vec4 materialAmbient;           // material ambient color
uniform vec4 materialDiffuse;           // material diffuse color
uniform vec4 materialSpecular;          // material specular color
uniform float materialShininess;        // material specular shininess

void main()
{
    // get color of duck in lighting 
    vec3 norm = normalize(normal);
    vec3 light;
    if(lightPosition.w == 0.0)
    {
        light = normalize(lightPosition.xyz);
    }
    else
    {
        light = normalize(lightPosition.xyz - position);
    }
    vec3 view = normalize(-position);
    vec3 halfv = normalize(light + view);

    vec3 color = lightAmbient.rgb * materialAmbient.rgb;        // begin with ambient
    float dotNL = max(dot(norm, light), 0.0);
    color += lightDiffuse.rgb * materialDiffuse.rgb * dotNL;    // add diffuse
                  // modulate texture map
    float dotNH = max(dot(norm, halfv), 0.0);
    color += pow(dotNH, materialShininess) * lightSpecular.rgb * materialSpecular.rgb; // add specular


    // code to determine how bullseye pixels are colored
    float ringRadius = length(position - bullsEyeCenter);

    // inner should be red 
    if (ringRadius < 0.4) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // should be same color as duck 
    } else if (ringRadius < 0.7) {
        gl_FragColor = vec4(color, materialDiffuse.a);
    // else, outer ring should be red       
    } else {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
)";

// Laser point vertex shader and fragment shader 
// Vertex Shaders source for bullseye target 
const char* vsLaserSource = R"(
// GLSL version
#version 110

varying vec2 uv;

void main(void) {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_PointSize = 12.0;
}
)";

// Fragment Shaders source for bullseye target
const char* fsLaserSource = R"(
// GLSL version
#version 110
void main(void) {
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

// Global variables
void* font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraX;
float cameraZ;
GLdouble spinX = 0.0;
float cameraDistance;
int drawMode;
bool vboSupported = true;;
GLuint vboId1, vboId2;      // IDs of VBO for vertex arrays
GLuint iboId1, iboId2;      // IDs of VBO for index array
GLuint texId;
int imageWidth;
int imageHeight;
Matrix4 matrixModelView;
Matrix4 matrixProjection;
// GLSL
GLuint progId = 0;                  // ID of GLSL program (bullseye)
GLuint progId2 = 1;                 // ID of GLSL program (laser)
GLuint skyboxTexID;                 // skybox
bool glslSupported;

// Variables
GLint uniformBullsEyeCenter;
GLint uniformLightPosition;
GLint uniformLightAmbient;
GLint uniformLightDiffuse;
GLint uniformLightSpecular;
GLint uniformMaterialAmbient;
GLint uniformMaterialDiffuse;
GLint uniformMaterialSpecular;
GLint uniformMaterialShininess;
GLint attribVertexPosition;
GLint attribVertexNormal;
GLint attribVertexTexCoord;

// Duck Targets
DuckTarget* duckTarget;
DuckTarget* duckTarget2;
DuckTarget* duckTarget3;
DuckTarget* duckTarget4;
DuckTarget* duckTarget5;
DuckTarget* duckTarget6;

// Gun
Gun* gun;

// Booth consists of top, sides and bottom
CubeMesh* boothTop = NULL;
CubeMesh* boothLeftSide = NULL;
CubeMesh* boothRightSide = NULL;
CubeMesh* boothFront = NULL;
bool drawBoothFront = true;
bool moving = false;

// A flat open mesh
// Default Mesh Size
int meshSize = 16;
QuadMesh* groundMesh = NULL;

// Water waves
static SineWaveMesh sineWaveMesh;


// Arcade Booth 
GLfloat booth_ambient[] = { 0.05f, 0.0f, 0.0f, 1.0f };
GLfloat booth_diffuse[] = { 0.05f, 0.0f, 0.0f, 1.0f };
GLfloat booth_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat booth_shininess[] = { 4.0F };

// Booth textures
GLuint boothSideTexture;
GLuint boothTopTexture;
GLuint boothFrontTexture;
GLuint groundMeshTexture;

// Hit sounds
SDL_AudioSpec spec;
Uint8* soundBuffer = NULL;
Uint32 soundLength = 0;
SDL_AudioStream* stream;

#include <direct.h>


// skybox
GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height;
    unsigned char* image;

    for (int i = 0; i < faces.size(); i++) {
        image = SOIL_load_image(faces[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);

        if (image) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            SOIL_free_image_data(image);
        }
        else {
            std::cout << "Cubemap failed to load: " << faces[i] << std::endl;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return textureID;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // init global vars
    initGlobalVariables();

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();
    InitGLEW();

    // ducks immediately start moving as soon as program starts running
    moving = true;
    glutTimerFunc(10, animationHandler, 0);

    // load textures
    loadTextures();

    // hide cursor
    glutSetCursor(GLUT_CURSOR_NONE);


    std::cout << "Video card supports GLSL." << std::endl;
    // compile shaders and create GLSL program
    // If failed to create GLSL, reset flag to false
    glslSupported = initGLSL();

    // pass shaders to duck targets
    duckTarget->getShaders(progId);
    duckTarget2->getShaders(progId);
    duckTarget3->getShaders(progId);
    duckTarget4->getShaders(progId);
    duckTarget5->getShaders(progId);
    duckTarget6->getShaders(progId);

    groundMesh->CreateMeshVBO(meshSize, attribVertexPosition, attribVertexNormal);

    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char** argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow("Duck Carnival @davidxk3");     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(10, timerCB, 0);             // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutPassiveMotionFunc(moveGun);           // move gun with mouse movement 

    return handle;
}


void InitGLEW()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "There was a problem initializing GLEW. Exiting..." << std::endl;
        exit(-1);
    }

    // Check for 3.3 support.
    // I've specified that a 3.3 forward-compatible context should be created.
    // so this parameter check should always pass if our context creation passed.
    // If we need access to deprecated features of OpenGL, we should check
    // the state of the GL_ARB_compatibility extension.
    if (!GLEW_VERSION_3_3)
    {
        std::cerr << "OpenGL 3.3 required version support not present." << std::endl;
        exit(-1);
    }

#ifdef _WIN32
    if (WGLEW_EXT_swap_control)
    {
        wglSwapIntervalEXT(0); // Disable vertical sync
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    glClearColor(0.02f, 0.02f, 0.1f, 1.0f);     // background color - changed so sky is dark blue 
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initSounds();
    initLights();

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


///////////////////////////////////////////////////////////////////////////////
// create glsl programs
///////////////////////////////////////////////////////////////////////////////
bool initGLSL()
{

    const int MAX_LENGTH = 2048;
    char log[MAX_LENGTH];
    int logLength = 0;

    // create shader and program for bullseye
    GLuint vsId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsId = glCreateShader(GL_FRAGMENT_SHADER);
    progId = glCreateProgram();

    // create shader and program for laser 
    GLuint vsId2 = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsId2 = glCreateShader(GL_FRAGMENT_SHADER);
    progId2 = glCreateProgram();

    // load shader sources
    glShaderSource(vsId, 1, &vsBullseyeSource, NULL);
    glShaderSource(fsId, 1, &fsBullseyeSource, NULL);
    glShaderSource(vsId2, 1, &vsLaserSource, NULL);
    glShaderSource(fsId2, 1, &fsLaserSource, NULL);

    // compile shader sources
    glCompileShader(vsId);
    glCompileShader(fsId);
    glCompileShader(vsId2);
    glCompileShader(fsId2);

    //@@ debug - only for bullseye
    int vsStatus, fsStatus;
    glGetShaderiv(vsId, GL_COMPILE_STATUS, &vsStatus);
    if (vsStatus == GL_FALSE)
    {
        glGetShaderiv(vsId, GL_INFO_LOG_LENGTH, &logLength);
        glGetShaderInfoLog(vsId, MAX_LENGTH, &logLength, log);
        std::cout << "===== Vertex Shader Log =====\n" << log << std::endl;
        return false;
    }
    glGetShaderiv(fsId, GL_COMPILE_STATUS, &fsStatus);
    if (fsStatus == GL_FALSE)
    {
        glGetShaderiv(fsId, GL_INFO_LOG_LENGTH, &logLength);
        glGetShaderInfoLog(fsId, MAX_LENGTH, &logLength, log);
        std::cout << "===== Fragment Shader Log =====\n" << log << std::endl;
        return false;
    }

    // attach shaders to the program
    glAttachShader(progId, vsId);
    glAttachShader(progId, fsId);
    glAttachShader(progId2, vsId2);
    glAttachShader(progId2, fsId2);

    // link program for bullseye
    glLinkProgram(progId);

    // get uniform/attrib locations
    glUseProgram(progId);
    uniformBullsEyeCenter = glGetUniformLocation(progId, "targetCenter");
    uniformLightPosition = glGetUniformLocation(progId, "lightPosition");
    uniformLightAmbient = glGetUniformLocation(progId, "lightAmbient");
    uniformLightDiffuse = glGetUniformLocation(progId, "lightDiffuse");
    uniformLightSpecular = glGetUniformLocation(progId, "lightSpecular");
    uniformMaterialAmbient = glGetUniformLocation(progId, "materialAmbient");
    uniformMaterialDiffuse = glGetUniformLocation(progId, "materialDiffuse");
    uniformMaterialSpecular = glGetUniformLocation(progId, "materialSpecular");
    uniformMaterialShininess = glGetUniformLocation(progId, "materialShininess");

    // Set uniform values
    float centerObjectCoords[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float lightPosition[] = { -4.0f, 8.0f, 8.0f, 1.0f };
    float lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1 };
    float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1 };
    float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1 };
    float materialAmbient[] = { 0.957f, 0.74f, 0.047f, 1.0f };
    float materialDiffuse[] = { 0.957f, 0.74f, 0.047f, 1.0f };
    float materialSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    float materialShininess = 100.0F;

    glUniform3f(uniformBullsEyeCenter, 0, 0, 0);
    glUniform4fv(uniformLightPosition, 1, lightPosition);
    glUniform4fv(uniformLightAmbient, 1, lightAmbient);
    glUniform4fv(uniformLightDiffuse, 1, lightDiffuse);
    glUniform4fv(uniformLightSpecular, 1, lightSpecular);
    glUniform4fv(uniformMaterialAmbient, 1, materialAmbient);
    glUniform4fv(uniformMaterialDiffuse, 1, materialDiffuse);
    glUniform4fv(uniformMaterialSpecular, 1, materialSpecular);
    glUniform1f(uniformMaterialShininess, materialShininess);

    // unbind GLSL
    glUseProgram(0);

    // link program for laser
    glLinkProgram(progId2);


    // check GLSL status
    int linkStatus;
    int linkStatus2;
    glGetProgramiv(progId, GL_LINK_STATUS, &linkStatus);
    glGetProgramiv(progId2, GL_LINK_STATUS, &linkStatus2);
    if (linkStatus == GL_FALSE)
    {
        glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &logLength);
        glGetProgramInfoLog(progId, MAX_LENGTH, &logLength, log);
        std::cout << "===== GLSL Program Log =====\n" << log << std::endl;
        return false;
    }
    else if (linkStatus2 == GL_FALSE) {
        glGetProgramiv(progId2, GL_INFO_LOG_LENGTH, &logLength);
        glGetProgramInfoLog(progId2, MAX_LENGTH, &logLength, log);
        std::cout << "===== Pointer Laser Log =====\n" << log << std::endl;
        return false;
    }
    else
    {
        return true;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// Initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initGlobalVariables()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraX = 0.0f;
    cameraZ = CAMERA_DISTANCE;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    // Create Targets
    // on wave, x coordinate initially at -8.0f
    duckTarget = new DuckTarget();
    // on wave, x coordinate is -8.0f + 8.0f = 0.0f
    duckTarget2 = new DuckTarget(0.f);
    // on wave, x coordinate is -8.0f + 16.0f = 8.0f
    duckTarget3 = new DuckTarget(8.0f);

    // below wave, x coordinate is same as ducks above but flipped 
    duckTarget4 = new DuckTarget(8.0f, true);
    duckTarget5 = new DuckTarget(0.f, true);
    duckTarget6 = new DuckTarget(-8.0f, true);

    // add gun 
    gun = new Gun();

    // Set up ground quad mesh
    Vector3 origin = Vector3(-32.0f, -9.0f, 48.0f);
    Vector3 dir1v = Vector3(1.0f, 0.0f, 0.0f);
    Vector3 dir2v = Vector3(0.0f, 0.0f, -1.0f);
    groundMesh = new QuadMesh(meshSize, 64.0);
    groundMesh->InitMesh(meshSize, origin, 64.0, 64.0, dir1v, dir2v);


    Vector3 ambient = Vector3(0.0f, 1.0f, 0.0f);
    Vector3 diffuse = Vector3(0.0f, 0.8f, 0.0f);
    Vector3 specular = Vector3(0.04f, 0.04f, 0.04f);
    float shininess = 0.2;
    groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

    boothTop = new CubeMesh();
    ambient = Vector3(0.2f, 0.00f, 0.0f);
    diffuse = Vector3(0.9f, 0.9f, 0.9f);
    specular = Vector3(0.5f, 0.5f, 0.5f);
    shininess = 4.0;
    boothTop->setMaterial(ambient, diffuse, specular, shininess);

    boothFront = new CubeMesh();
    boothFront->setMaterial(ambient, diffuse, specular, shininess);

    boothLeftSide = new CubeMesh();
    ambient = Vector3(0.2f, 0.2f, 0.2f);
    diffuse = Vector3(0.7f, 0.7f, 0.7f);
    specular = Vector3(1.0f, 1.0f, 1.0f);
    shininess = 4.0;
    boothLeftSide->setMaterial(ambient, diffuse, specular, shininess);

    boothRightSide = new CubeMesh();

    boothRightSide->setMaterial(ambient, diffuse, specular, shininess);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// audio for hitting target
///////////////////////////////////////////////////////////////////////////////
void initSounds() {
    // Start audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log(SDL_GetError());
        return;
    }

    // Load WAV file
    if (!SDL_LoadWAV("./src/hitSound.wav", &spec, &soundBuffer, &soundLength)) {
        SDL_Log("Failed to load WAV: %s", SDL_GetError());
        return;
    }

    // Create audio stream for playback
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

    if (!stream) {
        SDL_Log(SDL_GetError());
        return;
    }

    // Start audio device now that we know it's valid
    SDL_ResumeAudioStreamDevice(stream);
}


///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
    // clean up VBOs
    if (vboSupported)
    {
        glDeleteBuffers(1, &vboId1);
        glDeleteBuffers(1, &iboId1);
        glDeleteBuffers(1, &vboId2);
        glDeleteBuffers(1, &iboId2);
        vboId1 = iboId1 = 0;
        vboId2 = iboId2 = 0;
    }
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = { .05f, .05f, .05f, 1.0f };  // ambient light
    GLfloat lightKd[] = { 0.7f, 0.8f, 1.0f, 1.0f };  // diffuse light
    GLfloat lightKs[] = { 1, 1, 1, 1 };           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = { -4.0, 8.0f, 8.0f, 1.0f }; // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 

}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



void loadTextures()
{
    // load side of the booths (left and right)
    boothSideTexture = SOIL_load_OGL_texture(
        "./src/boothSides.bmp",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    if (boothSideTexture == 0) {
        printf("SOIL loading error: %s for booth sides\n", SOIL_last_result());
    }


    // load top of the booth
    boothTopTexture = SOIL_load_OGL_texture(
        "./src/boothTop.bmp",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    if (boothTop == 0) {
        printf("SOIL loading error: '%s for booth top\n", SOIL_last_result());
    }


    // load top of the booth
    boothFrontTexture = SOIL_load_OGL_texture(
        "./src/boothFront.bmp",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    if (boothFrontTexture == 0) {
        printf("SOIL loading error: '%s' for booth front\n", SOIL_last_result());
    }


    // for ground texture 
    groundMeshTexture = SOIL_load_OGL_texture(
        "./src/groundMesh.bmp",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    if (groundMeshTexture == 0) {
        printf("SOIL loading error: '%s' for ground mesh\n", SOIL_last_result());
    }

    // for skybox
    std::vector<std::string> skyBoxFaces = {
        "./src/skybox/right.png",
        "./src/skybox/left.png",
        "./src/skybox/top.png",
        "./src/skybox/bot.png",
        "./src/skybox/front.png",
        "./src/skybox/back.png"
    };

    skyboxTexID = loadCubemap(skyBoxFaces);
}

// function to draw skybox
void drawSkybox(float size) {
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);
    glEnable(GL_TEXTURE_CUBE_MAP);

    glBegin(GL_QUADS);


    glTexCoord3f(1, -1, -1); glVertex3f(size, -size, -size);
    glTexCoord3f(1, -1, 1); glVertex3f(size, -size, size);
    glTexCoord3f(1, 1, 1); glVertex3f(size, size, size);
    glTexCoord3f(1, 1, -1); glVertex3f(size, size, -size);

    glTexCoord3f(-1, -1, 1); glVertex3f(-size, -size, size);
    glTexCoord3f(-1, -1, -1); glVertex3f(-size, -size, -size);
    glTexCoord3f(-1, 1, -1); glVertex3f(-size, size, -size);
    glTexCoord3f(-1, 1, 1); glVertex3f(-size, size, size);

    glTexCoord3f(-1, 1, -1); glVertex3f(-size, size, -size);
    glTexCoord3f(1, 1, -1); glVertex3f(size, size, -size);
    glTexCoord3f(1, 1, 1); glVertex3f(size, size, size);
    glTexCoord3f(-1, 1, 1); glVertex3f(-size, size, size);

    glTexCoord3f(-1, -1, 1); glVertex3f(-size, -size, size);
    glTexCoord3f(1, -1, 1); glVertex3f(size, -size, size);
    glTexCoord3f(1, -1, -1); glVertex3f(size, -size, -size);
    glTexCoord3f(-1, -1, -1); glVertex3f(-size, -size, -size);

    glTexCoord3f(-1, -1, 1); glVertex3f(-size, -size, size);
    glTexCoord3f(-1, 1, 1); glVertex3f(-size, size, size);
    glTexCoord3f(1, 1, 1); glVertex3f(size, size, size);
    glTexCoord3f(1, -1, 1); glVertex3f(size, -size, size);

    glTexCoord3f(1, -1, -1); glVertex3f(size, -size, -size);
    glTexCoord3f(1, 1, -1); glVertex3f(size, size, -size);
    glTexCoord3f(-1, 1, -1); glVertex3f(-size, size, -size);
    glTexCoord3f(-1, -1, -1); glVertex3f(-size, -size, -size);

    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_CUBE_MAP);
    glDepthMask(GL_TRUE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


///////////////////////////////////////////////////////////////////////////////
// Set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    const float N = 0.2f;
    const float F = 100.0f;
    const float DEG2RAD = 3.141592f / 180;
    const float FOV_Y = 60.0f * DEG2RAD;

    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // construct perspective projection matrix
    float aspectRatio = (float)(screenWidth) / screenHeight;
    float tangent = tanf(FOV_Y / 2.0f);     // tangent of half fovY
    float h = N * tangent;                  // half height of near plane
    float w = h * aspectRatio;              // half width of near plane
    matrixProjection.identity();
    matrixProjection[0] = N / w;
    matrixProjection[5] = N / h;
    matrixProjection[10] = -(F + N) / (F - N);
    matrixProjection[11] = -1;
    matrixProjection[14] = -(2 * F * N) / (F - N);
    matrixProjection[15] = 0;

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrixProjection.get());
    //@@ equivalent fixed pipeline
    //glLoadIdentity();
    //gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 0.2f, 40.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//=============================================================================
// Shooting gun (bullet) animation
//=============================================================================
// check if gun is in motion

void shootGun(int value) {
    // get boolean to check if bullet is in motion (ex. bullet has been shot)
    bool inMotion = gun->isInMotion();
    if (inMotion) {
        // get current bullet position
        Vector3 bulletCoords = gun->getBulletWorldCoords();

        // check if bullet hits any of the ducks and flip them if they do 
        if (duckTarget->hit(bulletCoords)) {
            duckTarget->flip();
            // play sound 
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, soundBuffer, soundLength);
        }

        // check if bullet hits any of the ducks and flip them if they do 
        if (duckTarget2->hit(bulletCoords)) {
            duckTarget2->flip();
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, soundBuffer, soundLength);
        }

        // check if bullet hits any of the ducks and flip them if they do 
        if (duckTarget3->hit(bulletCoords)) {
            duckTarget3->flip();
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, soundBuffer, soundLength);
        }

        // check if bullet hits any of the ducks and flip them if they do 
        if (duckTarget4->hit(bulletCoords)) {
            duckTarget4->flip();
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, soundBuffer, soundLength);
        }

        // check if bullet hits any of the ducks and flip them if they do 
        if (duckTarget5->hit(bulletCoords)) {
            duckTarget5->flip();
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, soundBuffer, soundLength);

        }

        // check if bullet hits any of the ducks and flip them if they do 
        if (duckTarget6->hit(bulletCoords)) {
            duckTarget6->flip();
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, soundBuffer, soundLength);

        }

        // move the bullet (animate)
        gun->moveBullet();
        glutPostRedisplay();
        glutTimerFunc(10, shootGun, 0);
    }

}


//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // Clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0, 0, 0, cameraX, 2.0, cameraZ, 0, 1, 0);
    drawSkybox(50.0f);


    if (!vboSupported || !glslSupported)
        return;


    glBindTexture(GL_TEXTURE_2D, groundMeshTexture); // texture for ground mesh (repeat it)
    groundMesh->DrawMesh(meshSize);
    glBindTexture(GL_TEXTURE_2D, 0); // reset textures

    // Draw everything else using fixed pipeline and immediate mode rendering
    glLoadIdentity();

    // Create Viewing Matrix V
    gluLookAt(cameraX, 2.0, cameraZ, 0.0, 2.0, 0.0, 0.0, 1.0, 0.0);

    // Draw duck target in five different positions
    // use fragment shader to determine which target pixels to replace with bullseye ring pixels

    // glUseProgram(progId);
    duckTarget->draw();
    duckTarget2->draw();
    duckTarget3->draw();
    duckTarget4->draw();
    duckTarget5->draw();
    duckTarget6->draw();
    // glUseProgram(0);        // turn off shaders

    // draw gun
    gun->draw();

    // draw/render laser
    gun->drawLaser(progId2);

    // Draw Booth 
    glBindTexture(GL_TEXTURE_2D, boothTopTexture); // texture for booth top
    glPushMatrix();
    glTranslatef(0, 12.0f, -8.0);
    glScalef(16.0f, 2.0f, 2.0f);
    boothTop->drawCubeMesh();
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, boothSideTexture); // texture for booth sides

    glPushMatrix();
    glTranslatef(-14.0, 0.0f, -8.0);
    glScalef(1.0f, 10.0f, 2.0f);
    boothLeftSide->drawCubeMesh();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(14.0, 0.0f, -8.0);
    glScalef(1.0f, 10.0f, 2.0f);
    boothRightSide->drawCubeMesh();
    glPopMatrix();



    if (drawBoothFront)
    {
        glBindTexture(GL_TEXTURE_2D, boothFrontTexture); // texture for booth front
        glPushMatrix();
        glTranslatef(0, -6.0, -6.0);
        glScalef(12.0f, 4.0f, 0.5f);
        boothFront->drawCubeMesh();
        glPopMatrix();
    }

    glBindTexture(GL_TEXTURE_2D, 0); // reset textures

    // Draw water waves with sine wave function
    glPushMatrix();
    glTranslatef(0.0, -6.0, -14.0);
    glRotatef(-180, 0, 1, 0);
    drawSineWaveMesh();
    glPopMatrix();

    glutSwapBuffers();
}


void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}

bool stop = false;

void keyboardCB(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESCAPE
        clearSharedMem();
        exit(0);
        break;
    default:
        ;
    }
}

void animationHandler(int param)
{
    if (moving) {
        // animate ducks around track
        duckTarget->animate(true);
        duckTarget2->animate(true);
        duckTarget3->animate(true);
        duckTarget4->animate(true);
        duckTarget5->animate(true);
        duckTarget6->animate(true);
        glutPostRedisplay();
        glutTimerFunc(12, animationHandler, 0);
    }

}


void mouseCB(int button, int state, int x, int y) {
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            // if the mouse was left clicked, shoot a bullet (animate it)
            mouseLeftDown = true;
            gun->shoot();
            glutTimerFunc(10, shootGun, 0);
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }

    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if (state == GLUT_UP)
            mouseMiddleDown = false;
    }
}

void moveGun(int x, int y)
{
    // move the gun around the screen, 0.01 works well so the sensitivity isn't too high
    gun->moveGun(-0.01 * (mouseX - x), 0.01 * (mouseY - y));
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}