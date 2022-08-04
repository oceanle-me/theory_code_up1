#ifndef MY_GPIO_HPP_INCLUDED
#define MY_GPIO_HPP_INCLUDED

#include <pigpio.h>

#define WIDTH 320
#define HEIGHT 320


void InitPIN(void);
void control_motor(int x,float index_speed);

void Stop_PIN(void);



#endif // MY_GPIO_HPP_INCLUDED
