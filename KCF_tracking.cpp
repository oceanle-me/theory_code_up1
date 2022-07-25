#include "KCF_tracking.hpp"

#if 0

int TrackingObj(void){
    while(1){//always running until this function returning
        wait_start:
        if(start_stop_sign){
            start_stop_sign=false;
            cout <<"Tracking_start";
retracking:{
            cout <<"Init tracking";
            track_bbox = detect_bbox;
            bbox_according=true;
            tracking_run=true;
            //Resizing box if it is too big

            if(track_bbox.width>70){
                track_bbox.x=track_bbox.x + track_bbox.width/2 - 35;
                track_bbox.width=70;
            }
            if(track_bbox.height>150){
                track_bbox.y=track_bbox.y+10;
                track_bbox.height=150;
            }

            Ptr<Tracker> tracker;
            tracker = TrackerKCF::create();
            //tracker = TrackerCSRT::create();
            //tracker = TrackerGOTURN::create();
            tracker->init(frame, track_bbox);

            do
            {
                uint64_t timer = cv::getTickCount();
                // Update the tracking result
                bool ok = tracker->update(frame, track_bbox);

                if (ok){
                    // Tracking success : Draw the tracked object
                    rectangle(frame, track_bbox, Scalar( 255, 0, 0 ), 2, 1 );

                    if(start_stop_sign==true){
                        start_stop_sign = false;
                        tracking_run=false;
                        goto wait_start;
                    } else { //comparing bboxes
                        //I have to compare multi boxes
                        if (!bbox_according){
                            goto retracking;
                        }
                    }


                } else {
                    tracking_run=false;
                    cout << "Tracking failure detected \n";
                    gpioPWM(12,0);
                    gpioPWM(13,0);
                    while(!human_detected){ //waiting until can be retrack
                        if(end_program) return -1;
                    }
                    goto retracking;
                }

                float fps = cv::getTickFrequency() / (float)(cv::getTickCount() - timer);
                putText(frame, format("FPS %0.2f",fps),Point(10,20),FONT_HERSHEY_SIMPLEX,0.6, Scalar(0, 0, 255));

                // Display frame.
                imshow("Detector and Tracking", frame);
                control_motor(track_bbox.x+track_bbox.width/2,0);

                // Exit if ESC pressed.
                int k = waitKey(5);
                if(k == 27)
                {
                    cout <<"ESC - out from tracking\n";
                    tracking_run=false;
                    end_program=true;
                    gpioPWM(12,0);
                    gpioPWM(13,0);
                    gpioWrite(17,0);
                    gpioWrite(27,0);
                    return -1;
                }

            } while(get_frame(frame));
    }
            tracking_run=false;
            end_program=true;
            gpioPWM(12,0);
            gpioPWM(13,0);
            gpioWrite(17,0);
            gpioWrite(27,0);
            cout <<"Can't not grab the next frame, out from tracking\n";
            return 1;
        }
    }

}


#endif


int KCF_tracking(void){
    while(1){//always running until this function returning
            DEBUG("KCF thread created\n");

Init_KCF:

        KCF_run=false;
        if(!q_kc_f.empty()) { q_kc_f.pop();}
        if(!q_kc_b.empty()) { q_kc_b.pop();}

        while(q_ck_f.empty()||q_ck_b.empty()){
            if(end_program){
                DEBUG("End program from KCF\n")
                return -1;
            }
        }

        { //this scope for retracking perpose

            KCF_frame = q_ck_f.front(); q_ck_f.pop();
            KCF_bbox = q_ck_b.front();  q_ck_b.pop();
            DEBUG("KCF: get q_ck_b, q_ck_f \n");

            Ptr<Tracker> tracker;
            tracker = TrackerKCF::create();
            //tracker = TrackerCSRT::create();
            //tracker = TrackerGOTURN::create();
            tracker->init(KCF_frame, KCF_bbox);
            DEBUG("KCF: Init KCF tracking succeeded\n");

            while(1){
                if(get_frame(KCF_frame)){}
                else {
                    DEBUG("KCF: Cant get a frame from camera, end program\n");
                    end_program = true;
                    return -1;
                }

                KCF_run = true;
                q_kc_f.push(KCF_frame);


                bool tracking_update_success = tracker->update(KCF_frame, KCF_bbox);



                if(end_program){
                    DEBUG("KCF: End program \n");
                    return -1;
                } else {}//do nothing

                if(0){
                    //Check IR here
                    goto Init_KCF;
                } else {}//do nothing

                if( ! CSRT_run){
                    DEBUG("KCF: CSRT failed\n");
                    goto Init_KCF;
                } else {}//do nothing

                if(    ! tracking_update_success){
                    DEBUG("KCF: tracking failed\n");
                    goto Init_KCF;
                } else {
                cv::rectangle(KCF_frame, KCF_bbox, Scalar( 255, 0, 0 ), 2, 1 );
                cv::imshow("KCF",KCF_frame);
                }

                if(   ! que_bbox_not_according.empty()){
                    que_bbox_not_according.pop();
                    DEBUG("KCF: bboxed are not according\n");
                    goto Init_KCF;
                } else {}//do nothing

                control_motor(KCF_bbox.x + KCF_bbox.width/2,1);

                if(q_kc_f.empty()){
                    q_kc_b.push(KCF_bbox);
                } else {}//do nothing

            }

        }
    }

}


