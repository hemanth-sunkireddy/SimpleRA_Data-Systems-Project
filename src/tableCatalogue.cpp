#include "global.h"

void TableCatalogue::insertTable(Table* table)
{
    logger.log("TableCatalogue::~insertTable"); 
    this->tables[table->tableName] = table;
}

void MatrixCatalogue::insertmatrix(Matrix* matrix)
{
    logger.log("MatrixCatalogue::~insertmatrix");
    this -> matrices [matrix->matrixname] = matrix;
}

void TableCatalogue::deleteTable(string tableName)
{
    logger.log("TableCatalogue::deleteTable"); 
    this->tables[tableName]->unload();
    delete this->tables[tableName];
    this->tables.erase(tableName);
}

void MatrixCatalogue::deletematrix(string matrixname)
{
    logger.log("MatrixCatalogue :: deleteMatrix");
    this -> matrices[matrixname] -> unload();
    delete this->matrices[matrixname];
    this -> matrices.erase(matrixname);
}
Table* TableCatalogue::getTable(string tableName)
{
    logger.log("TableCatalogue::getTable"); 
    Table *table = this->tables[tableName];
    return table;
}
Matrix* MatrixCatalogue::getmatrix(string matrixname)
{
    logger.log("MatrixCatalogue::getMatrix");
    Matrix *matrix = this -> matrices[matrixname];
    return matrix;
}
bool TableCatalogue::isTable(string tableName)
{
    logger.log("TableCatalogue::isTable"); 
    if (this->tables.count(tableName))
        return true;
    return false;
}
bool MatrixCatalogue::ismatrix(string matrixname)
{
    logger.log("MatrixCatalogue::ismatrix");
    if (this->matrices.count(matrixname))
        return true;
    return false;
}

bool TableCatalogue::isColumnFromTable(string columnName, string tableName)
{
    logger.log("TableCatalogue::isColumnFromTable"); 
    if (this->isTable(tableName))
    {
        Table* table = this->getTable(tableName);
        if (table->isColumn(columnName))
            return true;
    }
    return false;
}

void TableCatalogue::print()
{
    logger.log("TableCatalogue::print"); 
    cout << "\nRELATIONS" << endl;

    int rowCount = 0;
    for (auto rel : this->tables)
    {
        cout << rel.first << endl;
        rowCount++;
    }
    printRowCount(rowCount);
}

TableCatalogue::~TableCatalogue(){
    logger.log("TableCatalogue::~TableCatalogue"); 
    for(auto table: this->tables){
        table.second->unload();
        delete table.second;
    }
}

MatrixCatalogue::~MatrixCatalogue(){
    logger.log("MatrixCatalogue :: ~MatrixCatalogue");
    for (auto matrix : this -> matrices)
    {
        matrix.second -> unload();
        delete matrix.second;
    }
}

void MatrixCatalogue::updateMatrix(Matrix* matrix) {
    logger.log("MatrixCatalogue::updateMatrix");
    // Overwrite the existing matrix with the updated one
    this->matrices[matrix->matrixname] = matrix;
}
