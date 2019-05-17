#pragma once

#include <PololuHD44780.h>
#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <deprecated.h>

const String WelcomeMessage = "Dani Mocanu va ureaza o zi frumoasa! Multumim ca ati ales DM Trans!";
const String ReadingMessage = "Se citeste cardul...";
const String SuccessMessage = "Calatorie placuta!";
const String FailureMessage = "Validare esuata!";
const String RepeatedMessage = "Card deja validat!";
const String IdleMessage = "Apropiati cardul!";

class Validator {
	PololuHD44780 Display;
	MFRC522Extended CardReader;
	String lastUID;
	String validatedCards[50];
	uint8_t nr, rows, columns;
public:
	enum TextAllign { LEFT, CENTER, RIGHT };

	/*Creates a Pololu HD LCD that resembles a Card Validator*/
	Validator(uint8_t maxY, uint8_t maxX, uint8_t rs, uint8_t e, uint8_t db4, uint8_t db5,
		uint8_t db6, uint8_t db7, uint8_t ss_pin, uint8_t rst_pin) : Display(rs, e, db4, db5, db6, db7), CardReader(ss_pin, rst_pin), nr(0), rows(maxX), columns(maxY) {}

	void initialize() { displayMessage(IdleMessage); CardReader.PCD_Init(); }

	void clearScreen() { Display.clear(); }

	/*Shift the on-screen text _positions_ cells to the left*/
	void shiftLeft(uint8_t positions = 1) {	for (uint8_t pos = 0; pos < positions; pos++) { Display.scrollDisplayLeft(); delay(300); } }

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
	String getLastUID() const {
		String UID = "";
		for (byte i = 0; i < CardReader.uid.size; i++) {
			UID.concat(String(CardReader.uid.uidByte[i] < 0x10 ? " 0" : " "));
			UID.concat(String(CardReader.uid.uidByte[i], HEX));
		}
		return UID;
	}

	bool alreadyValidated(String Checked) {
		displayMessage(ReadingMessage);
		delay(2000);
		for (uint8_t i = 0; i < nr; ++i)
			if (validatedCards[i] == Checked)
				return true;
		return false;
	}

	/*Checks the reader for cards. Displays the card UID if one is found*/
	void checkReader() {
		if (!CardReader.PICC_IsNewCardPresent() || !CardReader.PICC_ReadCardSerial())
			return;
		String UID = getLastUID();
		UID.toUpperCase();
		//Serial.println(UID);
		if (!alreadyValidated(UID)) {
			//Serial.println("Validated " + UID);
			validatedCards[nr++] = UID;
			displayMessage(SuccessMessage);
		}
		else {
			//Serial.println("Already registered " + UID);
			displayMessage(RepeatedMessage);
		}
		/*if (lastUID != UID) {
			displayMessage(SuccessMessage);
			lastUID = UID;
		}
		else {
			displayMessage(RepeatedMessage);
		}*/
		delay(3000);
		displayMessage(IdleMessage);
	}

	~Validator() {}
};