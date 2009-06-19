#include "headers.h"
#include "mainwindow.h"
#include "constants.h"

#ifdef Q_WS_X11
#define LEAKDETECTOR_NO_MEMORY_WATCH
#include <leakdetector.h>
#endif

void messageHandler(QtMsgType type, const char *msg)
{
	switch(type) {
#ifdef __DEBUG__
		case QtDebugMsg:
			fprintf(stderr, "Debug: %s\n", msg);
			break;
#endif
		case QtWarningMsg:
			fprintf(stderr, "Warning: %s\n", msg);
			break;

		case QtCriticalMsg:
			QMessageBox::critical(QApplication::activeWindow(), QTranslator::tr("Error"), msg);
			fprintf(stderr, "Critical: %s\n", msg);
			break;

		case QtFatalMsg:
			QMessageBox::critical(QApplication::activeWindow(), QTranslator::tr("Fatal Error"), msg);
			fprintf(stderr, "Fatal: %s\n", msg);
			abort();
			break;

		default:
			break;
	}
}

//TODO: move needed files (default theme) to qrc
//TODO: movie association (read for movie files in argc)
//TODO: one TibiaEye instance allowed (maybe using QtSingleApplication)
int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(tibiaeye);

	// setup message handler
	qInstallMsgHandler(messageHandler);

	// init application
	QApplication app(argc, argv);

	// first create splash screen
	QPixmap pixmap(":/images/tibiaeye_splash.png");
	QSplashScreen splash(pixmap);
	splash.show();

	// set cursor to busy
	app.setOverrideCursor(Qt::BusyCursor);

	// set application default settings
	QCoreApplication::setApplicationName("Tibia Eye");
	QCoreApplication::setApplicationVersion(Constants::APP_VERSION_LONG);
	QCoreApplication::setOrganizationName("Tibia Eye");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	// load settings
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Tibia Eye","Tibia Eye");

	// detects language to be used
	QString locale = QLocale::system().name();
	QVariant language = settings.value("Language");
	if(language.isValid())
		locale = language.toString();

	// install translator
	QTranslator translator;
	translator.load(QCoreApplication::applicationDirPath() + "/translations/" + locale + ".qm");
	app.installTranslator(&translator);

	// set app icon
	qApp->setWindowIcon(QIcon(":/images/tibiaeye_logo_32x32.png"));

#ifndef __DEBUG__
	// sleep
	QTime dieTime = QTime::currentTime().addSecs(2);
	while(QTime::currentTime() < dieTime)
		app.processEvents(QEventLoop::AllEvents, 100);
#endif
	
	// create main window
	MainWindow mainWindow(&settings);

	// restore cursor
	app.restoreOverrideCursor();

	// finish splash and show the main window
	splash.finish(&mainWindow);
	mainWindow.show();

	// run app
	return app.exec();
}
