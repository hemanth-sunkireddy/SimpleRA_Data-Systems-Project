#include"global.h"

void executeCommand(){

    switch(parsedQuery.queryType){
        case CLEAR: executeCLEAR(); break;
        case CROSS: executeCROSS(); break;
        case DISTINCT: executeDISTINCT(); break;
        case EXPORT: executeEXPORT(); break;
        case EXPORT_MATRIX : executeEXPORT_MATRIX(); break;
        case INDEX: executeINDEX(); break;
        case JOIN: executeJOIN(); break;
        case LIST: executeLIST(); break;
        case LOAD: executeLOAD(); break;
        case GROUP_BY: executeGROUP_BY(); break;
        case ORDERBY: executeORDER_BY(); break;
        case LOAD_MATRIX : executeLOAD_MATRIX(); break;
        case PRINT: executePRINT(); break;
        case PRINT_MATRIX:executePRINT_MATRIX(); break;
        case PROJECTION: executePROJECTION(); break;
        case RENAME: executeRENAME(); break;
        case SELECTION: executeSELECTION(); break;
        case SEARCH: executeSEARCH(); break;
        case SORT: executeSORT(); break;
        case SOURCE: executeSOURCE(); break;
        case ROTATE_MATRIX: executeROTATE_MATRIX(); break;
        case CROSSTRANSPOSE: executeCROSSTRANSPOSE(); break;
        case CHECKANTISYM: executeCHECKANTISYM(); break;
        case INSERT: executeINSERT(); break;
        case UPDATE: executeUPDATE(); break;
        case DELETE: executeDELETE(); break;
        default: cout<<"PARSING ERROR"<<endl;
    }

    return;
}

void printRowCount(int rowCount){
    cout<<"\n\nRow Count: "<<rowCount<<endl;
    return;
}