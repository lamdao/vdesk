#include <stdio.h>
#include "Database.h"
//----------------------------------------------------------------------------
// Table
//----------------------------------------------------------------------------
Table::Table( ifstream &dbFile )
{
	string value, label;

	getline( dbFile, value );
	Name = value.substr( 1, value.size()-1 );

	while( dbFile ) {
		dbFile >> label;

		if( label == "end" ) break;
		if( label[label.size()-1] == ':' ) {
			label = label.substr( 0, label.size()-1 );
			getline( dbFile, value );
			value = value.substr( 1, value.size()-1 );
			Set( label, value );
		}
	}
} 
//----------------------------------------------------------------------------
void Table::Set( string l, string v )
{
	int idx = QueryAsIndex( l );
	if( idx>=0 )
		Value[idx] = v;
	else {
		Label.push_back( l );
		Value.push_back( v );
		return;
	}
}
//----------------------------------------------------------------------------
void Table::Set( string l, int v )
{
	char s[32];
	sprintf( s, "%d", v );
	Set( l , string(s) );
}
//----------------------------------------------------------------------------
void Table::Set( string l, double v )
{
	char s[32];
	sprintf( s, "%.2f", v );
	Set( l , string(s) );
}
//----------------------------------------------------------------------------
void Table::Set( string l, bool v )
{
	Set( l , v ? string("true") : string("false") );
}
//----------------------------------------------------------------------------
string Table::Query( string l )
{
	for( int i=0; i<Label.size(); i++ )
		if( Label[i] == l ) return Value[i];

	return "";
}
//----------------------------------------------------------------------------
int Table::QueryAsIndex( string l )
{
	for( int i=0; i<Label.size(); i++ )
		if( Label[i] == l ) return i;

	return -1;
}
//----------------------------------------------------------------------------
int Table::QueryAsInt( string l, int d )
{
	string s = Query( l );
	if( s!="" )
		return atoi( s.c_str() );
	return d;
}
//----------------------------------------------------------------------------
bool Table::QueryAsBool( string l, bool b )
{
	string s = Query( l );
	if( s!="" )
		return !strcasecmp( s.c_str(), "true" );
	return b;
}
//----------------------------------------------------------------------------
double Table::QueryAsFloat( string l, double f )
{
	string s = Query( l );
	if( s!="" )
		return atof( s.c_str() );
	return f;
}
//----------------------------------------------------------------------------
char *Table::QueryAsStr( string l, char *s )
{
	for( int i=0; i<Label.size(); i++ )
		if( Label[i] == l ) return (char *)Value[i].c_str();

	return s;
}
//----------------------------------------------------------------------------
bool Table::Contains( string l )
{
	for( int i=0; i<Label.size(); i++ )
		if( Label[i] == l ) return true;

	return false;
}
//----------------------------------------------------------------------------
ofstream& operator << ( ofstream& os, const Table* t )
{
	os << "table " << t->Name << endl;
	for( int i=0; i<t->Label.size(); i++ )
		os << "  " << t->Label[i] << ": " << t->Value[i] << endl;
	os << "end\n\n";

	return os;
}
//----------------------------------------------------------------------------
// Database
//----------------------------------------------------------------------------
Database::Database( string f )
{
	ifstream dbFile( (File = f).c_str() );

	while( dbFile ) {
		string buffer;
		dbFile >> buffer;
		if( buffer == "table" )
			Tables.push_back( new Table( dbFile ) );
	}
	dbFile.close();
}
//----------------------------------------------------------------------------
Database::~Database()
{
	for( int i=0; i<Tables.size(); i++ )
		delete Tables[i];
}
//----------------------------------------------------------------------------
Table* Database::Query( string t, bool r )
{
	for( int i=0; i<Tables.size(); i++ )
		if( Tables[i]->GetName() == t ) {
			Table *t = Tables[i];
			if( r ) Tables.erase( Tables.begin() + i );
			return t;
		}

	return NULL;
}
//----------------------------------------------------------------------------
void Database::Add( Table *t )
{
	Tables.push_back( t );
}
//----------------------------------------------------------------------------
void Database::Save()
{
	ofstream dbFile( File.c_str() );
	for( int i=0; i<Tables.size(); i++ )
		dbFile << Tables[i];
	dbFile.close();
}
//----------------------------------------------------------------------------
void Database::SaveToFile( string dbf, Table *t )
{
	ofstream dbFile( dbf.c_str() );
	dbFile << t;
	dbFile.close();
}
//----------------------------------------------------------------------------
