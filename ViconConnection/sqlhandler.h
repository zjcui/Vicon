#ifndef SQLHANDLER_H
#define SQLHANDLER_H

#include <QtSql>

class SqlHandler{
public:
	SqlHandler();
	bool openDB(const QString &hostName, const QString &databaseName, 
				const QString &userName, const QString &password);
	QSqlQuery queryCommand(const QString &queryCommand);

	void closeDB( void );

private:
	QSqlDatabase database;
};

#endif
