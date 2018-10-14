// ******************************
// mesh.cpp
//
// Mesh class, which stores a list
// of vertices & a list of triangles.
//
// Jeff Somers
// Copyright (c) 2002
//
// jsomers@alumni.williams.edu
// March 27, 2002
// ******************************

#include <assert.h>
#include <float.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "mesh.h"


Mesh::Mesh(string& filename)
{
    _numVerts = _numTriangles = 0;
    if (!loadFromFile(filename))
    {
        // we failed to load mesh from the file
        _numVerts = _numTriangles = 0;
        _vlist.clear();
        _plist.clear();
        texlist.clear();
        texColors.clear();
    }
}

Mesh::Mesh(const Mesh& m)
{
    _numVerts = m._numVerts;
    _numTriangles = m._numTriangles;
    _vlist = m._vlist; // NOTE: triangles are still pointing to original mesh
    _plist = m._plist;
    texlist = m.texlist;
    texColors = m.texColors;
    // NOTE: should reset tris in _vlist, _plist
}

Mesh& Mesh::operator=(const Mesh& m)
{
    if (this == &m) return *this; // don't assign to self
    _numVerts = m._numVerts;
    _numTriangles = m._numTriangles;
    _vlist = m._vlist; // NOTE: triangles are still pointing to original mesh
    _plist = m._plist;
    texlist = m.texlist;
    texColors = m.texColors;
    // NOTE: should reset tris in _vlist, _plist
    return *this;
}

Mesh::~Mesh()
{
    _numVerts = _numTriangles = 0;
    //_vlist.erase(_vlist.begin(), _vlist.end());
    //_plist.erase(_plist.begin(), _plist.end());
    _vlist.clear();
    _plist.clear();
    texlist.clear();
    texColors.clear();
}

// Helper function for reading PLY mesh file
bool Mesh::readPlyHeader(ifstream &ifs)
{
    char line[1024];
    
    while (ifs.getline(line, 1024)){
        ChangeStrToLower(line);
        string s(line);
        if (s == "end_header")
            break;
        
        istringstream iss(s);
        string v;
        iss >> v;
        if (v == "element"){
            iss >> v;
            if (v == "vertex"){
                iss >> v;
                _numVerts = stoi(v);
            }
            else if (v == "face"){
                iss >> v;
                _numTriangles = stoi(v);
            }
        }
        else if (v == "comment"){
            iss >> v;
            if (v == "texturefile"){
                iss >> v;
                
                cout << "Loading texture file: " << v << endl;
                SDL_Surface *image = IMG_Load(v.c_str());
                GLuint texture_id;
                int mode;
                if (image->format->BytesPerPixel == 3){
                    mode = GL_RGB;
                }
                else if (image->format->BytesPerPixel == 4){
                    mode = GL_RGBA;
                }
                
                glGenTextures(1, &texture_id);
                glBindTexture(GL_TEXTURE_2D, texture_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, // target
                    0,  // level, 0 = base, no minimap,
                    mode, // internalformat
                    image->w,  // width
                    image->h,  // height
                    0,  // border, always 0 in OpenGL ES
                    mode,  // format
                    GL_UNSIGNED_BYTE, // type
                    image->pixels);
                glBindTexture(GL_TEXTURE_2D, 0);
                texlist.push_back(texture_id);
                
                unsigned long long R, G, B;
                R = G = B = 0;
                for (int y = 0; y < image->h; y++){
                    for (int x = 0; x < image->w; x++){
                        Uint8 *p = (Uint8 *)image->pixels + y * image->pitch + x * image->format->BytesPerPixel;
                        uint8_t r;
                        uint8_t g;
                        uint8_t b;
                        SDL_GetRGB(*(uint32_t *)p, image->format ,  &r, &g, &b);
                        R += r;
                        G += g;
                        B += b;
                    }
                }
                int size = image->w * image->h;
                R /= size;
                G /= size;
                B /= size;
                
                texColors.push_back(Vec3(R, G, B));
                
                SDL_FreeSurface(image);
            }
        }
    }

    return true;
}

// Helper function for reading PLY mesh file
bool Mesh::readPlyVerts(ifstream &ifs)
{
    char line[1024];
    
    for (int i = 0; i < _numVerts; i++)
    {
        ifs.getline(line, 1024);
        string s(line);
        istringstream iss(s);
        float x, y, z;
        int r, g, b;
        iss >> x >> y >> z >> r >> g >> b;
        vertex v(x, y, z, r, g, b);
        v.setIndex(i);

        _vlist.push_back(v); // push_back puts a *copy* of the element at the end of the list
    }
    return true;
}

// Helper function for reading PLY mesh file
bool Mesh::readPlyTris(ifstream &ifs)
{
    char line[1024];
    
    for (int i = 0; i < _numTriangles; i++){
        ifs.getline(line, 1024);
        string s(line);
        istringstream iss(s);
        
        int v1, v2, v3, v4;
        float x1, x2, x3, y1, y2, y3;
        int n1, n2;
        
        iss >> n1 >> v1 >> v2 >> v3 >> n2 >> x1 >> y1 >> x2 >> y2 >> x3 >> y3 >> v4;
        
        if (n1 != 3 || n2 != 6){
            return false;
        }

        // make sure verts in correct range
        assert(v1 < _numVerts && v2 < _numVerts && v3 < _numVerts);

        triangle t(this, v1, v2, v3, v4, x1, y1, x2, y2, x3, y3);
        t.setIndex(i);

        _plist.push_back(t); // push_back puts a *copy* of the element at the end of the list

        // update each vertex w/ its neighbors (vertrices & triangles)
        _vlist[v1].addTriNeighbor(i);
        _vlist[v1].addVertNeighbor(v2);
        _vlist[v1].addVertNeighbor(v3);

        _vlist[v2].addTriNeighbor(i);
        _vlist[v2].addVertNeighbor(v1);
        _vlist[v2].addVertNeighbor(v3);

        _vlist[v3].addTriNeighbor(i);
        _vlist[v3].addVertNeighbor(v1);
        _vlist[v3].addVertNeighbor(v2);
    }
    return true;
}


// Load mesh from PLY file
bool Mesh::loadFromFile(string& filename)
{
    ifstream ifs(filename);
    if (!ifs.is_open()){
        return false;
    }

    // read header to PLY file
    if (!readPlyHeader(ifs))
    {
        return false;
    }

    // read vertex data from PLY file
    if (!readPlyVerts(ifs))
    {
        return false;
    }

    // read triangle data from PLY file
    if (!readPlyTris(ifs))
    {
        return false;
    }

    calcVertNormals();

    return true;
}


// Recalculate the normal for one vertex
void Mesh::calcOneVertNormal(unsigned vert)
{
    vertex& v = getVertex(vert);
    const set<int>& triset = v.getTriNeighbors();

    set<int>::iterator iter;

    Vec3 vec;

    for (iter = triset.begin(); iter != triset.end(); ++iter)
    {
        // get the triangles for each vertex & add up the normals.
        vec += getTri(*iter).getNormalVec3();
    }

    vec.normalize(); // normalize the vertex    
    v.setVertNomal(vec);
}


// Calculate the vertex normals after loading the mesh.
void Mesh::calcVertNormals()
{
    // Iterate through the vertices
    for (unsigned i = 0; i < _vlist.size(); ++i)
    {
        calcOneVertNormal(i);
    }
}


// Used for debugging
void Mesh::dump()
{
    std::cout << "*** Mesh Dump ***" << std::endl;
    std::cout << "# of vertices: " << _numVerts << std::endl;
    std::cout << "# of triangles: " << _numTriangles << std::endl;
    for (unsigned i = 0; i < _vlist.size(); ++i)
    {
        std::cout << "\tVertex " << i << ": " << _vlist[i] << std::endl;
    }
    std::cout << std::endl;
    for (unsigned i = 0; i < _plist.size(); ++i)
    {
        std::cout << "\tTriangle " << i << ": " << _plist[i] << std::endl;
    }
    std::cout << "*** End of Mesh Dump ***" << std::endl;
    std::cout << std::endl;
}

// Get min, max values of all verts
void Mesh::setMinMax(float min[3], float max[3])
{
    max[0] = max[1] = max[2] = -FLT_MAX;
    min[0] = min[1] = min[2] = FLT_MAX;

    for (unsigned int i = 0; i < _vlist.size(); ++i)
    {
        const float* pVert = _vlist[i].getArrayVerts();
        if (pVert[0] < min[0]) min[0] = pVert[0];
        if (pVert[1] < min[1]) min[1] = pVert[1];
        if (pVert[2] < min[2]) min[2] = pVert[2];
        if (pVert[0] > max[0]) max[0] = pVert[0];
        if (pVert[1] > max[1]) max[1] = pVert[1];
        if (pVert[2] > max[2]) max[2] = pVert[2];
    }
}

// Center mesh around origin.
// Fit mesh in box from (-1, -1, -1) to (1, 1, 1)
void Mesh::Normalize()  
{
    float min[3], max[3], Scale;

    setMinMax(min, max);

    Vec3 minv(min);
    Vec3 maxv(max);

    Vec3 dimv = maxv - minv;
    
    if (dimv.x >= dimv.y && dimv.x >= dimv.z) Scale = 2.0f/dimv.x;
    else if (dimv.y >= dimv.x && dimv.y >= dimv.z) Scale = 2.0f/dimv.y;
    else Scale = 2.0f/dimv.z;

    Vec3 transv = minv + maxv;

    transv *= 0.5f;

    for (unsigned int i = 0; i < _vlist.size(); ++i)
    {
        _vlist[i].getXYZ() -= transv;
        _vlist[i].getXYZ() *= Scale;
    }
}

void Mesh::Normalize(float scale)  
{
    float min[3], max[3], Scale;

    setMinMax(min, max);

    Vec3 minv(min);
    Vec3 maxv(max);

    Vec3 dimv = maxv - minv;
    
    if (dimv.x >= dimv.y && dimv.x >= dimv.z) Scale = 2.0f/dimv.x;
    else if (dimv.y >= dimv.x && dimv.y >= dimv.z) Scale = 2.0f/dimv.y;
    else Scale = 2.0f/dimv.z;
    
    Scale *= scale;

    Vec3 transv = minv + maxv;

    transv *= 0.5f;

    for (unsigned int i = 0; i < _vlist.size(); ++i)
    {
        _vlist[i].getXYZ() -= transv;
        _vlist[i].getXYZ() *= Scale;
    }
}

