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
    cout << "EXECUTING LOADING..." << endl;
    Table *table = new Table(parsedQuery.loadRelationName);
    // cout << "LOADING : " << parsedQuery.loadRelationName << endl;
    // cout << "PATH : " << table->sourceFileName << endl;
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

// CROSSTRANPOSE OF TWO MATRICES INDEPENDENT OF CLASSES
void crossTranspose(Matrix *matrix1, Matrix *matrix2){
    cout << "CROSS TRANSPOSE LOGIC HERE" << endl;
    int matrix1_size = matrix1->rowCount;
    int matrix2_size = matrix2->columnCount;
   
    
   vector<vector<int>> matrix1_transpose(matrix2_size, vector<int>(matrix2_size));
   vector<vector<int>> matrix2_transpose(matrix1_size, vector<int>(matrix1_size));

   // Calculate transpose of matrices and store in matrix1_tranaspose and matrix2_transpose
    for (int i = 0; i < matrix1_size; ++i) {
        for (int j = 0; j < matrix1_size; ++j) {
            matrix1_transpose[j][i] = matrix1->get_element(i,j);
            cout << matrix1_transpose[j][i] << " ";
        }
        cout << endl;
    }
    cout << endl;
    for (int i = 0; i < matrix2_size; ++i) {
        for (int j = 0; j < matrix2_size; ++j) {
            matrix2_transpose[j][i] = matrix2->get_element(i,j);
            cout << matrix2_transpose[j][i] << " ";
        }
        cout << endl;
    }

    cout << endl;

    // Now update the original matrices with Cross Transpose of other matrix
    for (int i = 0; i < matrix2_size; ++i) {
        for (int j = 0; j < matrix2_size; ++j) {
            matrix1->set_element(i,j, matrix2_transpose[i][j]);
        }
    }
    for (int i = 0; i < matrix1_size; ++i) {
        for (int j = 0; j < matrix1_size; ++j) {
            matrix2->set_element(i,j, matrix1_transpose[i][j]);
        }
    }
    matrixCatalogue.updateMatrix(matrix1);
    matrixCatalogue.updateMatrix(matrix2);
    cout << "SUCCESSFULLY CROSSTRANSPOSED BOTH MATRICES" << endl;
}

void executeCROSSTRANSPOSE(){
    logger.log("executeCROSSTRANSPOSE");

    Matrix *matrix1 = matrixCatalogue.getmatrix(parsedQuery.Matrix1);
    Matrix *matrix2 = matrixCatalogue.getmatrix(parsedQuery.Matrix2);
    crossTranspose(matrix1, matrix2);
}