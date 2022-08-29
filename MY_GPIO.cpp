#include "MY_GPIO.hpp"


void InitPIN(void){
    gpioInitialise();

    //Init PWM
    gpioSetMode(12, PI_ALT0);
    gpioSetPullUpDown(12, PI_PUD_DOWN);
    gpioSetPWMfrequency(12,10000);
    gpioPWM(12,0);

    gpioSetMode(13, PI_ALT0);
    gpioSetPullUpDown(13, PI_PUD_DOWN);
    gpioSetPWMfrequency(13,10000);
    gpioPWM(13,0);


    //Init LED
    gpioSetMode(17,PI_OUTPUT);
    gpioSetMode(27,PI_OUTPUT);
    gpioWrite(17,0);
    gpioWrite(27,0);

    gpioSetMode(22,PI_INPUT);
    gpioSetPullUpDown(22, PI_PUD_UP);

}

void Stop_PIN(void){
    gpioPWM(12,0);
    gpioPWM(13,0);
    gpioWrite(17,0);
    gpioWrite(27,0);



    gpioTerminate();
}

void control_motor(int x,float index_speed){ //input: x= (x1+x2)/2; y = (y1+y2)/2 -------320xx320
    //signal scaler in -+ 200:1000
    //cout <<x << "  ";
    if( (x>150) && (x<170) )
    {
        gpioPWM(12,0);
        gpioPWM(13,0);

    } else {

        if (x<(WIDTH/2)){
            unsigned int scaler = (((WIDTH/2)-(float)x)/(WIDTH/2))*185 + 70 ;
            scaler = scaler*index_speed;
            //cout <<scaler << " \n ";
            gpioPWM(12,0);
            gpioPWM(13,scaler);
        } else {
            unsigned int scaler = (((float)x-(WIDTH/2))/(WIDTH/2))*185 + 70 ;
            scaler = scaler*index_speed;
            //cout <<scaler << " \n ";
            gpioPWM(12,scaler);
            gpioPWM(13,0);
        }

    }

}



int LED_control(void){

    DEBUG("LED control thread created\n")
    e_LEDstate new_LEDstate;
    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    static e_LEDstate curr_LEDstate = LEDOFF;

    while(1){
    if(end_program){
        DEBUG("End LEd thread\n")
        return -1;
    }

    if (curr_LEDstate==MANUAL_MOVE || curr_LEDstate==AUTO_DETECT ){
        cv_LED.wait_for(lck, std::chrono::milliseconds(250));
    } else{
        cv_LED.wait(lck);
    }
    if( ! q_LEDstate.empty()){
        new_LEDstate = q_LEDstate.Front_pop();
    }

    switch(new_LEDstate){
    case LEDOFF:
        DEBUG("LED off \n")
        gpioWrite(17,0);
        gpioWrite(27,0);
        curr_LEDstate = LEDOFF;
        break;
    case MANUAL_STAND:
        DEBUG("MANUAL_STAND, on RED, off Green\n")
        gpioWrite(27,1);
        gpioWrite(17,0);
        curr_LEDstate = MANUAL_STAND;
        break;
    case MANUAL_MOVE:
        DEBUG("MANUAL_MOVE, blink red, off green\n")
        gpioWrite(27,gpioRead(27)^1);
        gpioWrite(17,0);
        curr_LEDstate = MANUAL_MOVE;
        break;
    case AUTO_DETECT:
        DEBUG("AUTO_DETECT, blink green, off red\n")
        gpioWrite(17,gpioRead(17)^1);
        gpioWrite(27,0);
        curr_LEDstate = AUTO_DETECT;
        break;
    case AUTO_MOVE:
        DEBUG("AUTO_MOVE, on green, off red\n")
        gpioWrite(17,1);
        gpioWrite(27,0);
        curr_LEDstate = AUTO_MOVE;
        break;
    default:
        std::cout << "????? Wrong  \n" << __LINE__;
    }

    }
}
