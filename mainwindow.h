#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class FancyTabWidget;

enum eViewIndex {
	VIEW_WATCH_MOVIES = 0,
	VIEW_RECORD_MOVIES,
	/*
	VIEW_WATCH_STREAMS,
	VIEW_STREAM_MOVIES,
	VIEW_DOWNLOAD_MOVIES,
	VIEW_TIBIAEYE_ACCOUNT,
	*/
	VIEW_OPTIONS,
	VIEW_ABOUT
};

class OptionsView;
class ModeManager;
class MemoryInjection;
class RSA;

class MainWindow : public QMainWindow, public Singleton<MainWindow>
{
    Q_OBJECT

public:
	MainWindow(QSettings *settings, QWidget *parent = 0);
	virtual ~MainWindow();

	bool setTheme(const QString &theme);
	const QString &theme() { return mTheme; }

protected:
	void closeEvent(QCloseEvent *event);

public slots:
	void about();
	void exit();

private:
	void setupSearchPaths();
	void createDefaultViews();

	void readSettings();
	void writeSettings();

	QString mTheme;

	QSettings *mSettings;
	OptionsView *mOptions;
	FancyTabWidget *mFancyTab;
	ModeManager *mModeManager;
	MemoryInjection *mMemoryInjection;
	RSA *mRSA;
};

#endif // MAINWINDOW_H
