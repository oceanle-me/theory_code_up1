#include "my_global.hpp"




std::vector<std::string> Labels;

e_human_position e_detecting_position;

cv::Rect KCF_bbox_forAccording;
int counter_unAccording;
bool _person_detected;



bool Check_End_Program(void){
    if(gpioRead(22)==0){
        end_program = true;
        DEBUG("END program\n")
        return true;
    }
    return false;
}



int main(int argc,char ** argv)
{
#ifdef DISPLAY
    cv::startWindowThread();
 //   cv::namedWindow("Detecting");
    cv::namedWindow("KCF");
  //  cv::namedWindow("CSRT");
#endif // DISPLAY

    InitPIN();
    sleep(1);
    //Init Infrared Pin  - 18
    Hasher ir(18, IR_signal_come);

    uint64_t timer = cv::getTickCount();
    float fps_detect;

    std::thread thread_KCF(KCF_tracking);
    std::thread  thread_LED_Control(LED_control);


    // Load model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("/home/pi/Desktop/ssdlite_mobiledet_coco_qat_postprocess.tflite");
    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
    interpreter->AllocateTensors();

    cout << "Start grabbing, press ESC on Live window to terminate" << endl;
bool turning=false;
Wait_IR_label:

    q_LEDstate.push(MANUAL_STAND);
    cv_LED.notify_one();
    while(1){
        if(Check_End_Program()) goto End_program_label;

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
                    q_LEDstate.push(MANUAL_STAND);
                    cv_LED.notify_one();
                } else{
                    control_motor(100,1);
                    turning = true;
                    q_LEDstate.push( MANUAL_MOVE);
                    cv_LED.notify_one();
                }
                break;
            case LEFT_IR:
                DEBUG("Detect: Left manual\n")
                if(turning){//turning right
                    turning = false;
                    control_motor(160,0);
                    q_LEDstate.push(MANUAL_STAND);
                    cv_LED.notify_one();
                } else{
                    control_motor(220,1);
                    turning = true;
                    q_LEDstate.push(MANUAL_MOVE);
                    cv_LED.notify_one();
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
    q_LEDstate.push(AUTO_DETECT);
    cv_LED.notify_one();

    while(1){
        if(Check_End_Program()) goto End_program_label;

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
            if(Check_End_Program()) goto End_program_label;
    }
    DEBUG("detect: KCF is running\n")

    counter_unAccording=0;

    q_LEDstate.push(AUTO_MOVE);
    cv_LED.notify_one();

frame_fromKCF_to_detect_label:

    timer = cv::getTickCount();


    mtx_protect_getframe.lock();
    detect_frame = KCF_frame;
    mtx_protect_getframe.unlock();

    KCF_bbox_forAccording = KCF_bbox;
    _person_detected = false;
    detect_bbox = Detect_3_persons(detect_frame, MID);
    if(detect_bbox.x != 0){
        DEBUG("Detect: Person detected\n")
        Resize_bbox(detect_bbox);
        _person_detected = true;
    }


    if(Check_End_Program()){
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
                e_detecting_position = e_human_position::LEFT;
                goto Detect_get_frame_label;
            case LEFT_IR:
                DEBUG("Detect: Left booxx\n")
                allow_KCF = false;
                e_detecting_position = e_human_position::RIGHT;
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

    if( ( ! Motor_running()) && _person_detected && ( ! Bboxes_according(detect_bbox,KCF_bbox_forAccording)) ){
        if(counter_unAccording == 1) {//2 times not according
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
    DEBUG("detect..."<< fps_detect <<"\n")

    goto frame_fromKCF_to_detect_label;


End_program_label:

    cv_LED.notify_all();
  //  if(thread_CSRT.joinable()) thread_CSRT.join();
    if(thread_KCF.joinable()) thread_KCF.join();
    if(thread_LED_Control.joinable()) thread_LED_Control.join();
#ifdef DISPLAY
    cv::destroyAllWindows();

#endif

    cap.release();
    gpioSetAlertFuncEx(18, nullptr, nullptr);
    Stop_PIN();
    DEBUG("RETURN MAIN\n Byebye \n")

    return 0;

}





