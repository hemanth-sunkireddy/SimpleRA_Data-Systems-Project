#include "table.h"

/**
 * @brief The TableCatalogue acts like an index of tables existing in the
 * system. Everytime a table is added(removed) to(from) the system, it needs to
 * be added(removed) to(from) the tableCatalogue. 
 *
 */
class TableCatalogue
{

    unordered_map<string, Table*> tables;

public:
    TableCatalogue() {}
    void insertTable(Table* table);
    void deleteTable(string tableName);
    Table* getTable(string tableName);
    bool isTable(string tableName);
    bool isColumnFromTable(string columnName, string tableName);
    void print();
    ~TableCatalogue();
};

class MatrixCatalogue {
    unordered_map<string, Matrix*> matrices;

public :
    MatrixCatalogue() {}
    void insertmatrix(Matrix* matrix);
    void deletematrix(string matrixname);
    Matrix* getmatrix(string matrixname);
    bool ismatrix(string matrixname);
    void updateMatrix(Matrix* matrix);
    void print();
    ~MatrixCatalogue();
};