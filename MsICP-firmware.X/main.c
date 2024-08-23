/*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.2
 *
 * @version Package Version: 3.1.2
 */

#include "main.h"
#include "anim.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/reset/rstctrl.h"

uint8_t buffer[BUFFER_LEN];

uint16_t frame_hues[LED_COUNT];
bool frame_hues_directions[LED_COUNT];
uint8_t frame_intensities[LED_COUNT];

uint16_t frame_next_hues[LED_COUNT];
uint8_t frame_next_intensities[LED_COUNT];
bool frame_next_touched[LED_COUNT];

char rx_buf[32];
uint8_t rx_idx = 0;

uint16_t pause_setting = 0;
uint8_t saturation_setting = 0;
uint8_t intensity_setting = 0;

volatile bool interacted = false;

// -----------------------------------------------------------------------------

bool wait(uint16_t seconds) {
    uint16_t wait = (seconds * 10) + 1;
    while (wait > 0) {
        DELAY_milliseconds(100);
        wait--;
        if (interacted) {
            return true;
        }
    }
    return false;
}

void reseed() {
    uint16_t seed = 0;

    ADC0_Enable();
    DELAY_milliseconds(10);

    for (uint8_t i = 0; i < 8; i++) {
        seed = seed << 2;
        seed = seed | (ADC0_GetConversion(ADC0_channel_DACREF0) & 0x03);
        DELAY_milliseconds(10);
    }

    printf("Info: New seed %u\n", seed);
    srand(seed);

    ADC0_Disable();
}

uint8_t intensity_max() {
    return intensity_setting;
}

uint8_t load_value(uint8_t pos) {
    return EEPROM_Read(EEPROM_START + pos);
}

void save_value(uint8_t pos, uint8_t value) {
    if (load_value(pos) == value) {
        return;
    }
    EEPROM_Write(EEPROM_START + pos, value);
}

void store() {
    printf("Info: Saving configuration\n");
    save_value(0, 0x23);
    save_value(1, pause_setting >> 8);
    save_value(2, pause_setting);
    save_value(3, saturation_setting);
    save_value(4, intensity_setting);

    for (uint8_t led = 0; led < LED_COUNT; led++) {
        frame_next_touched[led] = true;
    }
}

void load(bool force_default) {
    if (force_default || load_value(0) != 0x23) {
        printf("Info: Setting configuration to default\n");
        pause_setting = 3600;
        saturation_setting = 255;
        intensity_setting = 40;
        store();
    } else {
        printf("Info: Loading configuration\n");
        pause_setting = (((uint16_t) load_value(1)) << 8) | load_value(2);
        saturation_setting = load_value(3);
        intensity_setting = load_value(4);
    }

    if (pause_setting < SETTING_PAUSE_MIN) {
        pause_setting = SETTING_PAUSE_MIN;
    }
    if (pause_setting > SETTING_PAUSE_MAX) {
        pause_setting = SETTING_PAUSE_MAX;
    }

    if (saturation_setting < SETTING_SATURATION_MIN) {
        saturation_setting = SETTING_SATURATION_MIN;
    }
    if (saturation_setting > SETTING_SATURATION_MAX) {
        saturation_setting = SETTING_SATURATION_MAX;
    }

    if (intensity_setting < SETTING_INTENSITY_MIN) {
        intensity_setting = SETTING_INTENSITY_MIN;
    }
    if (intensity_setting > SETTING_INTENSITY_MAX) {
        intensity_setting = SETTING_INTENSITY_MAX;
    }

    for (uint8_t led = 0; led < LED_COUNT; led++) {
        frame_next_touched[led] = true;
    }
}

// -----------------------------------------------------------------------------

void menu_show() {
    printf("-- Actions ---------------------------------------------------------\n");
    printf(" '*' = Reset to default configuration\n");
    printf(" '-' = Reboot\n");
    printf(" '?' = Show this help\n");
    printf("\n");

    printf("-- Configuration ---------------------------------------------------\n");
    printf("P <0-3600> = Pause between animations\n");
    printf("S <1-255>  = LED saturation\n");
    printf("I <1-255>  = LED intensity\n");
    printf("\n");
}

void settings_show() {
    printf("-- Current configuration -- '?' for help ---------------------------\n");
    printf("Pause     : %d sec\n", pause_setting);
    printf("Saturation: %d\n", saturation_setting);
    printf("Intensity : %d\n", intensity_setting);
    printf("\n");
}

bool interpret_line() {
    bool err = false;
    bool show = false;
    bool setting_modified = false;

    if (rx_idx < 2) {
        return false;
    }

    if (rx_idx == 2) {
        if (rx_buf[0] == '?') {
            menu_show();
            settings_show();
        } else if (rx_buf[0] == '*') {
            load(true);
            show = true;
        } else if (rx_buf[0] == '-') {
            printf("Resetting in 2 seconds...\n");
            DELAY_milliseconds(2000);
            RSTCTRL_reset();
        } else {
            err = true;
        }
    } else {
        char c = 0;
        uint16_t d = 0;
        sscanf(rx_buf, "%c%u", &c, &d);

        if (c == 'P') { //P 0-3600
            if (d < SETTING_PAUSE_MIN) {
                err = true;
            } else if (d > SETTING_PAUSE_MAX) {
                err = true;
            } else {
                pause_setting = d;
                setting_modified = true;
            }
        } else if (c == 'S') { //S 1-255
            if (d < SETTING_SATURATION_MIN) {
                err = true;
            } else if (d > SETTING_SATURATION_MAX) {
                err = true;
            } else {
                saturation_setting = d;
                show = true;
                setting_modified = true;
            }
        } else if (c == 'I') { //I 1-255
            if (d < SETTING_INTENSITY_MIN) {
                err = true;
            } else if (d > SETTING_INTENSITY_MAX) {
                err = true;
            } else {
                intensity_setting = d;
                show = true;
                setting_modified = true;
            }
        } else {
            err = true;
        }
    }

    rx_idx = 0;

    if (err) {
        printf("Unknown command or parameter ('?' for help): %s", rx_buf);
        return false;
    }

    if (setting_modified) {
        store();
        settings_show();
    }

    return show;
}

bool interact_key() {
    bool show = false;

    while (USART0_IsRxReady()) {
        while (USART0_IsRxReady()) {
            char c = USART0_Read();
            interacted = false;

            if (c == '\r' || c == ' ') {
                continue;
            }
            if (c >= 97 && c <= 122) {
                c = c - 32;
            }

            rx_buf[rx_idx] = c;
            rx_idx++;

            if (c == '\n') {
                show |= interpret_line();
                continue;
            }

            if (rx_idx > 32) {
                rx_idx = 0;
            }
        }
        DELAY_milliseconds(100);
    }

    return show;
}

// -----------------------------------------------------------------------------

void frame_send(uint8_t force) {
    uint8_t r, g, b;
    uint16_t addr = 4;

    for (uint8_t led = 0; led < LED_COUNT; led++) {
        // https://www.vagrearg.org/content/hsvrgb

        if (!force && !frame_next_touched[led]) {
            addr += 4;
            continue;
        }

        fast_hsv2rgb_8bit(frame_hues[led], saturation_setting, frame_intensities[led], &r, &g, &b);
        //fast_hsv2rgb_32bit(frame_hues[i], saturation_setting, frame_intensities[i], &r, &g, &b);

        addr++; // Intensity being set globally
        buffer[addr++] = b;
        buffer[addr++] = g;
        buffer[addr++] = r;

        frame_next_touched[led] = false;
    }

    SPI0_BufferWrite(buffer, BUFFER_LEN);
}

void frame_switch(uint8_t force) {
    for (uint8_t led = 0; led < LED_COUNT; led++) {
        frame_hues[led] = frame_next_hues[led];
        frame_intensities[led] = frame_next_intensities[led];
    }
    frame_send(force);
}

void frame_fade(uint8_t speed_ms) {
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        uint16_t a = frame_hues[i];
        uint16_t b = frame_next_hues[i];
        if (a > b) {
            if (HSV_HUE_MAX - a + b > a - b) {
                frame_hues_directions[i] = false;
            } else {
                frame_hues_directions[i] = true;
            }
        } else {
            if (HSV_HUE_MAX - b + a > b - a) {
                frame_hues_directions[i] = true;
            } else {
                frame_hues_directions[i] = false;
            }
        }
    }

    while (1) {
        bool same = true;
        for (uint8_t led = 0; led < LED_COUNT; led++) {
            if (frame_hues[led] != frame_next_hues[led]) {
                same = false;
                frame_next_touched[led] = true;
                if (frame_hues_directions[led]) {
                    if (frame_hues[led] < HSV_HUE_MAX) {
                        frame_hues[led] = frame_hues[led] + 1;
                    } else {
                        frame_hues[led] = 0;
                    }
                } else {
                    if (frame_hues[led] > 0) {
                        frame_hues[led] = frame_hues[led] - 1;
                    } else {
                        frame_hues[led] = HSV_HUE_MAX;
                    }
                }
            }
            if (frame_intensities[led] != frame_next_intensities[led]) {
                same = false;
                frame_next_touched[led] = true;
                if (frame_next_intensities[led] > frame_intensities[led]) {
                    frame_intensities[led] = frame_intensities[led] + 1;
                } else {
                    frame_intensities[led] = frame_intensities[led] - 1;
                }
            }
        }

        frame_send(false);

        if (same) {
            return;
        }

        DELAY_milliseconds(speed_ms);
    }
}

void frame_fade_intensity(uint8_t speed_ms) {
    for (uint8_t led = 0; led < LED_COUNT; led++) {
        frame_hues[led] = frame_next_hues[led];
        frame_next_touched[led] = true;
    }
    frame_fade(speed_ms);
}

void frame_next_set_h(uint8_t led, uint16_t hue) {
    if (frame_hues[led] != hue) {
        frame_next_hues[led] = hue;
        frame_next_touched[led] = true;
    }
}

void frame_next_set_i(uint8_t led, uint8_t intensity) {
    if (frame_intensities[led] != intensity) {
        frame_next_intensities[led] = intensity;
        frame_next_touched[led] = true;
    }
}

void frame_next_set_hi(uint8_t led, uint16_t hue, uint8_t intensity) {
    frame_next_set_h(led, hue);
    frame_next_set_i(led, intensity);
}

void frame_next_clear_i() {
    for (uint8_t led = 0; led < LED_COUNT; led++) {
        frame_next_set_i(led, 0x00);
    }
}

// -----------------------------------------------------------------------------

void anim() {
    uint8_t max = pause_setting > 0 ? 4 : 3;
    uint8_t rnd = rand() % max;
    printf("Info: Starting animation #%d\n", rnd);
    switch (rnd) {
        case 0:
            anim_rainbow_scroll();
            break;
        case 1:
            anim_full_color_fade_rnd();
            break;
        case 2:
            anim_sparkle();
            break;
        case 3:
            anim_rainbow_script();
            break;
        default:
            printf("Info: Unknown animation, skipping!\n");
    }
}

void rx() {
    interacted = true;
}

bool hasinteracted() {
    return interacted;
}

// -----------------------------------------------------------------------------

int main(void) {
    SYSTEM_Initialize();

    sei();

    SPI0_Open(0);

    for (uint8_t i = 0; i < 4; i++) {
        buffer[i] = 0x00;
    }
    for (uint8_t led = 0; led < LED_COUNT; led++) {
        buffer[4 + (led * 4) + 0] = 0b11111111;
    }
    for (uint8_t i = 0; i < 18; i++) {
        buffer[BUFFER_LEN - 1 - i] = 0xFF;
    }

    frame_next_clear_i();
    frame_switch(true);

    DELAY_milliseconds(1000);

    printf("\n");
    printf("Welcome Ms ICP!\n");
    printf("https://github.com/kiu/MsICP ");
    printf("REV %c ", REVISION);
    printf("VER %02x ", VERSION);
    printf("\n");
    printf("\n");
    reseed();

    load(false);

    menu_show();

    settings_show();

    USART0_RxCompleteCallbackRegister(rx);

    wait(2);

    while (1) {
        anim();
        if (pause_setting > 0 && !interacted) {
            frame_next_clear_i();
            frame_fade_intensity(20);

            uint16_t seconds = pause_setting;
            printf("Info: Pausing for %d seconds...\n", seconds);
            wait(seconds);
        }

        if (interacted) {
            if (interact_key()) {
                frame_next_clear_i();
                frame_switch(false);

                uint16_t hue = 0;
                for (uint8_t led = 0; led < LED_COUNT; led++) {
                    frame_next_set_hi(led, hue, intensity_max());
                    hue += HSV_HUE_MAX / LED_COUNT;
                }
                frame_switch(false);

                wait(6);

                frame_next_clear_i();
                frame_switch(false);
            }
        }
    }
}