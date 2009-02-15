#include "headers.h"
#include "clientproxy.h"
#include "protocollogin.h"
#include "protocolgame.h"
#include "tibiasocket.h"

ClientProxy::ClientProxy() : QTcpServer()
{
	qDebug("ClientProxy::ClientProxy");

	mClientSocket = NULL;
	mListenPort = 0;
}

ClientProxy::~ClientProxy()
{
	qDebug("ClientProxy::~ClientProxy");
}

bool ClientProxy::startListen()
{
	qDebug("ClientProxy::startListen");

	close();
	if(!listen(QHostAddress::LocalHost, mListenPort)) {
		qCritical() << qPrintable(tr("Unable to start listen socket."));
		return false;
	}

	mListenPort = serverPort();
	return true;
}

void ClientProxy::stopListen()
{
	qDebug("ClientProxy::stopListen");

	close();
}


bool ClientProxy::isConnected()
{
	qDebug("ClientProxy::isConnected");
	return (mClientSocket && mClientSocket->isValid());
}

void ClientProxy::disconnect()
{
	qDebug("ClientProxy::disconnect");

	if(mClientSocket)
		mClientSocket->disconnectLater();
}

void ClientProxy::sendMessage(NetworkMessage &msg)
{
	qDebug("ClientProxy::sendMessage");

	if(mClientSocket)
		mClientSocket->sendMessage(msg);
}

void ClientProxy::incomingConnection(int socketDescriptor)
{
	qDebug("ClientProxy::incomingConnection");

	if(mClientSocket) {
		qWarning() << "[ClientProxy::onClientConnect] Incoming client connection refused because we already have one client connected.";
	} else {
		qDebug("ClientProxy::incomingConnection BEGIN NEW SESSION");

		mClientSocket = new TibiaSocket(this);
		mClientSocket->setSocketDescriptor(socketDescriptor);

		connect(mClientSocket, SIGNAL(messageRecived(NetworkMessage&)), this, SLOT(onClientMessage(NetworkMessage&)));
		connect(mClientSocket, SIGNAL(disconnected()), this, SLOT(onClientDisconnect()));
	}
}

void ClientProxy::onClientMessage(NetworkMessage &msg)
{
	qDebug("ClientProxy::onClientMessage");

	Protocol *protocol = NULL;

	switch(msg.getByte()) {
		case 0x01:
			protocol = new ProtocolLogin(this);
			break;
		case 0x0A:
			protocol = new ProtocolGame(this);
			break;
		default:
			qWarning() << "[ClientProxy::onClientMessage] Invalid protocol.";
			disconnect();
			break;
	}

	if(protocol) {
		connect(mClientSocket, SIGNAL(messageRecived(NetworkMessage&)), protocol, SLOT(onRecvClientMessage(NetworkMessage&)));
		connect(mClientSocket, SIGNAL(disconnected()), protocol, SLOT(onClientDisconnect()));
		QObject::disconnect(mClientSocket, SIGNAL(messageRecived(NetworkMessage&)), this, SLOT(onClientMessage(NetworkMessage&)));
		protocol->onRecvFirstClientMessage(msg);
	}
}

void ClientProxy::onClientDisconnect()
{
	qDebug("ClientProxy::onClientDisconnect");

	mClientSocket->deleteLater();
	mClientSocket = NULL;

	emit clientDisconnected();
}
