#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cassert>
#include <deque>
#include <sys/stat.h>
#include <glm/glm.hpp>
#include <cv.h>

#define WIDTH 1280
#define HEIGHT 960
#define HOLE_FILL_ROUND 5

using namespace std;
using namespace cv;

int getIndex(int x, int y){
    return y * WIDTH + x;
}

int main(int argc, char *argv[]){
    if (argc < 4){
        cout << "usage: ./warp_simulator [server_dir] [client_dir] [delay] [buffer_size]" << endl;
        return -1;
    }
    
    struct stat buffer;
    string server_dir(argv[1]);
    if (server_dir[server_dir.length() - 1] != '/')
        server_dir += "/";
    string client_dir(argv[2]);
    if (client_dir[client_dir.length() - 1] != '/')
        client_dir += "/";
    int delay = atoi(argv[3]);
    int buffer_size = 1;
    if (argc > 4)
        buffer_size = atoi(argv[4]);
    
    string fname;
    vector<ofstream> ofs(buffer_size);
    for (int i = 0; i < buffer_size; i++){
        fname = client_dir + string("PSNR_delay_") + to_string(delay) + string("_buffer_") + to_string(i + 1) + ".txt";
        ofs[i].open(fname);
        assert(ofs[i].is_open());
        ofs[i] << "pts MSE PSNR SSIM(R) SSIM(G) SSIM(B) FILLED_PIXELS" << endl;
    }
    
    deque<unsigned char*> client_diff;
    deque<float *> server_depth;
    
    unsigned char *warped_diff = new unsigned char[3 * WIDTH * HEIGHT];
    unsigned char *warped_diff2 = new unsigned char[3 * WIDTH * HEIGHT];
    unsigned char *client_rgb = new unsigned char[3 * WIDTH * HEIGHT];
    unsigned char *client_patched_rgb = new unsigned char[3 * WIDTH * HEIGHT];
    unsigned char *server_rgb = new unsigned char[3 * WIDTH * HEIGHT];
    float *warped_depth = new float[WIDTH * HEIGHT];
    //glm::mat4 mvp_rgb, mvp_diff;
    glm::mat4 mvp_rgb;
    deque<glm::mat4> mvp_diff;
    glm::mat4 tmp_mat;
    
    double R = 255 * 255;
    
    float zNear = 0.0F;
    float zFar = 1.0F;
    
    float halfHeight = (float)HEIGHT / 2;
    float halfWidth = (float)WIDTH / 2;
    float zPlus = (zFar + zNear) / 2;
    float zMinus = (zFar - zNear) / 2;
    
    int pts = 80;
    while (true){
        fname = client_dir + to_string(pts) + string(".rgb");
        if (stat(fname.c_str(), &buffer) != 0)
            break;
                
        ifstream ifs(fname);
        assert(ifs.is_open());
        ifs.read((char*)client_rgb, 3 * WIDTH * HEIGHT);
        ifs.close();
        
        fname = server_dir + to_string(pts) + string(".rgb");
        ifs.open(fname);
        assert(ifs.is_open());
        ifs.read((char*)server_rgb, 3 * WIDTH * HEIGHT);
        ifs.close();
        
        mvp_diff.push_front(tmp_mat);
        if (client_diff.size() < buffer_size){
            client_diff.push_front(new unsigned char[3 * WIDTH * HEIGHT]);
            server_depth.push_front(new float[WIDTH * HEIGHT]);
        }
        else {
            client_diff.push_front(client_diff.back());
            client_diff.pop_back();
            server_depth.push_front(server_depth.back());
            server_depth.pop_back();
            mvp_diff.pop_back();
        }
        fname = client_dir + to_string(pts - delay) + string("_diff.rgb");
        ifs.open(fname);
        assert(ifs.is_open());
        ifs.read((char*)client_diff[0], 3 * WIDTH * HEIGHT);
        ifs.close();
        
        if (delay == 0){
            memcpy(warped_diff, client_diff[0], 3 * WIDTH * HEIGHT);
            /*
            double MSE = 0;
            unsigned char vs, vc;
            for (int i = 0; i < 3 * WIDTH * HEIGHT; i++){
                client_rgb[i] = (unsigned char)max(min((2 * ((int)warped_diff[i] - 127) + (int)client_rgb[i]), 255), 0);
                int diff = (int)client_rgb[i] - (int)server_rgb[i];
                MSE += diff * diff;
            }

            MSE /= (3 * WIDTH * HEIGHT);
            double PSNR = 10 * log10(R / MSE);
            
            cout << endl << pts << " " << MSE << " " << PSNR << endl;
            //ofs << pts << " " << MSE << " " << PSNR << endl;
            double rssim, gssim, bssim;
            ssim(client_rgb, server_rgb, rssim, gssim, bssim);
            
            ofs[0] << pts << " " << MSE << " " << PSNR << " " << rssim << " " << gssim << " " << bssim << " " << WIDTH * HEIGHT << endl;
            */
        }
        else {
            fname = server_dir + to_string(pts - delay) + string(".depth");
            ifs.open(fname);
            assert(ifs.is_open());
            ifs.read((char*)server_depth[0], sizeof(float) * WIDTH * HEIGHT);
            ifs.close();
            
            fname = client_dir + to_string(pts) + string(".mvp");
            ifs.open(fname);
            assert(ifs.is_open());
            ifs.read((char*)&mvp_rgb[0][0], sizeof(float) * 16);
            ifs.close();
            
            fname = client_dir + to_string(pts - delay) + string(".mvp");
            ifs.open(fname);
            assert(ifs.is_open());
            ifs.read((char*)&mvp_diff[0][0][0], sizeof(float) * 16);
            ifs.close();
            
            //memset(warped_diff, 127, 3 * WIDTH * HEIGHT);
            //memset(warped_depth, 0, sizeof(float) * WIDTH * HEIGHT);
            for (int buf_id = 0; buf_id < client_diff.size(); buf_id++){
                memset(warped_diff, 0, 3 * WIDTH * HEIGHT);
                memset(warped_depth, 0, sizeof(float) * WIDTH * HEIGHT);
                glm::mat4 mvp = mvp_rgb * glm::inverse(mvp_diff[buf_id]);
                
                // warping
                int index = 0;
                for (int h = 0; h < HEIGHT; h++){
                    for (int w = 0; w < WIDTH; w++){
                        float d = server_depth[buf_id][index];
                        glm::vec4 v((float)w / halfWidth - 1, (float)h / halfHeight - 1, (d - zPlus) / zMinus, 1);
                        v = mvp * v;
                        int x = round(((v[0] / v[3]) + 1) * halfWidth);
                        int y = round(((v[1] / v[3]) + 1) * halfHeight);
                        float z = zMinus * (v[2] / v[3]) + zPlus;
                        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT){
                            int newIndex = y * WIDTH + x;
                            if (warped_depth[newIndex] == 0 || warped_depth[newIndex] > z){
                                memcpy(&warped_diff[3 * newIndex], &(client_diff[buf_id][3 * index]), 3);
                                warped_depth[newIndex] = z;
                            }
                        }
                        index++;
                    }
                }
                
                cout << pts << endl;
                if (pts == 101){
                    ofstream screenshot;
                    
                    fname = client_dir + string("screenshot_") + to_string(buf_id) + ".rgb";
                    screenshot.open(fname);
                    screenshot.write((char *)warped_diff, 3 * WIDTH * HEIGHT);
                    screenshot.close();
                }
                
                int filled_pixels = 0;
                index = 0;
                for (int h = 0; h < HEIGHT; h++){
                    for (int w = 0; w < WIDTH; w++){
                        if (warped_depth[index] != 0)
                            filled_pixels++;
                        index++;
                    }
                }
                
                // hole filling
                /*
                for (int n = 0; n < HOLE_FILL_ROUND; n++){
                    index = WIDTH + 1;
                    memcpy(warped_diff2, warped_diff, 3 * WIDTH * HEIGHT);
                    for (int h = 1; h < HEIGHT - 1; h++){
                        for (int w = 1; w < WIDTH - 1; w++){
                            if (warped_depth[index] == 0){
                                int count = 0, rsum = 0, gsum = 0, bsum = 0;
                                for (int y = h - 1; y <= h + 1; y++){
                                    for (int x = w - 1; x <= w + 1; x++){
                                        int index2 = getIndex(x, y);
                                        if (warped_depth[index2] != -1){
                                            count++;
                                            rsum += (int)warped_diff2[3 * index2];
                                            gsum += (int)warped_diff2[3 * index2 + 1];
                                            bsum += (int)warped_diff2[3 * index2 + 2];
                                        }
                                    }
                                }
                                if (count > 0){
                                    warped_diff[3 * index] = rsum / count;
                                    warped_diff[3 * index + 1] = gsum / count;
                                    warped_diff[3 * index + 2] = bsum / count;
                                }
                            }
                            index++;
                        }
                        index += 2;
                    }
                }
                */
                /*
                double MSE = 0;
                unsigned char vs, vc;
                for (int i = 0; i < 3 * WIDTH * HEIGHT; i++){
                    client_patched_rgb[i] = (unsigned char)max(min((2 * ((int)warped_diff[i] - 127) + (int)client_rgb[i]), 255), 0);
                    int diff = (int)client_patched_rgb[i] - (int)server_rgb[i];
                    MSE += diff * diff;
                }

                MSE /= (3 * WIDTH * HEIGHT);
                double PSNR = 10 * log10(R / MSE);
                
                cout << endl << pts << " " << MSE << " " << PSNR << endl;
                double rssim, gssim, bssim;
                ssim(client_patched_rgb, server_rgb, rssim, gssim, bssim);
                
                ofs[buf_id] << pts << " " << MSE << " " << PSNR << " " << rssim << " " << gssim << " " << bssim << " " << filled_pixels << endl;
                */
            }
        }
        
        pts++;
    }
    
    delete[] client_rgb;
    delete[] server_rgb;
    //delete[] client_diff;
    delete[] warped_diff;
    delete[] warped_diff2;
    //delete[] server_depth;
    delete[] warped_depth;
    while (!client_diff.empty()){
        delete[] client_diff.front();
        client_diff.pop_front();
    }
    while (!server_depth.empty()){
        delete[] server_depth.front();
        server_depth.pop_front();
    }
    
    for (int i = 0; i < buffer_size; i++)
        ofs[i].close();
    
    return 0;
}
