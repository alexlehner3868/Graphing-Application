# Graphing Application
Culminating project for ECE243 using Bare C for an FPGA. 

Instructions:
1.	Load the program (main.c) into CPUlator using Google Chrome, compile it and run it
2.	 Select the coefficient to set:
a.	Option 1: On the devices side panel go to the first PS/2 Keyboard section and using the Key drop down menu select the coefficient you want to change (A, B, C, D) and then select the send button corresponding to make (ie the top one) 
b.	Option 2: you can type it into the “Type Here” field (you don’t need to hit enter)
3.	Set the value of the coefficient (0-9) using the same two methods as specified in step 2
4.	After every repetition of steps 2 and 3, the VGA will update to show the updated graph. The graph is drawn using Bezier’s curve drawing algorithm to achieve the best curvatures possible with the limited pixel amount. Additionally, the JTAG UTART section will output the current equation and the current coefficients are also displayed on the HEXs and the LEDs (A-C are on HEXs and D is on the LEDs). Furthermore an audio depiction of the graph is outputted after every change to the equation.
5.	You can use KEYs 0-2 to change the color of the graph drawn. Key 2 adds more red to the graph’s color, Key 1 adds more green, and key 2 adds more blue.
6.	KEY 3 resets all coefficients to 0 and the color of the graph back to black. 
