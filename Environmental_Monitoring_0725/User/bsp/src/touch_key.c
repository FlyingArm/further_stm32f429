#include "stm32f429i_discovery_lcd.h"

#define KEY_STATE_0         0       // 按键状态
#define KEY_STATE_1         1
#define KEY_STATE_2         2
#define KEY_STATE_3         3

#define LONG_KEY_TIME       300     // LONG_KEY_TIME*10MS = 3S
#define SINGLE_KEY_TIME     3       // SINGLE_KEY_TIME*10MS = 30MS

#define N_KEY    0                  // no click
#define S_KEY    1                  // single click
#define L_KEY    10                 // long press

unsigned char key_driver(void) 
{     
    static unsigned char key_state = 0;         // 按键状态变量
    static unsigned int key_time = 0;           // 按键计时变量
    unsigned char key_press, key_return; 

    key_return = N_KEY;                         // 清除 返回按键值

    key_press = GetTouchKeyState();             // 读取当前键值

    switch (key_state)     
    {       
        case KEY_STATE_0:                       // 按键状态0：判断有无按键按下
            if (!key_press)                     // 有按键按下
            {
                key_time = 0;                   // 清零时间间隔计数
                key_state = KEY_STATE_1;        // 然后进入 按键状态1
            }        
            break;

        case KEY_STATE_1:                       // 按键状态1：软件消抖（确定按键是否有效，而不是误触）。按键有效的定义：按键持续按下超过设定的消抖时间。
            if (!key_press)                     
            {
                key_time++;                     // 一次10ms
                if(key_time>=SINGLE_KEY_TIME)   // 消抖时间为：SINGLE_KEY_TIME*10ms = 30ms;
                {
                    key_state = KEY_STATE_2;    // 如果按键时间超过 消抖时间，即判定为按下的按键有效。按键有效包括两种：单击或者长按，进入 按键状态2， 继续判定到底是那种有效按键
                }
            }         
            else key_state = KEY_STATE_0;       // 如果按键时间没有超过，判定为误触，按键无效，返回 按键状态0，继续等待按键
            break; 

        case KEY_STATE_2:                       // 按键状态2：判定按键有效的种类：是单击，还是长按
            if(key_press)                       // 如果按键在 设定的长按时间 内释放，则判定为单击
            { 
                 key_return = S_KEY;            // 返回 有效按键值：单击
                 key_state = KEY_STATE_0;       // 返回 按键状态0，继续等待按键
            } 
            else
            {
                key_time++;                     

                if(key_time >= LONG_KEY_TIME)   // 如果按键时间超过 设定的长按时间（LONG_KEY_TIME*10ms=200*10ms=2000ms）, 则判定为 长按
                {
                    key_return = L_KEY;         // 返回 有效键值值：长按
                    key_state = KEY_STATE_3;    // 去状态3，等待按键释放
                }
            }
            break;

      case KEY_STATE_3:                         // 等待按键释放
          if (key_press) 
          {
              key_state = KEY_STATE_0;          // 按键释放后，进入 按键状态0 ，进行下一次按键的判定
          }         
          break; 

        default:                                // 特殊情况：key_state是其他值得情况，清零key_state。这种情况一般出现在 没有初始化key_state，第一次执行这个函数的时候
            key_state = KEY_STATE_0;
            break;
    }

    return key_return;                          // 返回 按键值
} 