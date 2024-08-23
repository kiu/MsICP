#include "anim.h"
#include "main.h"

const uint8_t anim_rows[4][11] = {
    {9, 0x00, 0x06, 0x0C, 0x0D, 0x0E, 0x12, 0x1A, 0x1B, 0x1C},
    {9, 0x01, 0x05, 0x07, 0x0B, 0x0F, 0x13, 0x19, 0x1D, 0x23},
    {10, 0x02, 0x04, 0x08, 0x0A, 0x10, 0x11, 0x14, 0x18, 0x1E, 0x22},
    {8, 0x03, 0x09, 0x15, 0x17, 0x16, 0x1F, 0x20, 0x21}
};

const uint8_t anim_cols[15][5] = {
    {4, 0x00, 0x01, 0x02, 0x03},
    {2, 0x04, 0x05},
    {1, 0x06},
    {2, 0x07, 0x08},
    {4, 0x09, 0x0A, 0x0B, 0x0C},
    {2, 0x0D, 0x10},
    {1, 0x0F},
    {2, 0x11, 0x0E},
    {4, 0x12, 0x13, 0x14, 0x15},
    {2, 0x18, 0x19},
    {2, 0x17, 0x1A},
    {2, 0x16, 0x1B},
    {4, 0x1C, 0x1D, 0x1E, 0x1F},
    {2, 0x20, 0x23},
    {2, 0x21, 0x22}
};

const uint8_t anim_letters[5][14] = {
    {13, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C},
    {5, 0x0D, 0x0E, 0x0F, 0x10, 0x11},
    {4, 0x12, 0x13, 0x14, 0x15},
    {6, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B},
    {8, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23}
};

// -----------------------------------------------------------------------------

uint16_t hue_add(uint16_t hue, uint16_t val) {
    if (hue + val > HSV_HUE_MAX) {
        return val - (HSV_HUE_MAX - hue);
    }
    return hue + val;
}

uint16_t hue_rem(uint16_t hue, uint16_t val) {
    if (hue < val) {
        return HSV_HUE_MAX - (val - hue);
    }
    return hue - val;
}

// -----------------------------------------------------------------------------

void anim_rainbow_script() {
    const uint16_t loops = 3;

    uint16_t rnd_hue = rand() % HSV_HUE_MAX;
    uint8_t rnd_steps = rand() % (HSV_HUE_MAX / 36) + 1;
    uint8_t rnd_len = (rand() % 20) + 6;
    uint8_t rnd_speed = (rand() % 30) + 20;

    for (uint16_t loop = 0; loop < loops; loop++) {
        for (uint8_t d = 0; d < 72; d++) {
            if (d < 36) {
                frame_next_set_hi(d, rnd_hue, intensity_max());
            }
            if (d >= rnd_len) {
                frame_next_set_hi(d - rnd_len, rnd_hue, 0);
            }
            if (hasinteracted()) return;
            frame_switch(false);
            if (hasinteracted()) return;
            DELAY_milliseconds(rnd_speed);
            if (hasinteracted()) return;
            if (d - rnd_len > 36) {
                break;
            }
            rnd_hue = hue_add(rnd_hue, rnd_steps);
        }
    }
}

void anim_rainbow_scroll() {
    const uint16_t loops = 300;

    uint16_t rnd_hue = rand() % HSV_HUE_MAX;
    uint8_t rnd_distance = rand() % 60 + 20;
    uint8_t rnd_steps = rand() % 20 + 10;
    bool rnd_direction_lr = rand() % 2;
    bool rnd_direction_hv = rand() % 2;

    uint8_t array_max = 0;
    uint8_t array_size = rnd_direction_hv ? 15 : 5;

    for (uint16_t loop = 0; loop < loops; loop++) {
        if (rnd_direction_lr) {
            rnd_hue = hue_rem(rnd_hue, rnd_steps);
        } else {
            rnd_hue = hue_add(rnd_hue, rnd_steps);
        }

        uint16_t tmp_hue = rnd_hue;
        for (uint8_t iarr = 0; iarr < array_size; iarr++) {
            if (rnd_direction_hv) {
                array_max = anim_cols[iarr][0];
            } else {
                array_max = anim_rows[iarr][0];
            }

            bool fade_in_out;
            if (rnd_direction_lr) {
                fade_in_out = (iarr < loop) && (array_size - iarr < loops - loop - 1);
            } else {
                fade_in_out = (array_size - iarr < loop) && (iarr < loops - loop - 1);
            }

            //uint8_t fade_intensity = fade_in_out ? intensity_max() : 0;

            for (uint8_t iled = 0; iled < array_max; iled++) {
                if (fade_in_out) {
                    if (rnd_direction_hv) {
                        frame_next_set_hi(anim_cols[iarr][iled + 1], tmp_hue, intensity_max());
                    } else {
                        frame_next_set_hi(anim_rows[iarr][iled + 1], tmp_hue, intensity_max());
                    }
                }
            }

            tmp_hue = hue_add(tmp_hue, rnd_distance);
        }
        if (hasinteracted()) return;
        frame_switch(false);
        DELAY_milliseconds(40);
        if (hasinteracted()) return;
    }
}

void anim_full_color_fade_rnd() {
    const uint16_t loops = 8;
    uint16_t hue_rnd = rand() % HSV_HUE_MAX;
    uint16_t distance_rnd = (rand() % 400) + 200;

    for (uint8_t loop = 0; loop < loops; loop++) {
        for (uint8_t led = 0; led < LED_COUNT; led++) {
            frame_next_set_hi(led, hue_rnd, intensity_max());
        }
        if (hasinteracted()) return;
        frame_fade(2);
        if (hasinteracted()) return;
        DELAY_milliseconds(2000);
        hue_rnd = hue_add(hue_rnd, distance_rnd);
    }
}

void anim_sparkle() {
    const uint16_t loops = 4;
    uint16_t rnd_hue = rand() % HSV_HUE_MAX;

    uint8_t pixels[LED_COUNT];
    for (uint8_t led = 0; led < LED_COUNT; led++) {
        pixels[led] = led;
    }

    for (uint16_t loop = 0; loop < loops; loop++) {

        for (uint8_t i = 0; i < LED_COUNT; i++) {
            uint8_t from = i;
            uint8_t to = i + (rand() % (LED_COUNT - i));
            uint8_t tmp = pixels[to];

            pixels[to] = pixels[from];
            pixels[from] = tmp;
        }


        rnd_hue = hue_add(rnd_hue, (rand() % 200) + 150);

        for (uint8_t led = 0; led < LED_COUNT; led++) {
            frame_next_set_hi(pixels[led], rnd_hue, intensity_max());
            if (hasinteracted()) return;
            frame_switch(false);
            DELAY_milliseconds(30);
            if (hasinteracted()) return;
        }

        DELAY_milliseconds(600);
        if (hasinteracted()) return;
    }
}
