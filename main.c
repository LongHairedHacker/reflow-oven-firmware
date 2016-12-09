#include<avr/io.h>
#include<util/delay.h>

#include "spi.h"
#include "st7735.h"
#include "st7735_gfx.h"
#include "st7735_font.h"

//#include "logo_bw.h"

#include "free_sans.h"

uint8_t profile_point_count = 5;
uint16_t profile[5][2] = {
    {0, 22},
    {90, 150},
    {180, 180},
    {210, 210},
    {270, 180},
};

uint16_t interpolate_profile(uint16_t t) {
    if(t > profile[profile_point_count -1][0]) {
        return profile[profile_point_count -1][1] * 100;
    }

    uint8_t pos;
    for(pos = 0; pos < profile_point_count; pos++) {
        if(profile[pos][0] > t) {
            break;
        }
    }

    int16_t time_diff = profile[pos][0] - profile[pos - 1][0];
    int16_t tmp_diff = (profile[pos][1] - profile[pos - 1][1]) * 100;
    int16_t gradient = tmp_diff / time_diff;

    return gradient * (t - profile[pos - 1][0]) + profile[pos - 1][1] * 100;
}


void splash_screen(void) {
    st7735_fill_rect(0, 0, 160, 128, ST7735_COLOR_BLACK);
    //st7735_draw_mono_bitmap(16, 4, &logo_bw, ST7735_COLOR_WHITE, ST7735_COLOR_BLACK);
    _delay_ms(2000);
}

void draw_plot_template(void) {
    st7735_fill_rect(0, 0, 160, 128, ST7735_COLOR_BLACK);

    for(uint8_t y = 28; y < 128; y += 10) {
        st7735_draw_fast_hline(0, y, 160, st7735_color(128,128,128));
    }

    for(uint8_t x = 15; x < 160; x+=15) {
        st7735_draw_fast_vline(x, 20, 128, st7735_color(128,128,128));
    }

    for(uint8_t t = 0; t < 160; t++) {
        uint16_t temp = interpolate_profile(t * 2);

        st7735_draw_pixel(t, 127 - (temp / 250), ST7735_COLOR_GREEN);
    }
}

void format_temp(uint16_t temp, char *buffer) {
    uint16_t a = temp % 100;
    uint16_t b = temp / 100;
    snprintf(buffer, 7, "%0.3d.%0.2d", b, a);
}

void update_plot(uint16_t t, uint16_t temp) {
    char tmp[7];
    tmp[0] = 0;

    st7735_fill_rect(0, 0, 80, 20, ST7735_COLOR_BLACK);
    format_temp(temp, tmp);
    st7735_draw_text(5, 15, tmp, &FreeSans, 1, ST7735_COLOR_RED);

    st7735_fill_rect(80, 0, 80, 20, ST7735_COLOR_BLACK);
    format_temp(interpolate_profile(t), tmp);
    st7735_draw_text(80, 15, tmp, &FreeSans, 1, ST7735_COLOR_GREEN);

    for(uint8_t x = 15; x < 160; x+=15) {
        st7735_draw_fast_vline(x, 20, 28, st7735_color(128,128,128));
    }
    st7735_draw_fast_vline(t / 2, 20, 8, ST7735_COLOR_CYAN);

    st7735_draw_pixel(t / 2, 127 - (temp / 250), ST7735_COLOR_RED);
}


int main(void) {
    spi_init();
    st7735_init();

    st7735_set_orientation(ST7735_LANDSCAPE);

    splash_screen();

    draw_plot_template();

    for(uint16_t t = 0; t < 320; t++) {
        update_plot(t, 2200);
        _delay_ms(1000);
    }


    while(1);
}
