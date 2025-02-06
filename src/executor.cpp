#include "global.h"

// comment to test github

void executeCommand()
{

    switch (parsedQuery.queryType)
    {
    case CLEAR:
        executeCLEAR();
        break;
    case CROSS:
        executeCROSS();
        break;
    case DISTINCT:
        executeDISTINCT();
        break;
    case EXPORT:
        executeEXPORT();
        break;
    case INDEX:
        executeINDEX();
        break;
    case JOIN:
        executeJOIN();
        break;
    case LIST:
        executeLIST();
        break;
    case LOAD:
        executeLOAD();
        break;
    case PRINT:
        executePRINT();
        break;
    case PROJECTION:
        executePROJECTION();
        break;
    case RENAME:
        executeRENAME();
        break;
    case SELECTION:
        executeSELECTION();
        break;
    case SORT:
        executeSORT();
        break;
    case SOURCE:
        executeSOURCE();
        break;
    
    //matrix

    case LOAD_MATRIX:
        return executeLOADMATRIX();
    case PRINT_MATRIX:
        executePRINTMATRIX();
        break;
    case EXPORT_MATRIX:
        executeEXPORTMATRIX();
        break;
    default:
        cout << "PARSING ERROR" << endl;
    }

    return;
}

void printRowCount(int rowCount)
{
    cout << "\n\nRow Count: " << rowCount << endl;
    return;
}