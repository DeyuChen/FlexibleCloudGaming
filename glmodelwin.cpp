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

    // Enable lights and lighting
    glEnable(GL_LIGHT0); // default value is (1.0, 1.0, 1.0, 1.0)
    glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE); // backface culling
    
    return true;
}

// Create OpenGL window
bool glModelWindow::createMyWindow(const char* title, int _width, int _height, unsigned char bits, bool fullscreenflag){
    width = _width;
    height = _height;
    
    warpingBuf.resize(3 * width * height);
    
    float halfHeight = (float)height / 2;
    float halfWidth = (float)width / 2;
    int index = 0;
    for (int h = 0; h < height; h++){
        for (int w = 0; w < width; w++){
            warpingBuf[index].pos[0] = (float)w / halfWidth - 1;
            warpingBuf[index].pos[1] = (float)h / halfHeight - 1;
            index++;
        }
    }

    bFullScreen_ = fullscreenflag;
    
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else {
        //Use OpenGL 2.1
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

void glModelWindow::reloadVertexBuffer(){
    if (VBO.empty()){
        VBO.resize(mesh_list.size());
        for (int i = 0; i < mesh_list.size(); i++){
            int numTex = mesh_list[i]->getNumTex() + 1;
            VBO[i].resize(numTex);
            for (int j = 0; j < mesh_list[i]->getNumTriangles(); j++){
                triangle t = mesh_list[i]->getTri(j);
                if (t.isActive()){
                    int texid = t.getTexnumber() + 1;
                    float *texcoord = t.getTexcoord();
                    const vertex& v1 = t.getVert1vertex();
                    VBO[i][texid].push_back(VertexBufferItem(v1.getArrayVerts(), v1.getArrayVertNorms(), v1.getArrayRGB(), texcoord));
                    const vertex& v2 = t.getVert2vertex();
                    VBO[i][texid].push_back(VertexBufferItem(v2.getArrayVerts(), v2.getArrayVertNorms(), v2.getArrayRGB(), texcoord + 2));
                    const vertex& v3 = t.getVert3vertex();
                    VBO[i][texid].push_back(VertexBufferItem(v3.getArrayVerts(), v3.getArrayVertNorms(), v3.getArrayRGB(), texcoord + 4));
                }
            }
        }
    }
    if (origMeshes.empty()){
        origMeshes.resize(mesh_list.size());
        for (int i = 0; i < mesh_list.size(); i++){
            GLfloat *buf = new GLfloat[9 * mesh_list[i]->getNumVerts()];
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
                memcpy(pbuf + 6, colors, 3 * sizeof(GLfloat));
                pbuf += 9;
            }

            glGenBuffers(1, &origMeshes[i].VBO);
            glBindBuffer(GL_ARRAY_BUFFER, origMeshes[i].VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9 * mesh_list[i]->getNumVerts(), buf, GL_STATIC_DRAW);

            delete[] buf;



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

                glBindBuffer(GL_ARRAY_BUFFER, origMeshes[i].VBO);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);

                glBindBuffer(GL_ARRAY_BUFFER, origMeshes[i].coordVBO[j]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * mesh_list[i]->getNumVerts(), texCoord[j].data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(3);

                origMeshes[i].indSizes[j] = indices[j].size();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, origMeshes[i].IBO[j]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * origMeshes[i].indSizes[j], indices[j].data(), GL_STATIC_DRAW);
            }
        }
    }
    
    for (int i = 0; i < SVBO.size(); i++){
        SVBO[i].clear();
    }
    SVBO.clear();
    
    SVBO.resize(pmesh_list.size());
    for (int i = 0; i < pmesh_list.size(); i++){
        int numTex = pmesh_list[i].mesh->getMesh()->getNumTex() + 1;
        SVBO[i].resize(numTex);
        for (int j = 0; j < pmesh_list[i].mesh->numTris(); j++){
            triangle t;
            if (pmesh_list[i].mesh->getTri(j, t) && t.isActive()){
                int texid = t.getTexnumber() + 1;
                float *texcoord = t.getTexcoord();
                const vertex& v1 = t.getVert1vertex();
                SVBO[i][texid].push_back(VertexBufferItem(v1.getArrayVerts(), v1.getArrayVertNorms(), v1.getArrayRGB(), texcoord));
                const vertex& v2 = t.getVert2vertex();
                SVBO[i][texid].push_back(VertexBufferItem(v2.getArrayVerts(), v2.getArrayVertNorms(), v2.getArrayRGB(), texcoord + 2));
                const vertex& v3 = t.getVert3vertex();
                SVBO[i][texid].push_back(VertexBufferItem(v3.getArrayVerts(), v3.getArrayVertNorms(), v3.getArrayRGB(), texcoord + 4));
            }
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
    if (VBO.empty()){
        reloadVertexBuffer();
    }
    
    // Set lookat point
    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);

    glRotatef(elevation, 1, 0, 0);
    glRotatef(azimuth, 0, 1, 0);
    glTranslatef(viewX, viewY, viewZ);
    
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
        glTranslatef(pmesh_list[m].pos.x, pmesh_list[m].pos.y, pmesh_list[m].pos.z);
        if (simplified){
            for (int i = 0; i < SVBO[m].size(); i++){
                if (conf.texture){
                    glBindTexture(GL_TEXTURE_2D, pmesh_list[m].mesh->getMesh()->getTexID(i - 1));
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexBufferItem), SVBO[m][i].data()->texcoord);
                }
                else {
                    Vec3 c = pmesh_list[m].mesh->getMesh()->getTexColor(i - 1);
                    glColor3ub(c.x, c.y, c.z);
                }

                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(3, GL_FLOAT, sizeof(VertexBufferItem), SVBO[m][i].data()->pos);
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_FLOAT, sizeof(VertexBufferItem), SVBO[m][i].data()->normal);
                
                glDrawArrays(GL_TRIANGLES, 0, SVBO[m][i].size());
                
                glDisableClientState(GL_NORMAL_ARRAY);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                
                if (conf.texture)
                    glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        else {
            for (int i = 0; i < VBO[pmesh_list[m].meshID].size(); i++){
                if (conf.texture){
                    glBindTexture(GL_TEXTURE_2D, pmesh_list[m].mesh->getMesh()->getTexID(i - 1));
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexBufferItem), VBO[pmesh_list[m].meshID][i].data()->texcoord);
                }
                else {
                    Vec3 c = pmesh_list[m].mesh->getMesh()->getTexColor(i - 1);
                    glColor3ub(c.x, c.y, c.z);
                }
                
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(3, GL_FLOAT, sizeof(VertexBufferItem), VBO[pmesh_list[m].meshID][i].data()->pos);
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_FLOAT, sizeof(VertexBufferItem), VBO[pmesh_list[m].meshID][i].data()->normal);
                
                glDrawArrays(GL_TRIANGLES, 0, VBO[pmesh_list[m].meshID][i].size());
                
                glDisableClientState(GL_NORMAL_ARRAY);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                
                if (conf.texture)
                    glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        glPopMatrix();
    }
    
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
    
    glm::mat4 modelview;
    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));
    glm::mat4 projection;
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));
    outframe->mvp = projection * modelview;
    
    return true;
}

bool glModelWindow::displayImage(unsigned char* inbuf){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (inbuf != NULL){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, inbuf);
    }
    
    SDL_GL_SwapWindow(window);
    
    return true;
}

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

    // GPU warping
    /*
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(transformPID);
    glUniformMatrix4fv(MVPID, 1, GL_FALSE, glm::value_ptr(mvp));
    
    // loading vertex
    index = 0;
    for (int h = 0; h < height; h++){
        for (int w = 0; w < width; w++){
            warpingBuf[index].pos[2] = (prevframe->depth[index] - zPlus) / zMinus;
            index++;
        }
    }
    glDrawArrays(GL_POINTS, 0, warpingBuf.size());
    glUseProgram(NULL);
    */
    
    unsigned char *warpedDiff = new unsigned char[3 * width * height];
    memset(warpedDiff, 127, 3 * width * height);
    float *warpedDepth = new float[width * height];
    memset(warpedDepth, 0, sizeof(float) * width * height);
    
    /*
    if (conf.frame_output_path.length() != 0){
        string fn = conf.frame_output_path + "/diff_" + to_string(prevframe->id) + ".rgb";
        ofstream ofs(fn);
        ofs.write((char *)prevframe->diff, 3 * width * height);
        ofs.close();
    }
    */
    
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
        
    /*
    if (conf.frame_output_path.length() != 0){
        string fn = conf.frame_output_path + "/warped_diff_" + to_string(prevframe->id) + ".rgb";
        ofstream ofs(fn);
        ofs.write((char *)warpedDiff, 3 * width * height);
        ofs.close();
    }
    */
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (renderingMode == 1){
        unsigned char* pixels = new unsigned char[3 * width * height];

        for (int i = 0; i < 3 * width * height; i++){
            pixels[i] = (unsigned char)max(min(2 * ((int)warpedDiff[i] - 127) + (int)curframe->image[i], 255), 0);
        }
        glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        
        /*
        if (conf.frame_output_path.length() != 0){
            string fn = conf.frame_output_path + "/patched_" + to_string(curframe->id) + ".rgb";
            ofstream ofs(fn);
            ofs.write((char *)pixels, 3 * width * height);
            ofs.close();
        }
        */
        
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

bool glModelWindow::patchFrame(unsigned char* inbuf, FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *frame){
    for (int i = 0; i < 3 * width * height; i++){
        frame->image[i] = (unsigned char)max(min(2 * ((int)inbuf[i] - 127) + (int)frame->image[i], 255), 0);
    }
    memcpy(frame->diff, inbuf, 3 * width * height);
    frame->patched = true;
    
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
