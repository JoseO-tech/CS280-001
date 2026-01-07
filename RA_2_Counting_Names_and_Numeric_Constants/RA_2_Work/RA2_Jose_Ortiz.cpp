#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

using namespace std;

static inline void strip_cr(string &s) {
    // Remove trailing CR or NL that can appear from Windows-style files
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

static inline bool isSpecialChar(char c) {
    return c == '_' || c == '&' || c == '.';
}

bool isName(const string &word) {
    if (word.empty()) return false;
    if (!isalpha(static_cast<unsigned char>(word[0]))) return false;

    for (size_t i = 1; i < word.size(); ++i) {
        char c = word[i];
        char prev = word[i - 1];
        if (isalnum(static_cast<unsigned char>(c))) continue;
        if (isSpecialChar(c)) {
            // no consecutive special characters allowed (including combinations)
            if (isSpecialChar(prev)) return false;
            continue;
        }
        // any other character invalidates a name
        return false;
    }
    return true;
}

bool isNumericConstant(const string &word) {
    if (word.empty()) return false;
    if (!isdigit(static_cast<unsigned char>(word[0]))) return false;

    bool hasDecimal = false;
    for (size_t i = 1; i < word.size(); ++i) {
        char c = word[i];
        if (isdigit(static_cast<unsigned char>(c))) continue;
        if (c == '.') {
            if (hasDecimal) return false;               // multiple dots not allowed
            if (i + 1 >= word.size()) return false;    // dot cannot be last
            if (!isdigit(static_cast<unsigned char>(word[i + 1]))) return false; // dot must be followed by digit
            hasDecimal = true;
            continue;
        }
        // any other character invalidates a numeric constant
        return false;
    }
    return true;
}

int main() {
    string fileName;
    cout << "Enter the file name to read from:" << endl;
    if (!(cin >> fileName)) return 0;

    ifstream inFile(fileName.c_str());
    if (!inFile) {
        cout << "CANNOT OPEN THE FILE: " << fileName << endl;
        return 0;
    }

    string line;
    long totalLines = 0;
    long nonBlankLines = 0;
    long totalWords = 0;
    long numNames = 0;
    long numNumericConstants = 0;
    bool anyLineRead = false;

    while (getline(inFile, line)) {
        anyLineRead = true;
        ++totalLines;

        // Per assignment note: a line having only whitespace characters is considered non-blank.
        // So a line is "blank" only when it's exactly empty (length == 0).
        if (!line.empty()) ++nonBlankLines;

        // Tokenize by whitespace (a "word" is any contiguous non-whitespace sequence)
        istringstream iss(line);
        string token;
        while (iss >> token) {
            strip_cr(token); // remove trailing CR if present
            ++totalWords;

            if (isName(token)) ++numNames;
            else if (isNumericConstant(token)) ++numNumericConstants;
        }
    }

    if (!anyLineRead) {
        cout << "File is empty." << endl;
        return 0;
    }

    cout << "Total Number of Lines: " << totalLines << endl;
    cout << "Number of non-blank lines: " << nonBlankLines << endl;
    cout << "Number of Words: " << totalWords << endl;
    cout << "Number of Numeric Constants: " << numNumericConstants << endl;
    cout << "Number of Names: " << numNames << endl;

    return 0;
}
