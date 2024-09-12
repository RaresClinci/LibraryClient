#include <iostream>      /* printf, sprintf */
#include <cstdlib>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <cstring>
#include <cctype>
#include <algorithm>
#include "json.hpp"
#include "helpers.h"
#include "requests.h"

using namespace std;
using json = nlohmann::json;

#define IP_SERVER "34.246.184.49"
#define PORT_SERVER 8080
#define COMMAND_SIZE 100
#define NUM_DIGITS 20

char* loginCookie;
char* libraryToken;

class Book {
    public:
        int id;
        string title;
        string author;
        string publisher;
        string genre;
        int page_count;

        Book(int _id, string _title) {
            id = _id;
            title = _title;
        }

        Book(string _title, string _author, string _publisher,
                string _genre, int _page_count) {
            title = _title;
            author = _author;
            publisher = _publisher;
            genre = _genre;
            page_count = _page_count;
        }

        char* getJSON() {
            json book;
            book["title"] = title;
            book["author"] = author;
            book["genre"] = genre;
            book["page_count"] = page_count;
            book["publisher"] = publisher;

            char *book_str;
            book_str = (char*)malloc(book.dump().length() + 1); 
            strcpy(book_str, book.dump().c_str());

            return book_str;
        }

};

bool get_status(string resp, string code, string success, string fail) {
    // finding the status line
    size_t status_line_end = resp.find("\r\n");
    string status_line = resp.substr(0, status_line_end);

    size_t status_code_pos = status_line.find(' ') + 1;
    string status_code = status_line.substr(status_code_pos, 3);
    if(status_code == code) {
        if(!success.empty()) {
            cout << success << endl;
        }
        return true;
    } else {
        cerr << fail << endl;
        return false;
    }
}

vector<string> extractCookies(const string& response) {
    vector<string> cookies;
    size_t start = 0;
    
    start = response.find("Set-Cookie: ", start);
    while (start != string::npos) {
        start += strlen("Set-Cookie: ");
        size_t end = response.find("\r\n", start);
        if (end != string::npos) {
            string cookie = response.substr(start, end - start);
            cookies.push_back(cookie);
            start = end + 2;
        } else {
            break;
        }

        start = response.find("Set-Cookie: ", start);
    }
    
    return cookies;
}

void registerUser() {
    string username;
    string password;
    
    // reading username and password
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, password);

    // validating data
    if(username.empty() || password.empty() || username.find(' ') != std::string::npos || password.find(' ') != std::string::npos) {
        cerr << "Eroare la inregistrare!" << endl;
        return;
    }

    // parsing json
    json reg;

    reg["username"] = username;
    reg["password"] = password;

    char *payload;
    payload = (char*)malloc(reg.dump().length() + 1); 
    strcpy(payload, reg.dump().c_str());

    // sending message
    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char *message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/register", "application/json", &payload, 1, NULL, 0);

    send_to_server(sockfd, message);
    free(payload);
    free(message);

    char* response = receive_from_server(sockfd);
    close(sockfd);

    // parsing the response
    string resp(response);
    get_status(resp, "201", "Utilizator înregistrat cu succes", "Eroare la inregistrare!");
}

void login() {
    string username;
    string password;
    
    // reading username and password
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, password);

    // validating data
    if(username.empty() || password.empty() || username.find(' ') != std::string::npos || password.find(' ') != std::string::npos) {
        cerr << "Eroare la inregistrare!" << endl;
        return;
    }

    // parsing json
    json reg;

    reg["username"] = username;
    reg["password"] = password;

    char *payload;
    payload = (char*)malloc(reg.dump().length() + 1); 
    strcpy(payload, reg.dump().c_str());

    // sending message
    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char *message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/login", "application/json", &payload, 1, NULL, 0);

    send_to_server(sockfd, message);
    free(payload);
    free(message);

    // getting response
    char* response = receive_from_server(sockfd);
    close(sockfd);
    
    // getting response status
    string resp(response);
    bool status = get_status(resp, "200", "Utilizatorul a fost logat cu succes", "Eroare la autentificare!");

    // extracting login cookie
    if(status) {
        vector<string> cookies = extractCookies(resp);

        string authCookie = cookies.front();
        loginCookie = (char*)malloc(authCookie.length() + 1);
        strcpy(loginCookie, authCookie.c_str());
    }
}

void logout() {
    if (loginCookie == NULL) {
        cerr << "Eroare la delogare!" << endl;
        return;
    }

    // loging out
    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char* message = compute_get_request(IP_SERVER, "/api/v1/tema/auth/logout", NULL, &loginCookie, 1);

    send_to_server(sockfd, message);
    free(message);

    char* response = receive_from_server(sockfd);
    close(sockfd);

    // checking status code
    string resp(response);
    bool status = get_status(resp, "200", "Utilizatorul s-a delogat cu succes", "Eroare la delogare!");

    if(status) {
        free(loginCookie);
        free(libraryToken);
    }
}

void enter_library() {
    if(loginCookie == NULL) {
        cerr << "Eroare la intrarea in biblioteca!" << endl;
        return;
    }

    // requesting access
    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char* message = compute_get_request(IP_SERVER, "/api/v1/tema/library/access", NULL, &loginCookie, 1);
    send_to_server(sockfd, message);
    free(message);

    char* response = receive_from_server(sockfd);
    close(sockfd);

    // checking status code
    string resp(response);
    bool status = get_status(resp, "200", "Utilizatorul are acces la biblioteca cu succes", "Eroare la intrarea in biblioteca!");

    // obtaining access token
    if(status) {
        size_t token_start = resp.find("{");
        string tok = resp.substr(token_start);

        json token = json::parse(tok);
        string header = "Authorization: Bearer ";
        string libTok = token["token"];

        libraryToken = (char *)malloc(libTok.length() + 1);
        strcpy(libraryToken, libTok.c_str());
    }
}

void get_books() {
    if(libraryToken == NULL) {
        cerr << "Eroare la afisare carti!" << endl;
        return;
    }

    // requesting books
    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char* message = compute_get_authorisation(IP_SERVER, "/api/v1/tema/library/books", NULL, libraryToken);

    send_to_server(sockfd, message);
    free(message);

    char* response = receive_from_server(sockfd);
    close(sockfd);

    // checking status
    string resp(response);
    bool status = get_status(resp, "200", string(), "Eroare la afisare carti!");

    // getting books
    if(status) {
        size_t books_start = resp.find("[");
        size_t books_end = resp.find("]");

        string books = resp.substr(books_start, books_end - books_start + 1);
        cout << books << endl;
    }
}

void get_book() {
    if(libraryToken == NULL) {
        cerr << "Eroare la afisare carte!" << endl;
        return;
    }

    // reading id
    int id;
    string id_str;
    cout << "id=";
    getline(cin, id_str);
    id = stoi(id_str);

    // requesting book
    char* line = (char*)calloc(strlen("/api/v1/tema/library/books/") + NUM_DIGITS + 1, sizeof(char));
    sprintf(line, "/api/v1/tema/library/books/%d", id);

    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char* message = compute_get_authorisation(IP_SERVER, line, NULL, libraryToken);

    send_to_server(sockfd, message);
    free(message);

    // getting response
    char* response = receive_from_server(sockfd);
    close(sockfd);

    // checking status
    string resp(response);
    bool status = get_status(resp, "200", string(), "Eroare la cautarea cartii");

    // getting book
    if(status) {
        size_t book_start = resp.find("{");
        size_t book_end = resp.find("}");

        string book = resp.substr(book_start, book_end - book_start + 1);
        cout << book << endl;
    }
}

void add_book() {
    if(libraryToken == NULL) {
        cerr << "Eroare la adaugare carte!" << endl;
        return;
    }

    string num_pages_str;
    string title, author, genre, publisher;
    int num_pages;

    // reading user input
    cout << "title=";
    getline(cin, title);
    cout << "author=";
    getline(cin, author);
    cout << "genre=";
    getline(cin, genre);
    cout << "publisher=";
    getline(cin, publisher);
    cout << "page_count=";
    getline(cin, num_pages_str);

    // checking if page count is valid
    for(char c : num_pages_str) {
        if(!isdigit(c)) {
            cerr << "EROARE: Tip de date incorect pentru numarul de pagini" << endl;
            return;
        }
    }

    // validating other fields
    if(title.empty() || author.empty() || genre.empty() || publisher.empty()) {
        cerr << "EROARE: Date nule invalide" << endl;
        return;
    }

    num_pages = stoi(num_pages_str);

    // creating the book object
    Book b(title, author, publisher, genre, num_pages);
    char* book = b.getJSON();

    // sending the book
    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char *message = compute_post_authorisation(IP_SERVER, "/api/v1/tema/library/books", "application/json", &book, 1, libraryToken);

    send_to_server(sockfd, message);
    free(book);
    free(message);

    // getting response
    char* response = receive_from_server(sockfd);
    close(sockfd);

    // checking status
    string resp(response);
    get_status(resp, "200", "Carte adaugata cu succes", "Eroare la adaugarea carti!");
}

void delete_book() {
    if(libraryToken == NULL) {
        cerr << "Eroare la stergere carte!" << endl;
        return;
    }

    // reading id
    int id;
    string id_str;
    cout << "id=";
    getline(cin, id_str);
    id = stoi(id_str);

    // requesting book
    char* line = (char*)calloc(strlen("/api/v1/tema/library/books/") + NUM_DIGITS + 1, sizeof(char));
    sprintf(line, "/api/v1/tema/library/books/%d", id);

    int sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char* message = compute_delete_request(IP_SERVER, line, NULL, libraryToken);

    send_to_server(sockfd, message);
    free(line);
    free(message);

    // getting response
    char* response = receive_from_server(sockfd);
    close(sockfd);

    // checking status
    string resp(response);
    get_status(resp, "200", "Carte stearsa cu succes", "Eroare la stergerea carti!");
}

int main() {
    char input[COMMAND_SIZE];
    while(1) {
        fgets(input, COMMAND_SIZE, stdin);
        if(strncmp(input, "register", strlen("register")) == 0) {
            registerUser();
        } else if(strncmp(input, "login", strlen("login")) == 0) {
            login();
        } else if(strncmp(input, "enter_library", strlen("enter_library")) == 0) {
            enter_library();
        } else if(strncmp(input, "get_books", strlen("get_books")) == 0) {
            get_books();
        } else if(strncmp(input, "get_book", strlen("get_book")) == 0) {
            get_book();
        } else if(strncmp(input, "add_book", strlen("add_book")) == 0) {
            add_book();
        } else if(strncmp(input, "delete_book", strlen("delete_book")) == 0) {
            delete_book();
        } else if(strncmp(input, "logout", strlen("logout")) == 0) {
            logout();
        } else if(strncmp(input, "exit", strlen("exit")) == 0) {
            cout << "Inchidere program" << endl;
            break;
        } else {
            cerr << "Invalid command" << endl;
        }
    }
}