#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
using namespace std;

bool isSpecialChar(char c) {
    return c == '%' || c == '\\' || c == '&' || c == '{' || c == '}';
}

bool isEscaped(const string &s, int pos) {
    if (pos > 0 && s[pos - 1] == '\\') {
        int backslashCount = 0;
        int i = pos - 1;
        while (i >= 0 && s[i] == '\\') {
            backslashCount++;
            i--;
        }
        return backslashCount % 2 != 0;
    }
    return false;
}

int findMatchingBrace(const string &line, int start) {
    int braceCount = 1;
    for (int i = start + 1; i < (int)line.length(); ++i) {
        if (line[i] == '{') {
            braceCount++;
        } else if (line[i] == '}') {
            braceCount--;
            if (braceCount == 0) {
                return i;
            }
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "NO SPECIFIED INPUT FILE NAME." << endl;
        return 0;
    }

    string filename = argv[1];
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "CANNOT OPEN THE FILE: " << filename << endl;
        return 0;
    }

    infile.seekg(0, ios::end);
    if (infile.tellg() == 0) {
        cout << "The File is Empty." << endl;
        return 0;
    }
    infile.seekg(0, ios::beg);

    string line;
    int comments = 0, commands = 0, paragraphs = 0;
    int bolds = 0, italics = 0, underlines = 0;
    bool inParagraph = false;
    bool lastCommandNeedsBlock = false;

    while (getline(infile, line)) {
        if (!line.empty() && line.find_first_not_of(" \t\r\n") != string::npos) {
            if (!inParagraph) {
                paragraphs++;
                inParagraph = true;
            }
        } else {
            inParagraph = false;
        }

        for (int i = 0; i < (int)line.length(); ++i) {
            char c = line[i];

            if (c == '&' && !isEscaped(line, i)) {
                cout << "Error Message: Illegal use of a special character in text: &" << endl;
                continue;
            }

            if (c == '\\') {
                if (i + 1 < (int)line.length()) {
                    string commandName;
                    int j = i + 1;
                    while (j < (int)line.length() && isalpha(line[j])) {
                        commandName += tolower(line[j]);
                        j++;
                    }
                    if (commandName == "bf") {
                        bolds++;
                        commands++;
                        i = j - 1;
                        lastCommandNeedsBlock = true;
                    } else if (commandName == "it") {
                        italics++;
                        commands++;
                        i = j - 1;
                        lastCommandNeedsBlock = true;
                    } else if (commandName == "ul") {
                        underlines++;
                        commands++;
                        i = j - 1;
                        lastCommandNeedsBlock = true;
                    } else if (!commandName.empty()) {
                        cout << "Error Message: Illegal use of '\\' special character in text." << endl;
                        i = j - 1;
                        lastCommandNeedsBlock = false;
                    } else if (isSpecialChar(line[i + 1])) {
                        i++;
                        lastCommandNeedsBlock = false;
                    } else {
                        cout << "Error Message: Illegal use of '\\' special character in text." << endl;
                        lastCommandNeedsBlock = false;
                    }
                }
            } else if (c == '%') {
                if (!isEscaped(line, i)) {
                    comments++;
                    break;
                }
            } else if (c == '{') {
                if (!isEscaped(line, i)) {
                    if (lastCommandNeedsBlock) {
                        int endBraceIndex = findMatchingBrace(line, i);
                        if (endBraceIndex != -1) {
                            i = endBraceIndex;
                        } else {
                            cout << "Error Message: Missing closing brace for a block." << endl;
                            break;
                        }
                    } else {
                        cout << "Error Message: Illegal use of a special character in text: {" << endl;
                    }
                }
            } else if (c == '}') {
                if (!isEscaped(line, i)) {
                    if (lastCommandNeedsBlock) {
                        cout << "Error Message: Missing openning brace for a block." << endl;
                    } else {
                        cout << "Error Message: Illegal use of a special character in text: }" << endl;
                    }
                }
            }
        }

        lastCommandNeedsBlock = false;
    }

    cout << endl;

    cout << "Number of Comments: " << comments << endl;
    cout << "Number of Commands: " << commands << endl;
    cout << "Number of Paragraphs: " << paragraphs << endl;
    cout << "Bold commands: " << bolds << endl;
    cout << "Italic commands: " << italics << endl;
    cout << "Underline commands: " << underlines << endl;

    return 0;
}
