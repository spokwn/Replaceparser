#include "replaceparser\ReplaceScanner.hh"
#include <iostream>

int main() {
    if (!ReplaceScanner::init()) {
        std::cerr << "Failed to initialize ReplaceScanner" << std::endl;
        return 1;
    }

    std::string fileInput;
    char continueCheck;

    do {
        system("cls");
        std::cout << "Enter the filename or filepath: ";
        std::getline(std::cin, fileInput);

        auto results = ReplaceScanner::scan(fileInput);

        if (results.empty()) {
            std::cout << "No matches found for the specified file." << std::endl;
        }
        else {
            for (const auto& res : results) {
                std::cout << "\nFound replacement type: " << res.replaceType;
                std::cout << "\nIn file: " << res.filename;
                std::cout << "\nReplacement details:\n" << res.details << std::endl;
            }
        }

        std::cout << "\nDo you want to check another file? (y/n): ";
        std::cin >> continueCheck;
        std::cin.ignore();

    } while (tolower(continueCheck) == 'y');

    ReplaceScanner::destroy();
    return 0;
}