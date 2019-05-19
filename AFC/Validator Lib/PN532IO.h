#include <PN532.h>
#include "BaseCardIOInterface.h"

class PN532IO : public BaseCardIOInterface {
	PN532 &CardReader;
	uint8_t *UID;
	uint8_t UIDSize;
	
public:
	PN532IO(PN532 & CardReader, uint8_t *UID, uint8_t UIDSize) : 
		CardReader(CardReader),
		UID(UID),
		UIDSize(UIDSize) {};

private:
	bool ReadBytesHandler(uint8_t Sector, uint8_t Block, uint8_t Offset, void * Data, size_t Count) {
		uint8_t CurrentBlock = 4 * Sector + Block;

		bool CanRead = CardReader.mifareclassic_AuthenticateBlock(UID, UIDSize, CurrentBlock, 0, KeyA);
		if (!CanRead)
			return false;

		uint8_t BlockData[16];

		bool Success = CardReader.mifareclassic_ReadDataBlock(CurrentBlock, BlockData);
		if (!Success)
			return false;

		memcpy(Data, BlockData + Offset, Count);
		return true;
	}

	bool WriteBytesHandler(uint8_t Sector, uint8_t Block, uint8_t Offset, void * Data, size_t Count) {
		uint8_t CurrentBlock = 4 * Sector + Block;

		uint8_t BlockData[16];
		bool GotOldData = ReadBytesHandler(Sector, Block, 0, &BlockData, sizeof(BlockData));
		if (!GotOldData)
			return false;

		memcpy(BlockData + Offset, Data, Count);

		bool CanWrite = CardReader.mifareclassic_AuthenticateBlock(UID, UIDSize, CurrentBlock, 1, KeyB);
		if (!CanWrite)
			return false;

		bool Success = CardReader.mifareclassic_WriteDataBlock(CurrentBlock, BlockData);
		if (!Success)
			return false;

		return true;
	}

};
