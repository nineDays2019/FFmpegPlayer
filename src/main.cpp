#include <iostream>
#include <opencv2/opencv.hpp>
#include <SDL/SDL.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace cv;
using namespace std;

int main() {
    Mat mat;
    avcodec_register_all();
    SDL_Init(0);
    cout << "HelloWorld" << endl;
    return 0;
}