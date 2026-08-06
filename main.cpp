#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <functional>

using namespace std;

vector<string> parseArgs(const string& line) {
    vector<string> args;
    string current;
    bool inQuotes = false;
    string takeRestCommand = "ECHO";
    bool takeRest = false;

    for (char ch : line) {
        if (ch == '"' && !inQuotes) { inQuotes = true; }
        else if (ch == '"' && inQuotes) { inQuotes = false; }
        else if (ch == ' ' && !inQuotes && !takeRest) {
            if (!current.empty()) { 
                args.push_back(current); 
                
                if (current == takeRestCommand) {
                    takeRest = true;
                }

                current.clear(); }
        } else { current += ch; }
    }
    if (!current.empty()) args.push_back(current);
    return args;
}

string toUpper(string s) {
    for (auto& c : s) c = toupper(c);
    return s;
}

string encodeSimpleString(const string& s) {
    return "+" + s + "\r\n";
}

string encodeError(const string& s) {
    return "-" + s + "\r\n";
}

string encodeInteger(const int& n) {
    return ":" + to_string(n) + "\r\n";
}

string encodeBulkString(const string& s) {
    return "$" + to_string(s.size()) + "\r\n" + s + "\r\n";
}

string encodeNullBulkString() {
    return "$-1\r\n";
}

string encodeUnknownCommand(const string& s) {
    return encodeError("ERR unknown command '" + s + "'\r\n");
}


string handlePING(const vector<string>& args) {
    string response = "+PONG\r\n";

    if (args.size() > 1) {
        response = encodeBulkString(args[1]);
    }

    return response;
}

string handleECHO(const vector<string>& args) {
    string response = encodeNullBulkString();

    if (args.size() > 1) {
        response = encodeBulkString(args[1]);
    }

    return response;
}

string handleCOMMAND_DOCS(const vector<string>& args) {
    if (args.size() < 2 && args[1] != "DOCS") {
        return encodeError(args[0]);
    }

    return encodeSimpleString("OK");
}


string handleCommand(const vector<string>& args) {
    string cmd = toUpper(args[0]);
    string response = encodeUnknownCommand(cmd);

    unordered_map<string, function<string(const vector<string>&)>> handlers = {
        {"PING", handlePING},
        {"ECHO", handleECHO},
        {"COMMAND", handleCOMMAND_DOCS}
    };

    if (handlers.find(cmd) != handlers.end()) {
        response = handlers[cmd](args);
    }

    return response;
}

int main() {
    string line;
    while (getline(cin, line)) {
        // trim trailing \r
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        auto args = parseArgs(line);
        cout << handleCommand(args);
        cout.flush();
    }
    return 0;
}
