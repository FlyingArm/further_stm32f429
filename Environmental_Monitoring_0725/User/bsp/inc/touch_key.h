#ifndef _TOUCH_KEY_H
#define _TOUCH_KEY_H


#define KEY_STATE_0         0       // 按键状态
#define KEY_STATE_1         1
#define KEY_STATE_2         2
#define KEY_STATE_3         3

#define LONG_KEY_TIME       300     // LONG_KEY_TIME*10MS = 3S
#define SINGLE_KEY_TIME     60       // SINGLE_KEY_TIME*10MS = 30MS

#define N_KEY    0                  // no click
#define S_KEY    1                  // single click
#define L_KEY    10                 // long press

unsigned char key_driver(void);

#endif