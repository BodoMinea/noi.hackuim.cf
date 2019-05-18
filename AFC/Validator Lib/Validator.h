#pragma once

#include <PololuHD44780.h>
#include <SPI.h>
#include <PN532.h>
#include <PN532_SPI.h>

const String WelcomeMessage = "Dani Mocanu va ureaza o zi frumoasa! Multumim ca ati ales DM Trans!";
const String ReadingMessage = "Se citeste cardul...";
const String SuccessMessage = "Calatorie placuta!";
const String FailureMessage = "Validare esuata!";
const String RepeatedMessage = "Card deja validat!";
const String IdleMessage = "Apropiati cardul!";

class Validator {
	PololuHD44780 Display;
	PN532_SPI SPIRoot;
	PN532 CardReader;
	String lastUID;
	String validatedCards[50];
	uint8_t nr, rows, columns;
public:
	enum TextAllign { LEFT, CENTER, RIGHT };

	/*Creates a Pololu HD LCD that resembles a Card Validator*/
	Validator(uint8_t maxY, uint8_t maxX, uint8_t rs, uint8_t e, uint8_t db4, uint8_t db5,
		uint8_t db6, uint8_t db7, uint8_t ss) : Display(rs, e, db4, db5, db6, db7), SPIRoot(SPI, ss), CardReader(SPIRoot), nr(0), rows(maxX), columns(maxY) {}

	void initialize() { Display.init(); displayMessage(IdleMessage); SPI.begin();  CardReader.begin(); CardReader.SAMConfig(); 	Serial.println("Initialized Validator!"); }

	void clearScreen() { Display.clear(); }

	/*Shift the on-screen text _positions_ cells to the left*/
	void shiftLeft(uint8_t positions = 1) { for (uint8_t pos = 0; pos < positions; pos++) { Display.scrollDisplayLeft(); delay(300); } }

	/*Shift the on-screen text _positions_ cells to the right*/
	void shiftRight(uint8_t positions = 1) { for (uint8_t pos = 0; pos < positions; pos++) { Display.scrollDisplayRight(); delay(200); } }

	/*Fetches the number of the lines on the LCD*/
	uint8_t get_rows() const { return rows; }

	/*Fetches the number of the columns on the LCD*/
	uint8_t get_columns() const { return columns; }

	/*Checks whether the provided coordinates describe an on-screen position*/
	bool validPosition(uint8_t Y, uint8_t X) const { return (X * Y < rows * columns && X >= 0 && Y >= 0); }

	/*Checks whether the given text fits on the LCD*/
	bool textFits(String Text) { return (Text.length() <= rows * columns); }

	/*Attempts to display a message on the LCD, alligned as intended*/
	bool displayMessage(String Message, TextAllign Allign = CENTER) {
		clearScreen();
		if (!textFits(Message))
			return false;
		switch (Allign)
		{
		case LEFT: {
			uint8_t currentRow = 0;
			uint8_t stringPos = 0;
			String substring;
			do {
				if (Message.length() >= columns) {
					uint8_t write = columns;
					if (Message.charAt(stringPos + columns) != ' ') {
						do {
							--write;
						} while (Message.charAt(stringPos + write) != ' ');
					}
					substring = Message.substring(0, stringPos + write);
					stringPos += write + 1;
				}
				else {
					substring = Message;
					stringPos += Message.length() + 1;
				}
				Display.setCursor(0, currentRow++);
				Display.print(substring);
				Message = Message.substring(stringPos);
			} while (Message.length() > 0);
			break;
		}
		case CENTER: {
			uint8_t currentRow = 0;
			uint8_t stringPos = 0;
			String substring;
			do {
				if (Message.length() >= columns) {
					uint8_t write = columns;
					if (Message.charAt(stringPos + columns) != ' ') {
						do {
							--write;
						} while (Message.charAt(stringPos + write) != ' ');
					}
					substring = Message.substring(0, stringPos + write);
					stringPos += write + 1;
				}
				else {
					substring = Message;
					stringPos += Message.length() + 1;
				}
				Display.setCursor(ceil((columns - 1) / 2.0 - (substring.length() - 1) / 2.0), currentRow++);
				Display.print(substring);
				Message = Message.substring(stringPos);
			} while (Message.length() > 0);
			break;
		}
		default: {
			break;
		}
		}
		return true;
	}

	/*void displayLeftShiftingMessage(String Message) {
		uint8_t maxChar = columns;
		Display.clear();
		Display.setCursor(columns, 0);
		for (uint8_t pos = 0; pos < Message.length(); pos += maxChar) {
			uint8_t end;
			if (pos + maxChar >= Message.length())
				end = Message.length() - 1;
			else
				end = pos + maxChar;
			Display.print(Message.substring(pos, end));
			shiftLeft(end - pos);
			delay(300);
		}
	}*/

	/*Obtains the last card UID, if existent*/
	String getStringUID(uint8_t uid[], uint8_t length) const {
		String UID = "";
		for (uint8_t i = 0; i < length; i++) {
			UID.concat(String(uid[i] < 0x10 ? " 0" : " "));
			UID.concat(String(uid[i], HEX));
		}
		return UID;
	}

	/*Checks whether the UID is already contained*/
	bool alreadyValidated(String UID) {
		displayMessage(ReadingMessage);
		delay(1500);
		for (uint8_t i = 0; i < nr; ++i)
			if (validatedCards[i] == UID)
				return true;
		return false;
	}

	/*Checks the reader for cards. Displays the card UID if one is found*/
	void checkReader() {
		boolean success;
		uint8_t uid[7];
		uint8_t uidLength;
		success = CardReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
		if (!success)
			return;
		if (success < 0) {
			Serial.println("Failed to read card!");
			displayMessage(FailureMessage);
			delay(200);
			return;
		}
		Serial.println("Found card!");
		uint8_t selectApdu[] = { 0x00, /* CLA */
							  0xA4, /* INS */
							  0x04, /* P1  */
							  0x00, /* P2  */
							  0x07, /* Length of AID  */
							  0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /* AID defined on Android App */
							  0x00  /* Le  */ };

		uint8_t response[32], responseLength = 32;
		success = CardReader.inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength);
		if (success) {
			Serial.print("responseLength: "); Serial.println(responseLength);
			CardReader.PrintHexChar(response, responseLength);
			do {
				uint8_t apdu[] = { 'H', 'e', 'l', 'l', 'o', 'f', 'r', 'o', 'm', 'A', 'r', 'd', 'u', 'i', 'n', 'o', '!' };
				uint8_t back[32];
				uint8_t length = 32;
				success = CardReader.inDataExchange(apdu, sizeof(apdu), back, &length);
				if (success) {
					Serial.print("responseLength: "); Serial.println(length);
					CardReader.PrintHexChar(back, length);
				}
				else {
					Serial.println("Broken connection?");
				}
			} while (success);
		}
		else {
			String UID = getStringUID(uid, uidLength);
			UID.toUpperCase();
			//Serial.println(UID);
			if (!alreadyValidated(UID)) {
				Serial.println(("Validated " + UID) + "!");
				validatedCards[nr++] = UID;
				displayMessage(SuccessMessage);
			}
			else {
				Serial.println(("Already registered " + UID) + "!");
				displayMessage(RepeatedMessage);
			}
			//while (CardReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength));
			delay(2000);
			displayMessage(IdleMessage);
		}
	}

	~Validator() {}
};