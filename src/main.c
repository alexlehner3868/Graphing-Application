#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

//Define constants 
#define WIDTH 320
#define HEIGHT 240
#define AXIS  120
#define BLANK 0x0000

//function declarations 
void clear_screen();
void wait_for_vsync();
void draw_rect(int, int, int, int, short int);
void draw_line(int, int, int, int, short int);
void plot_pixel(int, int, short int);
void swap(int *, int *);
void draw_graph(double y[320]);
void draw_axis();
int power(int base, int exponent);
int upper_hex_bits(int a);
int hex_num(int num);
int lower_hex_bits(int b, int c);
int get_binary_num(int num);


volatile int pixel_buffer_start; // global variable
//INSTRUCTIONS: use the PS2 keyboard to set the value of coefficents. 
//Input the captial letter of the coefficent first (A->x^3, B -> x^2, C->x, D->constant)
//Then use the PS2 keyboard to input a number (0-9) for the coefficent
//Look at the VGA to see the graph
//The current coefficents A, B, C are displayed on the HEXs and D is on the LEDs

int main(void) {
	
	//get pointers to I/O devices
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address
	volatile int * HEX3_HEX0_ptr = (int *) 0xFF200020; // HEX3_HEX0 address
	volatile int * HEX6_HEX4_ptr = (int *) 0xFF200030; // HEX6_HEX4 address
	volatile int * LED_ptr = (int *) 0xFF200000; // LED address
	
	
	
	//initialize hex displays 
	int HEX_bits = 0x00000000; // initial pattern for HEX displays
	*(HEX6_HEX4_ptr) = HEX_bits; 
	*(HEX3_HEX0_ptr) = HEX_bits; 
	
	/* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
	
	//variables related to the equation: y= ax^3 + bx^2 +cx +d
	int a= 0;  
	int b=0; 
	int c=0;
	int d=0;
	double y[320];
	double p[4]; //stores control values of bezier curves
	double x; //a counter variables 
	
	//variables for keyboard input
	int PS2_data, RVALID;
	unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;
	
	//booleans to update coefficents values
	bool changed = false;
	bool changeA = false;
	bool changeB = false;
	bool changeC = false;
	bool changeD = false;
	bool erase = false;
	
	//clear screen and draw the axis
	clear_screen();
	draw_axis();
	
	//program contstantly loops 
	while(1){
		
		PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field
		if (RVALID != 0)
		{
			/* always save the last three bytes received */
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
		}
		
		//the letter of the coefficient to be changd is inputted 
		if(byte3 == 0x1C){ //A -> x^3
			changeA = true;	
		}else if(byte3 == 0x32){ //B -> x^2
			changeB = true;
		}else if(byte3 == 0x21){ //C -> x
			changeC = true;
		}else if(byte3 == 0x23){ //D -> constant
			changeD = true;
		}else if(byte3 == 0x66) { //backspace to clear graph
			a= 0;
			b=0;
			c=0;
			d=0; //NOT FINISHED ERASE
		}
		
		//A number between 0-9 is inputted
		if(byte3 == 0x45 || byte3 == 0x16 ||byte3 == 0x1E ||byte3 == 0x26 ||
		  byte3 == 0x25 ||byte3 == 0x2E||byte3 == 0x36 ||byte3 == 0x3D ||
		  byte3 == 0x3E ||byte3 == 0x46){ 
			if(changeA){
				a= byte3;
				changed = true;
				changeA = false;
			}else if(changeB){
				b= byte3;
				changed = true;
				changeB = false;
			}else if(changeC){
				c= byte3;
				changed = true;
				changeC = false;
			}else if(changeD){
				d= byte3;
				changed = true;
				changeD = false;
			}
		}
		
		//update HEX and LED values 
		HEX_bits = upper_hex_bits(a);
		*(HEX6_HEX4_ptr) = HEX_bits; 
		HEX_bits = lower_hex_bits(b, c);
		*(HEX3_HEX0_ptr) = HEX_bits;
		*(LED_ptr) = get_binary_num(d); 
		
		//(re)set counter variable to 20
		x=-20;
		
		//create all the points of the curve 
		if(a!=0){ //cubic function
			
			//calculate the control points
			for(int i=0; i < 4; i++){
				p[i] = (a * power(x,3)) + (b * power(x,2)) + c*x + d;
				x=x+10;
			}
			
			x=-20;
			
			//calculate the points using Bezier Curves
			for(int i=0; i <320; i++){
				y[i] = power((1-x),3)*p[0];
				y[i] += 3* x * power((1-x), 2) *p[1];
				y[i] += 3 * power(x,2) * (1-x) * p[2];
				y[i] += power(x,3) *p[3];
				if(a<2){
					y[i] /=1000000;
				}else{
					y[i] /=7000000;
				}
				y[i] =120 - y[i];	
				y[i] = (int) y[i];
				x= x+0.1254;
			}
			
		}else if(b!=0){ //quad 
			
			//calculate control points
			for(int i=0; i < 3; i++){
				p[i] = (b * power(x,2)) + c*x + d;
				x=x+13;
			}
			
			x=-20;
			
			//calculate the points using Bezier Curves
			for(int i=0; i <320; i++){
				y[i] = power((1-x),2)*p[0];
				y[i] += 2* x * (1-x)*p[1];
				y[i] += power(x,2) *p[2];	
				y[i] /=70000;
				y[i] =120 - y[i];	
				y[i] = (int) y[i];
				x= x+0.1254;
			}

		}else if(c!=0){ //linear
			
			//get points on the line
			for(int i=0; i <320; i++){
				y[i] = c*x + d;
				y[i] =120 - y[i];	
				x= x+0.1254;
			}
			
		}else if (d!=0){ //const
			
			//set all y values = to d
			for(int i=0; i <320; i++){
				y[i] = 120 -d;
			}
		}

	//if a coefficient has been changed redraw the screen
	if(changed){		
		clear_screen();
		draw_axis();
		draw_graph(y);
		changed = false;
	}
	
	}
}

int upper_hex_bits(int a){
	int HexCode = 0x77; //A 
	HexCode = HexCode<<8; //shift A over to next HEX
	HexCode +=hex_num(a); //get hex code of A and add it in
	return HexCode;
}

int lower_hex_bits(int b, int c){
	int HexCode = 0x7C; //B
	HexCode = HexCode<<24; //shift B over 
	HexCode += hex_num(b)<< 16;  //get Hex code of B and shift it 
	HexCode += (0x39 <<8); //C
	HexCode += hex_num(c); //add in C
	return HexCode;
}

int hex_num(int num){
	if(num==0x45 || num==0){ //0
		return 0x3F;
	}else if(num==0x16){ //1
		 return 0b00000110;
	}else if(num==0x1E){ //2
		 return 0b01011011;
	}else if(num==0x26){ //3
		return 0b01001111;
	}else if(num==0x25){ //4
		return 0b01100110;
	}else if(num==0x2E){ //5
		return 0b01101101;
	}else if(num==0x36){ //6
		return 0b01111101;
	}else if(num==0x3D){ //7
		return 0b00000111;
	}else if(num==0x3E){ //8
		return 0b01111111;
	}else if(num==0x46){ //9
		return 0b01100111;
	}
}

int get_binary_num(int num){
	if(num==0x45 || num==0){ //0
		return 0x0;
	}else if(num==0x16){ //1
		 return 0x1;
	}else if(num==0x1E){ //2
		 return 0x2;
	}else if(num==0x26){ //3
		return 0x3;
	}else if(num==0x25){ //4
		return 0x4;
	}else if(num==0x2E){ //5
		return 0x5;
	}else if(num==0x36){ //6
		return 0x6;
	}else if(num==0x3D){ //7
		return 0x7;
	}else if(num==0x3E){
		return 0x8;
	}else if(num==0x46){
		return 0x9;
	}
}

void draw_axis(){
	draw_line(0, AXIS, WIDTH, AXIS, 0xFFFF); //x-axis
	draw_line(WIDTH/2, 0, WIDTH/2, HEIGHT, 0xFFFF); //y-axis
}
void draw_graph(double y[320]){
	for(int i=1; i <320; i++){	
		if(i%8==0){ 
			draw_line(i, 118, i, 122, 0xFFFF);
		}
		if(y[i] >= 0 && y[i] <= 240){		
			draw_line(i-1, y[i-1], i, y[i], 0x07E0);   // this line is blue
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
