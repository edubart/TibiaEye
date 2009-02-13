#include "headers.h"
#include "fancytabwidget.h"

FancyTabWidget::FancyTabWidget(QWidget *parent)
	: QWidget(parent)
{
	mSplitIndex = -1;

	mButtonsWidget = new QWidget(this);
	mButtonsWidget->setObjectName("fancyTabButtonsWidget");

	mTopButtonsLayout = new QVBoxLayout;
	mTopButtonsLayout->setSpacing(0);
	mTopButtonsLayout->setMargin(0);

	mBottomButtonsLayout = new QVBoxLayout;
	mBottomButtonsLayout->setSpacing(0);
	mBottomButtonsLayout->setMargin(0);

	QVBoxLayout *buttonsLayout = new QVBoxLayout;
	buttonsLayout->setSpacing(0);
	buttonsLayout->setMargin(0);
	buttonsLayout->addLayout(mTopButtonsLayout);
	buttonsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	buttonsLayout->addLayout(mBottomButtonsLayout);
	mButtonsWidget->setLayout(buttonsLayout);

	mPagesWidget = new QStackedWidget(this);
	mPagesWidget->setObjectName("fancyTabPagesWidget");

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->addWidget(mButtonsWidget);
	mainLayout->addWidget(mPagesWidget,1);
	setLayout(mainLayout);

	mButtons = new QButtonGroup(mButtonsWidget);
	mButtons->setExclusive(true);
	
	connect(mButtons, SIGNAL(buttonClicked(int)), this, SLOT(changePage(int)));
}

void FancyTabWidget::insertTab(int index, QWidget *tab, const QString &label)
{
	QPushButton *button = new QPushButton(label, mButtonsWidget);
	button->setCheckable(true);
	button->setFocusPolicy(Qt::NoFocus);
	button->setProperty("tabIndex",index);

	mPagesWidget->insertWidget(index, tab);
	mButtons->addButton(button, index);

	if(index > mSplitIndex)
		mBottomButtonsLayout->insertWidget(index, button);
	else
		mTopButtonsLayout->insertWidget(index, button);
	
	if(mButtons->buttons().size() == 1)
		button->setChecked(true);
}

void FancyTabWidget::setLocked(int index, bool locked)
{
	if(index == mPagesWidget->currentIndex() && locked)
		return;
	mButtons->buttons().at(index)->setEnabled(!locked);
}

void FancyTabWidget::setSplitIndex(int index)
{
	mSplitIndex = index;
}

void FancyTabWidget::changePage(int index)
{
	mPagesWidget->setCurrentIndex(index);
}
