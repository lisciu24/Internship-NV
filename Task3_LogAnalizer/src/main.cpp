#include <iostream>
#include "LogAnalyzer.hpp"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << "\n";
        std::cout << "LogAnalyzer <filepath> <query_string>";
        return 1;
    }

    std::string query_str = argv[2];
    for (int i = 3; i < argc; i++)
    {
        query_str.append(";");
        query_str.append(argv[i]);
    }

    LogAnalyzer::LogQueryEngine la;
    la.loadEntriesFromFile(argv[1]);
    LogAnalyzer::Query q = la.parseQuery(query_str);
    la.executeQuery(q, std::cout);
}