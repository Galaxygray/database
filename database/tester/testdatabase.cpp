#include "testdatabase.h"

testdatabase::testdatabase()
{
}
void testdatabase::trytoInitializeTable(Table* temp)
{
    vector<string> clname;
    vector<DataBaseType*> cltype;
    clname.push_back("userid");
    clname.push_back("password");
    clname.push_back("age");
    string t1 = "INTE";
    string t2 = "CHAR";
    string t3 = "INTE";
    DataBaseType* type1 = UIC::reconvert(t1.data(), 20, true);
    DataBaseType* type2 = UIC::reconvert(t2.data(), 20, true);
    DataBaseType* type3 = UIC::reconvert(t3.data(), 2, true);
    cltype.push_back(type1);
    cltype.push_back(type2);
    cltype.push_back(type3);
    temp->createTable(clname, cltype);
    temp->Initialize();
}

void testdatabase::begintest()
{
    cout << "test database begin" << endl;
    string dbname = "Mydb";
    string filename = "database.db";
    remove(filename.c_str());
    string table1 = "tb1.tb";
    string table2 = "tb2.tb";
    Database onedb;
    onedb.setfilename(filename);
    onedb.setname(dbname);
    onedb.Initialize();
    Table* t1 = new FixedSizeTable();
    t1->setfilename(table1);
    t1->setname(table1);
    Table* t2 = new FixedSizeTable();
    t2->setfilename(table2);
    t1->setname(table2);
    onedb.addTable(t1);
    onedb.addTable(t2);
    trytoInitializeTable(t1);
    trytoInitializeTable(t2);
    onedb.removeTable(0);
    cout << "test database end" << endl;
}
