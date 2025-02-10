#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 4096

void error_exit(const std::string &msg) {
    std::cerr << msg << " Error: " << WSAGetLastError() << std::endl;
    WSACleanup();
    exit(1);
}

std::string resolve_hostname(const std::string &hostname) {
    addrinfo hints = {}, *res;
    hints.ai_family = AF_INET; // IPv4のみ
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0) {
        std::cerr << "Failed to resolve hostname: " << hostname << std::endl;
        return "";
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((sockaddr_in *)res->ai_addr)->sin_addr, ip_str,
              sizeof(ip_str));
    freeaddrinfo(res);

    return std::string(ip_str);
}
// サービス名（http など）をポート番号に解決
int resolve_service(const std::string &service) {
    addrinfo hints = {}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(nullptr, service.c_str(), &hints, &res) != 0) {
        std::cerr << "Failed to resolve service: " << service << std::endl;
        return -1;
    }

    int port = ntohs(((sockaddr_in*)res->ai_addr)->sin_port);
    freeaddrinfo(res);
    return port;
}


// サーバーモード（-l 指定時）
void run_server(std::string port, bool no_resolve) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        error_exit("WSAStartup failed");

	int resolved_port = -1;
	if (isdigit(port[0])) resolved_port = std::stoi(port);
	else if (!no_resolve) resolved_port = resolve_service(port);
	else {
		error_exit("You must specify a port as number");
	}
    if (resolved_port == -1) error_exit("Invalid port/service");

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) error_exit("Socket creation failed");

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(resolved_port);

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) ==
        SOCKET_ERROR)
        error_exit("Bind failed");

    if (listen(server_socket, 1) == SOCKET_ERROR) error_exit("Listen failed");

    std::cerr << "Listening on port " << resolved_port << "...\n";

    sockaddr_in client_addr;
    int client_size = sizeof(client_addr);
    SOCKET client_socket =
        accept(server_socket, (sockaddr *)&client_addr, &client_size);
    if (client_socket == INVALID_SOCKET) error_exit("Accept failed");

    std::cerr << "Connection received\n";

    _setmode(_fileno(stdout), _O_BINARY); // バイナリモードに設定

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        std::cout.write(buffer, bytes_received);
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}

// クライアントモード（-p 指定時）
void run_client(const std::string &host, std::string port, bool no_resolve) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        error_exit("WSAStartup failed");

	int resolved_port = -1;
	if (isdigit(port[0])) resolved_port = std::stoi(port);
	else if (!no_resolve) resolved_port = resolve_service(port);
	else {
		error_exit("You must specify a port as number");
	}
	if (resolved_port == -1) error_exit("Invalid port/service");

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) error_exit("Socket creation failed");

    std::string resolved_ip = host;
	if (!no_resolve) resolved_ip = resolve_hostname(host);
    if (resolved_ip.empty()) error_exit("Hostname resolution failed");

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, resolved_ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(resolved_port);

    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) ==
        SOCKET_ERROR)
        error_exit("Connect failed");

    std::cerr << "Connected to " << resolved_ip << ":" << port << "\n";

    _setmode(_fileno(stdin), _O_BINARY); // 標準入力をバイナリモードに変更

    std::string buffer;
    while (std::getline(std::cin, buffer)) { // 1行ずつ入力を取得
        buffer += "\n";                      // 改行を保持
        send(client_socket, buffer.c_str(), buffer.size(), 0);
    }

    closesocket(client_socket);
    WSACleanup();
}

void show_help(const char *progname) {
    std::cerr << "Usage:\n"
			  << "\t" << progname << " [-ln] [-p port] [host] [port]\n"
			  << "Options:\n"
			  << "\t-l\t\tListen mode\n"
			  << "\t-p port\t\tSpecify port\n"
			  << "\t-n\t\tDo not resolve hostname and port\n";
}

// メイン関数
int main(int argc, char *argv[]) {
    std::string host;
	std::string port;

    bool l = false;
	std::string p;
    bool n = false;

    auto has_next = [&argc](int i) -> bool { return i + 1 < argc; };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            arg = arg.substr(1);

            int require_value_cnt = 0;
            char require_value_opt = '\0';
            for (char c : arg) {
                switch (c) {
                case 'l': l = true; break;
                case 'p':
                    require_value_cnt++;
                    require_value_opt = c;
                    if (require_value_cnt > 1) {
                        std::cerr << "You cannot use -p and -"
                                  << require_value_opt << " like -p"
                                  << require_value_opt << std::endl;
                        return 1;
                    }
                    if (!has_next(i)) {
                        std::cerr << "Missing port number" << std::endl;
                        return 1;
                    }
                    p = argv[++i];
                    break;
                case 'n': n = true; break;
                default:
                    std::cerr << "Unknown option: -" << c << std::endl;
                    return 1;
                }
            }
        }
        else {
            if (host.empty()) { host = arg; }
            else if (port.empty()) { port = arg; }
            else {
                std::cerr << "Unknown argument: " << arg << std::endl;
                return 1;
            }
        }
    }

    if (l) { run_server(p, n); }
    else {
		if (host.empty() || port.empty()) {
			std::cerr << "Missing host or port" << std::endl;
			return 1;
		}
		run_client(host, port, n);
	}

    return 0;
}
