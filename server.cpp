/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, standard IO, and, strings
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string>
#include <netinet/in.h>

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

AVPixelFormat YUV_FORMAT = AV_PIX_FMT_YUV444P;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes matrices and clear color
//bool initGL();

//Input handler
void handleKeys(unsigned char key, int x, int y);

void handleMouse(int x, int y);

//Frees media and shuts down SDL
void close();

//vector<Mesh*> mesh_list;
//vector<PMesh*> pmesh_list;

// Edge Collapse Options
//PMesh::EdgeCost g_edgemethod = PMesh::QUADRICTRI;

//The window we'll be rendering to
//SDL_Window* gWindow = NULL;
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
    if (!gWindow || !gWindow->createMyWindow("Server Window", conf.screen_width, conf.screen_height, depth, bFullScreen)){
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

void handleKeys(Sint32 key, int x, int y){
    //Toggle quad
    gWindow->keyPress(key, x, y);
}

void handleMouse(int x, int y){
    gWindow->mouseMotion(x, y, false, false);
}

void close(){
    gWindow->killMyWindow();
}

int main(int argc, char* args[])
{
    if (argc < 2){
        cout << "No configuration argument" << endl;
        return 1;
    }
        
    conf.open(args[1]);
    
    int sockfd, clisockfd;
    int bufsize = 4096;
    char *inbuf = new char[bufsize];
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> frame_high;
    FrameInfo<DEFAULT_WIDTH, DEFAULT_HEIGHT> frame_low;
    unsigned short depth_diff[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    
    unsigned long long pts = 0;
    int got_output;
    int framesize = conf.screen_width * conf.screen_height;
    int RGBFramesize = framesize * 3;
    int YUVFramesize = framesize * 3 / 2;
    char *outbuf = new char[RGBFramesize];
    AVCodec *codec, *codec_depth;
    AVCodecContext *c, *c_depth;
    AVPacket pkt, pkt_depth;
    AVFrame *frameRGB, *frameYUV, *frameDepthRGB, *frameDepthYUV;
    SwsContext* swsContext = sws_getContext(conf.screen_width, conf.screen_height, AV_PIX_FMT_RGB24, conf.screen_width, conf.screen_height, YUV_FORMAT, SWS_SPLINE, NULL, NULL, NULL);
    if (!swsContext){
        perror("Failed to get sws context!");
        return 1;
    }
    SwsContext* swsContextR = sws_getContext(conf.screen_width, conf.screen_height, YUV_FORMAT, conf.screen_width, conf.screen_height, AV_PIX_FMT_RGB24, SWS_SPLINE, NULL, NULL, NULL);
    if (!swsContextR){
        perror("Failed to get sws context!");
        return 1;
    }
    SwsContext* swsContext2 = sws_getContext(2 * conf.screen_width, conf.screen_height, AV_PIX_FMT_BGR8, conf.screen_width, conf.screen_height, YUV_FORMAT, SWS_SPLINE, NULL, NULL, NULL);
    if (!swsContext2){
        perror("Failed to get sws context!");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(conf.server_port);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("ERROR on binding");
        exit(1);
    }
    
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    
    clisockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (clisockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }
    
    //Start up SDL and create window
    if (!init()){
        printf("Failed to initialize!\n");
    }
    else {
        //Main loop flag
        bool quit = false;
        
        gWindow->setColor(conf.color);
        gWindow->setRenderingMode(conf.renderingMode);
        
        //Start the encoder
        avcodec_register_all();
        av_register_all();
        
        // codec for image residue
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!codec){
            perror("cannot find encoder libx264");
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
        //av_opt_set(c->priv_data, "qp", 0, 0);
        av_opt_set(c->priv_data, "rc-lookahead", "0", 0);
        av_opt_set(c->priv_data, "sync-lookahead", "0", 0);
        if (conf.lossless)
            av_opt_set(c->priv_data, "crf", "0", 0);
        
        if (avcodec_open2(c, codec, NULL) < 0) {
            perror("Could not open codec");
            quit = true;
        }
        
        
        // codec for depth residue
        codec_depth = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!codec_depth){
            perror("cannot find encoder libx264");
            quit = true;
        }
        
        c_depth = avcodec_alloc_context3(codec_depth);
        if (!c_depth){
            perror("AVCodecContext cannot be allocated");
            quit = true;
        }
        c_depth->bit_rate = conf.bandwidth;
        c_depth->width = 4 * conf.screen_width;
        c_depth->height = conf.screen_height;
        c_depth->time_base = (AVRational){conf.framerate_1, conf.framerate_2};
        c_depth->gop_size = conf.gop;
        c_depth->max_b_frames = 0;
        c_depth->pix_fmt = YUV_FORMAT;
        av_opt_set(c_depth->priv_data, "tune", "zerolatency", 0);
        av_opt_set(c_depth->priv_data, "preset", "ultrafast", 0);
        av_opt_set(c_depth->priv_data, "qp", 0, 0);
        av_opt_set(c_depth->priv_data, "rc-lookahead", "0", 0);
        av_opt_set(c_depth->priv_data, "sync-lookahead", "0", 0);
        
        if (avcodec_open2(c_depth, codec_depth, NULL) < 0) {
            perror("Could not open codec");
            quit = true;
        }
        
        
        frameRGB = av_frame_alloc();
        frameYUV = av_frame_alloc();
        frameDepthRGB = av_frame_alloc();
        frameDepthYUV = av_frame_alloc();
        if (!frameRGB || !frameYUV || !frameDepthRGB || !frameDepthYUV){
            perror("Could not allocate video frame");
            quit = true;
        }
        frameRGB->format = AV_PIX_FMT_RGB24;
        frameYUV->format = YUV_FORMAT;
        frameDepthRGB->format = AV_PIX_FMT_BGR8;
        frameDepthYUV->format = YUV_FORMAT;
        
        frameRGB->width = frameYUV->width = conf.screen_width;
        frameRGB->height = frameYUV->height = conf.screen_height;
        frameDepthRGB->width = frameDepthYUV->width = 4 * conf.screen_width;
        frameDepthRGB->height = frameDepthYUV->height = conf.screen_height;
        if (!av_image_alloc(frameRGB->data, frameRGB->linesize, frameRGB->width, frameRGB->height, AV_PIX_FMT_RGB24, 32)){
            perror("Could not allocate raw picture buffer");
            quit = true;
        }
        if (!av_image_alloc(frameYUV->data, frameYUV->linesize, frameYUV->width, frameYUV->height, YUV_FORMAT, 32)){
            perror("Could not allocate raw picture buffer");
            quit = true;
        }
        if (!av_image_alloc(frameDepthRGB->data, frameDepthRGB->linesize, frameDepthRGB->width, frameDepthRGB->height, AV_PIX_FMT_BGR8, 32)){
            perror("Could not allocate raw picture buffer");
            quit = true;
        }
        if (!av_image_alloc(frameDepthYUV->data, frameDepthYUV->linesize, frameDepthYUV->width, frameDepthYUV->height, YUV_FORMAT, 32)){
            perror("Could not allocate raw picture buffer");
            quit = true;
        }
        
        for (int i = 0; i < conf.model_files.size(); i++){
            gWindow->loadMesh(conf.model_files[i]);
        }
        for (int i = 0; i < conf.models.size(); i++){
            gWindow->loadPMesh(conf.models[i].first, conf.models[i].second, conf.simp_algo, conf.mesh_percentage);
        }
        
        gWindow->setAzimuth(conf.azimuth);

        //Event handler
        SDL_Event e;

        //While application is running
        int n_screenshot = 0;
        while(!quit){
            bool capture = false;
            int size;
            int n = recv(clisockfd, inbuf, sizeof(size), 0);
            if (n <= 0) {
                quit = true;
                break;
            }
            memcpy(&size, inbuf, sizeof(size));
            while (n < size){
                int ret = recv(clisockfd, inbuf + n, min(bufsize, size - n), 0);
                if (ret <= 0){
                    quit = true;
                    break;
                }
                n += ret;
            }
            
            if (!quit){
                int x = 0, y = 0;
                int shift = 4;
                memcpy((void *)&x, inbuf + shift, sizeof(x));
                shift += sizeof(x);
                memcpy((void *)&y, inbuf + shift, sizeof(y));
                shift += sizeof(y);
                handleMouse(x, y);
                                
                //Handle events on queue
                while (shift < size){
                    Sint32 key;
                    memcpy((void *)&key, inbuf + shift, sizeof(Sint32));
                    shift += sizeof(Sint32);
                    handleKeys(key, x, y);
                    if (key == SDLK_c)
                        capture = true;
                }
                while (SDL_PollEvent(&e) != 0){
                    if (e.type == SDL_QUIT){
                        quit = true;
                    }
                }
                
                // render original content
                gWindow->displayMesh(NULL, &frame_high, false, false);
                
                // render simplified content
                gWindow->displayMesh(NULL, &frame_low, true, false);
                
                // calculate the difference
                gWindow->subImage(frameRGB->data[0], frame_high.image, frame_low.image);
                if (conf.send_depth_diff)
                    gWindow->subDepth((float *)frameDepthRGB->data[0], frame_high.depth, frame_low.depth);
                
                if (capture && conf.screenshot_path.length() != 0){
                    string fn = conf.screenshot_path + "/" + to_string(n_screenshot) + ".rgb";
                    ofstream ofs(fn);
                    ofs.write((char *)frame_low.image, RGBFramesize);
                    ofs.close();
                    
                    fn = conf.screenshot_path + "/diff_" + to_string(n_screenshot++) + ".rgb";
                    ofs.open(fn);
                    ofs.write((char *)frameRGB->data[0], RGBFramesize);
                    ofs.close();
                }
                
                string fn;
                ofstream ofs;
                if (conf.frame_output_path.length() != 0){
                    fn = conf.frame_output_path + "/" + to_string(pts) + ".rgb";
                    ofs.open(fn);
                    ofs.write((char *)frame_high.image, RGBFramesize);
                    ofs.close();
                    
                    fn = conf.frame_output_path + "/" + to_string(pts) + ".depth";
                    ofs.open(fn);
                    ofs.write((char *)frame_high.depth, framesize * sizeof(float));
                    ofs.close();
                    
                    // original diff
                    /*
                    fn = conf.frame_output_path + "/" + to_string(pts) + ".diff0";
                    ofs.open(fn);
                    ofs.write((char *)originalDiff, sizeof(short) * RGBFramesize);
                    ofs.close();
                    
                    fn = conf.frame_output_path + "/" + to_string(pts) + ".diff1";
                    ofs.open(fn);
                    ofs.write((char *)frameRGB->data[0], RGBFramesize);
                    ofs.close();
                    */
                }
                
                sws_scale(swsContext, frameRGB->data, frameRGB->linesize, 0, conf.screen_height, frameYUV->data, frameYUV->linesize);
                if (conf.send_depth_diff)
                    sws_scale(swsContext2, frameDepthRGB->data, frameDepthRGB->linesize, 0, conf.screen_height, frameDepthYUV->data, frameDepthYUV->linesize);
                
                // encode image residue
                av_init_packet(&pkt);
                pkt.data = NULL;
                pkt.size = 0;
                if (avcodec_encode_video2(c, &pkt, frameYUV, &got_output) < 0){
                    perror("Error encoding frame");
                    quit = true;
                }
                if (got_output){
                    /*
                    if (conf.bandwidth_log.is_open()){
                        conf.bandwidth_log << "image, " << pkt.size << endl;
                    }
                    */
                    
                    int pktsize = pkt.size + sizeof(pts) + sizeof(int);
                    memcpy(outbuf, &pktsize, sizeof(int));
                    memcpy(outbuf + sizeof(int), &pts, sizeof(pts));
                    memcpy(outbuf + sizeof(int) + sizeof(pts), pkt.data, pkt.size);
                    int ret = send(clisockfd, outbuf, pktsize, 0);
                    pts++;
                }
                else {
                    int pktsize = sizeof(pkt.size);
                    memcpy(outbuf, &pktsize, sizeof(pkt.size));
                    int ret = send(clisockfd, outbuf, pktsize, 0);
                }
                
                // encode depth residue
                if (conf.send_depth_diff){
                    av_init_packet(&pkt_depth);
                    pkt_depth.data = NULL;
                    pkt_depth.size = 0;
                    if (avcodec_encode_video2(c_depth, &pkt_depth, frameDepthYUV, &got_output) < 0){
                        perror("Error encoding frame");
                        quit = true;
                    }
                    if (got_output){
                        /*
                        if (conf.bandwidth_log.is_open()){
                            conf.bandwidth_log << "depth, " << pkt.size << endl;
                        }
                        */
                    }
                    else {
                    }
                }
            }
        }
        
        delete[] inbuf;
        delete[] outbuf;
    }

    //Free resources and close SDL
    close();

    return 0;
}
