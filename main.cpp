#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <glob.h>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
using namespace std;

string convertToLower(string str) {
    string result = "";
    for (int i = 0; i < str.length(); i++) {
        result += tolower(str[i]);
    }
    return result;
}

bool isSubstring(string s, string t) {
    if (t.length() < s.length()) return false;
    if (s.length() == t.length()) return s == t;
    if (s.length() == 0) return true;
    // both non0 AND t.lengh() > s.length()
    for (int i = 0; i < t.length(); i++) {
        if (t[i] != s[0]) continue;
        if (t.substr(i + 1).length() + 1 >= s.length() && t.substr(i, s.length()) == s) return true;
    }
    return false;
}

bool fileExists(string fileName) {
    struct stat fileInfo;
    return stat(fileName.c_str(), &fileInfo) == 0;
}

string getFileContents(string fileName) {
    if (!fileExists(fileName)) return "ERROR: The specified file doesn't exist.\n";
    
    ifstream myFile(fileName, ios::binary | ifstream::in);

    if (!myFile.is_open()) return "ERROR: Unable to successfully open the specified file.\n";

    char curChar = tolower(myFile.get());
    string fileContents = "";

    while (myFile.good()) {
        fileContents += curChar;
        curChar = tolower(myFile.get());
    }
    myFile.close();
    
    return fileContents;
}

vector<string> globVector(const string& pattern) {
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
    vector<string> files;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        files.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return files;
}

pair<bool, string> foundInside(string query, string fileName) {
    string fileContents = getFileContents(fileName);
    //cout << fileContents;
    bool isFound = false;
    bool fileErrorExists = fileContents.substr(0, 5) == "ERROR";
    if (!fileErrorExists) {
        isFound = isSubstring(convertToLower(query), fileContents);
    }
    
    pair<bool, string> result;
    result = make_pair(isFound, fileContents);
    return result;
}

int isDirectory(string fileName) {
    struct stat statbuf;
    if (stat(fileName.c_str(), &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

void bfs(string query, string parentDirectory, vector<string>& filesWithSearchTerm, vector<string>& contentOfFilesWithSearchTerm) {
    vector<string> directChildren = globVector(parentDirectory);
    
    for (int i = 0; i < directChildren.size(); i++) {
        string directChild = directChildren[i];
        cout << directChild << endl; // so that you know things are running and that the search isn't frozen if it takes some time
        if (!isDirectory(directChild)) { // process the current item (a file) in the current level
            pair<bool, string> foundInsideResult = foundInside(query, directChild);
            bool foundInside = foundInsideResult.first;
            string fileContents = foundInsideResult.second;
            if (foundInside) {
                filesWithSearchTerm.push_back(directChild);
                contentOfFilesWithSearchTerm.push_back(fileContents);
            }
        } else { // process the current item (a folder) in the current level
            bfs(query, directChild + "/*", filesWithSearchTerm, contentOfFilesWithSearchTerm);
        }
    }
}

pair<vector<string>, vector<string> > bfs(string query, string parentDirectory) {
    vector<string> filesWithSearchTerm;
    vector<string> contentOfFilesWithSearchTerm;
    bfs(query, parentDirectory, filesWithSearchTerm, contentOfFilesWithSearchTerm);
    pair<vector<string>, vector<string> > result;
    result = make_pair(filesWithSearchTerm, contentOfFilesWithSearchTerm);
    return result;
}

string formatData(vector<string>& files, vector<string>& contentOfFiles) {
    string formattedData = "";
    for (int i = 0; i < files.size(); i++) {
        string curFile = files[i];
        string contentOfCurFile = contentOfFiles[i];
        if (formattedData == "") {
            formattedData += to_string(i + 1) + ") " + curFile + "\n\n" + contentOfCurFile;
        } else {
            formattedData += "\n\n" + to_string(i + 1) + ") " + curFile + "\n\n" + contentOfCurFile;
        }
    }
    return formattedData;
}

void performSearch(string query, string parentDirectory) {
    pair<vector<string>, vector<string> > searchResult = bfs(query, parentDirectory);
    
    vector<string> filesWithSearchTerm = searchResult.first;
    vector<string> contentOfFilesWithSearchTerm = searchResult.second;
    
    if (filesWithSearchTerm.size() > 0) {
        string formattedData = formatData(filesWithSearchTerm, contentOfFilesWithSearchTerm);
        cout << "Result Found Inside:" << endl << formattedData << endl;
    } else {
        cout << "Result Not Found" << endl;
    }
}

void fileContentSearchHelper(string parentDirectory) {
    string query;
    
    cout << "What phrase do you want to search for? ";
    
    if (!std::getline(std::cin, query)) { /* I/O error! */ return; }
    
    if (!query.empty()) {
        if (parentDirectory[parentDirectory.length() - 1] != '/') {
            performSearch(query, parentDirectory + "/*");
        } else {
            performSearch(query, parentDirectory + "*");
        }
    } else {
        fileContentSearchHelper(parentDirectory);
    }
}

void fileContentSearch() {
    string parentDirectory;
    
    cout << "What folder do you want to look through? ";
    
    if (!std::getline(std::cin, parentDirectory)) { /* I/O error! */ return; }
    
    if (!parentDirectory.empty()) {
        fileContentSearchHelper(parentDirectory);
    } else {
        fileContentSearch();
    }
}

int main() {
    fileContentSearch();
    return 0;
}
