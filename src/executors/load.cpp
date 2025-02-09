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

// CROSSTRANPOSE OF TWO MATRICES INDEPENDENT OF CLASSES
void crossTranspose(Matrix *matrix1, Matrix *matrix2){
    cout << "CROSS TRANSPOSE LOGIC HERE" << endl;
    int matrix1_rowlen = matrix1->rowCount;
    int matrix1_collen = matrix1->columnCount;
    int matrix2_rowlen = matrix2->rowCount;
    int matrix2_collen = matrix2->columnCount;
    cout << matrix1_rowlen << " " << matrix2_rowlen << endl;
    
    string cross_transpose_a = "A_CT";
    string cross_transpose_b = "B_CT";

    Matrix *transposedMatrix1 = new Matrix(cross_transpose_a);
    transposedMatrix1->rowCount = matrix1_collen;
    transposedMatrix1->columnCount = matrix1_rowlen;

    Matrix *transposedMatrix2 = new Matrix(cross_transpose_b);
    transposedMatrix2->rowCount = matrix2_collen;
    transposedMatrix2->columnCount = matrix2_rowlen;

    for (int i = 0; i < matrix1_rowlen; ++i) {
        for (int j = 0; j < matrix1_collen; ++j) {
            transposedMatrix1->set_element(j, i, matrix1->get_element(i,j));
        }
    }
    for (int i = 0; i < matrix2_rowlen; ++i) {
        for (int j = 0; j < matrix2_collen; ++j) {
            transposedMatrix2->set_element(j, i, matrix2->get_element(i,j));
        }
    }
    transposedMatrix1 -> matrixname = matrix2->sourceFileName;
    transposedMatrix2 ->matrixname = matrix1->sourceFileName;
    matrixCatalogue.insertmatrix(transposedMatrix1);
    matrixCatalogue.insertmatrix(transposedMatrix2);
    cout << "SUCCESSFULLY CROSSTRANSPOSED BOTH MATRICES" << endl;
}

void executeCROSSTRANSPOSE(){
    logger.log("executeCROSSTRANSPOSE");

    Matrix *matrix1 = new Matrix(parsedQuery.Matrix1);
    Matrix *matrix2 = new Matrix(parsedQuery.Matrix2);
    if (matrix1 -> load())
    {
        matrixCatalogue.insertmatrix(matrix1);
        cout << "Loaded Matrix. Column count : " << matrix1->columnCount << " Row Count : " << matrix1 -> rowCount << endl;
    }
    if (matrix2 -> load())
    {
        matrixCatalogue.insertmatrix(matrix2);
        cout << "Loaded Matrix. Column count : " << matrix2->columnCount << " Row Count : " << matrix2 -> rowCount << endl;
    }
    crossTranspose(matrix1, matrix2);
}