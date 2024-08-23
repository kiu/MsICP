/* 
 * File:   main.h
 * Author: kiu
 *
 * Created on 4. August 2024, 01:10
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#define REVISION 'A'
#define VERSION 0x02

#define LED_COUNT 36
#define BUFFER_LEN 4 + (4 * LED_COUNT) + 18

#include "mcc_generated_files/timer/delay.h"
#include "mcc_generated_files/system/system.h"
#include "fast_hsv2rgb.h"
#include <stdlib.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define SETTING_PAUSE_MIN 0
#define SETTING_PAUSE_MAX 3600
#define SETTING_SATURATION_MIN 1
#define SETTING_SATURATION_MAX 255
#define SETTING_INTENSITY_MIN 1
#define SETTING_INTENSITY_MAX 255

    uint8_t intensity_max();
    bool hasinteracted();

    void frame_switch(uint8_t force);
    void frame_fade(uint8_t speed_ms);
    void frame_fade_intensity(uint8_t speed_ms);

    void frame_next_set_hi(uint8_t led, uint16_t hue, uint8_t intensity);
    void frame_next_set_h(uint8_t led, uint16_t hue);
    void frame_next_set_i(uint8_t led, uint8_t intensity);
    void frame_next_clear_i();

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

