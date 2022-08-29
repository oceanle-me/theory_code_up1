#ifndef MY_GLOBAL_HPP_INCLUDED
#define MY_GLOBAL_HPP_INCLUDED

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <iostream>
#include <opencv2/core/ocl.hpp>
#include <opencv2/tracking.hpp>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/model.h"
#include <cmath>

#include <thread>
#include <queue>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>

#include <cstdlib>

#include "MY_GPIO.hpp"
#include "ir_hasher.hpp"

#include "KCF_tracking.hpp"
#include "CSRT_tracking.hpp"
#include "detecting.hpp"
#include "my_debug.hpp"


#define FPS_DETECT 6
using namespace cv;
using namespace std;

#define DISPLAY

typedef enum e_LEDstate {LEDOFF, MANUAL_STAND, MANUAL_MOVE, AUTO_DETECT, AUTO_MOVE} e_LEDstate;


template <typename T, int MaxLen, typename Container=std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
public:
    void push(const T& value) {
        if (this->size() == MaxLen) {
           this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
    T Front_pop(void){
        T Rt = this->front();
        this->pop();
        return Rt;
    }
};




// Placing all the variables here temporally, after fishing this project, move these into source file and extern these here
extern const size_t width;
extern const size_t height;


extern bool KCF_running;
extern bool end_program;
extern bool allow_KCF;

extern FixedQueue<bool, 1> que_bbox_not_according;
extern FixedQueue<bool, 1> que_human_not_present;

extern cv::Rect detect_bbox;
extern cv::Rect KCF_bbox;


extern cv::Mat detect_frame;
extern cv::Mat KCF_frame;

extern std::mutex mtx_protect_getframe;

extern FixedQueue<uint32_t, 1> q_IR_signal;

extern FixedQueue<e_LEDstate,1> q_LEDstate;
extern std::condition_variable cv_LED;



extern std::unique_ptr<tflite::Interpreter> interpreter;
extern cv::VideoCapture cap;


bool get_frame(Mat &src);
bool Bboxes_according(cv::Rect bbox1, cv::Rect bbox2);
void Resize_bbox(cv::Rect &bbox);

void IR_signal_come(uint32_t hash);


#endif // MY_GLOBAL_HPP_INCLUDED
