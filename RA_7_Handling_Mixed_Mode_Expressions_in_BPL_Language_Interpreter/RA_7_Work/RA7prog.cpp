#include <iostream>
#include <string>
#include <vector> 
using namespace std; 
#include "val.h"


void ExecuteMult(const Value& val1, const Value& val2) 
{ 
	cout << val1 << " * " << val2 << " is " <<  val1 * val2 << endl;
}

void ExecuteLThan(const Value& val1, const Value& val2) 
{ 
	cout << val1 << " < " << val2 << " is " <<  (val1 < val2) << endl;
}

void ExecuteSEQ(const Value& val1, const Value& val2) 
{ 
	cout << val1 << " @eq " << val2 << " is " <<  (val1.SEQ(val2))<< endl;
}

void ExecuteCatenate(const Value& val1, const Value& val2) 
{ 
	cout << val1 << " . " << val2 << " is " <<  (val1.Catenate(val2)) << endl; 
}

void ExecuteRepeat(const Value& val1, const Value& val2) 
{ 
	cout << val1 << " .x. " << val2 << " is " <<  (val1.Repeat(val2)) << endl; 
}
	
int main(int argc, char *argv[])
{
	Value ErrorVal;
	double num1 = 9.5;
	Value doubleVal1(num1);
	double num2 = 4;
	Value doubleVal2(num2);
	string str1 = "CS280";
	Value StrVal1(str1); 
	string str2 = "Fall 2025";
	Value StrVal2(str2);
	string str3 = "25";
	Value StrVal3(str3);
	bool b1 = true;
	Value bVal1(b1);
	bool b2 = false;
	Value bVal2(b2);
	
	vector<Value> vals( {ErrorVal, doubleVal1, doubleVal2, StrVal1, StrVal2, bVal1, bVal2, StrVal3} );
	bool mulflag = false, lthflag = false, seqflag = false, catflag = false, repflag = false;
	
	if (argc == 1)
	{
		cerr << "No Specified arguments." << endl;
		return 0;
	}
	
	string arg( argv[1] );
	if( arg == "-mul" )
		mulflag = true;
	else if( arg == "-lth" )
		lthflag = true;
	else if( arg == "-seq" )
		seqflag = true;		
	else if( arg == "-cat" )
		catflag = true;
	else if( arg == "-rep" )
		repflag = true;
	else {
			cerr << "UNRECOGNIZED FLAG " << arg << endl;
			return 0;
	}
		
	
	cout << "ErrorVal=" << ErrorVal << endl; 
		
	cout << "doubleVal1=" << doubleVal1 << endl; 
	cout << "doubleVal2=" << doubleVal2 << endl;
	cout << "StrVal1=" << "\"" << StrVal1 << "\"" << endl; 
	cout << "StrVal2=" <<  "\"" << StrVal2 << "\"" << endl;
	cout << "StrVal3=" <<  "\"" << StrVal3 << "\"" << endl;
	
	if(mulflag)
	{
		for(int i = 0; i < vals.size(); i++ ) 
			for( int j = 0; j < vals.size(); j++ )
			/*	if( i != j )*/
					ExecuteMult( vals[i], vals[j] );
	}
	
	if(lthflag)
	{
		for(int i = 0; i < vals.size(); i++ ) 
		for( int j = 0; j < vals.size(); j++ )
		/*	if( i != j )*/
				ExecuteLThan( vals[i], vals[j] );
	}	
	
	if(seqflag)
	{				
		for(int i = 0; i < vals.size(); i++ ) 
		for( int j = 0; j < vals.size(); j++ )
		/*	if( i != j )*/
				ExecuteSEQ( vals[i], vals[j] );
	}
	if(catflag)
	{
		for(int i = 0; i < vals.size(); i++ ) 
		for( int j = 0; j < vals.size(); j++ )
		/*	if( i != j )*/
				ExecuteCatenate( vals[i], vals[j] );
	}
	
	if(repflag)
	{
		for(int i = 0; i < vals.size(); i++ ) 
		for( int j = 0; j < vals.size(); j++ )
		/*	if( i != j )*/
				ExecuteRepeat( vals[i], vals[j] );
	}
	return 0;
}

