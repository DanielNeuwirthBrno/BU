/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef UI_LOGWINDOW_H
#define UI_LOGWINDOW_H

// user interface for LogWindow class

#include <QDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>

class Ui_LogWindow {

    public:
        QIcon * logIcon;
        QTextEdit * logDisplayArea;

        QHBoxLayout * buttonsLayout;
        QPushButton * displayOnlyErrorsButton;
        QPushButton * closeButton;

        QVBoxLayout * windowLayout;

        void setupUi(QDialog * LogWindow, const QString & inLog) {

            // properties of main window
            logIcon = new QIcon(QStringLiteral(":/icons/icons/system-log-out.png"));
            LogWindow->setWindowIcon(*logIcon);
            LogWindow->setWindowTitle(QStringLiteral("Log: dotazy na databázi"));
            LogWindow->resize(800,400);

            logDisplayArea = new QTextEdit(inLog);

            // buttons
            buttonsLayout = new QHBoxLayout();
            displayOnlyErrorsButton = new QPushButton(QStringLiteral("Pouze chyby"));
            displayOnlyErrorsButton->setCheckable(true);
            closeButton = new QPushButton(QStringLiteral("Zavřít"));
            buttonsLayout->addStretch();
            buttonsLayout->addWidget(displayOnlyErrorsButton);
            buttonsLayout->addWidget(closeButton);

            windowLayout = new QVBoxLayout(LogWindow);
            windowLayout->addWidget(logDisplayArea);
            windowLayout->addLayout(buttonsLayout);

            QMetaObject::connectSlotsByName(LogWindow);
    }
};

#endif // UI_LOGWINDOW_H
