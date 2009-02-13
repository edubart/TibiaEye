#include "headers.h"
#include "modemanager.h"
#include "optionsview.h"
#include "memoryinjection.h"
#include "clientproxy.h"
#include "moviefile.h"

//TODO: better mode and stop system
ModeManager::ModeManager()
{
	qDebug("ModeManager::ModeManager");

	mClientProxy = NULL;
	mMovieFile = NULL;
	mMode = MODE_NONE;
}

ModeManager::~ModeManager()
{
	qDebug("ModeManager::~ModeManager");

	if(mClientProxy)
		delete mClientProxy;

	qDeleteAll(mCharList);
}

bool ModeManager::startMode(eModes mode)
{
	qDebug("ModeManager::startMode");

	if(isModeRunning() || mode == MODE_NONE) {
		qCritical() << qPrintable(tr("A mode is already running."));
		return false;
	}

	uint16 port = OptionsView::instance()->getProxyListenPort();

	MemoryInjection *memoryInjection = MemoryInjection::instance();
	if(!memoryInjection->injectTibia() || !memoryInjection->setServerHost("127.0.0.1", port)) {
		qCritical() << qPrintable(tr("Unable to find a supported Tibia client running."));
		return false;
	}

	if(!mClientProxy)
		mClientProxy = new ClientProxy();
	mClientProxy->setListenPort(port);
	
	if(mClientProxy->startListen()) {
		mMode = mode;
		return true;
	}

	return false;
}

void ModeManager::stopMode()
{
	qDebug("ModeManager::stopMode");

	if(!isModeRunning())
		return;

	MemoryInjection::instance()->desinjectTibia();
	mClientProxy->disconnect();
	mClientProxy->stopListen();

	mMode = MODE_NONE;
}
