#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

// Membaca file template HTML
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Render variabel {{KEY}} ke HTML
std::string render(std::string html, const std::map<std::string, std::string>& data) {
    for (const auto& [key, value] : data) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = html.find(placeholder, pos)) != std::string::npos) {
            html.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return html;
}

int main() {
    // Header HTTP wajib untuk CGI
    std::cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n";

    std::string html = readFile("index.html");
    if (html.empty()) {
        std::cout << "<h1>Error: File index.html tidak ditemukan!</h1>";
        return 1;
    }

    std::map<std::string, std::string> vars = {
        {"PAGE_TITLE", "Router / Device Portal (CGI C++)"},
        {"SERVER_STATUS", "CGI Script Active"},
        {"HERO_TITLE", "C++ CGI Web Page"},
        {"HERO_SUBTITLE", "Dijalankan melalui Common Gateway Interface (CGI) berkinerja tinggi."},
        {"OS_INFO", "Embedded / Native"},
        {"UPTIME_SECONDS", "N/A"}
    };

    std::cout << render(html, vars);
    return 0;
}
