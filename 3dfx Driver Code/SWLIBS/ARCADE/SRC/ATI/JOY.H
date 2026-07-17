#ifndef __JOY_H__
#define __JOY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* internal joystick functions, not part of ATB api */

int joystick_detect( void );   /* 0 = No joystick, 1 = joystick present */  
void joystick_read_raw( int *x, int *y, unsigned char *buttons );
void joystick_read( float *ret_x, float *ret_y, int *button1, int *buttons2, 
                    int *button3, int *button4 );
void joystick_calibrate( void );
void joystick_set_deadspot( float amount );

#ifdef __WIN32__
void joystick_calibrate_win( FxBool force );
#endif
/* end of joy.lib functions */

#ifdef __cplusplus
}
#endif

#endif /* __JOY_H__ */
