#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    string firstName, lastName;
    int sectionNumber;
    cout << "Welcome to CS 280 (Fall 2025)" << endl;
    cout << "What are your first name, last name, and section number?" << endl;
    cin >> firstName;
    cin >> lastName;
    cin >> sectionNumber;
    cout << "Hello " << firstName << ", Welcome to CS 280 Section " << sectionNumber << "." << endl;
    return 0;
}
