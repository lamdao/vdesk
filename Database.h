#ifndef __DATABASE_H
#define __DATABASE_H
//----------------------------------------------------------------------------
using namespace std;
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//----------------------------------------------------------------------------
class Table {
private:
	string Name;

	vector<string> Label;
	vector<string> Value;
public:
	Table( const string& name ): Name( name ) {}
	Table( ifstream& dbFile );

	string GetName() const { return Name; }

	void Set( const string &l, const string &v );
	void Set( const string &l, int v );
	void Set( const string &l, double v );
	void Set( const string &l, bool v );

	string Query( const string &l );
	int QueryAsIndex( const string &l );
	int QueryAsInt( const string &l, int d=0 );
	bool QueryAsBool( const string &l, bool b=false );
	double QueryAsFloat( const string &l, double f=0.0 );
	char *QueryAsStr( const string &l, char *s="" );

	bool Contains( const string &l );

	friend ofstream& operator << ( ofstream &os, const Table* t );
};
//----------------------------------------------------------------------------
class Database {
private:
	string File;
	vector<Table*> Tables;

public:
	Database( const string &f );
	~Database();

	Table *Query( const string &t, bool r=false );
	void Add( Table *t );
	void Save();

	static void SaveToFile( const string &dbf, Table *t );
};
//----------------------------------------------------------------------------
#endif
