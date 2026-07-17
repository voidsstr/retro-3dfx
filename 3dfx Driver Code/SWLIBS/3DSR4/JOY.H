#ifndef __JOY_H__
#define __JOY_H__

#ifdef __cplusplus
extern "C" {
#endif

int joystick_detect( void );   /* 0 = No joystick, 1 = joystick present */  
void joystick_read_raw( int *x, int *y, unsigned char *buttons );
void joystick_read( float *ret_x, float *ret_y, int *button1, int *buttons2, 
                    int *button3, int *button4 );
void joystick_calibrate( void );
void joystick_set_deadspot( float amount );

#ifdef __WATCOMC__
void clearInterrupts(void);
void restoreInterrupts(void);

#   pragma aux clearInterrupts =\
            ".586" \
            "cli";

#   pragma aux restoreInterrupts =\
            ".586" \
            "sti";

            
#else  /* Assume MSVC */
#   define clearInterrupts() _asm { cli }
#   define restoreInterrupts() _asm { sti }
#endif 

#ifdef __cplusplus
}
#endif

#endif /* __JOY_H__ */
