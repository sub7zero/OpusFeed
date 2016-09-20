//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef SQLITESTATEMENT_H
#define SQLITESTATEMENT_H

#include "SqliteDB.h"
#include <inttypes.h>

class SqliteStatement{
	public:
		SqliteStatement(const SqliteDB &db);
		~SqliteStatement();
		bool prepare(const char *str);
		bool bindStr(int n,const char *str);
		bool bindInt(int n,int i);
		bool bindInt64(int n,int64_t i);
		const char *getStr(int col);
		int getInt(int col);
		int64_t getInt64(int col);
		int step(); // 1 : row, 0 : done , -1 : error
		void free();
		void reset();
	private:
		sqlite3 *m_db;
		sqlite3_stmt *m_stmt;
};

#endif // SQLITESTATEMENT_H
