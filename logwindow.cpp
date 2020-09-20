/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#include <QPushButton>
#include <QString>
// #include <QVector>
// #include <QWidget>
#include "logwindow.h"

LogWindow::LogWindow(const QVector<LogDB> & inLog, const bool onlyErrors, QWidget * parent):
    QDialog(parent), log(inLog), ui(new Ui_LogWindow) {

    ui->setupUi(this, convertLogForDisplay(onlyErrors));

    connect(ui->displayOnlyErrorsButton, &QPushButton::toggled, this, &LogWindow::refillLogDisplayArea);
    connect(ui->closeButton, &QPushButton::clicked, this, &LogWindow::close);
}

QString LogWindow::convertLogForDisplay(const bool onlyErrors) const {

    QString parsedLog;
    uint16_t lineNo = 1;
    for (auto it: log)
        parsedLog += it.requestAndErrorDescription(lineNo++, onlyErrors);

    return parsedLog;
}

// [slot]
void LogWindow::refillLogDisplayArea() const {

    ui->logDisplayArea->setHtml(convertLogForDisplay(ui->displayOnlyErrorsButton->isChecked()));
    return;
}
