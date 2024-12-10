#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <filesystem>
#include "Replaceparser.h"

// Data Structures
struct ReplaceResult {
    std::string filename;
    std::string replaceType;
    std::string details;
};

// Utility Functions
std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// File Operations
bool WriteExeToTemp(const std::string& replaceParserDir) {
    std::string exePath = replaceParserDir + "\\replaceparser.exe";

    std::ofstream exeFile(exePath, std::ios::binary);
    if (!exeFile) {
        std::cerr << "Failed to create executable file: " << exePath << std::endl;
        return false;
    }

    exeFile.write(reinterpret_cast<const char*>(ReplaceParserHex), sizeof(ReplaceParserHex));
    exeFile.close();

    return true;
}

bool DeleteReplaceParserDir(const std::string& replaceParserDir) {
    try {
        std::filesystem::remove_all(replaceParserDir);
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error deleting directory " << replaceParserDir << ": " << e.what() << std::endl;
        return false;
    }
}

// Process Management
bool ExecuteReplaceParser(const std::string& replaceParserDir) {
    std::string exePath = replaceParserDir + "\\replaceparser.exe";
    std::string replacesTxtPath = replaceParserDir + "\\replaces.txt";
    std::string commandLine = "\"" + exePath + "\" \"" + replacesTxtPath + "\"";

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
        NULL,
        const_cast<char*>(commandLine.c_str()),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        replaceParserDir.c_str(),
        &si,
        &pi
    )) {
        std::cerr << "Failed to execute replaceparser.exe. Error: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

// Core Functionality
void FindReplace(const std::string& inputFileName, const std::string& replaceParserDir) {
    // Execute replace parser operations
    if (!WriteExeToTemp(replaceParserDir) || !ExecuteReplaceParser(replaceParserDir)) {
        return;
    }

    // Process results
    std::string logPath = replaceParserDir + "\\replaces.txt";
    std::string inputFileNameLower = ToLower(inputFileName);
    std::ifstream file(logPath);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << logPath << std::endl;
        return;
    }

    std::vector<ReplaceResult> results;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Parse replacement type and pattern
        std::string replaceType, pattern;
        if (line.rfind("Explorer replacement found in file: ", 0) == 0) {
            replaceType = "Explorer";
            pattern = "Explorer replacement found in file: ";
        }
        else if (line.rfind("Copy replacement found in file: ", 0) == 0) {
            replaceType = "Copy";
            pattern = "Copy replacement found in file: ";
        }
        else if (line.rfind("Type pattern found in file: ", 0) == 0) {
            replaceType = "Type";
            pattern = "Type pattern found in file: ";
        }
        else {
            continue;
        }

        // Extract and process filename
        size_t pos = line.find(pattern);
        if (pos == std::string::npos) continue;

        std::string foundFileName = line.substr(pos + pattern.size());
        std::string foundFileNameLower = ToLower(foundFileName);

        if (foundFileNameLower == inputFileNameLower) {
            // Collect replacement details
            std::string detailsLine;
            bool openBraceFound = false;
            std::string detailsCollected;

            while (std::getline(file, detailsLine)) {
                if (!openBraceFound) {
                    size_t bracePos = detailsLine.find('{');
                    if (bracePos != std::string::npos) {
                        openBraceFound = true;
                        if (bracePos + 1 < detailsLine.size()) {
                            detailsCollected += detailsLine.substr(bracePos + 1) + "\n";
                        }
                    }
                }
                else {
                    size_t closePos = detailsLine.find('}');
                    if (closePos != std::string::npos) {
                        if (closePos > 0) {
                            detailsCollected += detailsLine.substr(0, closePos);
                        }
                        break;
                    }
                    else {
                        detailsCollected += detailsLine + "\n";
                    }
                }
            }

            results.push_back({ foundFileName, replaceType, detailsCollected });
        }
    }

    file.close();

    // Display results
    if (!results.empty()) {
        for (const auto& res : results) {
            std::cout << std::endl << "Found replacement type: " << res.replaceType << std::endl;
            std::cout << std::endl << "In file: " << res.filename << std::endl;
            std::cout << std::endl << "Replacement details:" << std::endl;
            std::cout << std::endl << res.details << std::endl;
        }
    }
    else {
        std::cout << "No matches found for the specified file." << std::endl;
    }
}

int main() {
    // Get temporary directory path
    char tempPathBuffer[MAX_PATH];
    DWORD tempPathLen = GetTempPathA(MAX_PATH, tempPathBuffer);
    if (tempPathLen == 0 || tempPathLen > MAX_PATH) {
        std::cerr << "Failed to get temporary directory." << std::endl;
        return 1;
    }

    std::string tempPath = std::string(tempPathBuffer);
    if (tempPath.back() == '\\' || tempPath.back() == '/') {
        tempPath.pop_back();
    }

    // Create replace parser directory
    std::string replaceParserDir = tempPath + "\\replaceparser";
    if (!CreateDirectoryA(replaceParserDir.c_str(), NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            std::cerr << "Failed to create directory: " << replaceParserDir << std::endl;
            return 1;
        }
    }

    std::string fileName;
    char continueCheck;

    do {
        system("cls");
        std::cout << "Enter the filename (without path): ";
        std::getline(std::cin, fileName);

        FindReplace(fileName, replaceParserDir);

        std::cout << "\nDo you want to check another file? (y/n): ";
        std::cin >> continueCheck;
        std::cin.ignore();

    } while (tolower(continueCheck) == 'y');

    // Cleanup at the end
    if (!DeleteReplaceParserDir(replaceParserDir)) {
        std::cerr << "There was a problem deleting the replaceparser folder." << std::endl;
    }

    return 0;
}
