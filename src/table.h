#include "cursor.h"
#include "enums.h"
#include "bplustree.h"

enum IndexingStrategy
{
    BTREE,
    HASH,
    NOTHING
};

/**
 * @brief The Table class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT).
 *
 */
// Structure to hold index information
struct IndexInfo {
    string columnName;
    IndexingStrategy strategy;
    BPlusTree* bPlusTreeIndex;
    
    IndexInfo(string colName, IndexingStrategy strat, BPlusTree* index = nullptr) 
        : columnName(colName), strategy(strat), bPlusTreeIndex(index) {}
};

class Table
{
    vector<unordered_set<int>> distinctValuesInColumns;
    
    // Map to store multiple indices (column name -> index info)
    unordered_map<string, IndexInfo*> indices;
    
    // Keep this for backward compatibility
    BPlusTree* bPlusTreeIndex = nullptr;

public:
    string sourceFileName = "";
    string tableName = "";
    vector<string> columns;
    vector<uint> distinctValuesPerColumnCount;
    uint columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0;
    vector<uint> rowsPerBlockCount;
    
    // Keep these for backward compatibility
    bool indexed = false;
    string indexedColumn = "";
    IndexingStrategy indexingStrategy = NOTHING;

    bool extractColumnNames(string firstLine);
    bool blockify();
    void updateStatistics(vector<int> row);
    Table();
    Table(string tableName);
    Table(string tableName, vector<string> columns);
    Table(string tableName, vector<string> columns, bool ORDER_BY_OPERATION, int block_count);
    ~Table();
    bool load();
    bool isColumn(string columnName);
    void renameColumn(string fromColumnName, string toColumnName);
    void print();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    void sortTable(bool makePermanent = true);
    void externalSort();
    int getColumnIndex(string columnName);
    void unload();
    void groupBy();
    void deleteTable();
    void joinTables();
    void orderBy();
    void insertRow(const vector<string>& row);
    void updateRow(const vector<string>& row);
    void deleteRows(const vector<int>& rowIndices);
    void rebalanceBlocks();
    
    // Index related functions
    bool buildIndex(string columnName);
    vector<int> searchIndexed(string columnName, int value, BinaryOperator op);
    bool isIndexed(string columnName);

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Table::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row)
    {
        logger.log("Table::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};

class Matrix
{
    // vector<unordered_set<int>> distinctValuesInColumns;

public:
    string sourceFileName = "";
    string matrixname = "";
    // vector<string> columns;
    // vector<uint> distinctValuesPerColumnCount;
    long long int columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0;
    vector<uint> rowsPerBlockCount;
    bool indexed = false;
    string indexedColumn = "";
    IndexingStrategy indexingStrategy = NOTHING;

    // bool extractColumnNames(string firstLine);
    bool extractColumnCount(string firstline);
    bool blockify();
    void updateStatistics(vector<int> row);
    Matrix();
    Matrix(string matrixname);
    // Matrix(string matrixname, vector<string> columns);
    bool load();
    // bool isColumn(string columnName);
    // void renameColumn(string fromColumnName, string toColumnName);
    void print();
    void rotate();
    int get_element(int row_i, int col_j);
    void set_element (int row_i, int col_j, int elem);
    void makePermanent();
    bool isPermanent();
    
    
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    // int getColumnIndex(string columnName);
    void unload();
    // Cross Transpose
    void crossTranspose(Matrix *matrix2);
    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row)
    {
        logger.log("Table::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};