#pragma once
#include <string>
#include <vector>

struct ReplaceFileStruct {
    std::string filename;
    std::string replaceType;
    std::string details;
};

class ReplaceScanner {
public:
    static bool init();
    static bool destroy();
    static std::vector<ReplaceFileStruct> scan(const std::string& filePathOrName);
    static std::string getReplaceParserDir();

private:
    static std::string replaceParserDir;
    static std::string ToLower(const std::string& str);
    static bool WriteExeToTemp();
    static bool ExecuteReplaceParser();
    static std::string extractFileName(const std::string& filePath);
};