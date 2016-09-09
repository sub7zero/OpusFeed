//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "Tree.h"
//---
Tree::Tree(const string& name,Tree *parent){
	m_name=name;
	m_parent=parent;
}
//---
Tree::Tree(Tree *parent){
	m_parent=parent;
}
//---
Tree::Tree(const Tree &r){
	m_parent=NULL;
	m_name=r.m_name;
	m_text=r.m_text;
	m_attributes.insert(r.m_attributes.begin(),r.m_attributes.end());
	list<Tree*>::const_iterator i=r.m_children.begin();
	while(i!=r.m_children.end()){
		Tree *e=new Tree(**i);
		e->m_parent=this;
		m_children.push_back(e);
		i++;
	}
}
//---
Tree::~Tree(){
	#ifdef DEBUG
	string out=m_name;
	Tree *p=m_parent;
	while (p!=NULL){
		out=p->m_name+" > "+out;
		p=p->m_parent;
	}
	cout<<"DELETING TREE : "<<out<<endl;
	#endif
	deleteChildren();
}
//---
Tree &Tree::operator=(const Tree &r){
	deleteChildren();
	deleteAttributes();
	m_name=r.m_name;
	m_text=r.m_text;
	m_attributes.insert(r.m_attributes.begin(),r.m_attributes.end());
	list<Tree*>::const_iterator i=r.m_children.begin();
	while(i!=r.m_children.end()){
		Tree *e=new Tree(**i);
		e->m_parent=this;
		m_children.push_back(e);
		i++;
	}
	return *this;
}
//---
Tree *Tree::firstChild(const string &name){
	if (name.empty())
		return m_children.empty()?NULL:*m_children.begin();
	list<Tree*>::iterator i=m_children.begin();
	while(i!=m_children.end()){
		if ((*i)->m_name==name)
			return *i;
		i++;
	}
	return NULL;
}
//---
Tree *Tree::nextSibling(const string& name){
	if (m_parent==NULL)
		return NULL;
	list<Tree*>::iterator i=m_parent->m_children.begin();
	while(i!=m_parent->m_children.end()){
		if ((*i)==this){
			i++;
			while(i!=m_parent->m_children.end()){
				if (name.empty() || (*i)->m_name==name)
					return *i;
				i++;
			}
			return NULL;
		}
		i++;
	}
	return NULL;
}
//---
void Tree::deleteChild(Tree* c){
	if (c==NULL)
		return;
	m_children.remove(c);
	delete c;
}
//---
Tree *Tree::insertChild(const string& name){
	Tree *e=new Tree(name,this);
	m_children.push_back(e);
	return e;
}
//---
void Tree::setAttribute(const string& key,const string& val){
	m_attributes[key]=val;
}
//---
string Tree::getAttribute(const string& key){
	map<string,string>::iterator i=m_attributes.find(key);
	if (i!=m_attributes.end())
		return i->second;
	return string();
}
//---
void Tree::deleteAttribute(const string& key){
	m_attributes.erase(key);
}
//---
string Tree::getName(){
	return m_name;
}
//---
void Tree::setName(const string &str){
	m_name=str;
}
//---
string Tree::getText(){
	return m_text;
}
//---
void Tree::setText(const string &str){
	m_text=str;
}
//---
void Tree::deleteChildren(){
	list<Tree*>::iterator i=m_children.begin();
	while(i!=m_children.end()){
		delete *i;
		i++;
	}
	m_children.clear();
}
//---
void Tree::deleteAttributes(){
	m_attributes.clear();
}
//---
bool Tree::fromXML(const ByteArray &data){
	XMLDocument doc;
	if (doc.Parse(data.buffer(),data.size())!=XML_NO_ERROR)
		return false;
	XMLElement *element=doc.RootElement();
	if (element)
		parseXMLElement(element,this);
	return true;
}
//---
ByteArray Tree::toXML(){
	XMLDocument doc;
	XMLElement *element=doc.NewElement("");
	doc.InsertFirstChild(element);
	formatXMLElement(element,this);
	XMLPrinter printer;
	doc.Print(&printer);
	ByteArray data;
	data.append(printer.CStr(),printer.CStrSize()-1);
	return data;
}
//---
void Tree::formatXMLElement(XMLElement *e,Tree *t){
	e->SetName(t->m_name.c_str());
	e->SetText(t->m_text.c_str());
	map<string,string>::iterator i=t->m_attributes.begin();
	while(i!=t->m_attributes.end()){
		e->SetAttribute(i->first.c_str(),i->second.c_str());
		i++;
	}
	XMLDocument *doc=e->GetDocument();
	list<Tree*>::iterator l=t->m_children.begin();
	while(l!=t->m_children.end()){
		XMLElement *n=doc->NewElement("");
		e->InsertEndChild(n);
		formatXMLElement(n,*l);
		l++;
	}
}
//---
void Tree::parseXMLElement(XMLElement *e,Tree *t){
	t->m_name=e->Name();
	const char *text=e->GetText();
	if (text==NULL)
		t->m_text.clear();
	else
		t->m_text=text;
	const XMLAttribute *a=e->FirstAttribute();
	if (a){
		do{
			t->m_attributes[string(a->Name())]=string(a->Value());
		}while((a=a->Next()));
	}
	XMLElement *child=e->FirstChildElement();
	if (child){
		do{
			Tree *n=new Tree(t);
			t->m_children.push_back(n);
			parseXMLElement(child,n);
		}while((child=child->NextSiblingElement()));
	}
}
