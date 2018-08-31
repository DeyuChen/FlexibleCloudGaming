// ******************************
// glmodelwin.h
//
// This class contains the OpenGL processing
// code for the Mesh Simplification Viewer
// program.
//
// Jeff Somers
// Copyright (c) 2002
//
// jsomers@alumni.williams.edu
// March 27, 2002
// ******************************


#ifndef __glwin_h
#define __glwin_h

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>            // Header File For The OpenGL32 Library
#include <GL/glu.h>            // Header File For The GLu32 Library
#include <libavformat/avformat.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pmesh.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 960

// Model orientation
const float ORIG_ELEVATION = 0.0f;
const float ORIG_AZIMUTH = 0.0f;
const float ORIG_DIST = 3.0f;

const float MIN_DISTANCE = 0.1f;
const float MAX_DISTANCE = 100.0f;

class PMeshInfo {
public:
    PMeshInfo(){
        mesh = NULL;
        pos = Vec3(0, 0, 0);
        meshID = -1;
    }
    PMeshInfo(PMesh* _mesh, Vec3 _pos, int _meshID) : mesh(_mesh), pos(_pos), meshID(_meshID){}
    PMesh* mesh;
    Vec3 pos;
    int meshID;
};

class VertexBufferItem{
public:
    VertexBufferItem(){}
    VertexBufferItem(const float* _pos, const float* _normal, const unsigned char* _rgb, const float* _texcoord){
        memcpy(pos, _pos, 3 * sizeof(float));
        memcpy(normal, _normal, 3 * sizeof(float));
        memcpy(rgb, _rgb, 3 * sizeof(unsigned char));
        memcpy(texcoord, _texcoord, 2 * sizeof(float));
    }
    float pos[3];
    float normal[3];
    unsigned char rgb[3];
    float texcoord[2];
};

template<int WIDTH, int HEIGHT>
class FrameInfo {
public:    
    FrameInfo(){
        id = 0;
        patched = false;
        image = new unsigned char[3 * WIDTH * HEIGHT];
        diff = new unsigned char[3 * WIDTH * HEIGHT];
        depth = new float[WIDTH * HEIGHT];
    };
    
    ~FrameInfo(){
        delete[] image;
        delete[] diff;
        delete[] depth;
    }
    
    unsigned long long id;
    bool patched;
    unsigned char *image;
    unsigned char *diff;
    float *depth;
    glm::mat4 mvp;
    glm::vec4 pos;
};

class glModelWindow
{
public:

    // constructor
    glModelWindow() : window(NULL), renderingMode(1),
                      width(0), height(0), oldWidth(0), oldHeight(0),
                      oldX(0), oldY(0), newX(0), newY(0),
                      viewX(0), viewY(0), viewZ(0), moveSpeed(0.1),
                      bFullScreen_(false), bFill_(true), bSmooth_(false), color(false)
    {
        resetOrientation();
    };

    ~glModelWindow() {};

    // resize the OpenGL window
    GLvoid reSizeScene(GLsizei width, GLsizei height);

    // Initialize OpenGL
    int initOpenGL(GLvoid);

    // display the mesh within the window
    bool displayMesh(unsigned char* inbuf, unsigned char* outbuf, bool simplified, bool visible);
    bool displayMesh(unsigned char* inbuf, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *frame, bool simplified, bool visible);
    bool displayImage(unsigned char* inbuf);
    bool displayImage(FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *prevframe, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *curframe);
    bool patchFrame(unsigned char* inbuf, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *frame);
        
    bool subImage(unsigned char* diff, unsigned char* orig, unsigned char* simp);
    bool subImage(short* diff, unsigned char* orig, unsigned char* simp);
    bool subImage(AVFrame* orig, AVFrame* simp);
    bool subDepth(float* diff, float* orig, float* simp);

    // shutdown the OpenGL window
    GLvoid killMyWindow(GLvoid);

    // create the OpenGL window
    bool createMyWindow(int width, int height, unsigned char bits, bool fullscreenflag);

    // If in windowed mode, set to full screen mode.
    // If in full screen mode, reset to windowed mode (width, height parameters 
    // are ignored, the previous size of the window is used instead)
    void flipFullScreen(int width, int height);

    // deal with mesh rotation via mouse movement
    void mouseMotion(int x, int y, bool leftButton, bool rightButton);

    // set new mouse coordiantes
    void setNewXY(int x, int y) {newX = x; newY = y;}
    
    void keyPress(Sint32 key, int x, int y);

    // reset the mesh orientation
    void resetOrientation() {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        elevation = ORIG_ELEVATION;
        azimuth = ORIG_AZIMUTH;
        dist_ = ORIG_DIST;
        oldX = newX = width / 2;
        oldY = newY = height / 2;
    }
    
    void setAzimuth(float _azimuth){
        azimuth = _azimuth;
    }

    // In order to change the full screen indicator, call flipFullScreen.
    bool isFullScreen() {return bFullScreen_;}

    // The mesh may be displayed in wireframe or filled-triangle mode
    bool isFillTriMode() {return bFill_;}
    void setFillTriMode(bool newFillbool) {bFill_ = newFillbool;}

    // The mesh may be flat shaded (Lambert shaded) or smooth (Gouraud) shaded
    bool isSmoothShadingMode() {return bSmooth_;}
    void setSmoothShadingMode(bool newSmooth) {bSmooth_ = newSmooth;}
    void setColor(bool _color){color = _color;}
    
    void changeMesh(PMesh* g_pProgMesh, int n, bool percentage);
    
    SDL_Window* getSDLWindow(){return window;}
    
    void loadMesh(string filename);
    void loadPMesh(int meshID, Vec3 &pos, PMesh::EdgeCost g_edgemethod, int mesh_percentage);
    
    void reloadVertexBuffer();
    
    void setRenderingMode(int mode){renderingMode = mode;}
    
    GLuint loadShaderFromFile(std::string path, GLenum shaderType);
    
    bool loadTransformProgram();
    
    /*
    void getCamera(float& _x, float& _y, float& _z, float& _elevation, float& _azimuth){
        _x = viewX;
        _y = viewY;
        _z = viewZ;
        _elevation = elevation;
        _azimuth = azimuth;
    }
    
    void setCamera(float _x, float _y, float _z, float _elevation, float _azimuth){
        viewX = _x;
        viewY = _y;
        viewZ = _z;
        elevation = _elevation;
        azimuth = _azimuth;
    }
    */

    // Display title text for window
    //void displayWindowTitle();

private:
    // Window Handle
    SDL_Window* window;

    // Display context
    SDL_GLContext context;

    int renderingMode;
    
    // width, height of window
    int width;
    int height;

    // previous window width, height
    int oldWidth;
    int oldHeight;

    // used for mouse motion
    int oldX, oldY, newX, newY;
    
    float viewX, viewY, viewZ;
    float moveSpeed;

    // Mesh orientation variables
    float elevation;
    float azimuth;
    float dist_;

    // Full Screen display indicator
    bool bFullScreen_;

    // Fill in triangles (or use wireframe mode?)
    bool bFill_;

    // Use Gouraud shading?
    bool bSmooth_;
    
    bool color;
    
    GLuint transformPID;
    GLuint MVPID;

    // no assignment, copy ctor allowed (implementation not provided).
    glModelWindow(const glModelWindow&);
    glModelWindow& operator=(const glModelWindow&);
    
    vector<Mesh*> mesh_list;
    vector<PMeshInfo> pmesh_list;
    
    vector<vector<vector<VertexBufferItem>>> VBO;
    vector<vector<vector<VertexBufferItem>>> SVBO;
    
    vector<VertexBufferItem> warpingBuf;
    
    int getIndex(int x, int y);
};

#endif
