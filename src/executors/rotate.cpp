#include "global.h"

bool syntacticParseROTATE_MATRIX(){
    // rotate matrix
    logger.log("syntacticParseRotate");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = ROTATE_MATRIX;
    parsedQuery.rotateRelationName = tokenizedQuery[1];
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
    cout << "CHECK ANTI SYM LOGIC HERE" << endl;
    int matrix1_size = matrix1->rowCount;
    int matrix2_size = matrix2->columnCount;
    if (matrix1_size != matrix2_size){
        cout << "MATRIX SIZES ARE NOT SAME" << endl;
    }
   vector<vector<int>> matrix1_transpose(matrix1_size, vector<int>(matrix1_size));
   vector<vector<int>> matrix2_copy(matrix2_size, vector<int>(matrix2_size));

   // Calculate transpose of matrices and store in matrix1_tranaspose and matrix2_transpose
    for (int i = 0; i < matrix1_size; ++i) {
        for (int j = 0; j < matrix1_size; ++j) {
            if(matrix2->get_element(i,j) != -matrix1->get_element(j,i)){
                cout << "MATRICES ARE NOT ANTISYMMETRIC" << endl;
                return;
            }
        }
    }
    cout << "MATRICES ARE ANTI SYMMETRIC" << endl;
    return;
}

void executeCHECKANTISYM() {
    
    logger.log("executeCHECKANTISYM");
    Matrix *matrix1 = matrixCatalogue.getmatrix(parsedQuery.Matrix1);
    Matrix *matrix2 = matrixCatalogue.getmatrix(parsedQuery.Matrix2);
    checkAntiSym(matrix1, matrix2);
    return ;
}