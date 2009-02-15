#ifndef OPTIONSVIEW_H
#define OPTIONSVIEW_H

class MainWindow;

namespace Ui {
	class OptionsView;
}

class OptionsView : public QWidget, public Singleton<OptionsView>
{
	Q_OBJECT

public:
	OptionsView(MainWindow *mainWindow);
	virtual ~OptionsView();

	void readSettings(QSettings *settings);
	void writeSettings(QSettings *settings);

	uint16 getProxyListenPort() const;
	uint16 getServerPort() const;
	QString getServerHost() const;
	QString getMoviesDir() const;

private slots:
	void defaultThemeButtonClicked();
	void defaultLanguageButtonClicked();
	void changeFontButtonClicked();
	void browseButtonClicked();
	void tibiaServerChanged();

private:
	void listThemes();
	void listLanguages();

	MainWindow *mMainWindow;
	Ui::OptionsView *mUi;

	QString mLanguage;
};

#endif // OPTIONSVIEW_H
