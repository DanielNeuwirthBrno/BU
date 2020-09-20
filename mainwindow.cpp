/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#include <QApplication>
// #include <QComboBox>
// #include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QIODevice>
#include <QMessageBox>
// #include <QPair>
#include <QSqlDatabase>
// #include <QSqlError>
// #include <QString>
// #include <QStringList>
#include <QTextStream>
// #include <QUuid>
// #include <QVector>
// #include <QWidget>
#include "authcombo.h"
#include "bank.h"
#include "bankwindow.h"
#include "database.h"
#include "homebanking.h"
#include "iconlabel.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "s5object.h"

MainWindow::MainWindow(QWidget * parent):
    QDialog(parent), ui(new Ui_MainWindow) {

    db = new Database();
    db->addNewDatabaseConnection(QSqlDatabase(QSqlDatabase::addDatabase(
        db->getConnectionProfile()->getDriverName(), db->getConnectionProfile()->getConnectionName())));
    db->getDatabaseConnection()->setConnectOptions(
        QStringLiteral("SQL_ATTR_CONNECTION_TIMEOUT=") +
        QString::number(db->getConnectionProfile()->getConnectionTimeout()));
    hb = new Homebanking();
    bsg = new BankStatementGroup();

    ui->setupUi(this, rows);

    connect(ui->serverNameInput, &QLineEdit::editingFinished, this, &MainWindow::initServerConnection);
    connect(ui->userNameInput, &QLineEdit::editingFinished, this, &MainWindow::initServerConnection);
    connect(ui->passwordInput, &QLineEdit::editingFinished, this, &MainWindow::initServerConnection);
    connect(ui->authenticationResult, &IconAndMouseLabelWidget::iconClicked, this, &MainWindow::initServerConnection);
    connect(ui->connectToAgendaResult, &IconAndMouseLabelWidget::iconClicked, this, &MainWindow::initAgendaConnection);

    connect(ui->openFileDialogButton, &QPushButton::clicked, this, &MainWindow::selectFile);
    connect(this, &MainWindow::parseSelectedFile, this, &MainWindow::parseFile);
    connect(ui->filePathInput, &QLineEdit::editingFinished, this, &MainWindow::parseFile);
    connect(ui->filePathInput, &QLineEdit::textChanged, this, &MainWindow::compareHomebankingConfiguration);
    connect(this, &MainWindow::fileIsCorrupted, this,
            [this]() -> void { ui->fileProcessedResult->setIcon(IconLabelWidget::ERROR); });
    connect(this, &MainWindow::selectBankFromList, this, &MainWindow::displaySelectBankWindow);

    // connect combo boxes
    connect(ui->authenticationType, static_cast<void(AuthComboBoxWidget::*)(int)>
            (&AuthComboBoxWidget::currentIndexChanged), this, &MainWindow::showCredentialsInput);
    connect(ui->agendaNameInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &MainWindow::initAgendaConnection);
    connect(ui->agendaNameInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]() -> void { listOfBanks.clear(); });
    connect(ui->homebankingInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, [this]() -> void { ui->writeHomebankingResult->setIcon(IconLabelWidget::UNKNOWN); });
    connect(ui->homebankingInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &MainWindow::parseFile);
    connect(ui->homebankingInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &MainWindow::displayHomebankingFilePath);
    connect(ui->homebankingInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &MainWindow::compareHomebankingIDs);
    connect(ui->homebankingInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &MainWindow::compareHomebankingConfiguration);
    connect(ui->statementsGroupInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, [this]() -> void { ui->writeStatementsGroupResult->setIcon(IconLabelWidget::UNKNOWN); });
    connect(ui->statementsGroupInput, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &MainWindow::compareHomebankingIDs);

    // signals connected with enable pass buttons
    connect(ui->editBankConnection, &QCheckBox::toggled, this, &MainWindow::enablePassButtons);
    connect(ui->editBankAccount, &QCheckBox::toggled, this, &MainWindow::enablePassButtons);
    connect(ui->editStatementsGroup, &QCheckBox::toggled, this, &MainWindow::enablePassButtons);
    connect(ui->editHomebanking, &QCheckBox::toggled, this, &MainWindow::enablePassButtons);

    // bankConnection and bankAccount checkboxes' dependency
    connect(ui->editBankConnection, &QCheckBox::clicked, this, [this]() -> void
            { if (!ui->editBankConnection->isChecked() && ui->editBankAccount->isChecked())
                  ui->editBankAccount->setChecked(false); });
    connect(ui->editBankAccount, &QCheckBox::clicked, this, [this]() -> void
            { if (ui->editBankAccount->isChecked() && !ui->editBankConnection->isChecked() &&
              ui->editBankConnection->isEnabled()) ui->editBankConnection->setChecked(true); });

    // connect buttons
    connect(ui->quitButton, &QPushButton::clicked, this, &QApplication::quit);
    connect(ui->logButton, &QPushButton::clicked, this, &MainWindow::displayLogWindow);
    connect(ui->clearButton, &QPushButton::clicked, this, [this]() -> void { clearAgendaPartial(); });
    connect(ui->passSelectedButton, &QPushButton::clicked, this, &MainWindow::passSelectedValues);
    connect(ui->passAllButton, &QPushButton::clicked, this, &MainWindow::passAllValues);
}

MainWindow::~MainWindow() {

    db->closeDB();
    const QString connectionName = db->getConnectionProfile()->getConnectionName();
    delete db;
    QSqlDatabase::removeDatabase(connectionName);

    delete bsg;
    delete hb;
    delete ui;
}

void MainWindow::changeLayout(IconAndMouseLabelWidget * const p1, IconAndMouseLabelWidget * const p2,
                              QCheckBox * const p3, const bool state, Query * const query) const {

    p1->setIcon(IconLabelWidget::convertToResultType(state));
    if (!state)
        p2->setIcon(IconLabelWidget::UNKNOWN);
    p3->setEnabled(!state);

    const QString tooltip =
        (query != nullptr) ? ((!query->isNotEmpty()) ? QStringLiteral("empty result")
                                                     : query->lastError().text())
                           : QString();

    p1->setToolTip(tooltip);
    ui->passAllButton->setEnabled(!state);

    return;
}

bool MainWindow::openFile(QStringList * const fileContents) const {

    QFile file(ui->filePathInput->text());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream fileContentsComplete(&file);
    if (fileContentsComplete.device() == 0)
        return false;

    QString line;
    while (fileContentsComplete.readLineInto(&line))
        fileContents->append(line);

    file.close();
    return true;
}

QString MainWindow::directoryOfSelectedFile() const {

    const QString path = ui->filePathInput->text();
    int posOfDirAndFileSep = path.lastIndexOf('\\');

    if (posOfDirAndFileSep == -1 || posOfDirAndFileSep == path.indexOf('\\'))
        ++posOfDirAndFileSep;

    return path.left(posOfDirAndFileSep);
}

bool MainWindow::searchForAccountNumber(const QStringList * const fileContents) {

    using HB = HomebankingType;

    const QString selectedHBType =
        ui->homebankingInput->currentData(Qt::ToolTipRole).toString();
    HB::TypeOfHB typeOfHB = HB::UNKNOWN;
    bool displayFormatButton = true;

    // iterate over HB types (not configurations!) to find appropriate parameters
    for (auto it = hb->type.begin(); it != hb->type.end(); ++it)
        if (selectedHBType.contains((*it)[HB::SEARCH], Qt::CaseInsensitive)) {

            typeOfHB = static_cast<HB::TypeOfHB>(it - hb->type.begin());
            displayFormatButton = Homebanking::type[typeOfHB].hasInternalFormat();
            break;
        }

    loadBanksIntoList();
    // load bank code and account format from homebanking configuration
    const int index = ui->homebankingInput->currentIndex();
    const QString bankCodeFromConfig = (index > 0) ?
        hb->getHB()[index-1][HomebankingDetails::CODE] : QString();
    bool isInInternalFormat = true;
    if (index > 0 && hb->getHB()[index-1].getFormat() != BankAccount::INTERNAL)
        isInInternalFormat = false;

    QString account = QString();
    QString bankCode = QString();
    bool accountNumberFound = false;

    switch (typeOfHB) {

        case HB::AUTO: /* fall through all possible types of HB */

        case HB::MC: {

            if ((accountNumberFound = hb->searchForAccountNo_MC(fileContents, account, bankCode))) {

                hb->createAccount(bankAccountNumber, account, bankCode);
                typeOfHB = (HB::MC);
            }
            if (typeOfHB != HB::AUTO)
                break;
        }
        case HB::ABO: {

            if ((accountNumberFound = hb->searchForAccountNo_ABO(fileContents, account, bankCode))) {

                if (bankCode.isEmpty())
                    bankCode =  selectBank(displayFormatButton, isInInternalFormat, bankCodeFromConfig);
                else
                    selectAccountFormat(isInInternalFormat);

                hb->createAccount(bankAccountNumber, account, bankCode, isInInternalFormat);
                typeOfHB = (HB::ABO);
            }
            if (typeOfHB != HB::AUTO)
                break;
        }
        case HB::BEST: {

            if ((accountNumberFound = hb->searchForAccountNo_BEST(fileContents, account, bankCode))) {

                // bankCode = selectBank()

                hb->createAccount(bankAccountNumber, account, bankCode /*, isInInternalFormat */);
                typeOfHB = (HB::BEST);
            }
            if (typeOfHB != HB::AUTO)
                break;
        }
        case HB::CITI: {

            if ((accountNumberFound = hb->searchForAccountNo_Citi(fileContents, account, bankCode))) {

                // bankCode = selectBank()

                hb->createAccount(bankAccountNumber, account, bankCode /*, isInInternalFormat */);
                typeOfHB = (HB::CITI);
            }
            if (typeOfHB != HB::AUTO)
                break;
        }
        case HB::GEMINI: {

            if ((accountNumberFound = hb->searchForAccountNo_Gemini(fileContents, account, bankCode))) {

                if (bankCode.isEmpty()) {

                    // displayFormatButton: overwrite default value set by autodetection
                    if (typeOfHB == HB::AUTO)
                        for (auto it = hb->type.begin(); it != hb->type.end(); ++it)
                            if (((*it)[HB::SEARCH]) == "gemini")
                                { displayFormatButton = (*it).hasInternalFormat(); break; }

                    bankCode = selectBank(displayFormatButton, isInInternalFormat, bankCodeFromConfig);
                }

                hb->createAccount(bankAccountNumber, account, bankCode);
                typeOfHB = (HB::GEMINI);
            }
            if (typeOfHB != HB::AUTO)
                break;
        }
        case HB::UNKNOWN:
        default: ;
    }

    const QString detectedType = (static_cast<int>(typeOfHB) >= 1) ?
        hb->type[typeOfHB][HB::NAME] : QStringLiteral("nerozpoznáno");
    ui->homebankingType->setText(detectedType);

    return accountNumberFound;
}

void MainWindow::clearAll() const {

    ui->agendaNameInput->clear();
    ui->connectToAgendaResult->setIcon(IconLabelWidget::UNKNOWN);

    clearAgendaComplete();
    return;
}

void MainWindow::clearAgendaComplete() const {

    ui->homebankingType->clear();
    ui->homebankingInput->clear();
    hb->eraseAllRecords();
    ui->statementsGroupInput->clear();
    bsg->eraseAllRecords();
    ui->homebankingDirectory->clear();

    ui->readStatementsGroupResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->writeStatementsGroupResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->editStatementsGroup->setEnabled(false);
    ui->readHomebankingResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->writeHomebankingResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->editHomebanking->setEnabled(false);

    clearAgendaPartial();

    return;
}

// clearForm
void MainWindow::clearAgendaPartial() const {

    ui->filePathInput->clear();
    ui->fileNotCorruptedResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->fileProcessedResult->setIcon(IconLabelWidget::UNKNOWN);

    clearFileSpecs();

    ui->passAllButton->setEnabled(false);
    ui->passSelectedButton->setEnabled(false);

    return;
}

void MainWindow::clearFileSpecs() const {

    ui->bankAccountNumber->clear();
    ui->nameOfBank->clear();
    ui->homebankingType->clear();

    ui->editStatementsGroup->setChecked(false);
    ui->editHomebanking->setChecked(false);

    clearBankAccountInfo();

    return;
}

void MainWindow::clearBankAccountInfo() const {

    ui->bankConnectionID->clear();
    ui->bankAccountID->clear();

    ui->readBankConnectionResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->writeBankConnectionResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->editBankConnection->setChecked(false);
    ui->editBankConnection->setEnabled(false);
    ui->readBankAccountResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->writeBankAccountResult->setIcon(IconLabelWidget::UNKNOWN);
    ui->editBankAccount->setChecked(false);
    ui->editBankAccount->setEnabled(false);

    return;
}

bool MainWindow::populateComboBox(Query * const query, QComboBox * const comboBox,
                                  IconAndMouseLabelWidget * const label) const {
    query->execute();

    if (query->isProcessed())
        while (query->nextRecord())
            comboBox->addItem(query->value(0).toString());
    else
        label->setToolTip(query->lastError().text());

    if (comboBox->count() == 0)
        label->setToolTip(QStringLiteral("'") + query->getQuery() + QStringLiteral("' returned empty set."));

    return (query->isProcessed());
}

bool MainWindow::populateAgendaComboBox(Database * const db) const {

    const QString queryString =
        QStringLiteral("SELECT name FROM sys.databases WHERE name LIKE '%Agenda%' "
                       "AND name NOT LIKE '%_Doc%' ORDER BY name");
    Query * const query = new Query(*db->getDatabaseConnection(), queryString, true);

    const bool comboBoxFilledIn = populateComboBox(query, ui->agendaNameInput, ui->authenticationResult);
    db->writeToLog(query);

    delete query;
    return comboBoxFilledIn;
}

bool MainWindow::populateHomebankingComboBox(Database * const db) const {

    const QString queryString =
        QStringLiteral("SELECT Nazev, PluginClassName, ID, CAST(XMLConfiguration AS NVARCHAR(MAX)) "
                       "FROM Homebanking_KonfiguraceHomebankingu WHERE Deleted = 0 ORDER BY Nazev");
    Query * query = new Query(*db->getDatabaseConnection(), queryString, false);

    ui->homebankingInput->addItem(QStringLiteral("detekovat automaticky"));
    ui->homebankingInput->setItemData(0,
        QStringLiteral("volba detekovat automaticky neumožňuje upravit konfiguraci HB v Money"), Qt::ToolTipRole);
    const bool comboBoxFilledIn = populateComboBox(query, ui->homebankingInput, ui->connectToAgendaResult);

    query->first();
    for (int i = 1; i < ui->homebankingInput->count(); ++i, query->nextRecord()) {

        const QString configuration = query->value(3).toString();
        const QString extensions = HomebankingDetails::extractFileExtensions(configuration);
        const BankAccount::Format format = HomebankingDetails::extractAccountFormat(configuration);
        const QString filePath =
            HomebankingDetails::extractAdditionalDetails(configuration, HomebankingDetails::filePathXMLTag);
        const QString bankCode =
            HomebankingDetails::extractAdditionalDetails(configuration, HomebankingDetails::bankCodeXMLTag);

        HomebankingDetails recordFromDB(query->value(2).toUuid(), query->value(0).toString(),
                                        query->value(1).toString(), extensions, filePath, bankCode, format);
        hb->insertNewRecord(recordFromDB);
        ui->homebankingInput->setItemData(i,
            QStringLiteral("plug-in: ")+hb->getHB()[i-1][HomebankingDetails::PLUGIN], Qt::ToolTipRole);
    }

    db->writeToLog(query);
    delete query;
    return comboBoxFilledIn;
}

bool MainWindow::populateStatementsGroupComboBox(Database * const) const {

    const QString queryString =
        QStringLiteral("SELECT CASE WHEN Nazev IS NULL THEN 'Nezařazeno' ELSE Nazev END, "
                       "Configuration, ID FROM System_Groups "
                       "WHERE CAST(Configuration AS NVARCHAR(MAX)) LIKE '%<Homebanking_ID>%' "
                       "ORDER BY ISNULL(Parent_ID, NULL), Nazev");
    Query * query = new Query(*db->getDatabaseConnection(), queryString, false);

    const bool comboBoxFilledIn = populateComboBox(query, ui->statementsGroupInput, ui->readStatementsGroupResult);

    query->first();
    for (int i = 0; i < ui->statementsGroupInput->count(); ++i, query->nextRecord()) {

        const QUuid homebankingID = BankStatementGroupSettings::extractHomebankingID(query->value(1).toString());
        BankStatementGroupSettings recordFromDB(query->value(2).toUuid(), query->value(0).toString(), homebankingID);
        bsg->insertNewRecord(recordFromDB);
    }

    db->writeToLog(query);
    delete query;
    return comboBoxFilledIn;
}

void MainWindow::connectToServer() const {

    const QString server = ui->serverNameInput->text();
    const int auth = ui->authenticationType->currentIndex();
    QString userName = ui->userNameInput->text();
    QString password = ui->passwordInput->text();

    if (server.isEmpty() || (auth == SQL_AUTH && (userName.isEmpty() || password.isEmpty()))) {

        ui->agendaNameInput->clear();
        ui->authenticationResult->setIcon(IconLabelWidget::UNKNOWN);
        return;
    }

    if (auth == WIN_AUTH) // no credentials required
        userName = password = QString();

    const QString connectionDataSet =
        "DRIVER={SQL Server};Server=" + server + ";Database=;Uid=" + userName + ";Port=1433;Pwd=" + password + ";";
    db->closeDB();
    db->getDatabaseConnection()->setDatabaseName(connectionDataSet);

    const bool connectionEstablished = db->openDB();

    if (connectionEstablished) {

        ui->authenticationResult->setToolTip(QString());
        populateAgendaComboBox(db);
    }
    else
        ui->authenticationResult->setToolTip(db->getDatabaseConnection()->lastError().text());

    ui->authenticationResult->setIcon(IconLabelWidget::convertToResultType(connectionEstablished));

    return;
}

void MainWindow::connectToAgenda()  const {

    const QString server = ui->serverNameInput->text();
    const int auth = ui->authenticationType->currentIndex();
    QString userName = ui->userNameInput->text();
    QString password = ui->passwordInput->text();
    const QString database = ui->agendaNameInput->currentText();

    if (auth == WIN_AUTH) // no credentials required
        userName = password = QString();

    const QString connectionDataSet =
        "DRIVER={SQL Server};Server=" + server + ";Database=" + database +
        ";Uid=" + userName + ";Port=1433;Pwd=" + password + ";";
    db->closeDB();
    db->getDatabaseConnection()->setDatabaseName(connectionDataSet);

    const bool connectionEstablished = db->openDB();
    bool dataRetrieved = false;

    if (connectionEstablished) {

        ui->authenticationResult->setToolTip(QString());
        ui->connectToAgendaResult->setToolTip(QString());
        dataRetrieved = populateHomebankingComboBox(db);
        dataRetrieved = dataRetrieved && populateStatementsGroupComboBox(db);
    }
    else
        ui->authenticationResult->setToolTip(db->getDatabaseConnection()->lastError().text());

    if (!dataRetrieved)
        ui->connectToAgendaResult->setToolTip(db->lastError());

    ui->authenticationResult->setIcon(IconLabelWidget::convertToResultType(connectionEstablished));
    ui->connectToAgendaResult->setIcon(IconLabelWidget::convertToResultType(dataRetrieved));

    return;
}

void MainWindow::readBankConnectionFromDB() const {

    ui->bankConnectionID->clear();

    // check if exists bank connection
    const QString queryString =
        QStringLiteral("SELECT BS.ID FROM Adresar_BankovniSpojeni AS BS "
                       "INNER JOIN Ciselniky_Banka AS B ON BS.Banka_ID = B.ID "
                       "WHERE BS.Deleted = 0 AND BS.CisloUctu = '") + bankAccountNumber.getAccountNo() +
                       QStringLiteral("' AND B.CiselnyKod = '") + bankAccountNumber.getBankCode() +
                       QStringLiteral("'");
    Query * const query = new Query(*db->getDatabaseConnection(), queryString, true);

    query->execute();
    if (query->isProcessed())
        query->nextRecord();

    changeLayout(ui->readBankConnectionResult, ui->writeBankConnectionResult,
                 ui->editBankConnection, query->isProcessedAndNotEmpty(), query);

    db->writeToLog(query);
    if (query->isProcessedAndNotEmpty())
        ui->bankConnectionID->setText(query->value(0).toString());

    delete query;
    return;
}

void MainWindow::readBankAccountFromDB(const QString & bankConnectionID) const {

    ui->bankAccountID->clear();

    if (bankConnectionID.isEmpty()) {

        changeLayout(ui->readBankAccountResult, ui->writeBankAccountResult,
                     ui->editBankAccount, false, nullptr);
        return;
    }

    // check if exists bank account
    const QString queryString =
        QStringLiteral("SELECT ID FROM Finance_BankovniUcet WHERE Deleted = 0 AND BankovniSpojeni_ID = '") +
                       bankConnectionID + QStringLiteral("'");
    Query * const query = new Query(*db->getDatabaseConnection(), queryString, true);

    query->execute();
    if (query->isProcessed())
        query->nextRecord();

    changeLayout(ui->readBankAccountResult, ui->writeBankAccountResult, ui->editBankAccount,
                 query->isProcessedAndNotEmpty(), query);

    db->writeToLog(query);
    if (query->isProcessedAndNotEmpty())
        ui->bankAccountID->setText(query->value(0).toString());

    delete query;
    return;
}

void MainWindow::loadBanksIntoList() const {

    if (listOfBanks.isEmpty()) {

        const QString queryString =
            QStringLiteral("SELECT Nazev, CiselnyKod FROM Ciselniky_Banka ORDER BY CiselnyKod");
        Query * const query = new Query(*db->getDatabaseConnection(), queryString, true);

        query->execute();
        if (query->isProcessed()) {

            while (query->nextRecord()) {

                const QPair<QString, QString> newRecord(query->value(0).toString(),
                                                        query->value(1).toString());
                listOfBanks.push_back(newRecord);
            };
        }
        db->writeToLog(query);
    }
    return;
}

bool MainWindow::checkBankCode(const QString & bankCode) const {

    for (auto it: listOfBanks)
        if (it.second == bankCode)
            return true;

    return false;
}

QString MainWindow::selectBank(const bool displayFormatButton, bool & isInInternalFormat,
                               const QString & bankCodeFromConfig) const {

    if (listOfBanks.isEmpty())
        return QString();

    int resultCode = /* emit */
        selectBankFromList(listOfBanks, displayFormatButton, isInInternalFormat, bankCodeFromConfig);
    if (resultCode >= 1000)
        { isInInternalFormat = true; resultCode -= 1000; }
    else isInInternalFormat = false;

    return (resultCode != -1) ? listOfBanks[resultCode].second : QString();
}

void MainWindow::selectAccountFormat(bool & format) const {

    AccountFormatDialog formatDialog(format);
    int resultCode = formatDialog.exec();

    format = !(static_cast<bool>(resultCode));
    return;
}

void MainWindow::initServerConnection() const {

    this->clearAll();

    ui->authenticationResult->setIcon(IconLabelWidget::BUSY);
    ui->authenticationResult->repaint();

    connectToServer();
    return;
}

// [slot]
void MainWindow::showCredentialsInput(int index) const {

    if (index == WIN_AUTH) {

        ui->userNameInput->setEnabled(false);
        ui->passwordInput->setEnabled(false);

        initServerConnection();
    }

    const bool isHidden = (index == WIN_AUTH) ? true : false;
    ui->userName->setHidden(isHidden);
    ui->userNameInput->setHidden(isHidden);
    ui->password->setHidden(isHidden);
    ui->passwordInput->setHidden(isHidden);
    ui->userNameInput->setEnabled(true);
    ui->passwordInput->setEnabled(true);

    return;
}

// [slot]
void MainWindow::initAgendaConnection() const {

    this->clearAgendaComplete();

    ui->connectToAgendaResult->setIcon(IconLabelWidget::BUSY);
    ui->connectToAgendaResult->repaint();

    connectToAgenda();
    return;
}

// [slot]
void MainWindow::selectFile() {

    const int selectedHB = ui->homebankingInput->currentIndex();
    const QString fileFilter =
        (selectedHB > 0) ? QStringLiteral("Výpisy z účtu (") + hb->getHB()[selectedHB-1][HomebankingDetails::EXT] +
                           QStringLiteral(")") : QString();
    QString selectedFile = QFileDialog::getOpenFileName(this, QStringLiteral("Výběr souboru"),
                                                        directoryOfSelectedFile(), fileFilter);

    if (!selectedFile.isNull()) {

        ui->filePathInput->setText(selectedFile.replace('/','\\'));
        emit parseSelectedFile();
    }
    return;
}

// [slot]
void MainWindow::parseFile() {

    clearFileSpecs();
    bankAccountNumber.eraseAccount();
    fileName = ui->filePathInput->text();

    if (fileName.isEmpty() || ui->homebankingInput->count() == 0)
        return;

    QStringList * fileContents = new QStringList;
    bool continueWithNextStep = openFile(fileContents);
    ui->fileNotCorruptedResult->setIcon(IconLabelWidget::convertToResultType(continueWithNextStep));

    // if file can be opened and read
    if (continueWithNextStep) {

        continueWithNextStep = searchForAccountNumber(fileContents);
        ui->fileProcessedResult->setIcon(IconLabelWidget::convertToResultType(continueWithNextStep));

        const QString bankAccount = QStringLiteral("číslo účtu: ") + bankAccountNumber.accountNoWithCode();
        ui->bankAccountNumber->setText(bankAccount);

        // bank code not found in list (of banks) is drawn in red
        const int from = bankAccount.indexOf('/')+1;
        ui->highlightUnknownBankCode(!checkBankCode(bankAccountNumber.getBankCode()), from);
    }
    else {

        emit fileIsCorrupted();
        delete fileContents;
        return;
    }

    // if file contains account number
    if (continueWithNextStep && bankAccountNumber.isComplete()) {

        const QString queryString =
            QStringLiteral("SELECT TOP 1 B.ID, B.Nazev, B.CiselnyKod, B.SWIFT, S.Kod, S.Nazev, S.ID "
                           "FROM Ciselniky_Banka AS B "
                           "LEFT JOIN Ciselniky_Stat AS S ON B.Stat_ID = S.ID WHERE B.CiselnyKod = '") +
                           bankAccountNumber.getBankCode() + QStringLiteral("'");
        Query * const query = new Query(*db->getDatabaseConnection(), queryString, true);

        query->execute();
        if (query->isProcessed())
            query->nextRecord();

        Bank * const bankToAccountBinding =
            (query->isProcessedAndNotEmpty()) ? new Bank(query) : new Bank();
        bankAccountNumber.setBankDetails(bankToAccountBinding);
        delete bankToAccountBinding;

        if (query->isProcessedAndNotEmpty())
            ui->nameOfBank->setText(query->value(Bank::NAME).toString());
        else
            ui->nameOfBank->setText(QStringLiteral("neznámá banka"));

        db->writeToLog(query);
        delete query;
    }
    else {

        clearBankAccountInfo();
        delete fileContents;
        return;
    }

    // check if exists bank connection
    readBankConnectionFromDB();
    // check if exists bank account
    readBankAccountFromDB(ui->bankConnectionID->text());

    delete fileContents;
    return;
}

void MainWindow::displayHomebankingFilePath() const {

    const int index = ui->homebankingInput->currentIndex();

    if (index > 0) {

        const QString filePath = hb->getHB()[index-1][HomebankingDetails::PATH];
        if (filePath.isEmpty())
            ui->homebankingDirectory->setText(
                QStringLiteral("adresář pro soubory bankovních výpisů není určen"));
        else
            ui->homebankingDirectory->setText(filePath);
    }
    else
        ui->homebankingDirectory->clear();

    return;
}

// [slot]
void MainWindow::compareHomebankingIDs() const {

    if (ui->homebankingInput->currentIndex()) {

        const QUuid hbFromSelectedGroup =
            bsg->getSettings()[ui->statementsGroupInput->currentIndex()].homebanking_ID();
        const QUuid hbFromSelectedHB = hb->getHB()[ui->homebankingInput->currentIndex()-1].getID();
        changeLayout(ui->readStatementsGroupResult, ui->writeStatementsGroupResult,
                     ui->editStatementsGroup, hbFromSelectedGroup == hbFromSelectedHB);
    }
    else {

        ui->readStatementsGroupResult->setIcon(IconLabelWidget::UNKNOWN);
        ui->readStatementsGroupResult->setToolTip(QStringLiteral("N/A"));
        ui->editStatementsGroup->setEnabled(false);
    }

    enablePassButtons();
    return;
}

// [slot]
void MainWindow::compareHomebankingConfiguration() const {

    if (ui->homebankingInput->currentIndex() > 0) {

        // directory
        const QString filePathFromHB =
            hb->getHB()[ui->homebankingInput->currentIndex()-1][HomebankingDetails::PATH];
        const QString filePathSelected = directoryOfSelectedFile();

        // bank code
        const QString bankCodeFromHB =
            hb->getHB()[ui->homebankingInput->currentIndex()-1][HomebankingDetails::CODE];
        const QString bankCodeSelected = bankAccountNumber.getBankCode();
        const bool bankCodeAgreed =
            (bankCodeFromHB == bankCodeSelected || bankCodeSelected.isEmpty() || bankCodeFromHB.isEmpty());

        // account format
        const BankAccount::Format formatFromHB =
            hb->getHB()[ui->homebankingInput->currentIndex()-1].getFormat();
        const BankAccount::Format formatSelected = bankAccountNumber.getFormat();
        const bool formatAgreed =
            (formatFromHB == formatSelected || formatSelected == BankAccount::NONE || formatFromHB == BankAccount::NONE);

        changeLayout(ui->readHomebankingResult, ui->writeHomebankingResult, ui->editHomebanking,
                     filePathFromHB == filePathSelected && bankCodeAgreed && formatAgreed);
    }
    else {

        ui->readHomebankingResult->setIcon(IconLabelWidget::UNKNOWN);
        ui->readHomebankingResult->setToolTip(QStringLiteral("N/A"));
        ui->editHomebanking->setEnabled(false);
    }

    enablePassButtons();
    return;
}

// [slot]
void MainWindow::enablePassButtons() const {

    ui->passSelectedButton->setEnabled(ui->editBankConnection->isChecked() || ui->editBankAccount->isChecked() ||
                                       ui->editStatementsGroup->isChecked() || ui->editHomebanking->isChecked());
    ui->passAllButton->setEnabled(ui->editBankConnection->isEnabled() || ui->editBankAccount->isEnabled() ||
                                  ui->editStatementsGroup->isEnabled() || ui->editHomebanking->isEnabled());
    return;
}

// [slot]
void MainWindow::passSelectedValues(const bool all) const {

    QUuid bankConnectionID = ui->bankConnectionID->text();
    bool bankConnectionInsertFailed = false;

    // insert new bank connection into DB
    if ((all && ui->editBankConnection->isEnabled()) || ui->editBankConnection->isChecked()) {

        bool insertSuccessful = false;
        QString errorMessage = QString();
        ui->bankConnectionID->text().clear();

        if (bankAccountNumber.isBankCodeValid()) {

            S5_BankConnection * const s5_BankConnection =
                new S5_BankConnection(bankConnectionID, bankAccountNumber, db);

            insertSuccessful = s5_BankConnection->insertIntoDB(db);
            errorMessage = db->lastError();

            delete s5_BankConnection;
        }
        else
            errorMessage = QStringLiteral("invalid bank code");

        ui->writeBankConnectionResult->setIcon(IconLabelWidget::convertToResultType(insertSuccessful));
        ui->writeBankConnectionResult->setToolTip(errorMessage);

        if (insertSuccessful) {

            ui->bankConnectionID->setText(formatGuidString(bankConnectionID));
            ui->editBankConnection->setChecked(false);
            // read newly inserted bank connection from DB (check)
            readBankConnectionFromDB();
        }
        // in case of failure skip insertion of bank account
        else bankConnectionInsertFailed = true;
    }

    const int selectedHB = ui->homebankingInput->currentIndex();
    const QUuid homebankingID = (selectedHB) ? hb->getHB()[selectedHB-1].getID() : QUuid();
    QUuid bankAccountID = ui->bankAccountID->text();

    // insert new bank account into DB
    if ((all && ui->editBankAccount->isEnabled()) || ui->editBankAccount->isChecked()) {

        bool insertSuccessful = false;
        QString errorMessage = QString();
        ui->bankAccountID->text().clear();

        if (!bankConnectionInsertFailed) {

            S5_Balance * s5_Balance = nullptr;
            S5_BankAccount * const s5_BankAccount =
                new S5_BankAccount(bankAccountID, bankConnectionID, homebankingID,
                                   bankAccountNumber, s5_Balance, db);

            insertSuccessful = s5_BankAccount->insertIntoDB(db);
            if (insertSuccessful)
                insertSuccessful = s5_Balance->insertIntoDB(db);
            errorMessage = db->lastError();

            delete s5_BankAccount;
            delete s5_Balance;
        }
        else
            errorMessage = QStringLiteral("bank connection insert failed");

        ui->writeBankAccountResult->setIcon(IconLabelWidget::convertToResultType(insertSuccessful));
        ui->writeBankAccountResult->setToolTip(errorMessage);

        if (insertSuccessful) {

            ui->bankAccountID->setText(formatGuidString(bankAccountID));
            ui->editBankAccount->setChecked(false);
            // read newly inserted bank account from DB (check)
            readBankAccountFromDB(ui->bankConnectionID->text());
        }
    }

    // update homebanking configuration
    if ((all && ui->editHomebanking->isEnabled()) || ui->editHomebanking->isChecked()) {

        const bool isDirPathEmpty = (hb->getHB()[selectedHB-1][HomebankingDetails::PATH].isEmpty());
        const bool isBankCodeEmpty = (hb->getHB()[selectedHB-1][HomebankingDetails::CODE].isEmpty());

        const QPair<bool,bool> /* complete, partial */ updateHBSuccess =
            S5_Homebanking::updateDB(db, homebankingID, directoryOfSelectedFile(), isDirPathEmpty,
                bankAccountNumber.getBankCode(), isBankCodeEmpty, bankAccountNumber.getFormat());

        ui->writeHomebankingResult->setIcon(IconLabelWidget::convertToResultType(updateHBSuccess.first));
        ui->writeHomebankingResult->setToolTip(db->lastError());

        if (updateHBSuccess.second) {

            // update homebankingInput comboBox
            ui->homebankingInput->clear();
            hb->eraseAllRecords();
            const bool dataRetrieved = populateHomebankingComboBox(db);

            if (dataRetrieved && selectedHB <= ui->homebankingInput->count()) {

                ui->homebankingInput->setCurrentIndex(selectedHB);
                displayHomebankingFilePath();
                compareHomebankingConfiguration();
            }
            ui->editHomebanking->setChecked(false);
        }
    }

    // update statements group configuration
    if ((all && ui->editStatementsGroup->isEnabled()) || ui->editStatementsGroup->isChecked()) {

        const int currentIndex = ui->statementsGroupInput->currentIndex();
        const BankStatementGroupSettings currentGroup = bsg->getSettings()[currentIndex];

        const bool updateGroupSuccess = S5_Group::updateDB(db, currentGroup.getID(),
                                        homebankingID.toString().remove(QRegularExpression("[\\{|\\}]")));
        ui->writeStatementsGroupResult->setIcon(IconLabelWidget::convertToResultType(updateGroupSuccess));
        ui->writeStatementsGroupResult->setToolTip(db->lastError());

        if (updateGroupSuccess) {

            // update statementsGroupInput comboBox
            ui->statementsGroupInput->clear();
            bsg->eraseAllRecords();
            const bool dataRetrieved = populateStatementsGroupComboBox(db);

            if (dataRetrieved && currentIndex <= ui->statementsGroupInput->count()) {

                ui->statementsGroupInput->setCurrentIndex(currentIndex);
                compareHomebankingIDs();
            }
            ui->editStatementsGroup->setChecked(false);
        }
    }
    return;
}

// [slot]
void MainWindow::passAllValues() const {

    passSelectedValues(true);
    return;
}

// [slot]
int MainWindow::displayLogWindow() {

    LogWindow logWindow(db->logContents(), db->getShowOnlyErrors(), this);
    return logWindow.exec();
}

// [slot]
int MainWindow::displaySelectBankWindow(const QVector<QPair<QString, QString>> & inListOfBanks,
    const bool displayFormatButton, const bool stateOfFormatButton, const QString & bankCodeFromConfig) {

    const QPair<bool, bool> formatButton = QPair<bool, bool>(displayFormatButton, stateOfFormatButton);
    BankWindow bankWindow(inListOfBanks, formatButton, bankCodeFromConfig);
    return bankWindow.exec();
}
