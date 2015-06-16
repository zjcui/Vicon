#include "sqlhandler.h"

SqlHandler::SqlHandler(){
	database = QSqlDatabase::addDatabase("QMYSQL");
}

bool SqlHandler::openDB(const QString &hostName, const QString &databaseName, const QString &userName, const QString &password){
	database.setHostName(hostName);
	database.setDatabaseName(databaseName);
	database.setUserName(userName);
	database.setPassword(password);
	return database.open();
}

QSqlQuery SqlHandler::queryCommand(const QString &queryCommand){
	QSqlQuery query;
	query.exec(queryCommand);
	return query;
}

void SqlHandler::closeDB( void )
{
	database.close();
}