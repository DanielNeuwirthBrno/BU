/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QPair>
#include <QRegularExpression>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVector>
#include <QWidget>
#include "bank.h"
#include "database.h"
#include "homebanking.h"
#include "ui/ui_mainwindow.h"

namespace {

    QVector<QPair<QString, QString>> listOfBanks = {};
}

class MainWindow: public QDialog {

    Q_OBJECT

    public:
        enum AuthType { WIN_AUTH = 0, SQL_AUTH = 1 };

        const static uint8_t rows = 5;

        explicit MainWindow(QWidget * = nullptr);
        ~MainWindow();

        void changeLayout(IconAndMouseLabelWidget * const, IconAndMouseLabelWidget * const,
                          QCheckBox * const, const bool, Query * const = nullptr) const;

    private:
        void clearAll() const;
        void clearAgendaComplete() const;
        void clearAgendaPartial() const;
        void clearFileSpecs() const;
        void clearBankAccountInfo() const;

        bool openFile(QStringList * const) const;
        QString directoryOfSelectedFile() const;
        bool searchForAccountNumber(const QStringList * const);
        bool populateComboBox(Query * const, QComboBox * const, IconAndMouseLabelWidget * const) const;
        bool populateAgendaComboBox(Database * const) const;
        bool populateHomebankingComboBox(Database * const) const;
        bool populateStatementsGroupComboBox(Database * const) const;
        void connectToServer() const;
        void connectToAgenda() const;
        void readBankConnectionFromDB() const;
        void readBankAccountFromDB(const QString &) const;
        inline QString formatGuidString(const QUuid & id) const
            { return (id.toString().toUpper().remove(QRegularExpression("[\\{|\\}]"))); }
        void loadBanksIntoList() const;
        bool checkBankCode(const QString &) const;
        QString selectBank(const bool, bool &, const QString & = QString()) const;
        void selectAccountFormat(bool &) const;

        Database * db;
        Homebanking * hb;
        BankStatementGroup * bsg;
        BankAccount bankAccountNumber;
        QString fileName;
        Ui_MainWindow * ui;

    signals:
        void parseSelectedFile();
        void fileIsCorrupted() const;
        void homebankingConfigChanged() const;
        int selectBankFromList(const QVector<QPair<QString, QString>> &, const bool,
                               const bool, const QString &) const;

    private slots:
        void initServerConnection() const;
        void showCredentialsInput(int) const;
        void initAgendaConnection() const;
        void selectFile();
        void parseFile();
        void displayHomebankingFilePath() const;
        void compareHomebankingIDs() const;
        void compareHomebankingConfiguration() const;
        void enablePassButtons() const;
        void passSelectedValues(const bool = false) const;
        void passAllValues() const;
        // child windows
        int displayLogWindow();
        int displaySelectBankWindow(const QVector<QPair<QString, QString>> &,
                                    const bool, const bool, const QString &);
};

#endif // MAINWINDOW_H
