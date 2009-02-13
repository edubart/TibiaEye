#include "headers.h"
#include "tibiasocket.h"

NetworkMessage TibiaSocket::mMsg;

TibiaSocket::TibiaSocket(QObject *parent)
	: QTcpSocket(parent)
{
	connect(this, SIGNAL(readyRead()), this, SLOT(internalRead()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));
}

TibiaSocket::~TibiaSocket()
{

}

void TibiaSocket::sendMessage(NetworkMessage &msg)
{
	int64 size = msg.getMessageLength() + 6;
	int64 wrote = write((const char*)msg.getBuffer(), size);

	if(wrote <= -1) {
		qWarning() << "[TibiaSocket::sendMessage] Write error.";
		disconnectLater();
	}

	if(wrote != size) {
		qWarning() << "[TibiaSocket::sendMessage] Unexpected writing error: not all bytes sent.";
		disconnectLater();
	}

	if(!flush())
		qWarning() << "[TibiaSocket::sendMessage] Write buffer flush failed.";
}

void TibiaSocket::internalRead()
{
	uint16 size;
	int64 read;

	forever {
		if(bytesAvailable() < 6)
			break;

		if((read = peek((char *)&size, 2)) == -1) {
			qWarning() << "[TibiaSocket::internalRead] Peek error.";
			disconnectLater();
			break;
		}

		if(size < 4 || size + 2 > NETWORKMESSAGE_MAXSIZE) {
			qWarning() << "[TibiaSocket::internalRead]: Invalid packet size.";
			disconnectLater();
			break;
		}

		if(bytesAvailable() < size + 2)
			break;

		if((read = this->read((char *)mMsg.getBuffer(), size + 2)) == -1) {
			qWarning() << "[TibiaSocket::internalRead] Read error.";
			disconnectLater();
			break;
		}

		mMsg.reset();
		mMsg.setMessageLength(size - 4);
		if(!mMsg.checkSum()) {
			qWarning() << "TibiaSocket::internalRead]: Incorret checksum.";
			disconnectLater();
			break;
		}

		emit messageRecived(mMsg);
	}
}

void TibiaSocket::handleError(QAbstractSocket::SocketError error)
{
	if(error == QAbstractSocket::ConnectionRefusedError ||
	   error == QAbstractSocket::HostNotFoundError) {
		emit disconnected();
	} else if(error != QAbstractSocket::RemoteHostClosedError)
		qWarning() << "[TibiaSocket::handleError] Socket error code " << error << ": " << errorString();
}

void TibiaSocket::disconnectLater()
{
	QTimer::singleShot(0, this, SLOT(disconnect()));
}

void TibiaSocket::disconnect()
{
	disconnectFromHost();
}
