#ifndef MY_DEBUG_HPP_INCLUDED
#define MY_DEBUG_HPP_INCLUDED


#define Debuging

#ifdef Debuging
#define DEBUG(x) { std::cerr << x; }

#else
#define DEBUG(x)
#endif


#endif // MY_DEBUG_HPP_INCLUDED
