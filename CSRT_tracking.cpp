#include "CSRT_tracking.hpp"

int CSRT_tracking(void){
    while(1){
        DEBUG("CSRT thread created\n")



        while( ! allow_KCF){
            //do nothing
            if(end_program){
                return -1;
            }
        }
        CSRT_frame = detect_frame;
        CSRT_bbox = detect_bbox;
        DEBUG("CSRT: get frame and bôx from detect\n")

        { // re-init scope
            Ptr<Tracker> tracker;
           // tracker = TrackerKCF::create();
            tracker = TrackerCSRT::create();
            tracker->init(CSRT_frame, CSRT_bbox);
            DEBUG("CSRT: init succeed\n")

            while(1){
                CSRT_frame = KCF_frame;
                uint64_t timer = cv::getTickCount();
                bool ok_tracking = tracker->update(CSRT_frame, CSRT_bbox);


                if(end_program){
                    DEBUG("CSRT: end program, ending KCF thread\n")
                    return -1;
                }
                if( ! ok_tracking){
                        DEBUG("CSRT: failed, return \n")
                    return -1;
                }

                float fps = cv::getTickFrequency() / (float)(cv::getTickCount() - timer);
                DEBUG("CSRT: " << fps << "\n")

            } // get frame scope

        } // re-init scope
    }//thread always runs - scope
}
