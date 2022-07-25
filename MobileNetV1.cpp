#include "my_global.hpp"




std::vector<std::string> Labels;

#if 0

int main(int argc,char ** argv)
{
    InitPWM();
Bboxes_according(CSRT_bbox,CSRT_bbox);
    sleep(2);
    KCF_tracking();
    CSRT_tracking();
    Resize_bbox(CSRT_bbox);

    std::thread tracking_th(TrackingObj);
    cout <<"Creating tracking thread\n";

    // Load model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("handwave_head.tflite");
    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
    interpreter->AllocateTensors();

	// Get the names
	bool result = getFileContent("COCO_labels.txt");
	if(!result){ cout << "loading labels failed"; exit(-1); }


    cout << "Start grabbing, press ESC on Live window to terminate" << endl;
	while(!end_program){
        if (!get_frame(frame)){ goto break_all;} //getting the first frame

wait_start:
        if (detect_sign(frame)==detect_e::SIGN){
            cout <<"Start \n";
            gpioWrite(27,1);
            start_stop_sign = true;
        tracking:
            do{
                if(detect_sign(frame)==detect_e::SIGN){ //update bbox?? inside here
                    cout <<"Stop \n";
                    gpioWrite(27,0);
                    start_stop_sign = true;
                    goto wait_start;
                }
                if(end_program) goto break_all;
            }
            while(tracking_run);

            while(get_frame(frame)){  //retracking
                if(!detect_raw(frame)==detect_e::NO){ //human or sign is both well
                    //auto update bbox
                    human_detected =true;
                    goto tracking;
                }
            }


        }

        imshow("Detector and Tracking", frame);
        char esc = waitKey(5);
        if(esc == 27)
            goto break_all;
    }

break_all:
    gpioPWM(12,0);
    gpioPWM(13,0);
    cout << "Closing the camera" << endl;
    destroyAllWindows();
    cout << "Bye!" << endl;
    gpioTerminate();
    sleep(1);//waiting for closing all thread



  return 0;
}

#endif // 0



int main(int argc,char ** argv)
{
    cv::startWindowThread();
    cv::namedWindow("Detect");
    cv::namedWindow("KCF");
    cv::namedWindow("CSRT");

    InitPIN();
    sleep(1);
    //Init Infrared Pin  - 18
    Hasher ir(18, IR_signal_come);
    get_frame(detect_frame);

        cv::imshow("Detecting",       detect_frame);


    std::thread thread_KCF(KCF_tracking);
    std::thread thread_CSRT(CSRT_tracking);

    // Load model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("ssdlite_mobiledet_coco_qat_postprocess.tflite");
    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
    interpreter->AllocateTensors();

    cout << "Start grabbing, press ESC on Live window to terminate" << endl;

Wait_IR_label:
    while(1){
        if( ! q_IR_signal.empty()){
            uint32_t _tmp_IR = q_IR_signal.front(); q_IR_signal.pop();
            if( AUTO_MANUAL_IR == Check_IR_Type(_tmp_IR) ){
                DEBUG("Detect: OK IR signal, start detection\n");
                goto Get_frame_label;
            }
        }
    }
Get_frame_label:

    if( ! get_frame(detect_frame)){
        DEBUG("Detect: Getting a frame failed\n");
    }

    detect_bbox = Detect_3_persons(detect_frame,MID);
    if(detect_bbox.x == 0){ //no person detected
        goto Get_frame_label;
    }
    DEBUG("Detect: Person detected\n")
    DEBUG(detect_bbox);
    Resize_bbox(detect_bbox);



    q_dc_f.push(detect_frame);
    q_dc_b.push(detect_bbox);
    DEBUG("Detect: Push dc_f,dc_b \n");




Wait_que_cd_f_label:

    clock_t _timer = clock();
    while(q_cd_f.empty()){
        if( clock() - _timer > 3*CLOCKS_PER_SEC ){
            DEBUG("Detect: Time out 3s, q_cd_f empty, CSRT failure.\n");
            goto Get_frame_label;
        }
    }
    detect_frame = q_cd_f.front(); q_cd_f.pop();
    detect_bbox = Detect_3_persons(detect_frame,MID);

    if(end_program){
        DEBUG("Detect: End progam\n");
        ///////////////////////////////////////////////////////////////////////////do something
    }
    if( ! q_IR_signal.empty()){
        uint32_t _tmp_IR = q_IR_signal.front(); q_IR_signal.pop();
        if(AUTO_MANUAL_IR == Check_IR_Type(_tmp_IR)){
            if( ! que_human_not_present.empty()){
                que_human_not_present.pop();
            }
            goto Wait_IR_label;
        }
    }
    if( ! CSRT_run){
        DEBUG("Detect: CSRT_run = false\n")
        if( ! que_human_not_present.empty()){
            que_human_not_present.pop();
        }
    }
    if(detect_bbox.x == 0){// no human detected
        goto Wait_que_cd_f_label;
    }
    if(q_cd_b.empty()){
        DEBUG("Detect: q_cd_b empty, while waiting a long time??\n")
    }
    ///check human present??
     goto Wait_que_cd_f_label;


End_program_label:

    Stop_PIN();

    if(thread_CSRT.joinable()) thread_CSRT.join();
    if(thread_KCF.joinable()) thread_KCF.join();




return 0;



}
