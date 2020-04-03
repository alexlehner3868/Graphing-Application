#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
	
#define WIDTH 320
#define HEIGHT 240
#define AXIS  120
#define BLANK 0x0000

void clear_screen();
void wait_for_vsync();
void draw_rect(int, int, int, int, short int);
void draw_line(int, int, int, int, short int);
void plot_pixel(int, int, short int);
void swap(int *, int *);
int power(int base, int exponent);
volatile int pixel_buffer_start; // global variable

int main(void) {
     volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();

	//draw axis
	draw_line(0, AXIS, WIDTH, AXIS, 0xFFFF); //x-axis
	draw_line(WIDTH/2, 0, WIDTH/2, HEIGHT, 0xFFFF); //y-axis
	// y= ax^3 + bx^2 +cx +d
	int a, b, c, d;
//y-axis
	a= 0; 
	b=1;
	c=0;
	d=0;
	double y[320];
	double x =-20;
	int ymax =0;
	double temp;
	for(int i=0; i<320; i++){
		
    	temp = (a * power(x,3)) + (b * power(x,2)) + c*x + d;
    	y[i] = (int)(temp);
    
	  	y[i] = 120 - y[i];

		x= x+0.1254;
	}

	for(int i=1; i <320; i++){	
		if(i%8==0){ 
			draw_line(i, 118, i, 122, 0xFFFF);
		}
		if(y[i] >= 0 && y[i] <= 240){		
			draw_line(i-1, y[i-1], i, y[i], 0x001F);   // this line is blue
		}
		
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

int power(int base, int exponent){
  int result=1;
  for (int i=exponent; i>0; i--){
    result = result * base;
  }
  return result;
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