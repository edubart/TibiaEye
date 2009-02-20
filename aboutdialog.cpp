#include "headers.h"
#include "aboutdialog.h"
#include "constants.h"

using namespace Constants;

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent)
{
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setWindowTitle(tr("About Tibia Eye"));

	QLabel *imageLabel = new QLabel;
	imageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	imageLabel->setScaledContents(true);
	imageLabel->setPixmap(QPixmap(":/images/tibiaeye_logo_128x128.png"));

	QVBoxLayout *imageLayout = new QVBoxLayout;
	imageLayout->addWidget(imageLabel);
	imageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	const QString aboutText = QString("<h3>%1 %2</h3>"
								 "Built on %3<br/>"
#ifdef APP_REVISION
								 "Revision: %9<br/>"
#endif
								 "Author: %4<br/>"
								 "Contact: %5<br/>"
								 "Website: <a href=\"%6\">%6</a><br/>"
								 "<br/>"
								 "Copyright %7 %8. All rights reserved.<br/>"
//								 "<br/>"
//								 "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING"
//								 "THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A"
//								 "PARTICULAR PURPOSE."
								 ).arg(APP_NAME, APP_VERSION_LONG, APP_BUILD_LONG, APP_AUTHOR,
									   APP_CONTACT, APP_WEBSITE, APP_YEAR, APP_COMPANY
#ifdef APP_REVISION
									   , APP_REVISION_STR
#endif
								);

	QLabel *aboutLabel = new QLabel(aboutText);
	aboutLabel->setWordWrap(true);
	aboutLabel->setOpenExternalLinks(true);
	aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

	QHBoxLayout *aboutLayout = new QHBoxLayout;
	aboutLayout->addLayout(imageLayout);
	aboutLayout->addWidget(aboutLabel);

	QPushButton *checkUpdatesButton = new QPushButton(tr("Check Updates"));
	checkUpdatesButton->setEnabled(false);

	QPushButton *closeButton = new QPushButton(tr("Close"));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(closeButton);
	buttonsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	buttonsLayout->addWidget(checkUpdatesButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addLayout(aboutLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	closeButton->setFocus();
}
