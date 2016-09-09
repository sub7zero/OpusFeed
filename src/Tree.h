//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef TREE_H
#define TREE_H

#include <tinyxml2.h>
using namespace tinyxml2;

#include <string>
#include <list>
#include <map>
#include <iostream>
#include "ByteArray.h"
using namespace std;

class Tree{
	public:
		Tree(Tree *parent=NULL);
		Tree(const string &name=string(),Tree *parent=NULL);
		Tree(const Tree &r);
		~Tree();
		Tree &operator=(const Tree &r);
		Tree *firstChild(const string &name=string());
		Tree *nextSibling(const string &name=string());
		void deleteChild(Tree *c);
		Tree *insertChild(const string &name);
		void deleteAttribute(const string &key);
		string getAttribute(const string &key);
		void setAttribute(const string &key,const string &val);
		string getName();
		void setName(const string &str);
		string getText();
		void setText(const string &str);
		void deleteChildren();
		void deleteAttributes();
		//-
		bool fromXML(const ByteArray &data);
		ByteArray toXML();
	private:
		static void parseXMLElement(XMLElement *e,Tree *t);
		static void formatXMLElement(XMLElement *e,Tree *t);
	private:
		string m_name;
		string m_text;
		map<string,string> m_attributes;
		list<Tree*> m_children;
		Tree *m_parent;
};
#endif // TREE_H
