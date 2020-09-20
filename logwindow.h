/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QString>
#include <QVector>
#include <QWidget>
#include "database.h"
#include "ui/ui_logwindow.h"

class LogWindow: public QDialog {

    Q_OBJECT

    public:
        explicit LogWindow(const QVector<LogDB> &, const bool, QWidget * = nullptr);
        ~LogWindow() { delete ui; }

    private:
        QString convertLogForDisplay(const bool = false) const;

        const QVector<LogDB> & log;
        Ui_LogWindow * ui;

    private slots:
        void refillLogDisplayArea() const;
};

#endif // LOGWINDOW_H
