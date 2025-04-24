#include"global.h"

bool semanticParse(){
    logger.log("semanticParse");
    switch(parsedQuery.queryType){
        case CLEAR: return semanticParseCLEAR();
        case CROSS: return semanticParseCROSS();
        case DISTINCT: return semanticParseDISTINCT();
        case EXPORT: return semanticParseEXPORT();
        case EXPORT_MATRIX : return semanticParseEXPORT_MATRIX();
        case INDEX: return semanticParseINDEX();
        case JOIN: return semanticParseJOIN();
        case LIST: return semanticParseLIST();
        case LOAD: return semanticParseLOAD();
        case GROUP_BY: return semanticParseGROUP_BY();
        case ORDERBY: return semanticParseORDERBY();
        case LOAD_MATRIX : return semanticParseLOAD_MATRIX();
        case PRINT: return semanticParsePRINT();
        case PRINT_MATRIX : return semanticParsePRINT_MATRIX();
        case PROJECTION: return semanticParsePROJECTION();
        case RENAME: return semanticParseRENAME();
        case SELECTION: return semanticParseSELECTION();
        case SEARCH: return semanticParseSEARCH();
        case SORT: return semanticParseSORT();
        case SOURCE: return semanticParseSOURCE();
        case ROTATE_MATRIX: return semanticParseROTATE_MATRIX();
        case CROSSTRANSPOSE: return semanticParseCROSSTRANSPOSE();
        case CHECKANTISYM: return semanticParseCHECKANTISYM();
        case INSERT: return semanticParseInsert();
        case UPDATE: return semanticParseUpdate();
        case DELETE: return semanticParseDELETE();
        default: cout<<"SEMANTIC ERROR"<<endl;
    }

    return false;
}