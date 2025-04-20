//Server Code
#include "global.h"

using namespace std;

float BLOCK_SIZE = 1;
uint BLOCK_COUNT = 2;
uint PRINT_COUNT = 20;
Logger logger;
vector<string> tokenizedQuery;
ParsedQuery parsedQuery;
TableCatalogue tableCatalogue;
MatrixCatalogue matrixCatalogue;
BufferManager bufferManager;

void doCommand()
{
    // logger.log("doCommand");
    if (syntacticParse() && semanticParse())
        executeCommand();
    return;
}

void execute_line(string line, regex delim, int* quit_flag){
    // cout << "\n> " << line << endl;
    tokenizedQuery.clear();
    parsedQuery.clear();
    auto words_begin = std::sregex_iterator(line.begin(), line.end(), delim);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
        tokenizedQuery.emplace_back((*i).str());
    // cout << "Tokenized Query: " << tokenizedQuery.front() << endl;
    if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT")
    {
        *quit_flag = 1;
        return;
    }

    if (tokenizedQuery.empty()){
        // cout << "Tokenized Query: ";
        return;
    }

    if (tokenizedQuery.size() == 1){
        cout << "SYNTAX ERROR" << endl;
        return;
    }

    doCommand();
}

int main(void)
{
    regex delim("[^\\s,]+");
    string command;
    system("rm -rf ../data/temp");
    system("mkdir ../data/temp");

    int quit_flag = 0;

    while(!cin.eof() and !quit_flag)    
    {
        cout << "\n> ";
        tokenizedQuery.clear();
        parsedQuery.clear();
        // logger.log("\nReading New Command: ");
        getline(cin, command);
        // logger.log(command);

        auto words_begin = std::sregex_iterator(command.begin(), command.end(), delim);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i)
            tokenizedQuery.emplace_back((*i).str());

        if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT")
        {
            break;
        }

        if (tokenizedQuery.empty())
        {
            continue;
        }

        if (tokenizedQuery.size() == 1)
        {
            cout << "Please give more than 1 argument" << endl;
            continue;
        }

        if (tokenizedQuery.front() == "SOURCE")
        {
            // Source command implementation
            string file_name = tokenizedQuery.back() + ".ra";
            ifstream file(file_name);
            if (!file.is_open())
            {
                cout << "Error: Could not open file " << file_name << endl;
                continue;
            }
            string line;
            while (getline(file, line))
                execute_line(line, delim, &quit_flag);
            file.close();
        }
        else{
            doCommand();
        }
    }
}