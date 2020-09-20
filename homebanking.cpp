/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

// #include <QList>
// #include <QRegularExpression>
// #include <QString>
// #include <QStringList>
#include <QXmlQuery>
#include "homebanking.h"

const uint8_t Homebanking::keyForInternalFormat[Homebanking::accountNumberLength] =
    { 11, 12, 13, 14, 15, 16, 5, 6, 7, 8, 9, 4, 10, 2, 3, 1 };

const QList<HomebankingType> Homebanking::type = {
   HomebankingType(QStringLiteral(""), QStringLiteral("auto"), QString(), true),
   HomebankingType(QStringLiteral("074"), QStringLiteral("abo"), QStringLiteral("ABO"), true),
   HomebankingType(QStringLiteral(""), QStringLiteral("best"), QStringLiteral("BEST"), false),
   HomebankingType(QStringLiteral(""), QStringLiteral("citi"), QStringLiteral("CitiBank"), false),
   HomebankingType(QStringLiteral(""), QStringLiteral("gemini"), QStringLiteral("Gemini"), false),
   HomebankingType(QStringLiteral(":25:"), QStringLiteral("multicash"), QStringLiteral("MultiCash"), false) };

const QString HomebankingType::operator[](const int id) const {

    switch (id) {
        case 1: return identificationTokenInFile;
        case 2: return searchForThisStringInPluginName;
        case 3: return nameOfHomebankingTypeForDisplay;
        default: return (QString::number(id)); // error reporting
    }
}

const QString HomebankingDetails::extensionXMLTag = QStringLiteral("PriponaSouboruBV");
const QString HomebankingDetails::filePathXMLTag = QStringLiteral("AdresarBV");
const QString HomebankingDetails::bankCodeXMLTag = QStringLiteral("KodBanky");
const QString HomebankingDetails::accountFormatXMLTag = QStringLiteral("FormatCislaUctu");

QString HomebankingDetails::extractFileExtensions(const QString & inXMLConf) {

    QXmlQuery queryXMLConf;

    if (queryXMLConf.setFocus(inXMLConf)) {

        const QString xPathExpression = HomebankingDetails::buildXPathExpr(extensionXMLTag);
        queryXMLConf.setQuery(xPathExpression);
        QStringList listOfExtensions;

        if (queryXMLConf.isValid() && queryXMLConf.evaluateTo(&listOfExtensions))
            return listOfExtensions.join(' ');
    }

    // error encountered
    return QString();
}

QString HomebankingDetails::extractAdditionalDetails(const QString & inXMLConf, const QString & XMLTag) {

    QXmlQuery queryXMLConf;

    if (queryXMLConf.setFocus(inXMLConf)) {

        const QString xPathExpression = HomebankingDetails::buildXPathExpr(XMLTag);
        queryXMLConf.setQuery(xPathExpression);
        QString attribute;

        if (queryXMLConf.isValid() && queryXMLConf.evaluateTo(&attribute))
            return attribute.remove('\n');
    }

    // error encountered
    return QString();
}

BankAccount::Format HomebankingDetails::extractAccountFormat(const QString & inXMLConf) {

    QXmlQuery queryXMLConf;

    if (queryXMLConf.setFocus(inXMLConf)) {

        const QString xPathExpression = HomebankingDetails::buildXPathExpr(accountFormatXMLTag);
        queryXMLConf.setQuery(xPathExpression);
        QString format;

        if (queryXMLConf.isValid() && queryXMLConf.evaluateTo(&format)) {

            format.remove('\n');
            uint8_t formatPosition = 0;

            for (int i = 0; i < accountFormatIDs.size(); ++i)
                if (accountFormatIDs.at(i) == format)
                    formatPosition = i;

            return static_cast<BankAccount::Format>(formatPosition);
        }
    }

    // error encountered
    return BankAccount::NONE;
}

const QString HomebankingDetails::operator[](const int id) const {

    switch (id) {
        case 1: return name;
        case 2: return pluginName;
        case 3: return fileExtension;
        case 4: return filePath;
        case 5: return bankCode;
        default: return (QString::number(id)); // error reporting
    }
}

void Homebanking::separatePrefix(QString & accountNumber) const {

    const int8_t lengthDiff = accountNumber.length() - BankAccount::accountLength;

    if (lengthDiff > 0) {

        const QString emptyPrefix(lengthDiff, '0');
        if (!accountNumber.startsWith(emptyPrefix))
            accountNumber.insert(lengthDiff, '-');
    }
    return;
}

void Homebanking::convertFromInternalFormat(QString & accountNumber) const {

    const QString inInternalFormat = accountNumber;

    // change positions of individual digits according to specification
    uint8_t position = 0;
    for (QString::iterator it = accountNumber.begin(); it < accountNumber.end(); ++it)
        *it = inInternalFormat[Homebanking::keyForInternalFormat[position++]-1];

    return;
}

bool Homebanking::isBankCodeValid(const QString & bankCode) const {

    const QString bankCodeRegex = QStringLiteral("\\d{") +
        QString::number(BankAccount::bankCodeLength) + QStringLiteral("}");

    if (bankCode.length() != BankAccount::bankCodeLength ||
        !bankCode.contains(QRegularExpression(bankCodeRegex)))
        return false;

    return true;
}

void Homebanking::createAccount(BankAccount & bankAccount, QString & accountNo,
                                const QString & bankCode, const bool internalFormat) const {

    // convert account number from internal to static format
    if (internalFormat)
        convertFromInternalFormat(accountNo);
    separatePrefix(accountNo);

    // create account
    bankAccount.createAccount(accountNo, bankCode, static_cast<BankAccount::Format>(internalFormat+1));

    return;
}

bool Homebanking::searchForAccountNo_ABO(const QStringList * const fileContents,
                                         QString & account, QString & bankCode) const {

    const QString token = type[HomebankingType::ABO][HomebankingType::TOKEN];
    const int tokenLength = token.length();

    for (auto it: *fileContents) {

        int position = it.indexOf(token);
        if (position != -1) {

            const QString baseAccount = it.mid(position+tokenLength, Homebanking::accountNumberLength);
            if (baseAccount.length() < Homebanking::accountNumberLength || !containsValidChars(baseAccount))
                /* invalid account number */ continue;
            account = baseAccount;

            // search for bank code
            const uint8_t bankCodeOffset = 118;
            bankCode = it.mid(position+bankCodeOffset, BankAccount::bankCodeLength);
            if (!isBankCodeValid(bankCode))
                bankCode = QString();

            return true;
        }
    }
    return false;
}

bool Homebanking::searchForAccountNo_BEST(const QStringList * const fileContents,
                                          QString & account, QString & bankCode) const {
    return false;
}

bool Homebanking::searchForAccountNo_Citi(const QStringList * const fileContents,
                                         QString & account, QString & bankCode) const {
    return false;
}

bool Homebanking::searchForAccountNo_Gemini(const QStringList * const fileContents,
                                            QString & account, QString & bankCode) const {

    for (auto it: *fileContents) {

        const QString baseAccount = it.left(35).trimmed();
        if (!baseAccount.isEmpty() && containsValidChars(baseAccount)) {

            account = baseAccount;

            const uint8_t offset[] = { 72, 125, 175 };
            paymentType typeOfPayment = UNKNOWN;
            QPair<int16_t, int16_t> noOfDelim = { 0, 0 };

            if (it.at(offset[0]) == 'C' && it.at(offset[1]) == 'I' && it.at(offset[2]) == 'I')
                { typeOfPayment = CREDIT; noOfDelim.first = 3; } // received payment
            if (it.at(offset[0]) == 'D' && it.at(offset[1]) == 'O' && it.at(offset[2]) == 'O')
                { typeOfPayment = DEBIT; noOfDelim.first = 2; } // issued payment
            if (typeOfPayment == UNKNOWN)
                continue;

            noOfDelim.second = (noOfDelim.first+1)*2;
            uint8_t position = 1;
            const char delimiter = 4; // designates end of transaction

            for (auto it_row: it.mid(offset[2])) {

                if (it_row == delimiter) {

                    // extract bank code
                    if (--noOfDelim.first == 0)
                        bankCode = it.mid(offset[2]+position, BankAccount::bankCodeLength);

                    // extract bank account number
                    if (--noOfDelim.second == 0)
                        { account =  it.mid(offset[2]+position); position = 0; }
                    if (noOfDelim.second == -1)
                        { account.truncate(position-1); break; }
                }
                ++position;
            }

            if (!containsValidChars(account))
                { account = baseAccount; continue; }
            if (!isBankCodeValid(bankCode))
                { bankCode = QString(); continue; }

            return true;
        }
    }
    if (!account.isEmpty() && containsValidChars(account))
        return true;

    return false;
}

bool Homebanking::searchForAccountNo_MC(const QStringList * const fileContents,
                                        QString & account, QString & bankCode) const {

    const QString token = type[HomebankingType::MC][HomebankingType::TOKEN];
    const int tokenLength = token.length();

    for (auto it: *fileContents) {

        int position = it.indexOf(token);
        int start = 0, end = 0;

        if (position != -1) {

            end = it.indexOf(':', start = position+tokenLength);
            const QString baseAccount = it.mid(start, end-start);
            position = baseAccount.indexOf('/');
            account = baseAccount.mid(position+1);
            bankCode = baseAccount.left(position);
            if (!isBankCodeValid(bankCode))
                bankCode = QString();

            return true;
        }
    }
    return false;
}
