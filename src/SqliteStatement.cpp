//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "SqliteStatement.h"
//---
SqliteStatement::SqliteStatement(const SqliteDB &db){
	m_db=db.m_db;
	m_stmt=NULL;
}
//---
SqliteStatement::~SqliteStatement(){
	free();
}
//---
bool SqliteStatement::prepare(const char *str){
	return (sqlite3_prepare_v2(m_db,str,-1,&m_stmt,NULL)==SQLITE_OK);
}
//---
bool SqliteStatement::bindStr(int n,const char *str){
	return (sqlite3_bind_text(m_stmt,n,str,-1,NULL)==SQLITE_OK);
}
//---
bool SqliteStatement::bindInt(int n,int i){
	return (sqlite3_bind_int(m_stmt,n,i)==SQLITE_OK);
}
//---
bool SqliteStatement::bindInt64(int n,int64_t i){
	return (sqlite3_bind_int64(m_stmt,n,i)==SQLITE_OK);
}
//---
const char *SqliteStatement::getStr(int col){
	return (const char *)sqlite3_column_text(m_stmt,col);
}
//---
int SqliteStatement::getInt(int col){
	return sqlite3_column_int(m_stmt,col);
}
//---
int64_t SqliteStatement::getInt64(int col){
	return sqlite3_column_int64(m_stmt,col);
}
//---
int SqliteStatement::step(){
	int rc=sqlite3_step(m_stmt);
	if (rc==SQLITE_ROW)
		return 1;
	else if(rc==SQLITE_DONE)
		return 0;
	else
		return -1;
}
//---
void SqliteStatement::free(){
	if (m_stmt)
		sqlite3_finalize(m_stmt);
	m_stmt=NULL;
}
//---
void SqliteStatement::reset(){
	sqlite3_reset(m_stmt);
	sqlite3_clear_bindings(m_stmt);
}
