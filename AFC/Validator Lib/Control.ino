// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
	Name:       LearningArduino.ino
	Created:	17.05.2019 10:48:11
	Author:     Stoleru Vlad-Stefan
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts

#include <PololuHD44780.h>
#include <SPI.h>
#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <deprecated.h>
#include "Validator.h"

/////////////////Y///X//Rs/E/Db4/Db5/Db6/Db7/ss/rst
Validator Tester(16, 2, 2, 3, 4, 7, 8, 9, 9, 10);

void setup() {
	Serial.begin(9600);
	Serial.println(sizeof(String));
	SPI.begin();
	Tester.initialize();
	//Serial.println("Scan PICC to see UID and type...");
}

// Add the main program code into the continuous loop() function
void loop(){
	Tester.checkReader();
}
