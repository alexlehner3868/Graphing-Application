#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

//Define constants 
#define WIDTH 320
#define HEIGHT 240
#define AXIS  120
#define BLANK 0xFFFF
#define COLOR_LENGTH 9
#define BUF_SIZE 80000 // about 10 seconds of buffer (@ 8K samples/sec)
#define BUF_THRESHOLD 96 // 75% of 128 word buffer
	
//function declarations 
void clear_screen();
void wait_for_vsync();
void draw_rect(int, int, int, int, short int);
void draw_line(int, int, int, int, short int);
void plot_pixel(int, int, short int);
void swap(int *, int *);
void draw_graph(double y[320], short int color);
void draw_axis();
int power(int base, int exponent);
int upper_hex_bits(int a);
int hex_num(int num);
int lower_hex_bits(int b, int c);
int get_binary_num(int num);
short int make_color(short int r, short int g, short int b);
void play_wave(int y[320]);
void put_jtag(char);
void do_jtag(int a, int b, int c, int d);
char get_char(int num);

//INSTRUCTIONS: use the PS2 keyboard to set the value of coefficents. 
//Input the captial letter of the coefficent first (A->x^3, B -> x^2, C->x, D->constant)
//Then use the PS2 keyboard to input a number (0-9) for the coefficent
//Look at the VGA to see the graph
//The current coefficents A, B, C are displayed on the HEXs and D is on the LEDs
//The X axis' ticks are a change by 1
//the y axis ticks are a change by 10
//The current equation is displayed on the JTAG

volatile int pixel_buffer_start; // global variable


int main(void) {
	
	//get pointers to I/O devices
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address
	volatile int * HEX3_HEX0_ptr = (int *) 0xFF200020; // HEX3_HEX0 address
	volatile int * HEX6_HEX4_ptr = (int *) 0xFF200030; // HEX6_HEX4 address
	volatile int * LED_ptr = (int *) 0xFF200000; // LED address
	volatile int * KEY_ptr = (int *)0xFF200050; // pushbutton KEY address
	
	//color options 
	short int color;
	short int red_color =  0b0;
	short int blue_color = 0b0;
	short int green_color =0b0;
	bool color_change;
	
	int KEY_value;
	
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
		
		//updated color
		KEY_value = *(KEY_ptr); // read the pushbutton KEY values
		if (KEY_value != 0){ // check if any KEY was pressed
			color_change = true;
			if(KEY_value==1){
				blue_color=blue_color+2;
				if(blue_color==(0b11111+1)){
					blue_color = 0;
				}
			}else if(KEY_value==2){
				green_color = green_color +2;
				if(green_color==(0b111111+1)){
					green_color =0;
				}
			}else if(KEY_value==4){
				red_color = red_color +2;
				if(red_color==(1+0b11111)){
					red_color =0;
				}
			}else if(KEY_value == 8){
				red_color =  0;
				blue_color = 0;
				green_color =0;
				a =0;
				b=0;
				c=0;
				d=0;
				clear_screen();
				draw_axis();
				changed = false;
			}
			while (*KEY_ptr); // wait for pushbutton KEY release
		}

		//update HEX and LED values 
		HEX_bits = upper_hex_bits(a);
		*(HEX6_HEX4_ptr) = HEX_bits; 
		HEX_bits = lower_hex_bits(b, c);
		*(HEX3_HEX0_ptr) = HEX_bits;
		*(LED_ptr) = get_binary_num(d); 
		
		//(re)set counter variable to 20
		x=-20;
		
		//update the values of the coefficents to the binary #a
		a = get_binary_num(a);
		b = get_binary_num(b);
		c = get_binary_num(c);
		d = get_binary_num(d);
		
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
				y[i] /=1000000;
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
				y[i] /=20000;
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
		}else{
			for(int i=0; i <320; i++){
				y[i]=-1;
			}
		}
	
	color = make_color(red_color, green_color, blue_color);
	//if a coefficient has been changed redraw the screen
	if(changed){		
		clear_screen();
		draw_axis();
		draw_graph(y, color);
		play_wave(y);
		do_jtag(a, b, c, d);
		color_change =false;
		changed = false;
	}
	if(!changed && color_change){
		draw_graph(y, color);
		color_change = false;
	}
	}
}

void put_jtag( char c ){ //Function taken from intel FPGA University Program Manual 
	volatile int * JTAG_UART_ptr = (int *) 0xFF201000; // JTAG UART address
	int control;
	control = *(JTAG_UART_ptr + 1); // read the JTAG_UART control register
	if (control & 0xFFFF0000) // if space, echo character, else ignore
	*(JTAG_UART_ptr) = c;
}

void do_jtag(int a, int b, int c, int d){
	//Used to place JTAG
		char text_string[30];
		text_string[0]='y';
		text_string[1] = ' ';
		text_string[2]='=';
		text_string[3]=' ';
		text_string[4] = get_char(a);
		text_string[5]='*';
		text_string[6]='x';
		text_string[7]='^';
		text_string[8]='3';
		text_string[9]=' ';
		text_string[10]='+';
		text_string[11]=' ';
		text_string[12]=get_char(b);
		text_string[13]='*';
		text_string[14]='x';
		text_string[15]='^';
		text_string[16]='2';
		text_string[17]=' ';
		text_string[18]='+';
		text_string[19]=' ';
		text_string[20]=get_char(c);
		text_string[21]='*';
		text_string[22]='x';
		text_string[23]=' ';
		text_string[24]='+';
		text_string[25]=' ';
		text_string[26]=get_char(d);
		text_string[27]='\n';
		text_string[28]='>';
		text_string[29]='\0';
	
	for (int i=0; i <30; i++){
			put_jtag (text_string[i]);
	}
}
char get_char(int num){
	switch(num){
		case 0:
			return '0';
		case 1:
			return '1';
		case 2:
			return '2';
		case 3:
			return '3';
		case 4:
			return '4';
		case 5:
			return '5';
		case 6:
			return '6';
		case 7:
			return '7';
		case 8:
			return '8';
		case 9:
			return '9';
		
	}
}

void play_wave(int y[320]){
	volatile int * audio_ptr = (int *)0xFF203040; //base of audio port
	
	//used for the playback 
	int fifospace;
	int left_buffer[BUF_SIZE];
	int right_buffer[BUF_SIZE];
	
	int y_index =0;
	for(int i =0; i <BUF_SIZE; i++){
		if(i%250==0){
			y_index++;
		}
		left_buffer[i] = y[y_index]*4095/2;
		right_buffer[i] = y[y_index]*4095/2;
	} 
	
	for(int i =0; i <80000; i++){
		*(audio_ptr + 2) = left_buffer[i];
		*(audio_ptr + 3) = right_buffer[i];
	} 
	
}
short int make_color(short int r, short int g, short int b){
	short int color = (r<<11) +(g<<5) +b;
	return color;
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
	}else if(num==0x16|| num==1){ //1
		 return 0b00000110;
	}else if(num==0x1E|| num==2){ //2
		 return 0b01011011;
	}else if(num==0x26|| num==3){ //3
		return 0b01001111;
	}else if(num==0x25|| num==4){ //4
		return 0b01100110;
	}else if(num==0x2E|| num==5){ //5
		return 0b01101101;
	}else if(num==0x36|| num==6){ //6
		return 0b01111101;
	}else if(num==0x3D|| num==7){ //7
		return 0b00000111;
	}else if(num==0x3E|| num==8){ //8
		return 0b01111111;
	}else if(num==0x46|| num==9){ //9
		return 0b01100111;
	}
}

int get_binary_num(int num){
	if(num==0x45 || num==0){ //0
		return 0x0;
	}else if(num==0x16|| num==1){ //1
		 return 0x1;
	}else if(num==0x1E|| num==2){ //2
		 return 0x2;
	}else if(num==0x26|| num==3){ //3
		return 0x3;
	}else if(num==0x25|| num==4){ //4
		return 0x4;
	}else if(num==0x2E|| num==5){ //5
		return 0x5;
	}else if(num==0x36|| num==6){ //6
		return 0x6;
	}else if(num==0x3D|| num==7){ //7
		return 0x7;
	}else if(num==0x3E|| num==8){
		return 0x8;
	}else if(num==0x46|| num==9){
		return 0x9;
	}
}

void draw_axis(){
	draw_line(0, AXIS, WIDTH, AXIS, 0x0); //x-axis
	draw_line(WIDTH/2, 0, WIDTH/2, HEIGHT, 0x0); //y-axis
	//ticks on x-axis
	for(int i=0; i<320; i++){
		if(i%8==0){ 
			draw_line(i, 118, i, 122, 0x0);
		}
	}
	for(int i=0; i < 240; i++){
		if(i%10==0){
			draw_line(157, i, 163, i, 0x0);
		}
	}
}
void draw_graph(double y[320], short int color){
	for(int i=1; i <320; i++){	
		if(y[i] >= 0 && y[i] <= 240){		
			draw_line(i-1, y[i-1], i, y[i], color);   // this line is blue
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
	if(x>=0 && x<320 && y >=0 && y<240){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = color;
	}
}

void swap(int *a, int *b) {
    // use XOR swap (just for fun)
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}