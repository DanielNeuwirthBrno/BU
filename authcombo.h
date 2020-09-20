/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef AUTHCOMBO_H
#define AUTHCOMBO_H

// AuthComboBoxWidget class (custom QComboBox widget)

#include <QComboBox>
#include <QStringList>
#include <QWidget>

class AuthComboBoxWidget: public QComboBox {

    Q_OBJECT

    public:
        const QStringList Authentication {
            { QStringLiteral("Windows Authentication") },
            { QStringLiteral("SQL Server Authentication") }
        };

        explicit AuthComboBoxWidget(QWidget * parent = nullptr):
            QComboBox(parent) {}
        ~AuthComboBoxWidget() {}
};

#endif // AUTHCOMBO_H
