#include "json.h"

#include <cctype>
#include <string>

std::string extract_json_field(const std::string& body, const std::string& key) {
    // ... (Keep the existing implementation)
    std::string searchKey = "\"" + key + "\"";
    auto keyPos = body.find(searchKey);
    if (keyPos == std::string::npos) return "";

    auto colonPos = body.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    auto firstQuote = body.find('"', colonPos + 1);
    if (firstQuote == std::string::npos) return "";

    auto secondQuote = body.find('"', firstQuote + 1);
    while (secondQuote != std::string::npos && body[secondQuote - 1] == '\\') {
        secondQuote = body.find('"', secondQuote + 1);
    }

    if (secondQuote == std::string::npos) return "";
    return body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

// Single pass so an escaped backslash (\\) is consumed as one unit and its
// following char is not mis-decoded. Mirrors escape_json in request.cpp.
std::string unescape_json(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[++i]) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                default: out += s[i]; break;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string query_param(const std::string& path, const std::string& key) {
    const auto q = path.find('?');
    if (q == std::string::npos) return "";
    const std::string query = path.substr(q + 1);

    size_t pos = 0;
    while (pos <= query.size()) {
        const auto amp = query.find('&', pos);
        const std::string pair =
            query.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        const auto eq = pair.find('=');
        // 比對完整的鍵名，避免 "uuid" 誤中 "id"
        if (eq != std::string::npos && pair.substr(0, eq) == key) {
            return pair.substr(eq + 1);
        }
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

bool is_valid_job_id(const std::string& id) {
    if (id.rfind("job_", 0) != 0) return false;  // 必須有 job_ 前綴
    if (id.size() <= 4) return false;            // 前綴後要有實際內容
    for (char c : id) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}
