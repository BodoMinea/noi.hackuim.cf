// BaseCardIOInterface.h

#ifndef _BASECARDIOINTERFACE_h
#define _BASECARDIOINTERFACE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

struct IOableArea {
	uint8_t Sector;
	uint8_t Block;
};

class BaseCardIOInterface {
private:
	const IOableArea WritableAreas[6] = {
		{0, 1},
		{0, 2},
		{1, 0},
		{1, 1},
		{1, 2}
	};

	const size_t BlockSize = 16;
	const size_t MaxSize = sizeof(WritableAreas) / sizeof(IOableArea) * BlockSize;

protected:
	uint8_t KeyA[6] = {0xC1, 0xE5, 0x1C, 0x63, 0xB8, 0xF5};
	uint8_t KeyB[6] = {0xE8, 0x61, 0xFD, 0xE1, 0xB5, 0x9F};

public:
	bool ReadBytes(size_t Offset, void * Data, size_t Count) {
		if (Offset >= MaxSize || Offset + Count > MaxSize)
			return false;
		
		uint8_t DataCursor = 0;
		uint8_t RemainingBytes = Count;

		uint8_t CurrentArea = Offset / BlockSize;
		uint8_t AreaCursor = Offset % BlockSize;

		while (RemainingBytes > 0) {
			size_t BytesToRead = min(abs(RemainingBytes - AreaCursor), BlockSize);
			bool IsOkay = ReadBytesHandler(
				WritableAreas[CurrentArea].Sector,
				WritableAreas[CurrentArea].Block,
				AreaCursor,
				(char *)Data + DataCursor,
				BytesToRead
			);

			if (!IsOkay)
				return false;

			DataCursor += BytesToRead;
			RemainingBytes -= BytesToRead;

			CurrentArea++;
			AreaCursor = 0;
		}

		return true;
	}

	bool WriteBytes(size_t Offset, void * Data, size_t Count) {
		if (Offset >= MaxSize || Offset + Count > MaxSize)
			return false;
		
		uint8_t DataCursor = 0;
		uint8_t RemainingBytes = Count;

		uint8_t CurrentArea = Offset / BlockSize;
		uint8_t AreaCursor = Offset % BlockSize;

		while (RemainingBytes > 0) {
			size_t BytesToWrite = min(abs(RemainingBytes - AreaCursor), BlockSize);

			bool IsOkay = WriteBytesHandler(
				WritableAreas[CurrentArea].Sector,
				WritableAreas[CurrentArea].Block,
				AreaCursor,
				(char *)Data + DataCursor,
				BytesToWrite
			);

			if (!IsOkay)
				return false;

			DataCursor += BytesToWrite;
			RemainingBytes -= BytesToWrite;

			CurrentArea++;
			AreaCursor = 0;
		}

		return true;
	}

protected:
	virtual bool ReadBytesHandler(uint8_t Sector, uint8_t Block, uint8_t Offset, void * Data, size_t Count) = 0;
	virtual bool WriteBytesHandler(uint8_t Sector, uint8_t Block, uint8_t Offset, void * Data, size_t Count) = 0;
};

#endif

