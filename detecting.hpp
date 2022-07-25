#ifndef DETECTING_HPP_INCLUDED
#define DETECTING_HPP_INCLUDED

#include "my_global.hpp"



typedef enum {LEFT, MID, RIGHT} e_human_position;

cv::Rect Detect_3_persons(cv::Mat &src,e_human_position _position);



#endif // DETECTING_HPP_INCLUDED
