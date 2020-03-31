#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 320
#define HEIGHT 240

#define BLANK 0x0000

void clear_screen();
void wait_for_vsync();
void draw_rect(int, int, int, int, short int);
void draw_line(int, int, int, int, short int);
void plot_pixel(int, int, short int);
void swap(int *, int *);

volatile int pixel_buffer_start; // global variable

int main(void) {
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables
    const int N = 8; // number of rectangles
    const int width = 5; const int height = 5;
    int x[N]; int y[N]; // coordinates of each rectangle
    int x_step[N]; int y_step[N]; // step direction of each rectangle
    int color[N]; // color of each box
    // initialize location and direction of rectangles
    srand(time(NULL)); // use time as seed for `rand()`
    for (int i = 0; i < N; i++) {
        x[i] = rand() % (WIDTH - width);
        y[i] = rand() % (HEIGHT - height);
        x_step[i] = 2*(rand() % 2) - 1;
        y_step[i] = 2*(rand() % 2) - 1;
        color[i] = rand() % 0xFFFF;
    }

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    while (true) {
        /* Erase any boxes and lines that were drawn in the last iteration */
        clear_screen();

        // code for drawing the boxes and lines
        for (int i = 0; i < N; i++) {
            // draw line
            draw_line(x[i] + width / 2, y[i] + height / 2,
                      x[(i + 1) % N] + width / 2, y[(i + 1) % N] + height / 2,
                      0xFFFF - color[i]);
            draw_rect(x[i], y[i], width, height, color[i]); // draw box
        }
        // code for updating the locations of boxes
        for (int i = 0; i < N; i++) {
            // update location of box
            x[i] += x_step[i];
            y[i] += y_step[i];
            // update direction if needed
            if (x[i] == 0 || x[i] == WIDTH - width)
                x_step[i] *= -1;
            if (y[i] == 0 || y[i] == HEIGHT - height)
                y_step[i] *= -1;
        }

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

void clear_screen() {
    // loop through entire buffer
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            plot_pixel(x, y, BLANK); // plot pixel as BLANK
        }
    }
}

void wait_for_vsync() {
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    *(pixel_ctrl_ptr) = 1; // swap pixel buffers
    while (*(pixel_ctrl_ptr + 3) & 1); // wait until S bit is 0
}

void draw_rect(int x, int y, int width, int height, short int color) {
    // loop through coordinates of rectangle
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            plot_pixel(x + i, y + j, color); // plot pixel
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int color) {
    // check if line is steep
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);

    // swap coordinates appropriately if steep
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    // calculate deltas, error, y_step
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y_step = (y0 < y1) ? 1 : -1;

    // initialize y to y0
    int y = y0;
    // loop through x in line
    for (int x = x0; x < x1; x++) {
        // plot pixel appropriately
        if (is_steep)
            plot_pixel(y, x, color);
        else
            plot_pixel(x, y, color);

        // update error, y
        error += deltay;
        if (error >= 0) {
            y += y_step;
            error -= deltax;
        }
    }
}

void plot_pixel(int x, int y, short int color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = color;
}

void swap(int *a, int *b) {
    // use XOR swap (just for fun)
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
