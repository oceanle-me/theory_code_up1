#ifndef MY_GPIO_HPP_INCLUDED
#define MY_GPIO_HPP_INCLUDED

#include "my_global.hpp"
#include <pigpio.h>

#define WIDTH 320
#define HEIGHT 320


void InitPIN(void);
void control_motor(int x,float index_speed);

void Stop_PIN(void);



int LED_control(void);

#endif // MY_GPIO_HPP_INCLUDED
