//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "SqliteDB.h"
//---
SqliteDB::SqliteDB(){
	m_db=NULL;
}
//---
SqliteDB::~SqliteDB(){
	close();
}
//---
bool SqliteDB::open(const char *file){
	return (sqlite3_open(file,&m_db)==SQLITE_OK);
}
//---
void SqliteDB::close(){
	if (m_db){
		sqlite3_close(m_db);
		m_db=NULL;
	}
}
//---
bool SqliteDB::exec(const char *str){
	return (sqlite3_exec(m_db,str,NULL,NULL,NULL)==SQLITE_OK);
}
//---
int SqliteDB::changes(){
	return sqlite3_changes(m_db);
}
