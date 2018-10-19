/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, standard IO, and, strings
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <sys/epoll.h>
#include <deque>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "glmodelwin.h"
#include "mesh.h"
#include "pmesh.h"
#include "conf.h"

#define MAX_EPOLL_EVENTS 8
#define FRAME_QUEUE_SIZE 50
#define FRAME_PER_SECOND 10
#define MAX_NUM_TICKETS 5

AVPixelFormat YUV_FORMAT = AV_PIX_FMT_YUV444P;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Input handler
void handleKeys(unsigned char key, int x, int y);

void handleMouse(int x, int y);

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
glModelWindow* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

Configuration conf;

bool init(){
    //Initialization flag
    bool success = true;
    
    unsigned char depth = 16;
    bool bFullScreen = false;
    gWindow = new glModelWindow();
    if (!gWindow || !gWindow->createMyWindow("Client Window", conf.screen_width, conf.screen_height, depth, bFullScreen)){
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

void handleKeys(Sint32 key, int x, int y){
    //Toggle quad
    if (key == 109){
        conf.renderingMode = (conf.renderingMode + 1) % 3;
    }
    if (key == 110){
        conf.renderingMode = (conf.renderingMode - 1) % 3;
    }
    gWindow->keyPress(key, x, y);
}

void handleMouse(int x, int y){
    gWindow->mouseMotion(x, y, false, false);
}

void close(){
    gWindow->killMyWindow();
}

int main(int argc, char* args[]){
    if (argc < 2){
        cout << "No configuration argument" << endl;
        return 1;
    }
        
    conf.open(args[1]);
    
    //Start up SDL and create window
    if (!init()){
        printf("Failed to initialize!\n");
    }
    else {
        //Main loop flag
        bool quit = false;
        bool eof = false;
                
        gWindow->setColor(conf.color);
        gWindow->setRenderingMode(conf.renderingMode);
        
        avcodec_register_all();
        av_register_all();
        
        int sockfd;
        int outbufsize = 4096;
        char *outbuf = new char[outbufsize];
        struct sockaddr_in serv_addr, cli_addr;
        
        unsigned long long pts = 0;
        int got_output;
        int framesize = conf.screen_width * conf.screen_height;
        int RGBFramesize = framesize * 3;
        int YUVFramesize = framesize * 3 / 2;
        int inbufsize = RGBFramesize;
        char *inbuf = new char[inbufsize + FF_INPUT_BUFFER_PADDING_SIZE];
        AVCodec *codec;
        AVCodecContext *c;
        AVPacket pkt;
        AVFrame *frameRGB, *frameYUV;
        SwsContext* swsContext = sws_getContext(conf.screen_width, conf.screen_height, YUV_FORMAT, conf.screen_width, conf.screen_height, AV_PIX_FMT_RGB24, SWS_SPLINE, NULL, NULL, NULL);
        if (!swsContext){
            perror("Failed to get sws context!");
            return 1;
        }
        
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec){
            perror("cannot find decoder h264");
            quit = true;
        }
        
        c = avcodec_alloc_context3(codec);
        if (!c){
            perror("AVCodecContext cannot be allocated");
            quit = true;
        }
        c->bit_rate = conf.bandwidth;
        c->width = conf.screen_width;
        c->height = conf.screen_height;
        c->time_base = (AVRational){conf.framerate_1, conf.framerate_2};
        c->gop_size = conf.gop;
        c->max_b_frames = 0;
        c->pix_fmt = YUV_FORMAT;
        av_opt_set(c->priv_data, "tune", "zerolatency", 0);
        av_opt_set(c->priv_data, "preset", "ultrafast", 0);
        av_opt_set(c->priv_data, "rc-lookahead", "0", 0);
        av_opt_set(c->priv_data, "sync-lookahead", "0", 0);
        
        if (avcodec_open2(c, codec, NULL) < 0) {
            perror("Could not open codec");
            quit = true;
        }
        
        frameRGB = av_frame_alloc();
        frameYUV = av_frame_alloc();
        if (!frameRGB || !frameYUV){
            perror("Could not allocate video frame");
            quit = true;
        }
        frameRGB->format = AV_PIX_FMT_RGB24;
        frameYUV->format = YUV_FORMAT;
        frameRGB->width = frameYUV->width = c->width;
        frameRGB->height = frameYUV->height = c->height;
        if (!av_image_alloc(frameRGB->data, frameRGB->linesize, frameRGB->width, frameRGB->height, AV_PIX_FMT_RGB24, 32)){
            perror("Could not allocate raw picture buffer");
            quit = true;
        }
        if (!av_image_alloc(frameYUV->data, frameYUV->linesize, frameYUV->width, frameYUV->height, YUV_FORMAT, 32)){
            perror("Could not allocate raw picture buffer");
            quit = true;
        }
        
        //Connect to the server
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        if (sockfd < 0) {
            perror("ERROR opening socket");
            exit(1);
        }
        
        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(conf.server_ip.c_str());
        serv_addr.sin_port = htons(conf.server_port);
        
        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("ERROR on connecting");
            exit(1);
        }
        
        struct epoll_event events[MAX_EPOLL_EVENTS];
        struct epoll_event event;
        event.data.fd = sockfd;
        event.events = EPOLLIN;
        
        int pollfd = epoll_create1(0);
        if (pollfd == -1){
            perror("epoll_create");
            abort();
        }
        if (epoll_ctl(pollfd, EPOLL_CTL_ADD, sockfd, &event) == -1){
            perror("epoll_ctl");
            abort();
        }
        
        //Load mesh
        for (int i = 0; i < conf.model_files.size(); i++){
            gWindow->loadMesh(conf.model_files[i]);
        }
        for (int i = 0; i < conf.models.size(); i++){
            gWindow->loadPMesh(conf.models[i].first, conf.models[i].second, conf.simp_algo, conf.mesh_percentage);
        }
        
        gWindow->setAzimuth(conf.azimuth);

        //Event handler
        SDL_Event e;
        SDL_SetRelativeMouseMode(SDL_TRUE);
        
        //Leaky bucket
        //auto next_ts = chrono::high_resolution_clock::now();
        //int ntickets = 0;
        
        int buffer_size = conf.delay_frames + 1;
        FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *frames = new FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT>[buffer_size];
        size_t buffer_head = 0;    // to patch and display
        size_t buffer_tail = 0;    // to insert new frame
        FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> *delta = new FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT>;
        
        srand(time(NULL));

        //While application is running
        while(!quit){
            //Display latest rendered frame
            /*
            if (!framebuf.empty()){
                if (framebuf.back()->patched){
                    //cout << 1 << endl;
                    gWindow->displayImage(framebuf.back()->image);
                }
                else if (framebuf.front()->patched){
                    //cout << 2 << endl;
                    gWindow->displayImage(framebuf.front(), framebuf.back());
                    
                    if (conf.frame_output_path.length() != 0){
                        string fn = conf.frame_output_path + "/" + to_string(framebuf.back()->id) + ".rgb";
                        ofstream ofs(fn);
                        ofs.write((char *)framebuf.back()->image, RGBFramesize);
                        ofs.close();
                    }
                }
                else {
                    //cout << 3 << endl;
                    gWindow->displayImage(framebuf.back()->image);
                }
            }
            */
            
            char *pbuf = outbuf + sizeof(int);
            
            int x = 0, y = 0;
            if (conf.in_commands.is_open())
                conf.in_commands >> x >> y;
            else
                SDL_GetRelativeMouseState(&x, &y);
            handleMouse(x, y);
            if (conf.out_commands.is_open())
                conf.out_commands << x << " " << y;
                        
            memcpy(pbuf, &x, sizeof(x));
            pbuf += sizeof(x);
            memcpy(pbuf, &y, sizeof(y));
            pbuf += sizeof(y);
            
            //Handle events on queue
            while (SDL_PollEvent(&e) != 0){
                //User requests quit
                if (e.type == SDL_QUIT){
                    quit = true;
                }
                //Handle keypress with current mouse position
                else if (!conf.in_commands.is_open() && e.type == SDL_KEYDOWN){
                    handleKeys(e.key.keysym.sym, x, y);
                    if (conf.out_commands.is_open())
                        conf.out_commands << " " << e.key.keysym.sym;
                    
                    memcpy(pbuf, &(e.key.keysym.sym), sizeof(e.key.keysym.sym));
                    pbuf += sizeof(e.key.keysym.sym);
                }
            }
            if (conf.in_commands.is_open()){
                string s;
                getline(conf.in_commands, s);
                istringstream iss(s);
                Sint32 key;
                while (iss >> key){
                    handleKeys(key, x, y);
                    memcpy(pbuf, &(key), sizeof(key));
                    pbuf += sizeof(key);
                }
                if (conf.in_commands.eof()){
                    conf.in_commands.close();
                    quit = true;
                }
            }
            if (conf.out_commands.is_open())
                conf.out_commands << endl;
            
            int size = pbuf - outbuf;
            memcpy(outbuf, &size, sizeof(int));
            send(sockfd, outbuf, size, 0);
            
            frames[buffer_tail].id = pts;
            //cout << "render pts: " << pts << endl;
            
            gWindow->displayMesh(NULL, &frames[buffer_tail], true, false);
            buffer_tail = (buffer_tail + 1) % buffer_size;
            
            int n_events = epoll_wait(pollfd, events, MAX_EPOLL_EVENTS, -1); // wait indefinitely for test
            if (n_events > 0){
                for (int i = 0; i < n_events; i++){
                    av_init_packet(&pkt);
                    
                    memset(inbuf, 0, inbufsize + FF_INPUT_BUFFER_PADDING_SIZE);
                    int n = recv(sockfd, inbuf, sizeof(pkt.size), 0);
                    if (n <= 0){
                        quit = true;
                        break;
                    }
                    memcpy(&pkt.size, inbuf, sizeof(pkt.size));
                    while (n < pkt.size){
                        int ret = recv(sockfd, inbuf + n, min(inbufsize, pkt.size - n), 0);
                        if (ret <= 0){
                            quit = true;
                            break;
                        }
                        n += ret;
                    }
                    if (quit)
                        break;
                    pkt.size -= sizeof(pts) + sizeof(pkt.size);
                    
                    unsigned long long rcv_pts;
                    memcpy(&rcv_pts, inbuf + sizeof(pkt.size), sizeof(rcv_pts));
                    //cout << "rcv_pts: " << rcv_pts << endl;
                    //cout << "pts: " << pts << endl;
                    
                    if (pkt.size > 0){
                        pkt.data = (uint8_t *)(inbuf + sizeof(pts) + sizeof(pkt.size));
                        if (double(rand()) / RAND_MAX < conf.artificial_loss_rate)
                            pkt.size = 0;
                        
                        if (avcodec_decode_video2(c, frameYUV, &got_output, &pkt) < 0){
                            perror("Error decoding frame");
                            //quit = true;
                        }
                        
                        sws_scale(swsContext, frameYUV->data, frameYUV->linesize, 0, conf.screen_height, frameRGB->data, frameRGB->linesize);
                        
                        // copy data to delta frame
                        memcpy(delta->image, frameRGB->data[0], RGBFramesize);
                        memset(delta->depth, 0, sizeof(float) * framesize);
                        for (int i = 0; i < buffer_size; i++){
                            if (frames[i].id == rcv_pts){
                                delta->mvp = frames[i].mvp;
                                memcpy(delta->depth, &frames[i].depth, sizeof(float) * framesize);
                            }
                        }
                        // TODO: cases when a corresponding frame cannot be found
                        
                        if (conf.bandwidth_log.is_open()){
                            conf.bandwidth_log << pts << " " << pkt.size << endl;
                        }
                        
                        switch (conf.renderingMode){
                            case 0:     // display unpatched frame
                                gWindow->displayImage(frames[buffer_head].image);
                                break;
                            case 1:     // display patched frame
                                gWindow->patchFrame(delta, &frames[buffer_head]);
                                gWindow->displayImage(frames[buffer_head].image);
                                break;
                            case 2:     // display delta frame
                                gWindow->displayImage(frameRGB->data[0]);
                                break;
                        }

                        if (conf.frame_output_path.length() != 0){
                            string fn;
                            ofstream ofs;
                            
                            fn = conf.frame_output_path + "/" + to_string(pts) + "_diff.rgb";
                            ofs.open(fn);
                            ofs.write((char *)frameRGB->data[0], RGBFramesize);
                            ofs.close();
                            
                            fn = conf.frame_output_path + "/" + to_string(pts) + ".rgb";
                            ofs.open(fn);
                            ofs.write((char *)frames[buffer_head].image, RGBFramesize);
                            ofs.close();
                            
                            fn = conf.frame_output_path + "/" + to_string(pts) + ".mvp";
                            ofs.open(fn);
                            ofs.write((char *)&frames[buffer_head].mvp[0][0], sizeof(frames[buffer_head].mvp));
                            ofs.close();
                        }

                        buffer_head = (buffer_head + 1) % buffer_size;

                        pts++;
                    }
                }
            }
        }
        
        delete[] outbuf;
        delete[] inbuf;
        delete[] frames;
        delete delta;
    }

    //Free resources and close SDL
    close();

    return 0;
}
