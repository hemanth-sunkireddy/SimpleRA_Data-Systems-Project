#include "global.h"
/**
 * @brief 
 * SYNTAX: SOURCE filename
 */
bool syntacticParseSOURCE()
{
    logger.log("syntacticParseSOURCE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SOURCE;
    parsedQuery.sourceFileName = tokenizedQuery[1];
    return true;
}


bool syntacticParseCROSSTRANSPOSE()
{
    logger.log("syntacticParseCROSSTRANSPOSE");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR: Please give CROSSTRANSPOSE A B" << endl;
        return false;
    }
    parsedQuery.queryType = CROSSTRANSPOSE;
    parsedQuery.Matrix1 = tokenizedQuery[1];
    parsedQuery.Matrix2 = tokenizedQuery[2];
    return true;
}

bool syntacticParseCHECKANTISYM(){
    logger.log("syntacticParseCHECKANTISYM");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR: Please give CHECKANTISYM A B" << endl;
        return false;
    }
    parsedQuery.queryType = CHECKANTISYM;
    parsedQuery.Matrix1 = tokenizedQuery[1];
    parsedQuery.Matrix2 = tokenizedQuery[2];
    return true;
}

bool semanticParseSOURCE()
{
    logger.log("semanticParseSOURCE");
    if (!isQueryFile(parsedQuery.sourceFileName))
    {
        cout << "SEMANTIC ERROR: File doesn't exist" << endl;
        return false;
    }
    return true;
}

bool semanticParseCROSSTRANSPOSE()
{
    logger.log("semanticParseCROSSTRANSPOSE");
    cout << parsedQuery.Matrix1 << endl;
    if (!matrixCatalogue.ismatrix(parsedQuery.Matrix1))
    {
        cout << "SEMANTIC ERROR : " << parsedQuery.Matrix1 << " Doesn't exist." << endl;
        return false;
    }
    if (!matrixCatalogue.ismatrix(parsedQuery.Matrix2))
    {
        cout << "SEMANTIC ERROR : " << parsedQuery.Matrix2 << " Doesn't exist." << endl;
        return false;
    }
    return true;
}

void executeSOURCE()
{
    logger.log("executeSOURCE");
    return;
}
