/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef HOMEBANKING_H
#define HOMEBANKING_H

#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVector>
#include "bank.h"

namespace {

    const QVector<QString> accountFormatIDs =
        { { QStringLiteral("00000000-0000-0000-0000-000000000000") },
          { QStringLiteral("25dec118-80f7-4c3b-b225-3cdc7bed6243") /* STATIC */ },
          { QStringLiteral("1e962ffe-4107-4dd6-8b52-2ae0ab5c72b9") /* INTERNAL */ } };
}

class HomebankingType {

    public:
        enum TypeOfHB { UNKNOWN = -1, AUTO = 0, ABO = 1, BEST = 2, CITI = 3, GEMINI = 4, MC = 5 };
        enum Attribute { TOKEN = 1, SEARCH = 2, NAME = 3 };

        HomebankingType();
        HomebankingType(const QString id, const QString ss,
                        const QString inName, const bool inIntFormat):
            identificationTokenInFile(id), searchForThisStringInPluginName(ss),
            nameOfHomebankingTypeForDisplay(inName), internalFormat(inIntFormat) {}
        ~HomebankingType() {}

        inline bool hasInternalFormat() const { return internalFormat; }
        const QString operator[](const int) const;

    private:
        QString identificationTokenInFile;
        QString searchForThisStringInPluginName;
        QString nameOfHomebankingTypeForDisplay;
        bool internalFormat;
};

class HomebankingDetails {

    public:
        enum homebankingParams { ID = 0, NAME = 1, PLUGIN = 2, EXT = 3, PATH = 4, CODE = 5 };

        const static QString extensionXMLTag;
        const static QString filePathXMLTag;
        const static QString bankCodeXMLTag;
        const static QString accountFormatXMLTag;

        HomebankingDetails() {}
        HomebankingDetails(const QUuid & inID, const QString & inName, const QString & inPlugin,
                           const QString & inExtension, const QString & inFilePath,
                           const QString & inBankCode, BankAccount::Format inFormat):
            id(inID), name(inName), pluginName(inPlugin), fileExtension(inExtension),
            filePath(inFilePath), bankCode(inBankCode), format(inFormat) {}
        ~HomebankingDetails() {}

        static QString extractFileExtensions(const QString &);
        static QString extractAdditionalDetails(const QString &, const QString &);
        static BankAccount::Format extractAccountFormat(const QString &);

        inline QUuid getID() const { return id; }
        inline BankAccount::Format getFormat() const { return format; }
        const QString operator[](const int) const;

    private:
        inline static QString buildXPathExpr(const QString & tagName)
            { return (QStringLiteral("//") + tagName + QStringLiteral("/string()")); }

        QUuid id;
        QString name;
        QString pluginName;
        QString fileExtension;
        QString filePath;
        QString bankCode;
        BankAccount::Format format;
};

class Homebanking {

    public:
        enum paymentType { UNKNOWN = 0, CREDIT = 1, DEBIT = 2 };

        const static uint8_t accountNumberLength =
            BankAccount::prefixLength + BankAccount::accountLength;
        const static uint8_t keyForInternalFormat[accountNumberLength];
        const static QList<HomebankingType> type;

        Homebanking() {}
        ~Homebanking() {}

        inline QVector<HomebankingDetails> const & getHB() const
            { return homebanking; }
        inline void insertNewRecord(const HomebankingDetails & inHB)
            { homebanking.push_back(inHB); return; }
        inline void eraseAllRecords()
            { homebanking.clear(); return; }

        void createAccount(BankAccount &, QString &, const QString &, const bool = false) const;
        bool searchForAccountNo_ABO(const QStringList * const, QString &, QString &) const;
        bool searchForAccountNo_MC(const QStringList * const, QString &, QString &) const;
        bool searchForAccountNo_BEST(const QStringList * const, QString &, QString &) const;
        bool searchForAccountNo_Citi(const QStringList * const, QString &, QString &) const;
        bool searchForAccountNo_Gemini(const QStringList * const, QString &, QString &) const;

    private:
        void separatePrefix(QString &) const;
        void convertFromInternalFormat(QString &) const;
        inline bool containsValidChars(const QString & extractedAccountNo) const
            { return !extractedAccountNo.contains(QRegularExpression("[^\\d|^\\-|^\\/]")); }
        bool isBankCodeValid(const QString &) const;

        QVector<HomebankingDetails> homebanking;
};

#endif // HOMEBANKING_H
