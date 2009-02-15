#include "headers.h"
#include "protocolgame.h"
#include "modemanager.h"

ProtocolGame::ProtocolGame(ClientProxy *client)
	: Protocol(client)
{
	qDebug("ProtocolGame::ProtocolGame");

	mModeManager = ModeManager::instance();

	if(mModeManager->getMode() & MODE_PLAY)
		connect(this, SIGNAL(firstClientMessage(NetworkMessage&)), this, SLOT(parsePlayFirstClientMessage(NetworkMessage&)));
	else if(mModeManager->getMode() & MODE_RECORD)
		connect(this, SIGNAL(firstClientMessage(NetworkMessage&)), this, SLOT(parseRecordFirstClientMessage(NetworkMessage&)));

}

ProtocolGame::~ProtocolGame()
{
	qDebug("ProtocolGame::~ProtocolGame");
}

void ProtocolGame::disconnectWithError(const QString &message)
{
	qDebug("ProtocolGame::disconnectWithError");

	mMsg.reset();
	mMsg.addByte(0x14);
	mMsg.addString(message);
	sendClientMessage(mMsg);
	disconnectClient();
}
