#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <map>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define CLOSE_SOCKET(s) close(s)
#endif

// Waktu server mulai
auto startTime = std::chrono::steady_clock::now();

// Fungsi untuk mendapatkan uptime dalam detik
long long getUptimeSeconds() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
}

// Deteksi OS
std::string getOSName() {
#ifdef _WIN32
    return "Windows (UCRT64/x64)";
#elif defined(__linux__)
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Generic POSIX";
#endif
}

// Fungsi pembaca file
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Template Engine sederhana untuk mengganti placeholder {{KEY}}
std::string renderTemplate(std::string html, const std::map<std::string, std::string>& data) {
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

// Generator HTML dengan variabel dinamis
std::string getPageContent() {
    std::string templateHtml = readFile("index.html");
    if (templateHtml.empty()) {
        templateHtml = "<h1>404: Template index.html tidak ditemukan!</h1>";
    }

    std::map<std::string, std::string> vars = {
        {"PAGE_TITLE", "Dashboard C++ Web Template"},
        {"SERVER_STATUS", "Online (C++ Engine)"},
        {"HERO_TITLE", "Web Template Berbasis C++"},
        {"HERO_SUBTITLE", "Template responsif dan modern berkecepatan tinggi yang disajikan langsung menggunakan backend native C++."},
        {"OS_INFO", getOSName()},
        {"UPTIME_SECONDS", std::to_string(getUptimeSeconds())}
    };

    return renderTemplate(templateHtml, vars);
}

// Format respon HTTP
std::string createHttpResponse(int statusCode, const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    std::string statusMessage = (statusCode == 200) ? "OK" : "Not Found";
    
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    response << "Content-Type: " << contentType << "; charset=utf-8\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "Access-Control-Allow-Origin: *\r\n\r\n";
    response << body;
    return response.str();
}

int main(int argc, char* argv[]) {
    // Mode Ekspor Statis: jika dipanggil dengan argumen --export atau --generate
    if (argc > 1 && (std::string(argv[1]) == "--generate" || std::string(argv[1]) == "--export")) {
        std::cout << "[C++] Melakukan render template ke 'rendered.html'..." << std::endl;
        std::string rendered = getPageContent();
        std::ofstream out("rendered.html");
        out << rendered;
        out.close();
        std::cout << "[C++] Berhasil! File tersimpan di 'rendered.html'" << std::endl;
        return 0;
    }

    const int PORT = 8080;

#ifdef _WIN32
    // Inisialisasi Winsock untuk Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Gagal menginisialisasi Winsock!" << std::endl;
        return 1;
    }
#endif

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Gagal membuat socket!" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Set reuse address
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Gagal melakukan bind pada port " << PORT << "!" << std::endl;
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Gagal mendengarkan koneksi!" << std::endl;
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "=================================================" << std::endl;
    std::cout << "  C++ Web Server & Template Engine Siap!" << std::endl;
    std::cout << "  Buka di browser: http://localhost:" << PORT << std::endl;
    std::cout << "  Tekan Ctrl+C di terminal untuk berhenti." << std::endl;
    std::cout << "=================================================" << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        socket_t clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, 
#ifdef _WIN32
            &clientLen
#else
            (socklen_t*)&clientLen
#endif
        );

        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        std::vector<char> buffer(4096);
        int bytesReceived = recv(clientSocket, buffer.data(), buffer.size() - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string request(buffer.data());

            // Ekstrak URL method
            std::istringstream reqStream(request);
            std::string method, path, protocol;
            reqStream >> method >> path >> protocol;

            std::string response;

            if (path == "/" || path == "/index.html") {
                std::string page = getPageContent();
                response = createHttpResponse(200, "text/html", page);
            } else if (path == "/api/status") {
                // Endpoint REST API JSON
                std::ostringstream json;
                json << "{"
                     << "\"status\":\"Online (C++ Engine)\","
                     << "\"uptime\":" << getUptimeSeconds() << ","
                     << "\"os\":\"" << getOSName() << "\""
                     << "}";
                response = createHttpResponse(200, "application/json", json.str());
            } else {
                response = createHttpResponse(404, "text/plain", "404 Not Found");
            }

            send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
        }

        CLOSE_SOCKET(clientSocket);
    }

    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
