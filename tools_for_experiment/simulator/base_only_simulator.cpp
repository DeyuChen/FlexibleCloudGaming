#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cassert>
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

void ssim(unsigned char *rgb1, unsigned char *rgb2, double &rssim, double &gssim, double &bssim){
    Mat mat1(HEIGHT, WIDTH, CV_8UC3, rgb1);
    Mat mat2(HEIGHT, WIDTH, CV_8UC3, rgb2);
    
    // default settings
    double C1 = 6.5025, C2 = 58.5225;

    IplImage
        img1_temp, img2_temp,
        *img1=NULL, *img2=NULL, *img1_img2=NULL,
        *img1_sq=NULL, *img2_sq=NULL,
        *mu1=NULL, *mu2=NULL,
        *mu1_sq=NULL, *mu2_sq=NULL, *mu1_mu2=NULL,
        *sigma1_sq=NULL, *sigma2_sq=NULL, *sigma12=NULL,
        *ssim_map=NULL, *temp1=NULL, *temp2=NULL, *temp3=NULL;
    

    /***************************** INITS **********************************/
    img1_temp = IplImage(mat1);
    img2_temp = IplImage(mat2);

    int x=img1_temp.width, y=img1_temp.height;
    int nChan=img1_temp.nChannels, d=IPL_DEPTH_32F;
    CvSize size = cvSize(x, y);

    img1 = cvCreateImage( size, d, nChan);
    img2 = cvCreateImage( size, d, nChan);

    cvConvert(&img1_temp, img1);
    cvConvert(&img2_temp, img2);

    
    img1_sq = cvCreateImage( size, d, nChan);
    img2_sq = cvCreateImage( size, d, nChan);
    img1_img2 = cvCreateImage( size, d, nChan);
    
    cvPow( img1, img1_sq, 2 );
    cvPow( img2, img2_sq, 2 );
    cvMul( img1, img2, img1_img2, 1 );

    mu1 = cvCreateImage( size, d, nChan);
    mu2 = cvCreateImage( size, d, nChan);

    mu1_sq = cvCreateImage( size, d, nChan);
    mu2_sq = cvCreateImage( size, d, nChan);
    mu1_mu2 = cvCreateImage( size, d, nChan);
    

    sigma1_sq = cvCreateImage( size, d, nChan);
    sigma2_sq = cvCreateImage( size, d, nChan);
    sigma12 = cvCreateImage( size, d, nChan);

    temp1 = cvCreateImage( size, d, nChan);
    temp2 = cvCreateImage( size, d, nChan);
    temp3 = cvCreateImage( size, d, nChan);

    ssim_map = cvCreateImage( size, d, nChan);
    /*************************** END INITS **********************************/


    //////////////////////////////////////////////////////////////////////////
    // PRELIMINARY COMPUTING
    cvSmooth( img1, mu1, CV_GAUSSIAN, 11, 11, 1.5 );
    cvSmooth( img2, mu2, CV_GAUSSIAN, 11, 11, 1.5 );
    
    cvPow( mu1, mu1_sq, 2 );
    cvPow( mu2, mu2_sq, 2 );
    cvMul( mu1, mu2, mu1_mu2, 1 );


    cvSmooth( img1_sq, sigma1_sq, CV_GAUSSIAN, 11, 11, 1.5 );
    cvAddWeighted( sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq );
    
    cvSmooth( img2_sq, sigma2_sq, CV_GAUSSIAN, 11, 11, 1.5 );
    cvAddWeighted( sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq );

    cvSmooth( img1_img2, sigma12, CV_GAUSSIAN, 11, 11, 1.5 );
    cvAddWeighted( sigma12, 1, mu1_mu2, -1, 0, sigma12 );
    

    //////////////////////////////////////////////////////////////////////////
    // FORMULA

    // (2*mu1_mu2 + C1)
    cvScale( mu1_mu2, temp1, 2 );
    cvAddS( temp1, cvScalarAll(C1), temp1 );

    // (2*sigma12 + C2)
    cvScale( sigma12, temp2, 2 );
    cvAddS( temp2, cvScalarAll(C2), temp2 );

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    cvMul( temp1, temp2, temp3, 1 );

    // (mu1_sq + mu2_sq + C1)
    cvAdd( mu1_sq, mu2_sq, temp1 );
    cvAddS( temp1, cvScalarAll(C1), temp1 );

    // (sigma1_sq + sigma2_sq + C2)
    cvAdd( sigma1_sq, sigma2_sq, temp2 );
    cvAddS( temp2, cvScalarAll(C2), temp2 );

    // ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    cvMul( temp1, temp2, temp1, 1 );

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    cvDiv( temp3, temp1, ssim_map, 1 );


    CvScalar index_scalar = cvAvg( ssim_map );
    
    // through observation, there is approximately 
    // 1% error max with the original matlab program

    cout << index_scalar.val[2] * 100 << "%, ";
    cout << index_scalar.val[1] * 100 << "%, ";
    cout << index_scalar.val[0] * 100 << "%" << endl ;
    
    rssim = index_scalar.val[2];
    gssim = index_scalar.val[1];
    bssim = index_scalar.val[0];
    
    cvReleaseImage(&img1);
    cvReleaseImage(&img2);
    cvReleaseImage(&img1_sq);
    cvReleaseImage(&img2_sq);
    cvReleaseImage(&img1_img2);
    cvReleaseImage(&mu1);
    cvReleaseImage(&mu2);
    cvReleaseImage(&mu1_sq);
    cvReleaseImage(&mu2_sq);
    cvReleaseImage(&mu1_mu2);
    cvReleaseImage(&sigma1_sq);
    cvReleaseImage(&sigma2_sq);
    cvReleaseImage(&sigma12);
    cvReleaseImage(&temp1);
    cvReleaseImage(&temp2);
    cvReleaseImage(&temp3);
    cvReleaseImage(&ssim_map);

    // if you use this code within a program
    // don't forget to release the IplImages
    return;
}

int main(int argc, char *argv[]){
    if (argc < 4){
        cout << "usage: ./warp_simulator [server_dir] [client_dir] [delay]" << endl;
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
    
    string fname = client_dir + "PSNR_base.txt";
    ofstream ofs(fname);
    assert(ofs.is_open());
    ofs << "pts MSE PSNR SSIM(R) SSIM(G) SSIM(B)" << endl;
    
    unsigned char *client_rgb = new unsigned char[3 * WIDTH * HEIGHT];
    unsigned char *server_rgb = new unsigned char[3 * WIDTH * HEIGHT];
    
    double R = 255 * 255;
    
    int pts = delay;
    pts = delay;
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
        
        double MSE = 0;
        unsigned char vs, vc;
        for (int i = 0; i < 3 * WIDTH * HEIGHT; i++){
            int diff = (int)client_rgb[i] - (int)server_rgb[i];
            MSE += diff * diff;
        }

        MSE /= (3 * WIDTH * HEIGHT);
        double PSNR = 10 * log10(R / MSE);
        
        cout << endl << pts << " " << MSE << " " << PSNR << endl;
        double rssim, gssim, bssim;
        ssim(client_rgb, server_rgb, rssim, gssim, bssim);
        
        ofs << pts << " " << MSE << " " << PSNR << " " << rssim << " " << gssim << " " << bssim << endl;
        
        pts++;
    }
    
    delete[] client_rgb;
    delete[] server_rgb;
    
    ofs.close();
    
    return 0;
}