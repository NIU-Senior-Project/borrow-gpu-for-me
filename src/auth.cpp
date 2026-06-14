#include "auth.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

std::string auth_token() {
    const char* t = std::getenv("AUTH_TOKEN");
    return t ? std::string(t) : std::string();
}

bool auth_enabled() {
    return !auth_token().empty();
}

bool constant_time_equals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return diff == 0;
}

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

// 解析 "Authorization: Bearer <token>"，大小寫不敏感地比對標頭名與 scheme。
std::string extract_bearer(const std::string& raw_headers) {
    std::istringstream stream(raw_headers);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        if (to_lower(line.substr(0, colon)) != "authorization") continue;

        const std::string value = trim(line.substr(colon + 1));
        const std::string scheme = "bearer ";
        if (value.size() > scheme.size() &&
            to_lower(value.substr(0, scheme.size())) == scheme) {
            return trim(value.substr(scheme.size()));
        }
    }
    return "";
}

}  // namespace

bool is_request_authorized(const std::string& raw_headers) {
    if (!auth_enabled()) return true;
    return constant_time_equals(extract_bearer(raw_headers), auth_token());
}

std::string auth_header_line() {
    if (!auth_enabled()) return "";
    return "Authorization: Bearer " + auth_token() + "\r\n";
}
