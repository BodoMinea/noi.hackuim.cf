// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
	Name:       LearningArduino.ino
	Created:	17.05.2019 10:48:11
	Author:     DESKTOP-PCJ1ER5\Anca-Maria
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts

#include "BaseCardIOInterface.h"
#include <PololuHD44780.h>
#include <SPI.h>
#include <PN532.h>
#include <PN532_SPI.h>
#include <SoftwareSerial.h>

#include "Validator.h"

/////////////////Y///X//Rs/E/Db4/Db5/Db6/Db7/ss/mp/op/Btn1/Btn2/rx/tx
Validator Tester(16, 2, 2, 3, 4, 7, 8, 9, 10, 5, 6, A0, A1, A2, A3);

void setup() {
	Serial.begin(115120);
	Tester.initialize();
}

// Add the main program code into the continuous loop() function
void loop() {
	Tester.checkReader();
}

