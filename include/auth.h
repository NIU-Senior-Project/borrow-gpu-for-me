#pragma once

#include <string>

// 共用密鑰認證。Token 來自環境變數 AUTH_TOKEN；三個元件（CLI/節點、Manager、
// GUI）必須設定相同的值。空字串代表未設定 = 認證停用（dev 模式）。

std::string auth_token();
bool auth_enabled();

// Constant-time 字串相等比較，避免以回應時間旁路推測 token。
bool constant_time_equals(const std::string& a, const std::string& b);

// 從原始 HTTP 標頭區塊取出 "Authorization: Bearer <token>" 並與設定比對。
// 認證停用時一律回傳 true。
bool is_request_authorized(const std::string& raw_headers);

// 給客戶端：認證啟用時回傳 "Authorization: Bearer <token>\r\n"，否則回傳 ""。
std::string auth_header_line();
