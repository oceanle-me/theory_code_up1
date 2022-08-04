#include "detecting.hpp"



void _4boxes_to_3boxes(cv::Rect &l_rec, cv::Rect &m_rec, cv::Rect &r_rec, cv::Rect &new_box );



cv::Rect Detect_3_persons(Mat &src,e_human_position _position){

    Mat image;
    int cam_width =src.cols;
    int cam_height=src.rows;

    //copy image to input as input tensor
    cv::resize(src, image, Size(320,320));
    memcpy(interpreter->typed_input_tensor<uchar>(0), image.data, image.total() * image.elemSize());

    interpreter->SetAllowFp16PrecisionForFp32(true);
    interpreter->SetNumThreads(2);      //quad core
    interpreter->Invoke();      // run your model

    const float* detection_locations = interpreter->tensor(interpreter->outputs()[0])->data.f;
    const float* detection_classes=interpreter->tensor(interpreter->outputs()[1])->data.f;
    const float* detection_scores = interpreter->tensor(interpreter->outputs()[2])->data.f;
    const int    num_detections = *interpreter->tensor(interpreter->outputs()[3])->data.f;

    const float confidence_threshold = 0.6;

    cv::Rect l_box(0,0,1,1);
    cv::Rect m_box (0,0,1,1);
    cv::Rect r_box(320,0,1,1);


    for(int i = 0; i < num_detections; i++){
        if(detection_scores[i] > confidence_threshold){
            int  det_index = (int)detection_classes[i]+1;
            if (det_index==1){ //detecting person only
                float y1=detection_locations[4*i  ]*cam_height;
                float x1=detection_locations[4*i+1]*cam_width;
                float y2=detection_locations[4*i+2]*cam_height;
                float x2=detection_locations[4*i+3]*cam_width;

                //Rect:   x coordinate of the top-left corner, y coordinate of the top-left corner
                cv::Rect _box((int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1));
                _4boxes_to_3boxes(l_box, m_box, r_box, _box);

            }
        }
    }

    if(l_box.x == 0){ //no left person
        l_box = m_box;
    }
    if(r_box.x == 320){ //no right person
        r_box = m_box;
    }

    if(_position==LEFT){
        DEBUG("Detect: Choosing left box\n");
        return l_box;
    } else if (_position==MID){
  //      DEBUG("Choosing mid box\n");
                 //   cv::rectangle(detect_frame, detect_bbox, Scalar( 255, 0, 0 ), 2, 1 );
                 //   cv::imshow("Detecting",detect_frame);

        return m_box;
    } else if (_position==RIGHT){
        DEBUG("Detect: Choosing right box\n");
        return r_box;
    }


    DEBUG("Detect: Tao lao here"<< __LINE__);
    return m_box;
}

void _4boxes_to_3boxes(cv::Rect &l_rec, cv::Rect &m_rec, cv::Rect &r_rec, cv::Rect &new_box ){
     int _position_l_rec = 160 - (l_rec.x + l_rec.width/2);
     int _position_m_rec = 160 - (m_rec.x + m_rec.width/2);
     int _position_r_rec = 160 - (r_rec.x + r_rec.width/2);
     int _position_new_box = 160 - (new_box.x + new_box.width/2);

     if(abs(_position_new_box) < abs(_position_m_rec)){
        m_rec = new_box;
     } else if (_position_new_box>0){
         if(_position_new_box < _position_l_rec){
            l_rec=new_box;
         }
     } else if (_position_new_box < 0){
         if(_position_new_box > _position_r_rec){
            r_rec=new_box;
         }
     }
}


