#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD relation_name
 */
bool syntacticParseLOAD()
{
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD;
    parsedQuery.loadRelationName = tokenizedQuery[1];
    return true;
}

bool syntacticParseLOAD_MATRIX()
{
    logger.log("syntacticParseLOAD_MATRIX");
    if (tokenizedQuery.size()!=3)
    {
        cout << "SYNTAX ERROR {please follow grammer}" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD_MATRIX;
    parsedQuery.loadRelationName = tokenizedQuery[2];
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (tableCatalogue.isTable(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Relation already exists" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

bool semanticParseLOAD_MATRIX (){
    logger.log("semanticParseLOAD_MATRIX");
    if (matrixCatalogue.ismatrix(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR : Relation already existing" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR : Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeLOAD()
{
    logger.log("executeLOAD");

    Table *table = new Table(parsedQuery.loadRelationName);
    if (table->load())
    {
        tableCatalogue.insertTable(table);
        cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
    }
    return;
}

void executeLOAD_MATRIX(){
    logger.log("executeLOAD_MATRIX");

    Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
    if (matrix -> load())
    {
        matrixCatalogue.insertmatrix(matrix);
        cout << "Loaded Matrix. Column count : " << matrix->columnCount << " Row Count : " << matrix -> rowCount << endl;
    }
}