#include "my_global.hpp"

// Placing all the variables here temporally, after fishing this project, move these into source file and extern these here
const size_t width = 320;
const size_t height = 320;



bool KCF_running = false;
bool end_program = false;
bool allow_KCF = false;


FixedQueue<bool, 1> que_bbox_not_according;
FixedQueue<bool, 1> que_human_not_present;

cv::Rect detect_bbox;
cv::Rect KCF_bbox;


cv::Mat detect_frame;
cv::Mat KCF_frame;


FixedQueue<uint32_t, 1> q_IR_signal;

std::mutex mtx_protect_getframe;

FixedQueue<e_LEDstate,1> q_LEDstate;
std::condition_variable cv_LED;


std::unique_ptr<tflite::Interpreter> interpreter;
cv::VideoCapture cap("libcamerasrc ! video/x-raw,  width=(int)1280, height=(int)1280, framerate=(fraction)30/1 "
                     "! videoconvert ! videoscale ! video/x-raw, width=(int)320, height=(int)320 ! appsink", cv::CAP_GSTREAMER);


bool get_frame(Mat &src){
        mtx_protect_getframe.lock();
        cap >> src;
        if (src.empty()) {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            mtx_protect_getframe.unlock();
            return false;
        }
     //   imshow("RAW", src);
        cv::Mat lab_image;
        cv::cvtColor(src, lab_image, COLOR_BGR2Lab);

        // Extract the L channel
        std::vector<cv::Mat> lab_planes(3);
        cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]
        // apply the CLAHE algorithm to the L channel
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(4);
        cv::Mat dst;
        clahe->apply(lab_planes[0], dst);
        // Merge the the color planes back into an Lab image
        dst.copyTo(lab_planes[0]);
        cv::merge(lab_planes, lab_image);

        // convert back to RGB
        cv::Mat image_clahe;
        cv::cvtColor(lab_image, src, COLOR_Lab2BGR);
        mtx_protect_getframe.unlock();
        return true;
}

bool Bboxes_according(cv::Rect bbox1, cv::Rect bbox2){
    int _error = abs ((bbox1.x + bbox1.width/2) - (bbox2.x + bbox2.width/2));
    if (_error*5 > (bbox1.width) ){ //20% error
        return false;
    } else return true;
}

void Resize_bbox(cv::Rect &bbox){

            if(bbox.width>70){
                bbox.x=bbox.x + bbox.width/2 - 35;
                bbox.width=70;
            }
            if(bbox.height>150){
                bbox.y=bbox.y+10;
                bbox.height=150;
            }
}

void IR_signal_come(uint32_t hash)
{
    DEBUG("IR Signal comes\n");
    q_IR_signal.push(hash);
}



