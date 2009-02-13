#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

class FancyTabWidget : public QWidget
{
	Q_OBJECT

public:
	FancyTabWidget(QWidget *parent = 0);

	void insertTab(int index, QWidget *tab, const QString &label);

	void setLocked(int index, bool locked);
	void setSplitIndex(int index);

private slots:
	void changePage(int index);

private:
	QButtonGroup *mButtons;
	QWidget *mButtonsWidget;
	QVBoxLayout *mTopButtonsLayout;
	QVBoxLayout *mBottomButtonsLayout;
	QStackedWidget *mPagesWidget;

	int mSplitIndex;
};

#endif // FANCYTABWIDGET_H
