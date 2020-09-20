/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#include <array>
// #include <QDateTime>
// #include <QPair>
// #include <QSqlDatabase>
#include <QSqlError>
// #include <QSqlQuery>
// #include <QString>
// #include <QUuid>
#include <QVariant>
#include "bank.h"
#include "s5object.h"

const QString ConvertValueToString::ap = QStringLiteral("'");
const QString UserDataColumn::defaultContent = QStringLiteral("<UserData />");

bool UserDataColumn::checkForUserDataColumn(Database * const myDB,
    const ConvertValueToString * const to, const QString & name) {

    QSqlQuery * checkForUserDataColumnQuery = new QSqlQuery(*(myDB->getDatabaseConnection()));
    const QString queryString =
        QStringLiteral("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '") +
        name + QStringLiteral("' AND COLUMN_NAME LIKE '%UserData%' AND DATA_TYPE = 'xml'");

    bool storedByOriginalMethod = false;
    if (checkForUserDataColumnQuery->exec(queryString) && checkForUserDataColumnQuery->next()) {

        const QString columnName = checkForUserDataColumnQuery->value(0).toString();

        if (!columnName.isEmpty()) {

            this->nameOfUserDataColumn = columnName + QStringLiteral(", ");
            this->contentOfUserDataColumn = to->string(defaultContent) + to->string();
            storedByOriginalMethod = true;
        }
    }
    myDB->writeToLog(queryString, checkForUserDataColumnQuery->lastError());
    delete checkForUserDataColumnQuery;

    return storedByOriginalMethod;
}

const QString S5_BankConnection::tableName = QStringLiteral("Adresar_BankovniSpojeni");

S5_BankConnection::S5_BankConnection(QUuid & id, const BankAccount & inBankInfo, Database * const myDB):
    ID(QUuid::createUuid()),
    Group_ID(QUuid()),
    Deleted(false),
    Locked(false),
    Create_ID(QUuid()),
    Create_Date(QDateTime::currentDateTime()),
    Modify_ID(QUuid()),
    Modify_Date(QDateTime()),
    CisloUctu(inBankInfo.getAccountNo()),
    SpecSymbol(QString()),
    IBAN(inBankInfo.generateIBAN()),
    Banka_ID(inBankInfo.bankDetails()->getID()),
    Hidden(false),
    ZahranicniUcet(false),
    SWIFTText((*(inBankInfo.bankDetails()))[Bank::SWIFT]),
    Attachments(false),
    ZahranicniBankaStat_ID(inBankInfo.bankDetails()->getCountryID()),
    ZahranicniBankaStatText((*(inBankInfo.bankDetails()))[Bank::CCODE]),
    ZahranicniBankaText((*(inBankInfo.bankDetails()))[Bank::NAME]),
    ZahranicniBankaStatNazevText((*(inBankInfo.bankDetails()))[Bank::CNAME]),
    DatumPosledniKontrolyUctuWS(Create_Date),
    StavUctuPlatceDPHVracenyWS_Stav(0),
    PosledniOvereniZHistorie_ID(QUuid()) {

    id = ID;
    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    // retrieve MojeFirma record
    const QString queryString =
        QStringLiteral("SELECT TOP 1 MojeFirma_ID FROM System_AgendaDetail");
    Query * query = new Query(*db, queryString, true);

    query->execute();
    if (query->isProcessed() && query->nextRecord())
        Root_ID = Parent_ID = query->value(0).toUuid();

    myDB->writeToLog(query);
    delete query;
    delete db;
}

bool S5_BankConnection::insertIntoDB(Database * const myDB) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    // check in DB how are UserData (user columns) stored
    UserData.checkForUserDataColumn(myDB, &to, tableName);

    const QString queryString =
        QStringLiteral("INSERT INTO ") + tableName +
        QStringLiteral(" (ID, Parent_ID, Root_ID, Group_ID, Deleted, Locked, Create_ID, "
                       " Create_Date, Modify_ID, Modify_Date, ") + UserData.getUserColumnName() +
        QStringLiteral("CisloUctu, SpecSymbol, IBAN, Banka_ID, Hidden, ZahranicniUcet, SWIFTText, "
                       "Attachments, ZahranicniBankaStatNazevText, ZahranicniBankaStat_ID, "
                       "ZahranicniBankaStatText, ZahranicniBankaText, DatumPosledniKontrolyUctuWS, "
                       "StavUctuPlatceDPHVracenyWS_Stav, PosledniOvereniZHistorie_ID) VALUES (") +
                       to.string(ID) + to.string() + to.string(Parent_ID) + to.string() +
                       to.string(Root_ID) + to.string() + to.string(Group_ID) + to.string() +
                       to.string(Deleted) + to.string() + to.string(Locked) + to.string() +
                       to.string(Create_ID, false) + to.string() + to.string(Create_Date) +
                       to.string() + to.string(Modify_ID) + to.string() + to.string(Modify_Date) +
                       to.string() + UserData.getUserColumnContent() + to.string(CisloUctu) +
                       to.string() + to.string(SpecSymbol) + to.string() + to.string(IBAN) +
                       to.string() + to.string(Banka_ID) + to.string() + to.string(Hidden) +
                       to.string() + to.string(ZahranicniUcet) + to.string() + to.string(SWIFTText) +
                       to.string() + to.string(Attachments) + to.string() +
                       to.string(ZahranicniBankaStatNazevText) + to.string() +
                       to.string(ZahranicniBankaStat_ID) + to.string() +
                       to.string(ZahranicniBankaStatText) + to.string() +
                       to.string(ZahranicniBankaText) + to.string() +
                       to.string(DatumPosledniKontrolyUctuWS) + to.string() +
                       to.string(StavUctuPlatceDPHVracenyWS_Stav) + to.string() +
                       to.string(PosledniOvereniZHistorie_ID) + QStringLiteral(")");

    Query * insertQuery = new Query(*db, queryString);
    insertQuery->execute();

    myDB->writeToLog(insertQuery);
    delete insertQuery;
    delete db;

    return (insertQuery->isProcessed());
}

const QString S5_BankAccount::tableName = QStringLiteral("Finance_BankovniUcet");

S5_BankAccount::S5_BankAccount(QUuid & id, const QUuid & bankConnectionID,
                               const QUuid & homebankingID, const BankAccount & inBankInfo,
                               S5_Balance * & pBalance, Database * const myDB):
    ID(QUuid::createUuid()),
    Parent_ID(QUuid()),
    Root_ID(QUuid()),
    Group_ID(QUuid()),
    Deleted(false),
    Locked(false),
    Create_ID(QUuid()),
    Create_Date(QDateTime::currentDateTime()),
    Modify_ID(QUuid()),
    Modify_Date(QDateTime()),
    AktualniStav_ID(QUuid::createUuid()),
    TypZmenyPohybu(0),
    BankovniSpojeni_ID(bankConnectionID),
    Uver(0),
    TypKonta(0),
    PrimarniUcet_ID(QUuid()),
    BankaKurz_ID(QUuid()),
    HomebankingObjectName(QStringLiteral("Homebanking")),
    Hidden(0),
    Attachments(0),
    KonfiguraceHomebankingu_ID(homebankingID),
    PosunDataUplatneniDphPriUhrade(0) {

    id = ID;
    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    QUuid obdobi;
    QString mena;

    /* retrieve DruhKonta_ID and RadaObdobi_ID */ {
    const QString queryString =
        QStringLiteral("SELECT DK.ID, RO.ID, RO.AktivniObdobi_ID FROM EconomicBase_DruhKonta AS DK "
                       "INNER JOIN EconomicBase_RadaObdobi AS RO ON DK.ID = RO.Parent_ID "
                       "WHERE DK.KontoObjectName = 'BankovniUcet'");
    Query * query = new Query(*db, queryString, true);

    query->execute();
    if (query->isProcessed() && query->nextRecord()) {

        DruhKonta_ID = query->value(0).toUuid();
        RadaObdobi_ID = query->value(1).toUuid();
        obdobi = query->value(2).toUuid();
    }

    myDB->writeToLog(query);
    delete query;
    /* endOfScope */ }

    /* retrieve domestic currency */ {
    const QString queryString =
        QStringLiteral("SELECT TOP 1 M.ID, M.Kod FROM System_AgendaDetail AS AD "
                       "LEFT JOIN Meny_Mena AS M ON AD.DomaciMena_ID = M.ID");
    Query * query = new Query(*db, queryString, true);

    query->execute();
    if (query->isProcessed() && query->nextRecord()) {

        Jednotka_ID = query->value(0).toUuid();
        mena = query->value(1).toString();
    }

    myDB->writeToLog(query);
    delete query;
    /* endOfScope */ }
    delete db;

    Kod = QStringLiteral("BU") + ID.toString().mid(1,8);
    Nazev = QStringLiteral("Bankovní účet: ") + inBankInfo.accountNoWithCode() +
            QStringLiteral(" (") + mena + QStringLiteral(")");

    // initialize S5_FinanceBalance object
    pBalance = new S5_Balance(AktualniStav_ID, ID, obdobi);
}

bool S5_BankAccount::insertIntoDB(Database * const myDB) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    // check in DB how are UserData (user columns) stored
    UserData.checkForUserDataColumn(myDB, &to, tableName);

    const QString queryString =
        QStringLiteral("INSERT INTO ") + tableName +
        QStringLiteral(" (ID, Parent_ID, Root_ID, Group_ID, Deleted, Locked, Create_ID, "
                       "Create_Date, Modify_ID, Modify_Date, ") + UserData.getUserColumnName() +
        QStringLiteral("AktualniStav_ID, DruhKonta_ID, Jednotka_ID, Kod, Nazev, RadaObdobi_ID, "
                       "TypZmenyPohybu, BankovniSpojeni_ID, Uver, TypKonta, PrimarniUcet_ID, "
                       "BankaKurz_ID, HomebankingObjectName, Hidden, Attachments, "
                       "KonfiguraceHomebankingu_ID, PosunDataUplatneniDphPriUhrade) VALUES (") +
                       to.string(ID) + to.string() + to.string(Parent_ID) + to.string() +
                       to.string(Root_ID) + to.string() + to.string(Group_ID) + to.string() +
                       to.string(Deleted) + to.string() + to.string(Locked) + to.string() +
                       to.string(Create_ID, false) + to.string() + to.string(Create_Date) + to.string() +
                       to.string(Modify_ID) + to.string() + to.string(Modify_Date) + to.string() +
                       UserData.getUserColumnContent() + to.string(AktualniStav_ID) + to.string() +
                       to.string(DruhKonta_ID) + to.string() + to.string(Jednotka_ID) + to.string() +
                       to.string(Kod) + to.string() + to.string(Nazev) + to.string() +
                       to.string(RadaObdobi_ID) + to.string() + to.string(TypZmenyPohybu) + to.string() +
                       to.string(BankovniSpojeni_ID) + to.string() + to.string(Uver) + to.string() +
                       to.string(TypKonta) + to.string() + to.string(PrimarniUcet_ID) + to.string() +
                       to.string(BankaKurz_ID) + to.string() + to.string(HomebankingObjectName) +
                       to.string() + to.string(Hidden) + to.string() + to.string(Attachments) +
                       to.string() + to.string(KonfiguraceHomebankingu_ID) + to.string() +
                       to.string(PosunDataUplatneniDphPriUhrade) + QStringLiteral(")");

    Query * insertQuery = new Query(*db, queryString);
    insertQuery->execute();

    myDB->writeToLog(insertQuery);
    delete insertQuery;
    delete db;

    return (insertQuery->isProcessed());
}

const QString S5_Balance::tableName = QStringLiteral("Finance_StavFinanci");

S5_Balance::S5_Balance(const QUuid id, const QUuid parent, const QUuid obdobi):
    ID(id),
    Parent_ID(parent),
    Root_ID(parent),
    Group_ID(QUuid()),
    Deleted(false),
    Locked(false),
    Create_ID(QUuid()),
    Create_Date(QDateTime::currentDateTime()),
    Modify_ID(QUuid()),
    Modify_Date(QDateTime()),
    Konto_ID(parent),
    Obdobi_ID(obdobi),
    ObratCelkovaCena(0),
    ObratJednotkaCena(0),
    ObratMnozstvi(0),
    ObratPocetJednotek(0),
    PocatekCelkovaCena(0),
    PocatekJednotkaCena(0),
    PocatekMnozstvi(0),
    PocatekPocetJednotek(0),
    PrirustekCelkovaCena(0),
    PrirustekJednotkaCena(0),
    PrirustekMnozstvi(0),
    PrirustekPocetJednotek(0),
    UbytekCelkovaCena(0),
    UbytekJednotkaCena(0),
    UbytekMnozstvi(0),
    UbytekPocetJednotek(0),
    UzaverkaObdobi_ID(QUuid()),
    ZustatekCelkovaCena(0),
    ZustatekJednotkaCena(0),
    ZustatekMnozstvi(0),
    ZustatekPocetJednotek(0),
    Hidden(0),
    Attachments(0) {}

bool S5_Balance::insertIntoDB(Database * const myDB) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    // check in DB how are UserData (user columns) stored
    UserData.checkForUserDataColumn(myDB, &to, tableName);

    const QString queryString =
        QStringLiteral("INSERT INTO ") + tableName +
        QStringLiteral(" (ID, Parent_ID, Root_ID, Group_ID, Deleted, Locked, Create_ID, "
                       "Create_Date, Modify_ID, Modify_Date, ") + UserData.getUserColumnName() +
        QStringLiteral("Konto_ID, Obdobi_ID, ObratCelkovaCena, ObratJednotkaCena, ObratMnozstvi, "
                       "ObratPocetJednotek, PocatekCelkovaCena, PocatekJednotkaCena, PocatekMnozstvi, "
                       "PocatekPocetJednotek, PrirustekCelkovaCena, PrirustekJednotkaCena, "
                       "PrirustekMnozstvi, PrirustekPocetJednotek, UbytekCelkovaCena, "
                       "UbytekJednotkaCena, UbytekMnozstvi, UbytekPocetJednotek, UzaverkaObdobi_ID, "
                       "ZustatekCelkovaCena, ZustatekJednotkaCena, ZustatekMnozstvi, "
                       "ZustatekPocetJednotek, Hidden, Attachments) VALUES (") + to.string(ID) +
                       to.string() + to.string(Parent_ID) + to.string() + to.string(Root_ID) +
                       to.string() + to.string(Group_ID) + to.string() + to.string(Deleted) +
                       to.string() + to.string(Locked) + to.string() + to.string(Create_ID, false) +
                       to.string() + to.string(Create_Date) + to.string() + to.string(Modify_ID) +
                       to.string() + to.string(Modify_Date) + to.string() +
                       UserData.getUserColumnContent() + to.string(Konto_ID) +
                       to.string() + to.string(Obdobi_ID) + to.string() +
                       to.string(ObratCelkovaCena) + to.string() +
                       to.string(ObratJednotkaCena) + to.string() +
                       to.string(ObratMnozstvi) + to.string() +
                       to.string(ObratPocetJednotek) + to.string() +
                       to.string(PocatekCelkovaCena) + to.string() +
                       to.string(PocatekJednotkaCena) + to.string() +
                       to.string(PocatekMnozstvi) + to.string() +
                       to.string(PocatekPocetJednotek) + to.string() +
                       to.string(PrirustekCelkovaCena) + to.string() +
                       to.string(PrirustekJednotkaCena) + to.string() +
                       to.string(PrirustekMnozstvi) + to.string() +
                       to.string(PrirustekPocetJednotek) + to.string() +
                       to.string(UbytekCelkovaCena) + to.string() +
                       to.string(UbytekJednotkaCena) + to.string() +
                       to.string(UbytekMnozstvi) + to.string() +
                       to.string(UbytekPocetJednotek) + to.string() +
                       to.string(UzaverkaObdobi_ID) + to.string() +
                       to.string(ZustatekCelkovaCena) + to.string() +
                       to.string(ZustatekJednotkaCena) + to.string() +
                       to.string(ZustatekMnozstvi) + to.string() +
                       to.string(ZustatekPocetJednotek) + to.string() +
                       to.string(Hidden) + to.string() + to.string(Attachments) + QStringLiteral(")");

    Query * insertQuery = new Query(*db, queryString);
    insertQuery->execute();

    myDB->writeToLog(insertQuery);
    delete insertQuery;
    delete db;

    return (insertQuery->isProcessed());
}

/* not used */
bool S5_Group::insertIntoDB(Database * const myDB, const QUuid & id, const QString & configuration) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    const QString queryString =
        QStringLiteral("UPDATE System_Groups SET Configuration.modify ('insert text{\"") +
                       configuration + QStringLiteral("\"} into (//") +
                       BankStatementGroupSettings::hbConfXMLTag +
                       QStringLiteral(")[1]') WHERE ID = '") + id.toString() + QStringLiteral("'");

    Query * updateQuery = new Query(*db, queryString);
    updateQuery->execute();

    myDB->writeToLog(updateQuery);
    delete updateQuery;
    delete db;

    return (updateQuery->isProcessed());
}

bool S5_Group::updateDB(Database * const myDB, const QUuid & id, const QString & configuration) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    const QString queryString =
        QStringLiteral("UPDATE System_Groups SET Configuration.modify"
                       "('replace value of (//") + BankStatementGroupSettings::hbConfXMLTag +
        QStringLiteral("/text())[1] with \"") + configuration +
        QStringLiteral("\"') WHERE ID = '") + id.toString() + QStringLiteral("'");

    Query * updateQuery = new Query(*db, queryString);
    updateQuery->execute();

    myDB->writeToLog(updateQuery);
    delete updateQuery;
    delete db;

    return (updateQuery->isProcessed());
}

bool S5_Homebanking::updateDB_AccountNumberFormat(Database * const myDB, const QUuid & id,
                                                  const BankAccount::Format format) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    const QString queryString =
        QStringLiteral("UPDATE Homebanking_KonfiguraceHomebankingu SET XMLConfiguration.modify"
                       "('replace value of (//") + HomebankingDetails::accountFormatXMLTag +
        QStringLiteral("/text())[1] with \"") + accountFormatIDs[static_cast<int>(format)] +
        QStringLiteral("\"') WHERE ID = '") + id.toString() + QStringLiteral("'");

    Query * updateQuery = new Query(*db, queryString);
    updateQuery->execute();

    myDB->writeToLog(updateQuery);
    delete updateQuery;
    delete db;

    return (updateQuery->isProcessed());
}

bool S5_Homebanking::updateDB_Details(Database * const myDB, const QUuid & id,
                                      const QString & updatedValue, const QString & XMLTag) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    const QString queryString =
        QStringLiteral("UPDATE Homebanking_KonfiguraceHomebankingu SET XMLConfiguration.modify"
                       "('replace value of (//") + XMLTag +
                       QStringLiteral("/text())[1] with \"") + updatedValue +
                       QStringLiteral("\"') WHERE ID = '") + id.toString() + QStringLiteral("'");

    Query * updateQuery = new Query(*db, queryString);
    updateQuery->execute();

    myDB->writeToLog(updateQuery);
    delete updateQuery;
    delete db;

    return (updateQuery->isProcessed());
}

bool S5_Homebanking::updateDB_DirectoryPath(Database * const myDB, const QUuid & id, const QString & directory) {

    return (updateDB_Details(myDB, id, directory, HomebankingDetails::filePathXMLTag));
}

bool S5_Homebanking::updateDB_BankCode(Database * const myDB, const QUuid & id, const QString & bankCode) {

    return (updateDB_Details(myDB, id, bankCode, HomebankingDetails::bankCodeXMLTag));
}

bool S5_Homebanking::insertIntoDB_Details(Database * const myDB, const QUuid & id,
                                          const QString & newValue, const QString & XMLTag) {

    const QSqlDatabase * const db = new QSqlDatabase(*(myDB->getDatabaseConnection()));

    const QString queryString =
        QStringLiteral("UPDATE Homebanking_KonfiguraceHomebankingu SET XMLConfiguration.modify"
                       "('insert text{\"") + newValue + QStringLiteral("\"} into (//") + XMLTag +
        QStringLiteral(")[1]') WHERE ID = '") + id.toString() + QStringLiteral("'");

    Query * updateQuery = new Query(*db, queryString);
    updateQuery->execute();

    myDB->writeToLog(updateQuery);
    delete updateQuery;
    delete db;

    return (updateQuery->isProcessed());
}

bool S5_Homebanking::insertIntoDB_DirectoryPath(Database * const myDB, const QUuid & id, const QString & directory) {

    return (insertIntoDB_Details(myDB, id, directory, HomebankingDetails::filePathXMLTag));
}

bool S5_Homebanking::insertIntoDB_BankCode(Database * const myDB, const QUuid & id, const QString & bankCode) {

    return (insertIntoDB_Details(myDB, id, bankCode, HomebankingDetails::bankCodeXMLTag));
}

QPair<bool,bool> S5_Homebanking::updateDB(Database * const myDB, const QUuid & id,
    const QString & directory, const bool pathInXMLConfIsEmpty, const QString & bankCode,
    const bool bankCodeInXMLConfIsEmpty, const BankAccount::Format format) {

    std::array<bool, 5> querySuccessful = { true };

    // update directory path
    querySuccessful[0] &= querySuccessful[1] =
        (pathInXMLConfIsEmpty ? insertIntoDB_DirectoryPath(myDB, id, directory) :
                                updateDB_DirectoryPath(myDB, id, directory));

    // update bank code
    if (!bankCode.isEmpty()) // bank code in XML configuration is not to be erased
      querySuccessful[0] &= querySuccessful[2] =
          (bankCodeInXMLConfIsEmpty ? insertIntoDB_BankCode(myDB, id, bankCode) :
                                      updateDB_BankCode(myDB, id, bankCode));

    // update account number format
    if (format != BankAccount::NONE)
        querySuccessful[0] &= querySuccessful[3] = updateDB_AccountNumberFormat(myDB, id, format);

    for (auto it: querySuccessful)
        if (it) { querySuccessful[4] = it; break; }

    return (QPair<bool,bool>(querySuccessful[0], querySuccessful[4]));
}
