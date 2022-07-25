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
