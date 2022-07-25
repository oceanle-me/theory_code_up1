#include "CSRT_tracking.hpp"


int CSRT_tracking(void){
    while(1){
        DEBUG("CSRT thread created\n")

InitCSRT:

        CSRT_run= false;
        if( ! q_ck_f.empty()){
            q_ck_f.pop();
        }
        if( ! q_ck_b.empty()){
            q_ck_b.pop();
        }
        if( ! q_cd_f.empty()){
            q_cd_f.pop();
        }
        if( ! q_cd_b.empty()){
            q_cd_b.pop();
        }
        if( ! que_bbox_not_according.empty()){
            que_bbox_not_according.pop();
        }


        while(q_dc_f.empty()||q_dc_b.empty()){
            if(end_program){
                DEBUG("CSRT: End program\n")
                return -1;
            }
        }
        {   //this scope for retracking perpose, re init tracking object
            CSRT_frame = q_dc_f.front(); q_dc_f.pop();
            CSRT_bbox = q_dc_b.front();  q_dc_b.pop();


            DEBUG("CSRT: get dc_f, dc_b\n")

            Ptr<Tracker> tracker;
            tracker = TrackerCSRT::create();
            //tracker = TrackerGOTURN::create();
            tracker->init(CSRT_frame, CSRT_bbox);
            CSRT_run = true;

            DEBUG("CSRT: Init succeeded, push dc_f, dc_b\n")
            q_ck_f.push(CSRT_frame);
            q_ck_b.push(CSRT_bbox);

            clock_t _timer = clock();
            while(1){
                while(q_kc_f.empty()){
                    if(clock() - _timer > 2*CLOCKS_PER_SEC)
                    {
                        DEBUG("CSRT: Time out, q_kc_f empty, CSRT failure.\n");
                        goto InitCSRT;
                    }
                }

                CSRT_frame = q_kc_f.front(); q_kc_f.pop();
push_q_cd_f_PLACE:
                q_cd_f.push(CSRT_frame);
                bool tracking_update_success = tracker->update(CSRT_frame, CSRT_bbox);

                if(end_program){
                    DEBUG("CSRT: End program \n")
                    return -1;
                } else {}//do nothing

                if(0){
                    //Check IR here
                    goto InitCSRT;
                } else {}//do nothing

                if( ! tracking_update_success){
                    DEBUG("CSRT: tracking failed, CSRT failure\n");
                    goto InitCSRT;
                } else {
                    cv::rectangle(CSRT_frame, CSRT_bbox, Scalar( 255, 0, 0 ), 2, 1 );
                    cv::imshow("CSRT",CSRT_frame);
                }

                if( ! que_human_not_present.empty()){
                    que_human_not_present.pop();
                    DEBUG("CSRT: tracking failed, CSRT failure\n");
                    goto InitCSRT;
                } else {}//do nothing

                if(q_cd_f.empty()){
                    q_cd_b.push(CSRT_bbox);
                } else {}//do nothing

                q_ck_f.push(CSRT_frame);
                q_ck_b.push(CSRT_bbox);

                if( ! KCF_run){
                    if(que_bbox_not_according.empty()){
                        que_bbox_not_according.pop();
                        DEBUG("CSRT: KCL failed, CSRT get frame \n");
                        control_motor(CSRT_bbox.x + CSRT_bbox.width/2, 0.7);
                        get_frame(CSRT_frame);
                        goto push_q_cd_f_PLACE;
                    }
                }

                if( ! q_kc_b.empty()){
                    cv::Rect tmp_bbox = q_kc_b.front();  q_kc_b.pop();
                    if( ! Bboxes_according(CSRT_bbox,tmp_bbox)){  //
                        DEBUG("CSRT: pop kc; bboxes are not accorrding, CSRT control \n");
                        que_bbox_not_according.push(true);

                        control_motor(CSRT_bbox.x + CSRT_bbox.width/2, 0.7);
                        get_frame(CSRT_frame);
                        goto push_q_cd_f_PLACE;
                    } else {
                        DEBUG("CSRT: bbox according\n")
                    }
                } else {
                    DEBUG("CSRT: q_kc_b empty\n");
                }

            }//this scope for wait kc_f
        }//this scope for retracking perpose, re init tracking object
    }
}
