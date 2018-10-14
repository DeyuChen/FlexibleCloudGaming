#include "conf.h"
#include <iostream>
#include <sstream>

using namespace std;

Configuration::Configuration(string fname){
    open(fname);
}

Configuration::~Configuration(){
    if (in_commands.is_open())
        in_commands.close();
    if (out_commands.is_open())
        out_commands.close();
    if (bandwidth_log.is_open())
        bandwidth_log.close();
    if (timing_log.is_open())
        timing_log.close();
}

void Configuration::open(string fname){
    ifstream ifs(fname);
    string key, value;
    char line[1024];
    
    color = false;
    smooth = false;
    texture = false;
    mesh_percentage = 100;
    scale = 1;
    
    while (ifs.getline(line, 1024)){
        string s(line);
        istringstream iss(s);
        iss >> key >> value;
        
        if (key == "server_ip"){
            server_ip = value;
        }
        else if (key == "server_port"){
            server_port = stoi(value);
        }
        else if (key == "simplification_algorithm"){
            if (value == "SHORTEST")
                simp_algo = PMesh::SHORTEST;
            else if (value == "MELAX")
                simp_algo = PMesh::MELAX;
            else if (value == "QUADRIC")
                simp_algo = PMesh::QUADRIC;
            else if (value == "QUADRICTRI")
                simp_algo = PMesh::QUADRICTRI;
            else if (value == "MAX_EDGECOST")
                simp_algo = PMesh::MAX_EDGECOST;
            else
                cout << "Simplification algorithm not supported." << endl;
        }
        else if (key == "width"){
            screen_width = stoi(value);
        }
        else if (key == "height"){
            screen_height = stoi(value);
        }
        else if (key == "color"){
            if (value == "1")
                color = true;
        }
        else if (key == "texture"){
            if (value == "1")
                texture = true;
        }
        else if (key == "smooth"){
            if (value == "1")
                smooth = true;
        }
        else if (key == "scale"){
            scale = stoi(value);
        }
        else if (key == "input_command_log"){
            in_commands.open(value);
            if (out_commands.is_open())
                out_commands.close();
        }
        else if (key == "output_command_log"){
            if (!in_commands.is_open())
                out_commands.open(value);
        }
        else if (key == "bandwidth_log"){
            bandwidth_log.open(value);
        }
        else if (key == "timing_log"){
            timing_log.open(value);
        }
        else if (key == "frame_output_path"){
            frame_output_path = value;
        }
        else if (key == "model"){
            int id = stoi(value);
            iss >> value;
            if (model_files.size() <= id)
                model_files.resize(id + 1);
            model_files[id] = value;
        }
        else if (key == "model_pos"){
            int id = stoi(value);
            if (model_files.size() > id){
                float x, y, z;
                iss >> x >> y >> z;
                models.push_back(pair<int, Vec3>(id, Vec3(x, y, z)));
            }
        }
        else if (key == "mesh_percentage"){
            mesh_percentage = stoi(value);
        }
        else if (key == "bandwidth"){
            bandwidth = stoi(value);
        }
        else if (key == "gop"){
            gop = stoi(value);
        }
        else if (key == "framerate"){
            framerate_1 = stoi(value);
            iss >> value;
            framerate_2 = stoi(value);
        }
        else if (key == "screenshot_path"){
            screenshot_path = value;
        }
        else if (key == "renderingMode"){
            renderingMode = stoi(value);
        }
        else if (key == "azimuth"){
            azimuth = stof(value);
        }
        else if (key == "send_depth_diff"){
            if (value == "1")
                send_depth_diff = true;
        }
        else if (key == "delay_frames"){
            delay_frames = stoi(value);
        }
        else if (key == "offline_test"){
            if (value == "1")
                offline_test = true;
        }
        else if (key == "lossless"){
            if (value == "1")
                lossless = true;
        }
        else if (key == "showXYZ"){
            if (value == "1")
                showXYZ = true;
        }
        else if (key == "artificial_loss_rate"){
            artificial_loss_rate = stof(value);
        }
    }
}
