/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

// #include <QString>
// #include <QUuid>
#include <QVariant>
#include <QXmlQuery>
#include "bank.h"

Bank::Bank(Query const * const query): id(query->value(ID).toUuid()),
    name(query->value(NAME).toString()), bankCode(query->value(CODE).toString()),
    swiftCode(query->value(SWIFT).toString()), countryCode(query->value(CCODE).toString()),
    countryName(query->value(CNAME).toString()), countryID(query->value(CID).toUuid()) {}

const QString Bank::operator[](const int id) const {

    switch (id) {
        case 1: return name;
        case 2: return bankCode;
        case 3: return swiftCode;
        case 4: return countryCode;
        case 5: return countryName;
        default: return (QString::number(id)); // error reporting
    }
}

const QString BankAccount::accountNoWithCode() const {

    QString accountNumber = QStringLiteral("nenalezeno");
    const QString code = (!bankCode.isEmpty()) ? bankCode : QStringLiteral("????");

    if (!accountNo.isEmpty())
        accountNumber = accountNo + '/' + code;

    return accountNumber;
}

QString BankAccount::generateIBAN() const {

    QString IBAN = QString();

    // country code (2 chars)
    IBAN += (*bank)[Bank::CCODE];
    // check number (2 chars)
    IBAN += "00";
    // bank code (4 chars)
    IBAN += bankCode;

    // bank account prefix (6 chars)
    QString prefix(BankAccount::prefixLength, '0');
    if (accountNo.contains('-')) {

        int pos = accountNo.indexOf('-');
        prefix.replace(BankAccount::prefixLength-pos, pos, accountNo.left(pos));
    }
    IBAN += prefix;

    // bank account number (10 chars)
    QString accountNumber = QString(BankAccount::accountLength, '0');
    QString accountNumberRaw = (accountNo.contains('-')) ?
        accountNo.mid(accountNo.indexOf('-') + 1) : accountNo;
    accountNumber.replace(BankAccount::accountLength-accountNumberRaw.length(),
                          accountNumberRaw.length(), accountNumberRaw);

    IBAN += accountNumber;
    IBAN.replace(2, 2, calculateCheckNumber(IBAN));

    return IBAN;
}

void BankAccount::createAccount(const QString & inAccountNo, const QString & inBankCode, const Format inFormat) {

    accountNo = removeLeadingZeros(inAccountNo);
    bankCode = inBankCode;
    accountFormat = inFormat;

    return;
}

// private method
QString BankAccount::calculateCheckNumber(QString IBAN) const {

    // move the four initial characters to the end of the string
    IBAN = IBAN.mid(4) + IBAN.left(4);

    // replace the letters in the string with digits, expanding the string as necessary,
    // such that A or a = 10, B or b = 11, and Z or z = 35
    for (QString::iterator it = IBAN.begin(); it < IBAN.end(); ++it)
        if (it->isLetter()) {

            const QString currentLetter = QString::number(it->toUpper().toLatin1()-55);
            IBAN.replace(*it, currentLetter);
        }

    // convert the string to an integer (i.e. ignore leading zeroes) and
    // calculate mod-97 of the new number, which results in the remainder
    IBAN = removeLeadingZeros(IBAN);
    int remainder = calculateMod97(IBAN);

    // subtract the remainder from 98, and use the result for the two check digits
    // if the result is a single digit number, pad it with a leading 0 to make it a two-digit number
    remainder = 98 - remainder;
    const QString checkDigits = (remainder < 10) ? '0' + QString::number(remainder)
                                                 : QString::number(remainder);

    return checkDigits;
}

// private method
int BankAccount::calculateMod97(QString transformedIBAN) const {

    // starting from the leftmost digit of D (transformedIBAN),
    // construct a number using the first 9 digits and call it N
    int N = transformedIBAN.left(9).toInt();
    int mod97 = N % 97;
    transformedIBAN = transformedIBAN.mid(9);

    // construct a new 9-digit N by concatenating above result with the next 7 digits of D
    while (transformedIBAN.length() >= 7) {

        N = (QString::number(mod97) + transformedIBAN.left(7)).toInt();
        mod97 = N % 97;
        transformedIBAN = transformedIBAN.mid(7);
    }

    // if there are fewer than 7 digits remaining in D but at least one,
    // then construct a new N, which will have less than 9 digits,
    // from the above result followed by the remaining digits of D
    if (transformedIBAN.length() >= 1) {

        N = (QString::number(mod97) + transformedIBAN).toInt();
        mod97 = N % 97;
    }

    return mod97;
}

// private method
QString BankAccount::removeLeadingZeros(QString inAccountNo) const {

    // from account number
    uint8_t countLeadingZeros = 0;

    for (auto it: inAccountNo)
        if (it == '0')
            ++countLeadingZeros;
        else break;

    inAccountNo = inAccountNo.remove(0, countLeadingZeros);

    // from prefix
    countLeadingZeros = 0;
    const int from = inAccountNo.indexOf('-')+1;

    for (auto it: inAccountNo.mid(from))
        if (it == '0')
            ++countLeadingZeros;
        else break;

    inAccountNo.remove(from, countLeadingZeros);

    return inAccountNo;
}

const QString BankStatementGroupSettings::hbConfXMLTag = QStringLiteral("Homebanking_ID");

BankStatementGroupSettings::BankStatementGroupSettings
    (const QUuid & inID, const QString & inName, const QUuid & inHBID):
    id(inID), name(inName), homebankingID(inHBID) {}

QUuid BankStatementGroupSettings::extractHomebankingID(const QString & configuration) {

    QXmlQuery queryXMLConf;

    if (queryXMLConf.setFocus(configuration)) {

        const QString xPathExpression =
            QStringLiteral("//") + hbConfXMLTag + QStringLiteral("/string()");
        queryXMLConf.setQuery(xPathExpression);
        QString homebanking_ID;

        if (queryXMLConf.isValid() && queryXMLConf.evaluateTo(&homebanking_ID))
            return QUuid(homebanking_ID);
    }

    // error encountered
    return QUuid();
}
