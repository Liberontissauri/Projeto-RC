
//!!!!!!!!! só nao percebo porque é que podemos receber um
//WRP do server num logout e num unregister tendo em conta que o utilizador nao coloca password nos comandos 
//logout e unregister

#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
using namespace std;

bool is_valid_uid(const string& uid) {
    if (uid.length() == 6 ) {
        for (char c: uid) {
            if(!isdigit(c)) {
                return false;
            }
        }
        return true;
    }
    
    else {
        return false;
    }
}

bool is_valid_password(const string& password) {
    if (password.length() == 8) {
        for (char c: password) {
            if(!isalnum(c)) {
                return false;
            }
        }
        return true;
    }

    else {
        return false;
    }
}

bool is_valid_port(const string& port) {
    for (char c: port) {
        if (!isdigit(c)) {
            return false;
        }
    }

    int converted_port = stoi(port);
    
    if (converted_port >= 1 && converted_port <= 65535) {
        return true;
    }
    else {
        return false;
    }
}

void process_server_reply(const string& reply, bool& logged_in) {
    stringstream ss(reply);
    string code, status;

    ss >> code >> status;

    if (code == "RLI") {
        if (status == "OK") {
            cout << "> successful login" << endl;
            logged_in = true;
        }
        else if (status == "NOK") {
            cout << "> incorrect login attempt" << endl;
        }
        else if (status == "REG") {
            cout << "> new user registered" << endl;
            logged_in = true;
        }
    }

    if (code == "RLO") {
        if (status == "OK") {
            cout << "> successful logout" << endl;
            logged_in = false;
        }
        else if (status == "NLG") {
            cout << "> user not logged in" << endl;
        }
        else if (status == "UNR") {
            cout << "> unknown user" << endl;
        }
    }

    if (code == "RUR") {
        if (status == "OK") {
            cout << "> successful unregister" << endl;
            logged_in = false;
        }
        else if (status == "NOK") {
            cout << "> incorrect unregister attempt" << endl;
        }
        else if (status == "UNR") {
            cout << "> unknown user" << endl;
        }
    }
}

int main(int argc, char* argv[]) {
    string peerport = "";
    string dsip = "192.168.1.1";
    string dsport = "59000";

    //all optional arguments
    if (argc == 7) {
        peerport = argv[2];
        dsip = argv[4];
        dsport = argv[6];
    }
    
    //only one optional argument is passed
    else if (argc == 5) {
        peerport = argv[2];
        string flag = argv[3];

        if (flag == "-n") {
            dsip = argv[4];
        }
        else if (flag == "-p") {
            dsport = argv[4];
        }
    }

    //no optional arguments
    else if (argc == 3) {
        peerport = argv[2];
    }

    else {
        return 0;
    }


    if (!is_valid_port(dsport)) {
        return 0;
    }

    string line;
    string command;
    string uid;
    string password;
    string message;
    bool logged_in = false;

    int fd,errcode; 
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1) exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(dsip.c_str(), dsport.c_str(), &hints, &res);
    if(errcode != 0) exit(1);

    while (true) {
        getline(cin, line);
        stringstream ss(line);
        ostringstream message_ss;
        string reply;
        ss >> command;
        
        if (command == "login") {
            ss >> uid;
            ss >> password;
            if (is_valid_uid(uid) && is_valid_password(password)) {
                message_ss << "LIN " << uid << " " << password << " " << peerport << endl;
                message = message_ss.str();
            }
            else continue;
        }
        
        else if (command == "unregister") {
                message_ss << "UNR " << uid << " " << password << endl;
                message = message_ss.str();
        }

        else if (command == "logout") {
                message_ss << "LOU " << uid << " " << password << endl;
                message = message_ss.str();
        }

        else if (command == "exit") {
            if (logged_in) {
                cout << "< you need to first logout" << endl;
                continue;
            }
            else {
                break;
            }
        }

        else continue;

        n = sendto(fd, message.c_str(), message.length(), 0, res->ai_addr, res->ai_addrlen);
        if(n == -1) exit(1);

        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, sizeof(buffer)-1, 0, (struct sockaddr*) &addr, &addrlen);
        if(n == -1) exit(1);
        buffer[n] = '\0';

        reply = buffer;

        process_server_reply(reply, logged_in);
    }

    freeaddrinfo(res);
    close(fd);
    return 1;
}