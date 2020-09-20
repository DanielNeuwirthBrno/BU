/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef BANKWINDOW_H
#define BANKWINDOW_H

#include <QMessageBox>
#include <QPair>
#include <QString>
#include <QVector>
#include <QWidget>
#include "ui/ui_bankwindow.h"

class BankWindow: public QDialog {

    Q_OBJECT

    public:
        explicit BankWindow();
        explicit BankWindow(const QVector<QPair<QString, QString>> &, const QPair<bool, bool>,
                            const QString & = QString(), QWidget * = nullptr);
        ~BankWindow() { delete ui; }

    private:
        const bool accountFormatSwitch;
        Ui_BankWindow * ui;

    private slots:
        void returnValue();
        void changeAccountFormat() const;
        void sortSelectedColumn(const int) const;
};

class AccountFormatDialog: public QMessageBox {

    Q_OBJECT

    public:
        explicit AccountFormatDialog();
        explicit AccountFormatDialog(bool &);
        ~AccountFormatDialog() {}
};

#endif // BANKWINDOW_H
