/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef S5OBJECT_H
#define S5OBJECT_H

// Money Sx v.1.9.2 ->

#include <QDateTime>
#include <QPair>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QUuid>
#include "bank.h"
#include "database.h"
#include "homebanking.h"

class ConvertValueToString {

    public:
        ConvertValueToString() {}
        ~ConvertValueToString() {}

        const static QString ap;

        inline QString string() const
            { return QStringLiteral(", "); }
        inline QString string(const QUuid & guid, const bool nullsAllowed = true) const
            { return ((guid.isNull() && nullsAllowed) ? QStringLiteral("NULL")
                                                      : ap + guid.toString() + ap); }
        inline QString string(const QString & str) const
            { return (ap + str + ap); }
        inline QString string(const QDateTime & date) const
            { return ((date.isNull()) ? QStringLiteral("NULL")
                                      : ap + date.toString("yyyy-MM-dd HH:mm:ss.zzz") + ap); }
        inline QString string(const bool boolValue) const
            { return (string(static_cast<int>(boolValue))); }
        inline QString string(const int intValue) const
            { return (ap + QString::number(intValue) + ap); }
};

class UserDataColumn {

    public:
        UserDataColumn(): nameOfUserDataColumn(QString()), contentOfUserDataColumn(QString()) {}
        ~UserDataColumn() {}

        const static QString defaultContent;
        bool checkForUserDataColumn(Database * const, const ConvertValueToString * const, const QString &);

        QString getUserColumnName() const { return nameOfUserDataColumn; }
        QString getUserColumnContent() const { return contentOfUserDataColumn; }

    private:
        QString nameOfUserDataColumn;
        QString contentOfUserDataColumn;
};

class S5_BankConnection {

    public:
        S5_BankConnection();
        S5_BankConnection(QUuid &, const BankAccount &, Database * const);
        ~S5_BankConnection() {}

        bool insertIntoDB(Database * const);

    private:
        const static QString tableName;
        ConvertValueToString to;
        UserDataColumn UserData;
        QUuid ID;
        QUuid Parent_ID;
        QUuid Root_ID;
        QUuid Group_ID;
        bool Deleted;
        bool Locked;
        QUuid Create_ID;
        QDateTime Create_Date;
        QUuid Modify_ID;
        QDateTime Modify_Date;
        // QString UserData;
        QString CisloUctu;
        QString SpecSymbol;
        QString IBAN;
        QUuid Banka_ID;
        // Poznamka
        bool Hidden;
        bool ZahranicniUcet;
        QString SWIFTText;
        bool Attachments;
        // ZahranicniBankaMisto
        // ZahranicniBankaPSC_ID
        // ZahranicniBankaPSCText
        QUuid ZahranicniBankaStat_ID;
        QString ZahranicniBankaStatText;
        QString ZahranicniBankaText;
        // ZahranicniBankaUlice
        QString ZahranicniBankaStatNazevText;
        QDateTime DatumPosledniKontrolyUctuWS;
        uint8_t StavUctuPlatceDPHVracenyWS_Stav;
        QUuid PosledniOvereniZHistorie_ID;
};

class S5_Balance {

    public:
        S5_Balance();
        S5_Balance(const QUuid, const QUuid, const QUuid);
        ~S5_Balance() {}

        bool insertIntoDB(Database * const);

    private:
        const static QString tableName;
        ConvertValueToString to;
        UserDataColumn UserData;
        QUuid ID;
        QUuid Parent_ID;
        QUuid Root_ID;
        QUuid Group_ID;
        bool Deleted;
        bool Locked;
        QUuid Create_ID;
        QDateTime Create_Date;
        QUuid Modify_ID;
        QDateTime Modify_Date;
        // QString UserData;
        QUuid Konto_ID;
        QUuid Obdobi_ID;
        int ObratCelkovaCena;
        int ObratJednotkaCena;
        int ObratMnozstvi;
        int ObratPocetJednotek;
        int PocatekCelkovaCena;
        int PocatekJednotkaCena;
        int PocatekMnozstvi;
        int PocatekPocetJednotek;
        int PrirustekCelkovaCena;
        int PrirustekJednotkaCena;
        int PrirustekMnozstvi;
        int PrirustekPocetJednotek;
        int UbytekCelkovaCena;
        int UbytekJednotkaCena;
        int UbytekMnozstvi;
        int UbytekPocetJednotek;
        QUuid UzaverkaObdobi_ID;
        int ZustatekCelkovaCena;
        int ZustatekJednotkaCena;
        int ZustatekMnozstvi;
        int ZustatekPocetJednotek;
        bool Hidden;
        bool Attachments;
};

class S5_BankAccount {

    public:
        S5_BankAccount() {}
        S5_BankAccount(QUuid &, const QUuid &, const QUuid &, const BankAccount &,
                       S5_Balance * &, Database * const);
        ~S5_BankAccount() {}

        bool insertIntoDB(Database * const);

    private:
        const static QString tableName;
        ConvertValueToString to;
        UserDataColumn UserData;
        QUuid ID;
        QUuid Parent_ID;
        QUuid Root_ID;
        QUuid Group_ID;
        bool Deleted;
        bool Locked;
        QUuid Create_ID;
        QDateTime Create_Date;
        QUuid Modify_ID;
        QDateTime Modify_Date;
        // QString UserData;
        QUuid AktualniStav_ID;
        QUuid DruhKonta_ID;
        QUuid Jednotka_ID;
        QString Kod;
        QString Nazev;
        // Poznamka
        QUuid RadaObdobi_ID;
        uint8_t TypZmenyPohybu;
        QUuid BankovniSpojeni_ID;
        bool Uver;
        uint8_t TypKonta;
        QUuid PrimarniUcet_ID;
        QUuid BankaKurz_ID;
        QString HomebankingObjectName;
        bool Hidden;
        bool Attachments;
        QUuid KonfiguraceHomebankingu_ID;
        uint16_t PosunDataUplatneniDphPriUhrade;
};

class S5_Group {

    public:
        static bool insertIntoDB(Database * const, const QUuid &, const QString &);
        static bool updateDB(Database * const, const QUuid &, const QString &);
};

class S5_Homebanking {

    public:
        static QPair<bool,bool> updateDB(Database * const, const QUuid &, const QString &, const bool,
                                         const QString &, const bool, const BankAccount::Format);

    private:
        static bool updateDB_AccountNumberFormat(Database * const, const QUuid &, const BankAccount::Format);
        static bool updateDB_Details(Database * const, const QUuid &, const QString &, const QString &);
        static bool updateDB_DirectoryPath(Database * const, const QUuid &, const QString &);
        static bool updateDB_BankCode(Database * const, const QUuid &, const QString &);
        static bool insertIntoDB_Details(Database * const, const QUuid &, const QString &, const QString &);
        static bool insertIntoDB_DirectoryPath(Database * const, const QUuid &, const QString &);
        static bool insertIntoDB_BankCode(Database * const, const QUuid &, const QString &);
};

#endif // S5OBJECT_H
