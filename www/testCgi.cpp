#include <iostream>
#include <string>

using namespace std;

int main(int argc, char * argv[]) {
    string method(getenv("REQUEST_METHOD"));
    cout << "Request method: " << method << "</br>";

    if(method == "GET") {
        string query(getenv("QUERY_STRING"));
        cout << "Query string: " << query << "</br>";
    } else if(method == "POST") {
        string contentLength(getenv("CONTENT_LENGTH"));
        cout << "Content length: " << contentLength << "</br>";
        cout << "Content: " << "</br>";
        string content;
        while(getline(cin, content)) {
            cout << content << "</br>";
        }
    }

    return 0;
}