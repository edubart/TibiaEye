#include "headers.h"
#include "servervirtual.h"
#include "protocol.h"
#include "moviefile.h"

ServerVirtual::ServerVirtual(Protocol *protocol, MovieFile *movieFile)
	: AbstractServerProxy(protocol)
{
	qDebug("ServerVirtual::ServerVirtual");

	mProtocol = protocol;
	mMovieFile = movieFile;
	mCurrentPacket = NULL;
	mNextPacket = NULL;
	mState = VSTATE_IDLE;
}

ServerVirtual::~ServerVirtual()
{
	qDebug("ServerVirtual::~ServerVirtual");
}

void ServerVirtual::start()
{
	qDebug("ServerVirtual::start");

	mState = VSTATE_START;
	QTimer::singleShot(0, this, SLOT(onTimer()));
}

void ServerVirtual::disconnect()
{
	qDebug("ServerVirtual::disconnect");

	mState = VSTATE_STOP;
}

void ServerVirtual::onTimer()
{
	qDebug("ServerVirtual::onTimer");

	if(mState == VSTATE_STOP) {
		mProtocol->onServerDisconnect();
		mState = VSTATE_FINISHED;
		return;
	}

	int shotTime = -1;

	if(mMovieFile->getSpeed() > 0) {
		if(mState == VSTATE_START) {
			mProtocol->onServerConnect();
			mState = VSTATE_PLAYING;
			shotTime = 0;
		} else {
			shotTime = mMovieFile->playMessage(mReadMsg);
			if(shotTime >= 0) {
				mProtocol->onRecvServerMessage(mReadMsg, false);
			} else {
				mProtocol->onServerDisconnect();
				mState = VSTATE_FINISHED;
			}
		}
	} else {
		shotTime = 300;
	}

	if(shotTime >= 0)
		QTimer::singleShot(shotTime, this, SLOT(onTimer()));
}
