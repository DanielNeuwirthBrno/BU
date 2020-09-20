/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef BANK_H
#define BANK_H

#include <QString>
#include <QUuid>
#include <QVector>
#include "database.h"

class Bank {

    public:
        enum Attribute { ID = 0, NAME = 1, CODE = 2, SWIFT = 3, CCODE = 4, CNAME = 5, CID = 6 };

        Bank() {}
        Bank(Query const * const);
        ~Bank() {}

        inline QUuid getID() const { return id; }
        inline QUuid getCountryID() const { return countryID; }
        const QString operator[](const int) const;

    private:
        QUuid id;
        QString name;
        QString bankCode;
        QString swiftCode;
        QString countryCode;
        QString countryName;
        QUuid countryID;
};

class BankAccount {

    public:
        enum Format { NONE = 0, STATIC = 1, INTERNAL = 2 };

        BankAccount(): bank(new Bank()), accountFormat(NONE) {}
        ~BankAccount() { delete bank; }

        const static uint8_t bankCodeLength = 4;
        const static uint8_t prefixLength = 6;
        const static uint8_t accountLength = 10;

        inline const QString getAccountNo() const { return accountNo; }
        inline const QString getBankCode() const { return bankCode; }
        const QString accountNoWithCode() const;
        inline bool isComplete() const { return (!accountNo.isEmpty() && !bankCode.isEmpty()); }
        inline bool isBankCodeValid() const { return (!bank->getID().isNull()); }
        inline Format getFormat() const { return accountFormat; }

        QString generateIBAN() const;
        inline Bank * bankDetails() const { return bank; }

        inline void setBankDetails(Bank * const inBank)
            { *bank = *inBank; return; }
        void createAccount(const QString &, const QString &, const Format = NONE);
        inline void eraseAccount()
            { accountNo = QString(); bankCode = QString(); return; }

    private:
        QString calculateCheckNumber(QString) const;
        int calculateMod97(QString) const;
        QString removeLeadingZeros(QString) const;

        Bank * bank;
        QString accountNo;
        QString bankCode;
        Format accountFormat;
};

class BankStatementGroupSettings {

    public:
        const static QString hbConfXMLTag;

        BankStatementGroupSettings() {}
        BankStatementGroupSettings(const QUuid &, const QString &, const QUuid &);
        ~BankStatementGroupSettings() {}

        static QUuid extractHomebankingID(const QString &);

        const QUuid & getID() const { return id; }
        const QUuid & homebanking_ID() const { return homebankingID; }

    private:
        QUuid id;
        QString name;
        QUuid homebankingID;
};

class BankStatementGroup {

    public:
        BankStatementGroup() {}
        ~BankStatementGroup() {}

        inline QVector<BankStatementGroupSettings> const & getSettings() const
            { return settings; }
        inline void insertNewRecord(const BankStatementGroupSettings & inSettings)
            { settings.push_back(inSettings); return; }
        inline void eraseAllRecords()
            { settings.clear(); return; }

    private:
        QVector<BankStatementGroupSettings> settings;
};

#endif // BANK_H
