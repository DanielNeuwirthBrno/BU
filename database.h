/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QVector>

namespace {

    const QStringList sqlCommands =
        { " AND", " AS", " BY", " CASE", "CAST", " ELSE", " END", " FROM",
          " INNER", "INSERT", " INTO", " ISNULL", " JOIN", " LEFT", " LIKE",
          " MAX", " NOT", " ON", " ORDER", "SELECT", " SET", " THEN", " TOP",
          "UPDATE", " VALUES", " WHEN", " WHERE" };
}

class Profile {

    public:
        Profile(const uint16_t timeout = 0):
            driverName(QStringLiteral("QODBC3")),
            connectionName(QStringLiteral("connectionToMSSQL")),
            connectionTimeout(timeout) {}
        ~Profile() {}

        inline QString getDriverName() const
            { return driverName; }
        inline QString getConnectionName() const
            { return connectionName; }
        inline uint16_t getConnectionTimeout() const
            { return connectionTimeout; }

   private:
        const QString driverName;
        const QString connectionName;
        const uint16_t connectionTimeout;

};

class LogDB {

    public:
        const static QString lineBreak;
        const static QString openTagQueryNo;
        const static QString openTagError;
        const static QString openTagCommand;
        const static QString closeTag;

        LogDB() {}
        LogDB(const QString &, const QString &, const bool = true);
        ~LogDB() {}

        inline QString getError() const { return error; }
        QString requestAndErrorDescription(const uint16_t, const bool = false) const;

    private:
        QString request;
        QDateTime timeOfRequest;
        QString error;
        bool isEmpty;
};

class Query: public QSqlQuery {

    public:
        Query(): QSqlQuery() {}
        Query(QSqlDatabase, const QString &, bool = false);
        ~Query() {}

        inline bool isProcessed() const { return (querySuccessful); }
        inline bool isNotEmpty() const { return (!emptyResult); }
        inline bool isProcessedAndNotEmpty() const { return isProcessed() && isNotEmpty(); }
        inline const QString & getQuery() const { return (queryString); }
        inline void execute() {

            querySuccessful = this->exec();
            emptyResult = (this->isSelect()) ? querySuccessful : false;
            return;
        }
        bool nextRecord();

    private:
        QString queryString;
        bool querySuccessful;
        bool emptyResult;
};

class Database {

    public:
        Database(): showOnlyErrors(false), dbConnectionProfile(new Profile()),
                    dbConnection(new QSqlDatabase()) {}
        ~Database() { delete dbConnectionProfile; delete dbConnection; }

        inline void addNewDatabaseConnection(const QSqlDatabase newDatabase)
            { *dbConnection = newDatabase; return; }
        inline bool openDB() const
            { return (dbConnection->isOpen()) ? true : dbConnection->open(); }
        inline void closeDB() const
            { if (dbConnection->isOpen()) { dbConnection->close(); } return; }
        inline QSqlDatabase * getDatabaseConnection() const
            { return dbConnection; }
        inline Profile * getConnectionProfile() const
            { return dbConnectionProfile; }
        inline void writeToLog(const Query * const query)
            { writeToLog(query->getQuery(), query->lastError(), query->isNotEmpty()); return; }
        inline void writeToLog(const QString & request, const QSqlError & error, const bool notEmpty = true)
            { log.push_back(LogDB(request, error.text().trimmed(), notEmpty)); return; }
        inline QString lastError() const
            { return log.last().getError(); }
        inline const QVector<LogDB> & logContents() const { return log; }
        inline bool getShowOnlyErrors() const { return showOnlyErrors; }

    private:
        QVector<LogDB> log;
        bool showOnlyErrors;
        Profile * const dbConnectionProfile;
        QSqlDatabase * const dbConnection;
};

#endif // DATABASE_H
