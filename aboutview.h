#ifndef ABOUTVIEW_H
#define ABOUTVIEW_H

namespace Ui {
	class AboutView;
}

class AboutView : public QWidget
{
	Q_OBJECT

public:
	AboutView(QWidget *parent);
	virtual ~AboutView();

private:
	Ui::AboutView *mUi;
};

#endif // ABOUTVIEW_H
