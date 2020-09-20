/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

// #include <QDateTime>
// #include <QSqlDatabase>
// #include <QSqlQuery>
// #include <QString>
#include "database.h"

const QString LogDB::lineBreak = QStringLiteral("<br>");
const QString LogDB::openTagQueryNo = QStringLiteral("<span style=\"font-weight:bold\">");
const QString LogDB::openTagError = QStringLiteral("<span style=\"color:red\">");
const QString LogDB::openTagCommand = QStringLiteral("<span style=\"color:blue\">");
const QString LogDB::closeTag = QStringLiteral("</span>");

LogDB::LogDB(const QString & inRequest, const QString & inError, const bool notEmpty):
    timeOfRequest(QDateTime::currentDateTime()), error(inError), isEmpty(!notEmpty) {

    QString lessThanEscaped = inRequest;
    request = lessThanEscaped.replace('<',"&lt;");
}

QString LogDB::requestAndErrorDescription(const uint16_t lineNo, const bool onlyErrors) const {

    const bool error = !(this->error.isEmpty());
    // if report only errors is enabled
    if (!error && onlyErrors)
        return QString();

    const QString lineNoStr = QString::number(lineNo);
    QString completeDescription = QStringLiteral("[0000] ");
    completeDescription.replace(completeDescription.length()-lineNoStr.length()-2,
                                lineNoStr.length(), lineNoStr);
    completeDescription = openTagQueryNo + completeDescription + closeTag;

    if (error)
        completeDescription += openTagError;
    completeDescription +=
        timeOfRequest.toString("dd.MM.yyyy hh:mm:ss.zzz ");
    if (error)
        completeDescription += closeTag;

    completeDescription += request;

    if (isEmpty)
        completeDescription += QStringLiteral(" (prázdná množina)");

    if (error)
        completeDescription +=
            openTagError + QStringLiteral(" (") + this->error + QStringLiteral(")") + closeTag;

    completeDescription += lineBreak;

    // colour sql commands
    int position = 0;
    for (auto it: sqlCommands)
        while ((position = completeDescription.indexOf(it, position+openTagCommand.length()+1)) != -1) {

        completeDescription.insert(position+it.size(), closeTag);
        completeDescription.insert(position, openTagCommand);
    }

    return completeDescription;
}

Query::Query(QSqlDatabase db, const QString & inQueryString, bool forwardOnly):
    QSqlQuery(db), queryString(inQueryString), querySuccessful(false) {

    this->setForwardOnly(forwardOnly);
    emptyResult = this->prepare(queryString);
}

bool Query::nextRecord() {

    const bool nextRecordExists = this->next();
    if (nextRecordExists)
        emptyResult = false;

    return (nextRecordExists);
}
