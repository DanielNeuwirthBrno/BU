/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef UI_BANKWINDOW_H
#define UI_BANKWINDOW_H

// user interface for BankWindow class

#include <QDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QIcon>
#include <QPair>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVector>

class Ui_BankWindow {

    public:
        QIcon * bankIcon;

        QTableWidget * bankListTable;
        QHeaderView * headerH;
        QHeaderView * headerV;

        QHBoxLayout * buttonsLayout;
        QPushButton * selectAccountFormatButton;
        QPushButton * selectButton;
        QPushButton * cancelButton;

        QVBoxLayout * windowLayout;

        void setupUi(const QVector<QPair<QString, QString>> & listOfBanks,
                     const QPair<bool, bool> formatButtonState,
                     const QString & bankCodeFromConfig, QDialog * BankWindow) {

            // properties of main window
            bankIcon = new QIcon(QStringLiteral(":/icons/icons/wallet-open.png"));
            BankWindow->setWindowIcon(*bankIcon);
            BankWindow->setWindowTitle(QStringLiteral("Seznam bank"));
            BankWindow->resize(600,400);

            // table
            uint16_t row = 0;
            bankListTable = new QTableWidget(listOfBanks.size(), 2, BankWindow);
            const QStringList headers = { QStringLiteral("Název"), QStringLiteral("Kód") };
            bankListTable->setHorizontalHeaderLabels(headers);
            bankListTable->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
            QTableWidgetItem * currentItem = new QTableWidgetItem();

            for (auto it: listOfBanks) {

                QTableWidgetItem * leftColumnItem = new QTableWidgetItem(it.first);
                leftColumnItem->setFlags(leftColumnItem->flags() & ~Qt::ItemIsEditable);
                QTableWidgetItem * rightColumnItem = new QTableWidgetItem(it.second);
                rightColumnItem->setFlags(rightColumnItem->flags() & ~Qt::ItemIsEditable);

                if (it.second == bankCodeFromConfig) {

                    QFont * boldText = new QFont();
                    boldText->setBold(true);
                    leftColumnItem->setFont(*boldText);
                    rightColumnItem->setFont(*boldText);
                    currentItem = leftColumnItem;
                }

                bankListTable->setItem(row, 0, leftColumnItem);
                bankListTable->setItem(row++, 1, rightColumnItem);

            }
            bankListTable->setCurrentItem(currentItem);
            bankListTable->resizeColumnToContents(1);
            headerH = bankListTable->horizontalHeader();
            headerH->setSectionResizeMode(0, QHeaderView::Stretch);
            headerV = bankListTable->verticalHeader();
            headerV->setDefaultSectionSize(headerV->minimumSectionSize());

            // buttons
            buttonsLayout = new QHBoxLayout();
            selectButton = new QPushButton(QStringLiteral("OK"));
            selectButton->setDefault(true);
            cancelButton = new QPushButton(QStringLiteral("Zpět"));

            if (formatButtonState.first) {

                const QString labelOfFormatButton = (formatButtonState.second) ?
                    QStringLiteral("vnitřní formát") : QStringLiteral("ediční formát");
                selectAccountFormatButton = new QPushButton(labelOfFormatButton);
                buttonsLayout->addWidget(selectAccountFormatButton, 2);
                buttonsLayout->addStretch(4);
            }
            else
                buttonsLayout->addStretch(6);

            buttonsLayout->addWidget(selectButton, 1);
            buttonsLayout->addWidget(cancelButton, 1);

            windowLayout = new QVBoxLayout(BankWindow);
            windowLayout->addWidget(bankListTable);
            windowLayout->addLayout(buttonsLayout);

            QMetaObject::connectSlotsByName(BankWindow);
        }
};

#endif // UI_BANKWINDOW_H
