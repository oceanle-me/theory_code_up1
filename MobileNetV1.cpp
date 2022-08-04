#include "my_global.hpp"




std::vector<std::string> Labels;

e_human_position e_detecting_position;

cv::Rect KCF_bbox_forAccording;
int counter_unAccording;

int main(int argc,char ** argv)
{
  //  cv::startWindowThread();
 //   cv::namedWindow("Detecting");
  //  cv::namedWindow("KCF");
  //  cv::namedWindow("CSRT");

    InitPIN();
    sleep(1);
    //Init Infrared Pin  - 18
    Hasher ir(18, IR_signal_come);


    std::thread thread_KCF(KCF_tracking);
 //   std::thread thread_CSRT(CSRT_tracking);

    // Load model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("ssdlite_mobiledet_coco_qat_postprocess.tflite");
    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
    interpreter->AllocateTensors();

    cout << "Start grabbing, press ESC on Live window to terminate" << endl;
bool turning=false;
Wait_IR_label:
    while(1){
    if( ! q_IR_signal.empty()){
        uint32_t _tmp_IR = Check_IR_Type(q_IR_signal.Front_pop());
        switch(_tmp_IR){
            case AUTO_MANUAL_IR:
                DEBUG("Detect: Auto mode.\n");
                control_motor(160,0);
                e_detecting_position = e_human_position::MID;
                goto Detect_get_frame_label;
            case RIGHT_IR:
                DEBUG("Detect: RIGHT manual\n")
                if(turning){//turning right
                    turning = false;
                    control_motor(160,0);
                } else{
                    control_motor(100,1);
                    turning = true;
                }
                break;
            case LEFT_IR:
                DEBUG("Detect: Left manual\n")
                if(turning){//turning right
                    turning = false;
                    control_motor(160,0);
                } else{
                    control_motor(220,1);
                    turning = true;
                }
                break;
            case UNKNOWN_IR:
                break;//do nothing
            default:
                DEBUG("detect: IR tao tao" <<__LINE__)
        }
    }
    }



Detect_get_frame_label:
    while(1){
        if( ! get_frame(detect_frame)){
            DEBUG("Detect: Getting a frame failed\n");
        }
        detect_bbox = Detect_3_persons(detect_frame,e_detecting_position);
        if(detect_bbox.x != 0){
            DEBUG("Detect: Person detected\n")
            break;
        }

        if( ! q_IR_signal.empty()){
        uint32_t _tmp_IR = Check_IR_Type(q_IR_signal.Front_pop());
            if(_tmp_IR == AUTO_MANUAL_IR){
                DEBUG("Detect: Manual mode.\n")
                goto Wait_IR_label;
            }
        }
    }

    Resize_bbox(detect_bbox);

Allow_KCF_label:

    allow_KCF = true;
    while( ! KCF_running) {  //wait KCF for running
    }
    DEBUG("detect: KCF is running\n")

    counter_unAccording=0;

frame_fromKCF_to_detect_label:
    uint64_t timer = cv::getTickCount();
    float fps_detect;

    detect_frame = KCF_frame;
    KCF_bbox_forAccording = KCF_bbox;
    detect_bbox = Detect_3_persons(detect_frame, MID);
    Resize_bbox(detect_bbox);

    if(end_program){
        DEBUG("detect: end program\n");
        goto End_program_label;
    }

    if( ! q_IR_signal.empty()){
        uint32_t _tmp_IR = Check_IR_Type(q_IR_signal.Front_pop());
        switch(_tmp_IR){
            case AUTO_MANUAL_IR:
                DEBUG("Detect: Manual Mode\n");
                allow_KCF = false;
                goto Wait_IR_label;
            case RIGHT_IR:
                DEBUG("Detect: RIGHT boxx\n")
                allow_KCF = false;
                e_detecting_position = e_human_position::RIGHT;
                goto Detect_get_frame_label;
            case LEFT_IR:
                DEBUG("Detect: Left booxx\n")
                allow_KCF = false;
                e_detecting_position = e_human_position::LEFT;
                goto Detect_get_frame_label;
            case UNKNOWN_IR:
                break;//do nothing
            default:
                DEBUG("detect: IR tao tao" <<__LINE__)
        }
    }

    if( ! KCF_running){
        DEBUG("detect: KCF is not running\n")
        e_detecting_position = e_human_position::MID;
        goto Detect_get_frame_label;
    }

    if( ( ! Motor_running()) && ( ! Bboxes_according(detect_bbox,KCF_bbox_forAccording)) ){
        if(counter_unAccording == 5) {//3 times not according
            allow_KCF = false;
            while(KCF_running) {  //wait KCF for ptopping
            }
            DEBUG("Detect: not according bb, re Int KCF\n")
            goto Allow_KCF_label;

        } else{
            counter_unAccording ++;
            DEBUG("Detect: Not According --- "<<counter_unAccording<<"\n")
        }
    }else{
        counter_unAccording =0;
    }


    fps_detect = cv::getTickFrequency() / (float)(cv::getTickCount() - timer);
    DEBUG("detect: " << fps_detect <<"\n")

    goto frame_fromKCF_to_detect_label;


End_program_label:


  //  if(thread_CSRT.joinable()) thread_CSRT.join();
    if(thread_KCF.joinable()) thread_KCF.join();

    cv::destroyAllWindows();
    cap.release();
    Stop_PIN();
    DEBUG("RETURN MAIN\n Byebye \n")

    return 0;

}





