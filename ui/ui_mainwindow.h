/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

// user interface for MainWindow class

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputMethodEvent>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QTextCharFormat>
#include <QVBoxLayout>
#include <QVector>
#include "authcombo.h"
#include "bank.h"
#include "iconlabel.h"

class Ui_MainWindow {

    public:
        QIcon * mainIcon;
        QGroupBox * databaseGroupBox;
        QGridLayout * databaseGroupLayout;

        QLabel * serverName;
        QLineEdit * serverNameInput;
        AuthComboBoxWidget * authenticationType;
        IconAndMouseLabelWidget * authenticationResult;

        QLabel * userName;
        QLineEdit * userNameInput;
        QLabel * password;
        QLineEdit * passwordInput;

        QLabel * agendaName;
        QComboBox * agendaNameInput;
        IconAndMouseLabelWidget * connectToAgendaResult;

        QGroupBox * statementGroupBox;
        QVBoxLayout * statementGroupLayout;

        QHBoxLayout * filePathLayout;
        QLabel * filePath;
        QLineEdit * filePathInput;
        QPushButton * openFileDialogButton;

        QHBoxLayout * homebankingLayout;
        QLabel * homebanking;
        QLineEdit * homebankingType;
        QComboBox * homebankingInput;

        QGroupBox * parsingFileGroupBox;
        QGridLayout * parsingFileGroupLayout;
        QLabel * fileNotCorruptedLabel;
        IconLabelWidget * fileNotCorruptedResult;
        QPixmap * iconNotCorruptedResult;
        QLabel * fileProcessedLabel;
        IconLabelWidget * fileProcessedResult;
        QPixmap * iconProcessedResult;
        QLineEdit * bankAccountNumber;
        QLineEdit * nameOfBank;

        QGroupBox * compareToMoneyGroupBox;
        QGridLayout * compareToMoneyGroupLayout;
        QVector<IconLabelWidget *> iconReadFromDB;
        QVector<IconLabelWidget *> iconWriteToDB;
        // bank connection
        QLabel * bankConnectionLabel;
        QLineEdit * bankConnectionID;
        QCheckBox * editBankConnection;
        IconAndMouseLabelWidget * readBankConnectionResult;
        QPixmap * iconBankConnectionReadResult;
        IconAndMouseLabelWidget * writeBankConnectionResult;
        QPixmap * iconBankConnectionWriteResult;
        // bank account
        QLabel * bankAccountLabel;
        QLineEdit * bankAccountID;
        QCheckBox * editBankAccount;
        IconAndMouseLabelWidget * readBankAccountResult;
        QPixmap * iconBankAccountReadResult;
        IconAndMouseLabelWidget * writeBankAccountResult;
        QPixmap * iconBankAccountWriteResult;
        // bank statements group
        QLabel * statementsGroupLabel;
        QComboBox * statementsGroupInput;
        QCheckBox * editStatementsGroup;
        IconAndMouseLabelWidget * readStatementsGroupResult;
        QPixmap * iconStatementsGroupReadResult;
        IconAndMouseLabelWidget * writeStatementsGroupResult;
        QPixmap * iconStatementsGroupWriteResult;
        // homebanking
        QLabel * homebankingLabel;
        QLineEdit * homebankingDirectory;
        QCheckBox * editHomebanking;
        IconAndMouseLabelWidget * readHomebankingResult;
        IconAndMouseLabelWidget * writeHomebankingResult;

        QFrame * horizontalLine;
        QHBoxLayout * buttonsLayout;
        QPushButton * profileButton;
        QPushButton * passAllButton;
        QPushButton * passSelectedButton;
        QPushButton * clearButton;
        QPushButton * logButton;
        QPushButton * quitButton;

        QVBoxLayout * windowLayout;

        void setupUi(QDialog * MainWindow, const uint8_t rows) {

            // properties of main window
            mainIcon = new QIcon(QStringLiteral(":/icons/icons/view-bank-account.png"));
            MainWindow->setWindowIcon(*mainIcon);
            MainWindow->setWindowFlags(MainWindow->windowFlags() | Qt::WindowMinimizeButtonHint);
            MainWindow->setWindowTitle(QStringLiteral("Výpis z účtu"));
            MainWindow->resize(600,0);

            // database settings
            databaseGroupBox = new QGroupBox(QStringLiteral("Databáze"), MainWindow);
            databaseGroupLayout = new QGridLayout;
            // server and authentication
            serverName = new QLabel(QStringLiteral("SQL server"));
            serverNameInput = new QLineEdit;
            authenticationType = new AuthComboBoxWidget;
            authenticationType->addItems(authenticationType->Authentication);
            authenticationResult = new IconAndMouseLabelWidget;
            databaseGroupLayout->addWidget(serverName,0,0);
            databaseGroupLayout->addWidget(serverNameInput,0,1);
            databaseGroupLayout->addWidget(authenticationType,0,2,1,4);
            databaseGroupLayout->addWidget(authenticationResult,0,6);

            // username and password
            userName = new QLabel(QStringLiteral("Uživatelské jméno"));
            userName->setHidden(true);
            userNameInput = new QLineEdit;
            userNameInput->setHidden(true);
            password =  new QLabel(QStringLiteral("Heslo"));
            password->setHidden(true);
            passwordInput = new QLineEdit;
            passwordInput->setHidden(true);
            databaseGroupLayout->addWidget(userName,1,0);
            databaseGroupLayout->addWidget(userNameInput,1,1);
            databaseGroupLayout->addWidget(password,1,2);
            databaseGroupLayout->addWidget(passwordInput,1,3,1,4);

            // agenda
            agendaName = new QLabel(QStringLiteral("Agenda"));
            agendaNameInput = new QComboBox;
            connectToAgendaResult = new IconAndMouseLabelWidget;
            databaseGroupLayout->addWidget(agendaName,2,0);
            databaseGroupLayout->addWidget(agendaNameInput,2,1);
            databaseGroupLayout->addWidget(connectToAgendaResult,2,6);
            // layout
            databaseGroupBox->setLayout(databaseGroupLayout);

            // bank account statement
            statementGroupBox = new QGroupBox(QStringLiteral("Bankovní výpis"), MainWindow);
            statementGroupLayout = new QVBoxLayout;
            // file path
            filePathLayout = new QHBoxLayout;
            filePath = new QLabel(QStringLiteral("Cesta k souboru"));
            filePathInput = new QLineEdit;
            openFileDialogButton =
                new QPushButton(QIcon(QStringLiteral(":/icons/icons/folder-open.png")), QString());
            filePathLayout->addWidget(filePath);
            filePathLayout->addWidget(filePathInput);
            filePathLayout->addWidget(openFileDialogButton);
            // homebanking
            homebankingLayout = new QHBoxLayout;
            homebanking = new QLabel(QStringLiteral("Homebanking"));
            homebankingType = new QLineEdit();
            homebankingType->setAlignment(Qt::AlignHCenter);
            homebankingType->setEnabled(false);
            homebankingInput = new QComboBox;
            homebankingLayout->addWidget(homebanking,1);
            homebankingLayout->addWidget(homebankingType,1);
            homebankingLayout->addWidget(homebankingInput,2);
            // layout
            statementGroupLayout->addLayout(filePathLayout);
            statementGroupLayout->addLayout(homebankingLayout);
            statementGroupBox->setLayout(statementGroupLayout);

            // display parsing results
            parsingFileGroupBox = new QGroupBox(QStringLiteral("Načtení výpisu"), MainWindow);
            parsingFileGroupLayout = new QGridLayout;
            // set columns' width
            for (int i = 0; i < 4; ++i)
                parsingFileGroupLayout->setColumnStretch(i,7-(i % 2)*6);
            // open and process file
            fileNotCorruptedLabel = new QLabel(QStringLiteral("Otevření souboru"));
            fileNotCorruptedResult = new IconLabelWidget;
            fileProcessedLabel = new QLabel(QStringLiteral("Zpracování souboru"));
            fileProcessedResult = new IconLabelWidget;
            // display account number and bank
            bankAccountNumber = new QLineEdit;
            bankAccountNumber->setReadOnly(true);
            nameOfBank = new QLineEdit;
            nameOfBank->setReadOnly(true);
            // layout
            parsingFileGroupLayout->addWidget(fileNotCorruptedLabel,0,0);
            parsingFileGroupLayout->addWidget(fileNotCorruptedResult,0,1);
            parsingFileGroupLayout->addWidget(fileProcessedLabel,0,2);
            parsingFileGroupLayout->addWidget(fileProcessedResult,0,3);
            parsingFileGroupLayout->addWidget(bankAccountNumber,1,0,1,2);
            parsingFileGroupLayout->addWidget(nameOfBank,1,2,1,2);
            parsingFileGroupBox->setLayout(parsingFileGroupLayout);

            // compare loaded values with values stored in Money DB
            compareToMoneyGroupBox = new QGroupBox(QStringLiteral("Údaje v databázi Money"), MainWindow);
            compareToMoneyGroupLayout = new QGridLayout;
            compareToMoneyGroupLayout->setColumnStretch(0,2);
            compareToMoneyGroupLayout->setColumnStretch(1,1);
            compareToMoneyGroupLayout->setColumnStretch(2,6);

            iconReadFromDB = QVector<IconLabelWidget *>(rows);
            for (auto it = iconReadFromDB.begin(); it != iconReadFromDB.end(); ++it) {
                *it = new IconLabelWidget(IconLabelWidget::READ);
                (*it)->setToolTip(QStringLiteral("read from DB"));
            }
            iconWriteToDB = QVector<IconLabelWidget *>(rows);
            for (auto it = iconWriteToDB.begin(); it != iconWriteToDB.end(); ++it) {
                *it = new IconLabelWidget(IconLabelWidget::WRITE);
                (*it)->setToolTip(QStringLiteral("write to DB"));
            }

            uint8_t row = 0;
            // bank connection (bankovni spojeni v adresari firem) - elements
            bankConnectionLabel = new QLabel(QStringLiteral("Bankovní spojení"));
            bankConnectionID = new QLineEdit;
            bankConnectionID->setAlignment(Qt::AlignHCenter),
            bankConnectionID->setReadOnly(true);
            editBankConnection = new QCheckBox;
            editBankConnection->setEnabled(false);
            readBankConnectionResult = new IconAndMouseLabelWidget;
            writeBankConnectionResult = new IconAndMouseLabelWidget;
            // bank connection - layout
            compareToMoneyGroupLayout->addWidget(bankConnectionLabel, row, 0);
            compareToMoneyGroupLayout->addWidget(bankConnectionID, row, 1, 1, 2);
            compareToMoneyGroupLayout->addWidget(editBankConnection, row, 3);
            compareToMoneyGroupLayout->addWidget(iconReadFromDB[row], row, 4);
            compareToMoneyGroupLayout->addWidget(readBankConnectionResult, row, 5);
            compareToMoneyGroupLayout->addWidget(iconWriteToDB[row], row, 6);
            compareToMoneyGroupLayout->addWidget(writeBankConnectionResult, row++, 7);
            // bank account (bankovni ucet)
            bankAccountLabel = new QLabel(QStringLiteral("Bankovní účet"));
            bankAccountID = new QLineEdit;
            bankAccountID->setAlignment(Qt::AlignHCenter);
            bankAccountID->setReadOnly(true);
            editBankAccount = new QCheckBox;
            editBankAccount->setEnabled(false);
            readBankAccountResult = new IconAndMouseLabelWidget;
            writeBankAccountResult = new IconAndMouseLabelWidget;
            // bank account - layout
            compareToMoneyGroupLayout->addWidget(bankAccountLabel, row, 0);
            compareToMoneyGroupLayout->addWidget(bankAccountID, row, 1, 1, 2);
            compareToMoneyGroupLayout->addWidget(editBankAccount, row, 3);
            compareToMoneyGroupLayout->addWidget(iconReadFromDB[row], row, 4);
            compareToMoneyGroupLayout->addWidget(readBankAccountResult, row, 5);
            compareToMoneyGroupLayout->addWidget(iconWriteToDB[row], row, 6);
            compareToMoneyGroupLayout->addWidget(writeBankAccountResult, row++, 7);
            // line separator
            horizontalLine = new QFrame;
            horizontalLine->setFrameShape(QFrame::HLine);
            horizontalLine->setFrameShadow(QFrame::Sunken);
            compareToMoneyGroupLayout->addWidget(horizontalLine, row++, 0, 1, 8);
            // bank statements group
            statementsGroupLabel = new QLabel(QStringLiteral("Nastavení na skupině BV"));
            statementsGroupInput = new QComboBox;
            editStatementsGroup = new QCheckBox;
            editStatementsGroup->setEnabled(false);
            readStatementsGroupResult = new IconAndMouseLabelWidget;
            writeStatementsGroupResult = new IconAndMouseLabelWidget;
            // bank statements group - layout
            compareToMoneyGroupLayout->addWidget(statementsGroupLabel, row, 0, 1, 2);
            compareToMoneyGroupLayout->addWidget(statementsGroupInput, row, 2);
            compareToMoneyGroupLayout->addWidget(editStatementsGroup, row, 3);
            compareToMoneyGroupLayout->addWidget(iconReadFromDB[row], row, 4);
            compareToMoneyGroupLayout->addWidget(readStatementsGroupResult, row, 5);
            compareToMoneyGroupLayout->addWidget(iconWriteToDB[row], row, 6);
            compareToMoneyGroupLayout->addWidget(writeStatementsGroupResult, row++, 7);
            // homebanking group
            homebankingLabel = new QLabel(QStringLiteral("Konfigurace homebankingu"));
            homebankingDirectory = new QLineEdit;
            homebankingDirectory->setEnabled(false);
            editHomebanking = new QCheckBox;
            editHomebanking->setEnabled(false);
            readHomebankingResult = new IconAndMouseLabelWidget;
            writeHomebankingResult = new IconAndMouseLabelWidget;
            // homebanking group - layout
            compareToMoneyGroupLayout->addWidget(homebankingLabel, row, 0, 1, 2);
            compareToMoneyGroupLayout->addWidget(homebankingDirectory, row, 2);
            compareToMoneyGroupLayout->addWidget(editHomebanking, row, 3);
            compareToMoneyGroupLayout->addWidget(iconReadFromDB[row], row, 4);
            compareToMoneyGroupLayout->addWidget(readHomebankingResult, row, 5);
            compareToMoneyGroupLayout->addWidget(iconWriteToDB[row], row, 6);
            compareToMoneyGroupLayout->addWidget(writeHomebankingResult, row, 7);
            compareToMoneyGroupBox->setLayout(compareToMoneyGroupLayout);

            // buttons
            buttonsLayout = new QHBoxLayout();
            profileButton = new QPushButton(QStringLiteral("Profil"));
            profileButton->setEnabled(false);
            passAllButton = new QPushButton(QStringLiteral("Přenést vše"));
            passAllButton->setEnabled(false);
            passSelectedButton = new QPushButton(QStringLiteral("Přenést vybrané"));
            passSelectedButton->setEnabled(false);
            clearButton = new QPushButton(QStringLiteral("Smazat"));
            logButton = new QPushButton(QIcon(QStringLiteral(":/icons/icons/system-log-out.png")),
                                        QStringLiteral("Log"));
            quitButton = new QPushButton(QStringLiteral("Ukončit"));
            buttonsLayout->addWidget(profileButton);
            buttonsLayout->addWidget(passAllButton);
            buttonsLayout->addWidget(passSelectedButton);
            buttonsLayout->addWidget(clearButton);
            buttonsLayout->addWidget(logButton);
            buttonsLayout->addWidget(quitButton);

            windowLayout = new QVBoxLayout(MainWindow);
            windowLayout->addWidget(databaseGroupBox);
            windowLayout->addWidget(statementGroupBox);
            windowLayout->addWidget(parsingFileGroupBox);
            windowLayout->addWidget(compareToMoneyGroupBox);
            windowLayout->addLayout(buttonsLayout);

            QMetaObject::connectSlotsByName(MainWindow);
        }

        void highlightUnknownBankCode(bool unknownBankCode, int from) {

            // bank account number not found
            if (from == 0)
                unknownBankCode = false;

            bankAccountNumber->setReadOnly(false);

            int length;
            QTextCharFormat charFormat;

            if (unknownBankCode)  {

                from -= bankAccountNumber->cursorPosition();
                length = BankAccount::bankCodeLength;
                charFormat.setForeground(Qt::red);
            }
            else {

                from = 0;
                length = bankAccountNumber->text().length();
                charFormat.setForeground(Qt::black);
            }

            QList<QInputMethodEvent::Attribute> attributes;
            QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;

            attributes.append(QInputMethodEvent::Attribute(type, from, length, charFormat));
            QInputMethodEvent event(QString(), attributes);

            QCoreApplication::sendEvent(bankAccountNumber, &event);
            bankAccountNumber->setReadOnly(true);

            return;
        }
};

#endif // UI_MAINWINDOW_H
