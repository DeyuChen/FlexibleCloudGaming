#ifndef __CONF_H__
#define __CONF_H__

#include <string>
#include <fstream>
#include <vector>
#include "pmesh.h"

using namespace std;

class Configuration {
public:
    Configuration() : color(false), texture(false), smooth(false), mesh_percentage(100),
                      scale(1), screen_width(640), screen_height(480), renderingMode(1), azimuth(0),
                      send_depth_diff(false), delay_frames(0), offline_test(false), lossless(false), showXYZ(false),
                      artificial_loss_rate(0)
                      {};
    Configuration(string fname);
    ~Configuration();
    
    void open(string fname);
    
    string server_ip;
    int server_port;
    
    int screen_width;
    int screen_height;
    PMesh::EdgeCost simp_algo;
    ifstream in_commands;
    ofstream out_commands;
    ofstream bandwidth_log;
    ofstream timing_log;
    string frame_output_path;
    string screenshot_path;
    vector<string> model_files;
    vector<pair<int, Vec3>> models;
    bool color;                         // Default: false
    bool texture;                       // Default: false
    bool smooth;                        // Default: false
    float scale;                        // Default: 1
    int mesh_percentage;                // Default: 100
    int renderingMode;                  // Default: 1
    float azimuth;                      // Default: 0
    
    int bandwidth;
    int gop;
    int framerate_1;
    int framerate_2;
    
    bool send_depth_diff;               // Default: false
    
    int delay_frames;                   // Default: 0
    bool offline_test;                  // Default: false
    bool lossless;
    bool showXYZ;
    
    float artificial_loss_rate;
};

#endif
