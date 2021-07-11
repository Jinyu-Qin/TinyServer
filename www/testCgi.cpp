#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

int main(int argc, char * argv[]) {
    string method(getenv("REQUEST_METHOD"));
    cout << "Request method: " << method << "<br>";

    if(method == "GET") {
        string query(getenv("QUERY_STRING"));
        cout << "Query string: " << query << "<br>";
    } else if(method == "POST") {
        string contentLength(getenv("CONTENT_LENGTH"));
        cout << "Content length: " << contentLength << "<br>";
        cout << "Content: " << "<br>";
        int len = stoi(contentLength);
        char buf[len + 1] = {0};
        int n = read(0, buf, len);
        cout << buf << "<br>" << endl;
    }

    return 0;
}