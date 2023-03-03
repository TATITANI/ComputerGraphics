#include "common.h"
#include <fstream>
#include <sstream>

// optional - 어떤 값이 있는지 없는지를 포인터 없이 반환
std::optional<std::string> LoadTextFile(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        SPDLOG_ERROR("failed to open file: {}", filename);
        return {};
    }
    std::stringstream text;
    text << fin.rdbuf();
    return text.str();
}