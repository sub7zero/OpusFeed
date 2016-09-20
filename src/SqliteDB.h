//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <stdio.h>
#include "sqlite3.h"

class SqliteStatement;

class SqliteDB{
	friend SqliteStatement;
	public:
		SqliteDB();
		~SqliteDB();
		bool open(const char *file);
		void close();
		bool exec(const char *str);

		sqlite3 *db(){return m_db;}
	private:
		sqlite3 *m_db;
};

#endif // SQLITEDB_H
