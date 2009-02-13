#ifndef NETWORKMESSAGE_H
#define NETWORKMESSAGE_H

#include "consttibia.h"

class NetworkMessage
{
public:
	NetworkMessage();
	virtual ~NetworkMessage();

	void reset();

	uint8 getByte();
	uint16 getU16();
	uint32 getU32();
	uint16 getBytes(uint8 *buffer, uint16 size);
	QString getString();
	void skipBytes(int count) { mReadPos += count; }

	void addByte(uint8 value);
	void addU16(uint16 value);
	void addU32(uint32 value);
	void addBytes(const uint8 *bytes, uint16 size);
	void addString(const char *value);
	void addString(const QString &value) { addString(value.toAscii().constData()); }
	void addPaddingBytes(uint16 n);

	uint16 getMessageLength() const { return mMsgSize; }
	void setMessageLength(uint16 newSize) { mMsgSize = newSize; }
	void writeMessageLength() { *(uint16*)mMsgBuf = mMsgSize + 4; }

	uint16 getReadPos() const { return mReadPos; }
	void setReadPos(uint16 pos) { mReadPos = pos; }
	void resetReadPos() { mReadPos = 6; }

	uint8 *getBuffer() { return mMsgBuf; }
	uint8 *getBodyBuffer() { return mMsgBuf + 6; }

	bool end() const { return !(mReadPos < mMsgSize + 6); }

	bool checkSum();
	void writeSum();

private:
	bool canAdd(uint16 size);
	bool canGet(uint16 size);

	uint16 mMsgSize;
	uint16 mReadPos;
	uint8 mMsgBuf[NETWORKMESSAGE_MAXSIZE];
};

#endif // NETWORKMESSAGE_H
