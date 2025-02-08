#include "global.h"

bool syntacticParseROTATE_MATRIX(){
    // rotate matrix
    logger.log("syntacticParseRotate");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = ROTATE_MATRIX;
    parsedQuery.rotateRelationName = tokenizedQuery[2];
    return true;
}

bool semanticParseROTATE_MATRIX() {
    logger.log("semanticParseROTATE_MATRIX");
    if (!matrixCatalogue.ismatrix(parsedQuery.rotateRelationName))
    {
        cout << "SEMANTIC ERROR : Relation doesn;t exist" << endl;
        return false;
    }
    return true;
}

void executeROTATE_MATRIX() {
    logger.log("executeROTATE_MATRIX");
    Matrix *matrix = matrixCatalogue.getmatrix(parsedQuery.rotateRelationName);
    Matrix *rotated = matrix -> rotate();
    rotated->matrixname = matrix->matrixname;
    matrixCatalogue.deletematrix(matrix->matrixname);
    matrixCatalogue.insertmatrix(rotated);
    return;
}