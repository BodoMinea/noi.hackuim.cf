#pragma once

#include <PololuHD44780.h>
#include <SPI.h>
#include <PN532.h>
#include <PN532_SPI.h>
#include "PN532IO.h"
#include <SoftwareSerial.h>

const String WelcomeMessage = "Dani Mocanu va ureaza o zi frumoasa! Multumim ca ati ales DM Trans!";
const String ReadingMessage = "Se citeste cardul...";
const String SuccessMessage = "Calatorie placuta!";
const String FailureMessage = "Eroare la citire!";
const String RepeatMessage = "Repetati validarea!";
const String InvalidMessage = "Card invalid!";
const String InsufficientFundsMessage = "Fonduri insuficiente!";
const String RepeatedMessage = "Card deja validat!";
const String IdleMessage = "Apropiati cardul!";
const String RetrieveCardMessage = "Retrageti cardul!";
const String RetrievePhoneMessage = "Retrageti telefonul!";
uint8_t AppIdentifyCommands[] = { 0x00, /* CLA */
							  0xA4, /* INS */
							  0x04, /* P1  */
							  0x00, /* P2  */
							  0x07, /* Length of AID  */
							  0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /* AID defined on Android App */
							  0x00  /* Le  */ };
const float RideFare = 1.3;

class Validator {
	PololuHD44780 Display;
	PN532_SPI SPIRoot;
	PN532 CardReader;
	class LightBulb {
		const uint8_t MediumPin, OtherPin;
	public:
		enum STATE { IDLE, GREEN, RED };

		LightBulb(uint8_t mp, uint8_t op) : MediumPin(mp), OtherPin(op) { pinMode(MediumPin, OUTPUT); pinMode(OtherPin, OUTPUT); setState(IDLE); }

		void setState(STATE state) {
			switch (state) {
			case IDLE: {
				analogWrite(MediumPin, 0);
				analogWrite(OtherPin, 0);
				break;
			}
			case GREEN: {
				analogWrite(MediumPin, 127);
				analogWrite(OtherPin, 0);
				break;
			}
			case RED: {
				analogWrite(MediumPin, 127);
				analogWrite(OtherPin, 255);
				break;
			}
			default: {
				break;
			}
			}
		}

	} Bulb;
	class Button {
		const uint8_t ConnectedPin;
		uint16_t MomentOfActivation;
		const uint8_t ResetTime = 5;
		bool Activated;
	public:
		Button(uint8_t cp) : ConnectedPin(cp), MomentOfActivation(0), Activated(false) { pinMode(ConnectedPin, INPUT_PULLUP); }

		void checkState() {
			if (!Activated) {
				if (digitalRead(ConnectedPin) == LOW) {
					Activated = true;
					MomentOfActivation = millis() / 1000;
				}
			}
			else {
				uint16_t currentTime = millis() / 1000;
				if (currentTime - MomentOfActivation >= ResetTime) {
					Activated = false;
					MomentOfActivation = 0;
				}
			}
		}

		bool getState() { return Activated; }

		void setState(bool state) { Activated = state; }
	}Button1, Button2;
	SoftwareSerial Antenna;
	bool SpecialState;
	String lastUID;
	String validatedCards[4];
	uint8_t nr, rows, columns;
	struct CardData {
		float Sold;
	};
public:
	enum TextAllign { LEFT, CENTER, RIGHT };

	/*Creates a Pololu HD LCD that resembles a Card Validator*/
	Validator(uint8_t maxY, uint8_t maxX, uint8_t rs, uint8_t e, uint8_t db4, uint8_t db5,
		uint8_t db6, uint8_t db7, uint8_t ss, uint8_t mp, uint8_t op, uint8_t bttn1, uint8_t bttn2, uint8_t rx, uint8_t tx) : Display(rs, e, db4, db5, db6, db7), SPIRoot(SPI, ss), CardReader(SPIRoot), Bulb(mp, op), Button1(bttn1), Button2(bttn2), Antenna(rx, tx), nr(0), rows(maxX), columns(maxY) {
		pinMode(rx, INPUT);
		pinMode(tx, OUTPUT);
	}

	/*Initializez the Validator interface and functions*/
	void initialize() { Antenna.begin(4800); Antenna.setTimeout(10);  Display.init(); displayMessage(IdleMessage); SPI.begin();  CardReader.begin(); CardReader.SAMConfig(); 	CardReader.setPassiveActivationRetries(1); Serial.println("Initialized Validator!"); }

	/*Clears Display screen*/
	inline void clearScreen() { Display.clear(); }

	/*Shift the on-screen text _positions_ cells to the left*/
	inline void shiftLeft(uint8_t positions = 1) { for (uint8_t pos = 0; pos < positions; pos++) { Display.scrollDisplayLeft(); delay(300); } }

	/*Shift the on-screen text _positions_ cells to the right*/
	inline void shiftRight(uint8_t positions = 1) { for (uint8_t pos = 0; pos < positions; pos++) { Display.scrollDisplayRight(); delay(200); } }

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
				if (Message.length() > columns) {
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
				if (Message.length() > columns) {
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

	/*Attemts to read data present on the current card*/
	bool readData(uint8_t uid[], uint8_t length, CardData &data) {
		PN532IO Buffer(CardReader, uid, length);

		bool ok = Buffer.ReadBytes(0, &data, sizeof(data));
		if (!ok)
			return false;
		return true;
	}

	/*Attempts to write data on the current card*/
	bool writeData(uint8_t uid[], uint8_t length, CardData &data) {
		PN532IO Buffer(CardReader, uid, length);
		bool ok = Buffer.WriteBytes(0, &data, sizeof(data));
		if (!ok)
			return false;
		return true;
	}

	bool requestData(CardData &data, String uid) {
		Antenna.stopListening();
		String Message = uid + ", request";
		sendMessage(Message);
		Antenna.listen();
		uint8_t started = millis() / 1000, current;
		while (!Antenna.available()) {
			current = millis() / 1000;
			if (current - started >= 3)
				break;
		}
		if (!Antenna.available())
			return false;
		Antenna.readBytes((char *)&data, sizeof(data));
		Serial.println(data.Sold);
		Antenna.stopListening();
		return true;
	}

	void sendMessage(String message) {
		Serial.println(message.c_str());
		Antenna.stopListening();
		Antenna.write(message.c_str(), sizeof(message));
	}

	/*Obtains the last card UID, if existent*/
	String getStringUID(uint8_t uid[], uint8_t length) const {
		String UID = "";
		for (uint8_t i = 0; i < length; i++)
			UID.concat(String(uid[i], HEX));
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
		if (Button1.getState() || Button2.getState())
			SpecialState = true;
		Button1.checkState();
		if (!Button2.getState()) {
			if (Button1.getState()) {
				if (!SpecialState) {
					displayMessage("Consultare card");
					Bulb.setState(LightBulb::GREEN);
				}
			}
			else {
				displayMessage(IdleMessage);
				Bulb.setState(LightBulb::IDLE);
				SpecialState = false;
			}
		}
		Button2.checkState();
		if (!Button1.getState()) {
			if (Button2.getState()) {
				if (!SpecialState) {
					displayMessage("Validare multipla");
					Bulb.setState(LightBulb::GREEN);
				}
			}
			else {
				displayMessage(IdleMessage);
				Bulb.setState(LightBulb::IDLE);
				SpecialState = false;
			}
		}
		boolean success;
		success = CardReader.inListPassiveTarget();
		if (!success)
			return;
		uint8_t response[32], responseLength = 0;
		success = CardReader.inDataExchange(AppIdentifyCommands, sizeof(AppIdentifyCommands), response, &responseLength);
		if (success) {
			if (responseLength > 0) {
				for (int i = 0; i < responseLength; ++i) Serial.print(response[i]);
				Serial.println();
				displayMessage(SuccessMessage);
				Bulb.setState(LightBulb::GREEN);
				Serial.println("Validated mobile app card!");
				delay(2000);
				displayMessage(IdleMessage);
				Bulb.setState(LightBulb::IDLE);
			}
			else {
				displayMessage(InvalidMessage);
				Bulb.setState(LightBulb::RED);
				Serial.println("Got invalid device!");
				delay(2000);
				displayMessage(IdleMessage);
				Bulb.setState(LightBulb::IDLE);
			}
		}
		else {
			uint8_t uid[7];
			uint8_t uidLength;
			success = CardReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
			if (!success)
				return;
			if (success < 0) {
				Serial.println("Failed to read card!");
				displayMessage(FailureMessage);
				Bulb.setState(LightBulb::RED);
				delay(3000);
				if (CardReader.inListPassiveTarget()) {
					displayMessage(RetrieveCardMessage);
					while (CardReader.inListPassiveTarget()) { delay(1000); }
				}
				displayMessage(IdleMessage);
				Bulb.setState(LightBulb::IDLE);
			}
			else {
				CardData data;
				bool read = readData(uid, uidLength, data);
				if (!read) {
					Serial.println("Failed to read card!");
					displayMessage(FailureMessage);
					Bulb.setState(LightBulb::RED);
				}
				else {
					String UID = getStringUID(uid, uidLength);
					if (Button1.getState()) {
						Serial.print("Consulting: "); Serial.print(UID); Serial.println("!");
						if (Button2.getState()) {
							data.Sold += 10;
							writeData(uid, uidLength, data);
							Button2.setState(false);
						}
						String Message = "Credit: ";
						Message.concat(data.Sold);
						Message.concat("RON");
						displayMessage(Message);
						Bulb.setState(LightBulb::GREEN);
						delay(3000);
						if (CardReader.inListPassiveTarget()) {
							displayMessage(RetrieveCardMessage);
							while (CardReader.inListPassiveTarget()) { delay(1000); }
						}
						displayMessage(IdleMessage);
						Bulb.setState(LightBulb::IDLE);
						Button1.setState(false);
					}
					else {
						if (!alreadyValidated(UID)) {
							/*CardData dbData = { 0 };
							bool succes = requestData(dbData, UID);
							if (false) {
								Serial.print("Database sold: "); Serial.println(dbData.Sold);
								if (dbData.Sold < data.Sold)
									data.Sold = dbData.Sold;
							}*/
							Serial.print("Funds: ");  Serial.println(data.Sold);
							if (data.Sold < RideFare) {
								displayMessage(InsufficientFundsMessage);
								Bulb.setState(LightBulb::RED);
							}
							else {
								data.Sold -= RideFare;
								bool wrote = writeData(uid, uidLength, data);
								if (!wrote) {
									Serial.println("Failed to read card!");
									displayMessage(FailureMessage);
									Bulb.setState(LightBulb::RED);
								}
								else {
									Serial.print("Validated "); Serial.print(UID); Serial.println("!");
									validatedCards[nr++] = UID;
									String Message;
									Message.concat("Credit ramas: ");
									Message.concat(data.Sold);
									Message.concat("RON");
									displayMessage(Message);
									Bulb.setState(LightBulb::GREEN);
									String AntennaMessage = UID + ", -1.3";
									sendMessage(AntennaMessage);
									delay(2000);
									displayMessage(SuccessMessage);
								}
							}
						}
						else {
							/*CardData dbData = { 0 };
							bool succes = requestData(dbData, UID);
							if (false) {
								Serial.print("Database sold: "); Serial.println(dbData.Sold);
								if (dbData.Sold < data.Sold)
									data.Sold = dbData.Sold;
							}*/
							if (!Button2.getState()) {
								Serial.print("Already registered "); Serial.print(UID); Serial.println("!");
								displayMessage(RepeatedMessage);
								Bulb.setState(LightBulb::GREEN);
							}
							else {
								Serial.print("Funds: ");  Serial.println(data.Sold);
								if (data.Sold < RideFare) {
									displayMessage(InsufficientFundsMessage);
									Bulb.setState(LightBulb::RED);
								}
								else {
									data.Sold -= RideFare;
									bool wrote = writeData(uid, uidLength, data);
									if (!wrote) {
										Serial.println("Failed to read card!");
										displayMessage(FailureMessage);
										Bulb.setState(LightBulb::RED);
									}
									else {
										Serial.print("Validated (multiple) "); Serial.print(UID); Serial.println("!");
										validatedCards[nr++] = UID;
										String Message;
										Message.concat("Credit ramas: ");
										Message.concat(data.Sold);
										Message.concat("RON");
										displayMessage(Message);
										Bulb.setState(LightBulb::GREEN);
										String AntennaMessage = UID + ", -1.3";
										sendMessage(AntennaMessage);
										delay(2000);
										displayMessage(SuccessMessage);
									}
								}
								Button2.setState(false);
							}
						}
					}
				}
			}
			delay(2000);
			if (CardReader.inListPassiveTarget()) {
				displayMessage(RetrieveCardMessage);
				while (CardReader.inListPassiveTarget()) { delay(1000); }
			}
			displayMessage(IdleMessage);
			Bulb.setState(LightBulb::IDLE);
		}
	}

	~Validator() {}
};