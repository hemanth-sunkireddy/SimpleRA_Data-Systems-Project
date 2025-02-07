#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseEXPORT()
{
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT;
    parsedQuery.exportRelationName = tokenizedQuery[1];
    return true;
}

bool syntacticParseEXPORTMATRIX()
{
    logger.log("syntacticParseEXPORTMATRIX");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT_MATRIX;
    parsedQuery.exportRelationName = tokenizedQuery[2];
    return true;
}

// ROTATE MATRIX
bool syntacticParseROTATEMATRIX()
{
    logger.log("syntacticParseROTATEMATRIX");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR, please give ROTATE MATRIX A" << endl;
        return false;
    }
    parsedQuery.queryType = ROTATE_MATRIX;
    parsedQuery.exportRelationName = tokenizedQuery[2];
    return true;
}

// CROSS TRANSPOSE MATRIX
bool syntacticParseCROSSTRANSPOSE()
{
    logger.log("syntacticParseROTATEMATRIX");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR, please give CROSSTRANPOSE A B" << endl;
        return false;
    }
    parsedQuery.queryType = CROSSTRANSPOSE_MATRIX;
    parsedQuery.exportRelationName = tokenizedQuery[2];
    return true;
}


bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}



bool semanticParseEXPORTMATRIX()
{
    logger.log("semanticParseEXPORTMATRIX");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}

bool semanticParseROTATEMATRIX()
{
    logger.log("semanticParseROTATEMATRIX");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}

bool semanticParseCROSSTRANSPOSEMATRIX()
{
    logger.log("semanticParseROTATEMATRIX");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}



void executeEXPORT()
{
    logger.log("executeEXPORT");
    Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->makePermanent();
    return;
}

void executeEXPORTMATRIX()
{
    logger.log("executeEXPORTMARIX");
    Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->makePermanentMatrix();
    return;
}

void executeROTATEMATRIX()
{
    logger.log("executeROTATEMARIX");
    Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->rotateMatrix();
    return;
}

void executeCROSSTRANSPOSEMATRIX()
{
    logger.log("executeCROSSTRANSPOSEMARIX");
    Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->crossTransposeMatrix();
    return;
}