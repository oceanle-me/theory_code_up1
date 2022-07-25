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


template <typename T, int MaxLen, typename Container=std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
public:
    void push(const T& value) {
        if (this->size() == MaxLen) {
           this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
};




// Placing all the variables here temporally, after fishing this project, move these into source file and extern these here
extern const size_t width;
extern const size_t height;


extern bool KCF_run;
extern bool CSRT_run;
extern bool end_program;
extern FixedQueue<bool, 1> que_bbox_not_according;
extern FixedQueue<bool, 1> que_human_not_present;

extern cv::Rect detect_bbox;
extern cv::Rect KCF_bbox;
extern cv::Rect CSRT_bbox;

extern cv::Mat detect_frame;
extern cv::Mat KCF_frame;
extern cv::Mat CSRT_frame;

extern FixedQueue<cv::Mat, 1> q_dc_f;  //queue from detect to CSRT frame
extern FixedQueue<cv::Mat, 1> q_kc_f;  //queue from KCF to CSRT frame
extern FixedQueue<cv::Mat, 1> q_cd_f;
extern FixedQueue<cv::Mat, 1> q_ck_f;

extern FixedQueue<cv::Rect, 1> q_dc_b; //queue from detect to CSRT bounding box
extern FixedQueue<cv::Rect, 1> q_kc_b;
extern FixedQueue<cv::Rect, 1> q_cd_b;
extern FixedQueue<cv::Rect, 1> q_ck_b;

extern FixedQueue<uint32_t, 1> q_IR_signal;


extern std::unique_ptr<tflite::Interpreter> interpreter;
extern cv::VideoCapture cap;



bool get_frame(Mat &src);
bool Bboxes_according(cv::Rect bbox1, cv::Rect bbox2);
void Resize_bbox(cv::Rect &bbox);

void IR_signal_come(uint32_t hash);


#endif // MY_GLOBAL_HPP_INCLUDED
