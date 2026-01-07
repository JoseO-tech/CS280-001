#include <iostream>
#include <fstream>
#include <cctype>

/*
 * CS280 - Fall 2025
 * RA 4: Driver Program for testing the implementation of Time class members
*/
#include "MTime.h"

using namespace std;

int main(int argc, char *argv[])
{
	ifstream  file ;
	bool eqflag=false, neqflag=false, gtflag=false, lteflag = false;
	bool reg = false;
	string str1;
	int t1[3], t2[3];
	MTime T1, T2;
//	MTime * TimePtr;
	
	if (argc == 1)
	{
		cerr << "NO SPECIFIED INPUT FILE NAME." << endl;
		return 0;
	}
	else 
	{
		
		file.open(argv[1]);
		if( file.is_open() == false ) {
			cerr << "CANNOT OPEN THE FILE " << argv[1] << endl;
			return 0;
		}
	
		for( int i = 2; i<argc; i++ ) {
			string arg( argv[i] );
			if( arg == "-eq" )
				eqflag = true;
			else if( arg == "-neq" )
				neqflag = true;
			else if( arg == "-gt" )
				gtflag = true;
			else if( arg == "-lte" )
				lteflag = true;
			else if( arg == "-std")
				reg = true;
			
			else {
				cerr << "UNRECOGNIZED FLAG " << arg << endl;
				return 0;
			}
		}
	}
	
	for(int i = 0; i < 3; i++)
	{
		file >> t1[i];
		
	}
	//cout << endl;
	for(int i = 0; i < 3; i++)
	{
		file >> t2[i];
		
	}
        
	T1.SetTime(t1[0], t1[1], t1[2]);
	T2.SetTime(t2[0], t2[1], t2[2]);
	MTime T3(t1[0], t1[1]);
	MTime T4(t2[0]);
        		
	if(reg)
	{
		cout<< "Using 12-Hour Standard Time:"<< endl;
		cout << "Timestamp 1: " << T1 << endl;
		cout << "Timestamp 2: " << T2 << endl;
		cout << "Timestamp 3: " << T3 << endl;
		cout << "Timestamp 4: " << T4 << endl;
	}
	else
	{
		cout<< "Using 24-Hour Time Format:"<< endl;
		cout << "Value of Timestamp 1: " ;
		T1.DisplayTime() ;
		cout << endl;
		cout << "Value of Timestamp 2: " ;
		T2.DisplayTime() ;
		cout<< endl;
		cout << "Value of Timestamp 3: " ;
		T3.DisplayTime() ;
		cout<< endl;
		cout << "Value of Timestamp 4: " ;
		T4.DisplayTime() ;
		cout<< endl;
	}
	//cout<< endl;
	if(eqflag || neqflag)
	{
		if(T1 == T2)
		{
			cout << "Timestamps 1 & 2 are equal." << endl;
		}
		else
		{
			cout << "Timestamps 1 & 2 are not equal." << endl;
		}
	}
	
	if(gtflag)
	{
		if(T1 > T2 )
		{
			cout << "Timestamp 1 is greater than Timestamp 2." << endl;
		}
		else
		{
			cout << "Timestamp 1 is not greater than Timestamp 2" << endl;
		}
	}
	
	if(lteflag)
	{
		if(T1 > T2 )
		{
			cout << "Timestamp 1 is not less than or equal to Timestamp 2." << endl;
		}
		else
		{
			cout << "Timestamp 1 is less than or equal to Timestamp 2" << endl;
		}
	}
	return 0;
}
