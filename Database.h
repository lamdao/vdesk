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
	Table( string name ): Name( name ) {}
	Table( ifstream& dbFile );

	string GetName() const { return Name; }

	void Set( string l, string v );
	void Set( string l, int v );
	void Set( string l, double v );
	void Set( string l, bool v );
	string Query( string l );
	int QueryAsIndex( string l );
	int QueryAsInt( string l, int d=0 );
	bool QueryAsBool( string l, bool b=false );
	double QueryAsFloat( string l, double f=0.0 );
	char *QueryAsStr( string l, char *s="" );

	bool Contains( string l );

	friend ofstream& operator << ( ofstream &os, const Table* t );
};
//----------------------------------------------------------------------------
class Database {
private:
	string File;
	vector<Table*> Tables;

public:
	Database( string f );
	~Database();

	Table *Query( string t, bool r=false );
	void Add( Table *t );
	void Save();

	static void SaveToFile( string dbf, Table *t );
};
//----------------------------------------------------------------------------
#endif
