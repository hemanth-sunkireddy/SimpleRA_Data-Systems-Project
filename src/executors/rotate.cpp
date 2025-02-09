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

bool semanticParseCHECKANTISYM() {
    logger.log("semanticParseCHECKANTISYM");
    if (!matrixCatalogue.ismatrix(parsedQuery.Matrix1) || !matrixCatalogue.ismatrix(parsedQuery.Matrix2))
    {
        cout << "SEMANTIC ERROR : EITHER A OR B NOT EXIST" << endl;
        return false;
    }
    return true;
}

void executeROTATE_MATRIX() {
    // logger.log("executeROTATE_MATRIX");
    // Matrix *matrix = matrixCatalogue.getmatrix(parsedQuery.rotateRelationName);
    // // matrix -> rotate();
    // Matrix *rotated = matrix -> rotate();
    // rotated->matrixname = matrix->matrixname;
    // matrixCatalogue.deletematrix(matrix->matrixname);
    // string pageName = "../data/temp/"+ matrix->matrixname + "_Page" + to_string(0);
    // bufferManager.deleteFile(pageName);
    // matrix = rotated;
    // matrixCatalogue.insertmatrix(rotated);
    // return;
    
    logger.log("executeROTATE_MATRIX");
    Matrix *matrix = matrixCatalogue.getmatrix(parsedQuery.rotateRelationName);
    matrix->rotate();  // In-place rotation
    matrixCatalogue.updateMatrix(matrix);  // Ensure the catalogue reflects the updated matrix, if needed
    return;
}

void checkAntiSym(Matrix *matrix1, Matrix *matrix2){
    cout << "EXECUTING CHECKANTISYM" << endl;
    return;
}

void executeCHECKANTISYM() {
    
    logger.log("executeCHECKANTISYM");
    Matrix *matrix1 = new Matrix(parsedQuery.Matrix1);
    Matrix *matrix2 = new Matrix(parsedQuery.Matrix2);
    checkAntiSym(matrix1, matrix2);
    return ;
}