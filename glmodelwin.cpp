// ******************************
// glmodelwin.cpp
//
// This file contains the OpenGL - Win32
// processing code.
//
// Jeff Somers
// Copyright (c) 2002
//
// jsomers@alumni.williams.edu
// March 27, 2002
//
// Some of this code is based on Jeff Molofee's 
// OpenGL tutorials. See http://nehe.gamedev.net
//
// ******************************

#include <chrono>
#include "glmodelwin.h"
#include "conf.h"

#define HOLE_FILL_ROUND 5

extern Configuration conf;

int getIndex(int x, int y, int width){
    return y * width + x;
}

// Resize the OpenGL window
GLvoid glModelWindow::reSizeScene(GLsizei _width, GLsizei _height){
    if (_height == 0)
        _height = 1; // _height == 0 not allowed

    width = _width;
    height = _height;
    
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate The Perspective of the window
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, MIN_DISTANCE, MAX_DISTANCE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Initialize OpenGL
int glModelWindow::initOpenGL(GLvoid){
    glShadeModel(GL_SMOOTH); // Gouraud shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // load 3D shader program
    renderProgram.vertexShader = loadShaderFromFile("3DShader.vert", GL_VERTEX_SHADER);
    renderProgram.fragmentShader = loadShaderFromFile("3DShader.frag", GL_FRAGMENT_SHADER);

    renderProgram.id = glCreateProgram();

    glAttachShader(renderProgram.id, renderProgram.vertexShader);
    glAttachShader(renderProgram.id, renderProgram.fragmentShader);

    glBindAttribLocation(renderProgram.id, 0, "in_Position");
    glBindAttribLocation(renderProgram.id, 1, "in_Normal");
    glBindAttribLocation(renderProgram.id, 2, "in_Color");
    glBindAttribLocation(renderProgram.id, 3, "in_TexCoord");

    glLinkProgram(renderProgram.id);

    GLint isLinked = GL_TRUE;
    glGetProgramiv(renderProgram.id, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE){
        cerr << "Failed to link shader program" << endl;
        return false;
    }

    glUniform1i(glGetUniformLocation(renderProgram.id, "Texture"), 0);
    
    // load warp shader program
    warpProgram.vertexShader = loadShaderFromFile("warpShader.vert", GL_VERTEX_SHADER);
    warpProgram.fragmentShader = loadShaderFromFile("warpShader.frag", GL_FRAGMENT_SHADER);

    warpProgram.id = glCreateProgram();

    glAttachShader(warpProgram.id, warpProgram.vertexShader);
    glAttachShader(warpProgram.id, warpProgram.fragmentShader);

    glBindAttribLocation(warpProgram.id, 0, "in_Position");
    glBindAttribLocation(warpProgram.id, 1, "in_Color");
    glBindAttribLocation(warpProgram.id, 2, "in_Depth");

    glLinkProgram(warpProgram.id);

    glGetProgramiv(warpProgram.id, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE){
        cerr << "Failed to link shader program" << endl;
        return false;
    }

    pixelPoints.VAO.resize(1);
    pixelPoints.VBO.resize(3);
    pixelPoints.IBO.resize(1);
    glGenVertexArrays(1, pixelPoints.VAO.data());
    glGenBuffers(3, pixelPoints.VBO.data());
    glGenBuffers(1, pixelPoints.IBO.data());

    glBindVertexArray(pixelPoints.VAO[0]);

    // preset pixel 2D locations
    glBindBuffer(GL_ARRAY_BUFFER, pixelPoints.VBO[0]);
    GLfloat *pixelVertices = new GLfloat[2 * width * height];
    int count = 0;
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            pixelVertices[count++] = (GLfloat)j * 2.0 / width - 1.0;
            pixelVertices[count++] = (GLfloat)i * 2.0 / height - 1.0;
        }
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * width * height, pixelVertices, GL_STATIC_DRAW);
    delete[] pixelVertices;
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // colors
    glBindBuffer(GL_ARRAY_BUFFER, pixelPoints.VBO[1]);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // depths
    glBindBuffer(GL_ARRAY_BUFFER, pixelPoints.VBO[2]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pixelPoints.IBO[0]);
    GLuint *pixelIndices = new GLuint[width * height];
    for (int i = 0; i < width * height; i++){
        pixelIndices[i] = i;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * width * height, pixelIndices, GL_STATIC_DRAW);
    delete[] pixelIndices;

    glBindVertexArray(0);

    return true;
}

// Create OpenGL window
bool glModelWindow::createMyWindow(const char* title, int _width, int _height, unsigned char bits, bool fullscreenflag){
    width = _width;
    height = _height;
    
    bFullScreen_ = fullscreenflag;
    
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else {
        //Use OpenGL 3.0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        //Create window
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (window == NULL){
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else {
            //Create context
            context = SDL_GL_CreateContext(window);
            if (context == NULL){
                printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else {
                //Use Vsync
                if (SDL_GL_SetSwapInterval(1) < 0){
                    printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
                }
            }
        }
    }
    
    reSizeScene(width, height);
    resetOrientation();

    if (glewInit() != GLEW_OK || !initOpenGL()){
        killMyWindow();
        return false;
    }
    
    if (conf.smooth){
        setSmoothShadingMode(true);
    }

    return success;
}

// shut down OpenGL window
GLvoid glModelWindow::killMyWindow(GLvoid){
    if (window != NULL){
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();
}

// Handle mouse motion
void glModelWindow::mouseMotion(int x, int y, bool leftButton, bool rightButton){
    float RelX = x / (float)width;
    float RelY = y / (float)height;
    
    azimuth += (RelX * 180);
    elevation += (RelY * 180);
}

void glModelWindow::keyPress(Sint32 key, int x, int y){
    switch(key){
        case SDLK_w:
            viewZ += moveSpeed * cos(azimuth * 3.14159265 / 180.0);
            viewX -= moveSpeed * sin(azimuth * 3.14159265 / 180.0);
            break;
            
        case SDLK_d:
            viewZ -= moveSpeed * sin(azimuth * 3.14159265 / 180.0);
            viewX -= moveSpeed * cos(azimuth * 3.14159265 / 180.0);
            break;
            
        case SDLK_s:
            viewZ -= moveSpeed * cos(azimuth * 3.14159265 / 180.0);
            viewX += moveSpeed * sin(azimuth * 3.14159265 / 180.0);
            break;
            
        case SDLK_a:
            viewZ += moveSpeed * sin(azimuth * 3.14159265 / 180.0);
            viewX += moveSpeed * cos(azimuth * 3.14159265 / 180.0);
            break;
            
        case SDLK_PAGEUP:
            for (int i = 0; i < pmesh_list.size(); i++)
                changeMesh(pmesh_list[i].mesh, 5, true);
            break;
            
        case SDLK_PAGEDOWN:
            for (int i = 0; i < pmesh_list.size(); i++)
                changeMesh(pmesh_list[i].mesh, -5, true);
            break;
            
        case SDLK_UP:
            for (int i = 0; i < pmesh_list.size(); i++)
                changeMesh(pmesh_list[i].mesh, 1, false);
            break;
            
        case SDLK_DOWN:
            for (int i = 0; i < pmesh_list.size(); i++)
                changeMesh(pmesh_list[i].mesh, -1, false);
            break;
            
        case SDLK_m:
            renderingMode = (renderingMode + 1) % 3;
            break;

        case SDLK_n:
            renderingMode = (renderingMode + 2) % 3;
            break;
            
        case SDLK_t:
            conf.texture = !conf.texture;
            break;
    }
    
    if (conf.showXYZ){
        cout << viewX << " " << viewY << " " << viewZ << endl;;
    }
}

GLuint glModelWindow::loadShaderFromFile(string filename, GLenum shaderType){
	GLuint shaderID = 0;
	string content;
	ifstream ifs(filename.c_str());

    if (ifs){
        content.assign((istreambuf_iterator<char>(ifs) ), istreambuf_iterator<char>());

        shaderID = glCreateShader(shaderType);
        const char *pcontent = content.c_str();
        glShaderSource(shaderID, 1, (const GLchar**)&pcontent, NULL);
        glCompileShader(shaderID);

        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
        if (shaderCompiled != GL_TRUE){
            cerr << "Shader failed to compile" << endl;

            int maxLength;
            char *log;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
            log = (char *)malloc(maxLength);
            glGetShaderInfoLog(shaderID, maxLength, &maxLength, log);
            cout << log << endl;
            free(log);

            glDeleteShader(shaderID);
            shaderID = 0;
        }
	}
    else {
        cout << "Open shader file failed" << endl;
    }

	return shaderID;
}

void glModelWindow::reloadVertexBuffer(){
    if (origMeshes.empty()){
        origMeshes.resize(mesh_list.size());
        for (int i = 0; i < mesh_list.size(); i++){
            // load vertex information to a single VBO
            GLfloat *buf = new GLfloat[7 * mesh_list[i]->getNumVerts()];
            GLfloat *pbuf = buf;
            float colors[3];
            for (int j = 0; j < mesh_list[i]->getNumVerts(); j++){
                vertex v = mesh_list[i]->getVertex(j);
                const float *verts = v.getArrayVerts();
                const float *norms = v.getArrayVertNorms();
                const unsigned char *rgb = v.getArrayRGB();
                for (int k = 0; k < 3; k++){
                    colors[k] = (float)rgb[k] / 255.0;
                }
                memcpy(pbuf, verts, 3 * sizeof(GLfloat));
                memcpy(pbuf + 3, norms, 3 * sizeof(GLfloat));
                memcpy(pbuf + 6, rgb, 3 * sizeof(unsigned char));
                pbuf += 7;
            }

            origMeshes[i].VBO.resize(1);
            glGenBuffers(1, origMeshes[i].VBO.data());
            glBindBuffer(GL_ARRAY_BUFFER, origMeshes[i].VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 7 * mesh_list[i]->getNumVerts(), buf, GL_STATIC_DRAW);

            delete[] buf;



            // load texture related information, each texture use one VAO
            int numTex = mesh_list[i]->getNumTex() + 1;
            vector<vector<GLfloat>> texCoord(numTex, vector<GLfloat>(2 * mesh_list[i]->getNumVerts()));
            vector<vector<int>> indices(numTex);
            for (int j = 0; j < mesh_list[i]->getNumTriangles(); j++){
                triangle t = mesh_list[i]->getTri(j);
                if (t.isActive()){
                    int texid = t.getTexnumber() + 1;
                    float *coord = t.getTexcoord();
                    int v1 = t.getVert1Index();
                    int v2 = t.getVert2Index();
                    int v3 = t.getVert3Index();

                    memcpy(&texCoord[texid][2 * v1], coord, 2 * sizeof(GLfloat));
                    memcpy(&texCoord[texid][2 * v2], coord + 2, 2 * sizeof(GLfloat));
                    memcpy(&texCoord[texid][2 * v3], coord + 4, 2 * sizeof(GLfloat));
                    indices[texid].push_back(v1);
                    indices[texid].push_back(v2);
                    indices[texid].push_back(v3);
                }
            }

            origMeshes[i].VAO.resize(numTex);
            origMeshes[i].coordVBO.resize(numTex);
            origMeshes[i].IBO.resize(numTex);
            origMeshes[i].indSizes.resize(numTex, 0);
            glGenVertexArrays(numTex, origMeshes[i].VAO.data());
            glGenBuffers(numTex, origMeshes[i].coordVBO.data());
            glGenBuffers(numTex, origMeshes[i].IBO.data());
            for (int j = 0; j < numTex; j++){
                glBindVertexArray(origMeshes[i].VAO[j]);

                glBindBuffer(GL_ARRAY_BUFFER, origMeshes[i].VBO[0]);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);

                glBindBuffer(GL_ARRAY_BUFFER, origMeshes[i].coordVBO[j]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * mesh_list[i]->getNumVerts(), texCoord[j].data(), GL_STATIC_DRAW);
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(3);

                origMeshes[i].indSizes[j] = indices[j].size();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, origMeshes[i].IBO[j]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * origMeshes[i].indSizes[j], indices[j].data(), GL_STATIC_DRAW);

                glBindVertexArray(0);
            }
        }

        // initialize simplified mesh and related buffers
        simpMeshes.resize(pmesh_list.size());
        for (int i = 0; i < pmesh_list.size(); i++){
            simpMeshes[i].VBO.resize(1);
            glGenBuffers(1, simpMeshes[i].VBO.data());
            
            int numTex = pmesh_list[i].mesh->getMesh()->getNumTex() + 1;
            simpMeshes[i].VAO.resize(numTex);
            simpMeshes[i].coordVBO.resize(numTex);
            simpMeshes[i].IBO.resize(numTex);
            simpMeshes[i].indSizes.resize(numTex, 0);
            glGenVertexArrays(numTex, simpMeshes[i].VAO.data());
            glGenBuffers(numTex, simpMeshes[i].coordVBO.data());
            glGenBuffers(numTex, simpMeshes[i].IBO.data());
        }
    }
    
    for (int i = 0; i < pmesh_list.size(); i++){
        PMesh *pmesh = pmesh_list[i].mesh;
        // reload vertex information to a single VBO
        GLfloat *buf = new GLfloat[7 * pmesh->getMesh()->getNumVerts()];
        GLfloat *pbuf = buf;
        float colors[3];
        for (int j = 0; j < pmesh->getMesh()->getNumVerts(); j++){
            vertex v = pmesh->getMesh()->getVertex(j);
            const float *verts = v.getArrayVerts();
            const float *norms = v.getArrayVertNorms();
            const unsigned char *rgb = v.getArrayRGB();
            for (int k = 0; k < 3; k++){
                colors[k] = (float)rgb[k] / 255.0;
            }
            memcpy(pbuf, verts, 3 * sizeof(GLfloat));
            memcpy(pbuf + 3, norms, 3 * sizeof(GLfloat));
            memcpy(pbuf + 6, rgb, 3 * sizeof(unsigned char));
            pbuf += 7;
        }

        glBindBuffer(GL_ARRAY_BUFFER, simpMeshes[i].VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 7 * pmesh->getMesh()->getNumVerts(), buf, GL_STATIC_DRAW);

        delete[] buf;



        // load texture related information, each texture use one VAO
        int numTex = pmesh->getMesh()->getNumTex() + 1;
        vector<vector<GLfloat>> texCoord(numTex, vector<GLfloat>(2 * pmesh->getMesh()->getNumVerts()));
        vector<vector<int>> indices(numTex);
        for (int j = 0; j < pmesh->numTris(); j++){
            triangle t;
            if (pmesh->getTri(j, t) && t.isActive()){
                int texid = t.getTexnumber() + 1;
                float *coord = t.getTexcoord();
                int v1 = t.getVert1Index();
                int v2 = t.getVert2Index();
                int v3 = t.getVert3Index();

                memcpy(&texCoord[texid][2 * v1], coord, 2 * sizeof(GLfloat));
                memcpy(&texCoord[texid][2 * v2], coord + 2, 2 * sizeof(GLfloat));
                memcpy(&texCoord[texid][2 * v3], coord + 4, 2 * sizeof(GLfloat));
                indices[texid].push_back(v1);
                indices[texid].push_back(v2);
                indices[texid].push_back(v3);
            }
        }

        for (int j = 0; j < numTex; j++){
            glBindVertexArray(simpMeshes[i].VAO[j]);

            glBindBuffer(GL_ARRAY_BUFFER, simpMeshes[i].VBO[0]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glBindBuffer(GL_ARRAY_BUFFER, simpMeshes[i].coordVBO[j]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * pmesh->getMesh()->getNumVerts(), texCoord[j].data(), GL_STATIC_DRAW);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(3);

            simpMeshes[i].indSizes[j] = indices[j].size();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simpMeshes[i].IBO[j]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * simpMeshes[i].indSizes[j], indices[j].data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
    }
}

void glModelWindow::loadMesh(string filename){
    Mesh *mesh = new Mesh(filename);
    if (mesh)
        mesh->Normalize(conf.scale);
    mesh_list.push_back(mesh);
}

void glModelWindow::loadPMesh(int meshID, Vec3 &pos, PMesh::EdgeCost g_edgemethod, int mesh_percentage){
    if (meshID < mesh_list.size()){
        PMesh *pmesh = new PMesh(mesh_list[meshID], g_edgemethod);
        pmesh_list.push_back(PMeshInfo(pmesh, pos, meshID));
        if (mesh_percentage != 100){
            glModelWindow::changeMesh(pmesh, mesh_percentage - 100, true);
        }
    }
}

void glModelWindow::changeMesh(PMesh* mesh, int n, bool percentage){
    int old_tris = mesh->numVisTris();
    int size = 1;
    if (percentage)
        size = (mesh->numEdgeCollapses()) * abs(n) / 100;
    if (size == 0)
        size = 1;
    
    bool ret = true;
    for (int i = 0; ret && i < size; i++){
        if (n >= 0)
            ret = mesh->splitVertex();
        else
            ret = mesh->collapseEdge();
    }
    int new_tris = mesh->numVisTris();
    /*
    string title("Number of edges: ");
    title += to_string(new_tris) + "/" + to_string(mesh->getMesh()->getNumTriangles());
    SDL_SetWindowTitle(window, title.c_str());
    */
    
    reloadVertexBuffer();
    cout << "change number of triangles from " << old_tris << " to " << new_tris << endl;
}

bool glModelWindow::displayMesh(unsigned char* inbuf, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *outframe, bool simplified, bool visible){
    if (origMeshes.empty()){
        reloadVertexBuffer();
    }

    glUseProgram(renderProgram.id);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    
    // Set lookat point
    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);

    glRotatef(elevation, 1, 0, 0);
    glRotatef(azimuth, 0, 1, 0);
    glTranslatef(viewX, viewY, viewZ);

    glm::mat4 model, view, projection;
    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(renderProgram.id, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(renderProgram.id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glm::vec3 viewPos(viewX, viewY, viewZ);
    glm::vec3 lightPos(0.0, 0.0, 10.0);
    glm::vec3 lightColor(1.0, 1.0, 1.0);
    glUniform3fv(glGetUniformLocation(renderProgram.id, "viewPos"), 1, glm::value_ptr(viewPos));
    glUniform3fv(glGetUniformLocation(renderProgram.id, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(renderProgram.id, "lightColor"), 1, glm::value_ptr(lightColor));
    
    if (bSmooth_){
        glShadeModel(GL_SMOOTH); // already defined in initOpenGL
    }
    else {
        glShadeModel(GL_FLAT);
    }

    if (bFill_){
        glPolygonMode(GL_FRONT, GL_FILL);
    }
    else{
        glPolygonMode(GL_FRONT, GL_LINE);
    }
    
    if (color)
        glEnable(GL_COLOR_MATERIAL);
    
    if (conf.texture){
        glEnable(GL_TEXTURE_2D);
        glColor3ub(255, 255, 255);
    }
        
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int m = 0; m < pmesh_list.size(); m++){
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(pmesh_list[m].pos.x, pmesh_list[m].pos.y, pmesh_list[m].pos.z);
        glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(renderProgram.id, "model"), 1, GL_FALSE, glm::value_ptr(model));
        if (simplified){
            for (int i = 0; i < simpMeshes[m].VAO.size(); i++){
                if (conf.texture){
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, pmesh_list[m].mesh->getMesh()->getTexID(i - 1));
                }

                glBindVertexArray(simpMeshes[m].VAO[i]);
                glDrawElements(GL_TRIANGLES, simpMeshes[m].indSizes[i], GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
        else {
            int meshID = pmesh_list[m].meshID;
            for (int i = 0; i < origMeshes[meshID].VAO.size(); i++){
                if (conf.texture){
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, pmesh_list[m].mesh->getMesh()->getTexID(i - 1));
                }

                glBindVertexArray(origMeshes[meshID].VAO[i]);
                glDrawElements(GL_TRIANGLES, origMeshes[meshID].indSizes[i], GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
        glPopMatrix();
    }

    glUseProgram(0);
    
    if (inbuf != NULL && renderingMode != 0){
        if (renderingMode == 1){
            unsigned char* pixels = new unsigned char[3 * width * height];
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

            for (int i = 0; i < 3 * width * height; i++){
                pixels[i] = (unsigned char)max(min(2 * ((int)inbuf[i] - 127) + (int)pixels[i], 255), 0);
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            
            delete [] pixels;
        }
        else if (renderingMode == 2){
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, inbuf);
        }
    }
    
    if (visible){
        SDL_GL_SwapWindow(window);
    }
    
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, outframe->image);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, outframe->depth);
    
    outframe->mvp = projection * view;
    
    return true;
}

bool glModelWindow::displayImage(unsigned char* inbuf){
    if (inbuf != NULL){
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, inbuf);
    }
    
    SDL_GL_SwapWindow(window);
    
    return true;
}

#if 0
bool glModelWindow::displayImage(FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *prevframe, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *curframe){
    //translate
    glm::mat4 mvp = curframe->mvp * glm::inverse(prevframe->mvp);
    
    float zNear = 0.0F;
    float zFar = 1.0F;
    
    float halfHeight = (float)height / 2;
    float halfWidth = (float)width / 2;
    float zPlus = (zFar + zNear) / 2;
    float zMinus = (zFar - zNear) / 2;
    
    int index;

    unsigned char *warpedDiff = new unsigned char[3 * width * height];
    memset(warpedDiff, 127, 3 * width * height);
    float *warpedDepth = new float[width * height];
    memset(warpedDepth, 0, sizeof(float) * width * height);
    
    /* warping */
    index = 0;
    for (int h = 0; h < height; h++){
        for (int w = 0; w < width; w++){
            float d = prevframe->depth[index];
            glm::vec4 v((float)w / halfWidth - 1, (float)h / halfHeight - 1, (d - zPlus) / zMinus, 1);
            v = mvp * v;
            int x = round(((v[0] / v[3]) + 1) * halfWidth);
            int y = round(((v[1] / v[3]) + 1) * halfHeight);
            float z = zMinus * (v[2] / v[3]) + zPlus;
            if (x >= 0 && x < width && y >= 0 && y < height){
                int newIndex = y * width + x;
                if (warpedDepth[newIndex] == 0 || warpedDepth[newIndex] > z){
                    memcpy(&warpedDiff[3 * newIndex], &(prevframe->diff[3 * index]), 3);
                    warpedDepth[newIndex] = z;
                }
            }
            index++;
        }
    }
    
    // hole filling
    unsigned char *warpedDiff2 = new unsigned char[3 * width * height];
    
    for (int n = 0; n < HOLE_FILL_ROUND; n++){
        index = width + 1;
        memcpy(warpedDiff2, warpedDiff, 3 * width * height);
        for (int h = 1; h < height - 1; h++){
            for (int w = 1; w < width - 1; w++){
                if (warpedDepth[index] == 0){
                    int count = 0, rsum = 0, gsum = 0, bsum = 0;
                    for (int y = h - 1; y <= h + 1; y++){
                        for (int x = w - 1; x <= w + 1; x++){
                            int index2 = getIndex(x, y, width);
                            if (warpedDepth[index2] != -1){
                                count++;
                                rsum += (int)warpedDiff2[3 * index2];
                                gsum += (int)warpedDiff2[3 * index2 + 1];
                                bsum += (int)warpedDiff2[3 * index2 + 2];
                            }
                        }
                    }
                    if (count > 0){
                        warpedDiff[3 * index] = rsum / count;
                        warpedDiff[3 * index + 1] = gsum / count;
                        warpedDiff[3 * index + 2] = bsum / count;
                    }
                }
                index++;
            }
            index += 2;
        }
    }
    
    delete[] warpedDiff2;
        
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (renderingMode == 1){
        unsigned char* pixels = new unsigned char[3 * width * height];

        for (int i = 0; i < 3 * width * height; i++){
            pixels[i] = (unsigned char)max(min(2 * ((int)warpedDiff[i] - 127) + (int)curframe->image[i], 255), 0);
        }
        glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        
        delete [] pixels;
    }
    else if (renderingMode == 2){
        glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, warpedDiff);
    }
    else {
        glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, curframe->image);
    }
    
    delete[] warpedDiff;
    delete[] warpedDepth;
    
    SDL_GL_SwapWindow(window);
    
    return true;
}
#endif

bool glModelWindow::warpFrame(FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *frame, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *target){
    glUseProgram(warpProgram.id);

    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);

    glBindVertexArray(pixelPoints.VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, pixelPoints.VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char) * 3 * width * height, frame->image, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, pixelPoints.VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * width * height, frame->depth, GL_STATIC_DRAW);

    glm::mat4 mvp = target->mvp * glm::inverse(frame->mvp);
    glUniformMatrix4fv(glGetUniformLocation(warpProgram.id, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    float grey = 127.0 / 255.0;
    glClearColor(grey, grey, grey, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawElements(GL_POINTS, width * height, GL_UNSIGNED_INT, NULL);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, target->image);

    // hole filling
    int index;
    unsigned char *buf = new unsigned char[3 * width * height];
    float *depth = new float[width * height];
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
    
    for (int n = 0; n < HOLE_FILL_ROUND; n++){
        index = width + 1;
        memcpy(buf, target->image, 3 * width * height);
        for (int h = 1; h < height - 1; h++){
            for (int w = 1; w < width - 1; w++){
                if (depth[index] == 1){
                    int count = 0, rsum = 0, gsum = 0, bsum = 0;
                    for (int y = h - 1; y <= h + 1; y++){
                        for (int x = w - 1; x <= w + 1; x++){
                            int index2 = getIndex(x, y, width);
                            if (depth[index2] != 1){
                                count++;
                                rsum += (int)buf[3 * index2];
                                gsum += (int)buf[3 * index2 + 1];
                                bsum += (int)buf[3 * index2 + 2];
                            }
                        }
                    }
                    if (count > 0){
                        target->image[3 * index] = rsum / count;
                        target->image[3 * index + 1] = gsum / count;
                        target->image[3 * index + 2] = bsum / count;
                    }
                }
                index++;
            }
            index += 2;
        }
    }
    
    delete[] buf;
    delete[] depth;

    glBindVertexArray(0);
    glUseProgram(0);

    return true;
}

bool glModelWindow::patchFrame(FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *delta, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *frame){
    if (delta->mvp != frame->mvp){
        // warp needed
        FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> target;
        target.mvp = frame->mvp;
        warpFrame(delta, &target);
        for (int i = 0; i < 3 * width * height; i++){
            frame->image[i] = (unsigned char)max(min(2 * ((int)target.image[i] - 127) + (int)frame->image[i], 255), 0);
        }
    }
    else {
        // patch directly
        for (int i = 0; i < 3 * width * height; i++){
            frame->image[i] = (unsigned char)max(min(2 * ((int)delta->image[i] - 127) + (int)frame->image[i], 255), 0);
        }
    }
    
    return true;
}

bool glModelWindow::subImage(unsigned char* diff, unsigned char* orig, unsigned char* simp){
    for (int i = 0; i < 3 * width * height; i++){
        diff[i] = (unsigned char)max(min((((int)orig[i] - (int)simp[i]) / 2 + 127), 255), 0);
    }
    
    return true;
}

bool glModelWindow::subImage(short* diff, unsigned char* orig, unsigned char* simp){
    for (int i = 0; i < 3 * width * height; i++){
        diff[i] = (short)orig[i] - (short)simp[i];
    }
    
    return true;
}

bool glModelWindow::subImage(AVFrame* orig, AVFrame* simp){
    for (int i = 0; i < orig->linesize[0] * orig->height; i++){
        orig->data[0][i] = (unsigned char)(((int)orig->data[0][i] - (int)simp->data[0][i]) / 2 + 127);
    }
    for (int i = 0; i < orig->linesize[1] * orig->height / 2; i++){
        orig->data[1][i] = (unsigned char)(((int)orig->data[1][i] - (int)simp->data[1][i]) / 2 + 127);
    }
    for (int i = 0; i < orig->linesize[2] * orig->height / 2; i++){
        orig->data[2][i] = (unsigned char)(((int)orig->data[2][i] - (int)simp->data[2][i]) / 2 + 127);
    }
    
    return true;
}

bool glModelWindow::subDepth(float* diff, float* orig, float* simp){
    for (int i = 0; i < width * height; i++){
        diff[i] = orig[i] - simp[i];
    }
    
    return true;
}
