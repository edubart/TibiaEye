#ifndef PROTOCOL_H
#define PROTOCOL_H

class RSA;
class ClientProxy;
class AbstractServerProxy;
class NetworkMessage;

//TODO: integrate with TibiaSocket
class Protocol : public QObject
{
	Q_OBJECT

public:
	Protocol(ClientProxy *client);
	virtual ~Protocol();

signals:
	void serverConnected();
	void serverDisconnected();
	void clientDisconnected();
	void firstClientMessage(NetworkMessage&);
	void clientMessage(NetworkMessage&);
	void serverMessage(NetworkMessage&);

public slots:
	void onServerConnect();
	void onServerDisconnect();
	void onClientDisconnect();

	void onRecvFirstClientMessage(NetworkMessage& msg);
	void onRecvClientMessage(NetworkMessage& msg);
	void onRecvServerMessage(NetworkMessage& msg, bool decrypt = true);

protected slots:
	void sendClientMessage(NetworkMessage &msg);
	void sendServerMessage(NetworkMessage &msg);

	void disconnectClient();
	void disconnectServer();

	void enableXTEAEncryption() { mEncryptionEnabled = true; }
	void disableXTEAEncryption() { mEncryptionEnabled = false; }

protected:
	void setServer(AbstractServerProxy *server) { mUse++; mServer = server; }
	const AbstractServerProxy *server() const { return mServer; }
	const ClientProxy *client() const { return mClient; }



	void setXTEAKey(const uint32* key) {
		memcpy(mKey, key, sizeof(uint32)*4);
	}

	void XTEA_encrypt(NetworkMessage& msg);
	bool XTEA_decrypt(NetworkMessage& msg);
	bool RSA_decrypt(RSA* rsa, NetworkMessage& msg);
	bool RSA_encrypt(RSA* rsa, NetworkMessage& msg);

private:
	void decrenseUse();

	void onSendMessage(NetworkMessage& msg);
	bool onRecvMessage(NetworkMessage& msg, bool decrypt = true);

	ClientProxy *mClient;
	AbstractServerProxy *mServer;

	bool mEncryptionEnabled;
	uint32 mKey[4];
	int mUse;
};

#endif // PROTOCOL_H
