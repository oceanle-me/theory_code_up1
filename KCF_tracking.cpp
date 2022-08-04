#include "KCF_tracking.hpp"



int KCF_tracking(void){
    while(1){
        DEBUG("KCF thread created\n")

Init_KCF_label:
        allow_KCF = false;
        control_motor(160,0);

        KCF_running = false;

        while( ! allow_KCF){
            //do nothing
            if(end_program){
                return -1;
            }
        }
        KCF_frame = detect_frame;
        KCF_bbox = detect_bbox;
        DEBUG("KCF: get frame and bôx from detect\n")

        { // re-init scope
            Ptr<Tracker> tracker;
            tracker = TrackerKCF::create();
            //tracker = TrackerCSRT::create();
            tracker->init(KCF_frame, KCF_bbox);
            DEBUG("KCF: init succeed\n")

            while(get_frame(KCF_frame)){
                uint64_t timer = cv::getTickCount();
                bool ok_tracking = tracker->update(KCF_frame, KCF_bbox);


                if(end_program){
                    control_motor(160,0);
                    DEBUG("KCF: end program, ending KCF thread\n")
                    return -1;
                }
                if( ! allow_KCF){
                    DEBUG("KCF is not allow\n")
                    goto Init_KCF_label;
                }

                if( ! ok_tracking){
                    DEBUG("KCF tracking failed\n")
                    goto Init_KCF_label;
                }
                KCF_running = true;
                control_motor(KCF_bbox.x + KCF_bbox.width/2,1);

                float fps = cv::getTickFrequency() / (float)(cv::getTickCount() - timer);
                putText(KCF_frame, format("FPS %0.2f",fps),Point(10,20),FONT_HERSHEY_SIMPLEX,0.6, Scalar(0, 255, 0));
                cv::rectangle(KCF_frame, KCF_bbox, Scalar( 0, 255, 0 ), 2, 1 );
                cv::imshow("KCF",KCF_frame);
                int kk =waitKey(5) ;
                if(kk== 27){
                    DEBUG("detect: ESC pressed\n")
                    end_program = true;
                }

            } // get frame scope

        } // re-init scope
    }//thread always runs - scope
}


bool Motor_running(void){
    int _x = KCF_bbox.x + KCF_bbox.width/2;
    if( (_x>150) && (_x<170) ){
        return false;
    } else{
    return true;
    }
}
