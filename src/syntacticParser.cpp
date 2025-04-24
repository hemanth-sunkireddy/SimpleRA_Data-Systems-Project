#include "global.h"

bool syntacticParse()
{
    logger.log("syntacticParse");
    string possibleQueryType = tokenizedQuery[0];
    if (tokenizedQuery.size() < 2)
    {
        cout << "SYNTAX ERROR: PLEASE GIVE MORE THAN 1 ARUGMENTS" << endl;
        return false;
    }
    string possibleDataType = tokenizedQuery[1];
    if (tokenizedQuery.size() == 3)
    {
        string possibleMatrixName = tokenizedQuery[2];
    }

    if (possibleQueryType == "CLEAR")
        return syntacticParseCLEAR();
    else if (possibleQueryType == "INDEX")
        return syntacticParseINDEX();
    else if (possibleQueryType == "LIST")
        return syntacticParseLIST();
    else if (possibleQueryType == "LOAD"){
        if (tokenizedQuery.size() > 2 && possibleDataType == "MATRIX")
            return syntacticParseLOAD_MATRIX();
        else
            return syntacticParseLOAD();
    }
    else if (possibleQueryType == "PRINT"){
        if (tokenizedQuery.size() > 2 && possibleDataType == "MATRIX")
            return syntacticParsePRINT_MATRIX();
        else
            return syntacticParsePRINT();
    }
    else if (possibleQueryType == "RENAME")
        return syntacticParseRENAME();
    else if(possibleQueryType == "EXPORT")
    {
        if (tokenizedQuery.size() > 2 && possibleDataType == "MATRIX")
            return syntacticParseEXPORT_MATRIX();
        else
            return syntacticParseEXPORT();
    }
    else if(possibleQueryType == "SOURCE")
        return syntacticParseSOURCE();
    else if(possibleQueryType == "CROSSTRANSPOSE"){
        return syntacticParseCROSSTRANSPOSE();
    }
    else if(possibleQueryType == "CHECKANTISYM"){
        return syntacticParseCHECKANTISYM();
    }
    else if(possibleQueryType == "ROTATE")
        return syntacticParseROTATE_MATRIX();
    else if (possibleQueryType == "SORT"){
            return syntacticParseSORT();
    }
    else if (tokenizedQuery[2] == "GROUP" && tokenizedQuery[3] == "BY")
        return syntacticParseGROUP_BY();
    else if (tokenizedQuery[2] == "ORDER" && tokenizedQuery[3] == "BY")
        return syntacticParseORDERBY();
    else if(tokenizedQuery[0] == "INSERT")
        return syntaticParseInsert();
    else if(tokenizedQuery[0] == "UPDATE")
        return syntaticParseUpdate();
    else if(tokenizedQuery[0] == "DELETE")
        return syntacticParseDELETE();
    else
    {
        string resultantRelationName = possibleQueryType;
        if (tokenizedQuery[1] != "<-" || tokenizedQuery.size() < 3)
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        possibleQueryType = tokenizedQuery[2];
        if (possibleQueryType == "PROJECT")
            return syntacticParsePROJECTION();
        else if (possibleQueryType == "SELECT")
            return syntacticParseSELECTION();
        else if (possibleQueryType == "SEARCH")
            return syntacticParseSEARCH();
        else if (possibleQueryType == "JOIN")
            return syntacticParseJOIN();
        else if (possibleQueryType == "CROSS")
            return syntacticParseCROSS();
        else if (possibleQueryType == "DISTINCT")
            return syntacticParseDISTINCT();
        else
        {
            cout << "SYNTAX ERROR {basic error}" << endl;
            return false;
        }
    }
    return false;
}

ParsedQuery::ParsedQuery()
{
}

void ParsedQuery::clear()
{
    logger.log("ParseQuery::clear");
    this->queryType = UNDETERMINED;

    this->clearRelationName = "";

    this->crossResultRelationName = "";
    this->crossFirstRelationName = "";
    this->crossSecondRelationName = "";

    this->distinctResultRelationName = "";
    this->distinctRelationName = "";

    this->exportRelationName = "";

    this->indexingStrategy = NOTHING;
    this->indexColumnName = "";
    this->indexRelationName = "";

    this->joinBinaryOperator = NO_BINOP_CLAUSE;
    this->joinResultRelationName = "";
    this->joinFirstRelationName = "";
    this->joinSecondRelationName = "";
    this->joinFirstColumnName = "";
    this->joinSecondColumnName = "";

    this->loadRelationName = "";

    this->printRelationName = "";

    this->projectionResultRelationName = "";
    this->projectionColumnList.clear();
    this->projectionRelationName = "";

    this->renameFromColumnName = "";
    this->renameToColumnName = "";
    this->renameRelationName = "";

    this->selectType = NO_SELECT_CLAUSE;
    this->selectionBinaryOperator = NO_BINOP_CLAUSE;
    this->selectionResultRelationName = "";
    this->selectionRelationName = "";
    this->selectionFirstColumnName = "";
    this->selectionSecondColumnName = "";
    this->selectionIntLiteral = 0;

    this->sortingStrategy = NO_SORT_CLAUSE;
    this->sortResultRelationName = "";
    this->sortColumnName = "";
    this->sortRelationName = "";

    this->sourceFileName = "";
}

/**
 * @brief Checks to see if source file exists. Called when LOAD command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isFileExists(string tableName)
{
    string fileName = "../data/" + tableName + ".csv";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

/**
 * @brief Checks to see if source file exists. Called when SOURCE command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isQueryFile(string fileName){
    fileName = "../data/" + fileName + ".ra";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}
