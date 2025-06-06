#include "global.h"
/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName 
 * @param pageIndex 
 */
Page::Page(string tableName, int pageIndex)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    
    Table* tempTable = tableCatalogue.getTable(tableName);
    if (!tempTable) {
        cerr << "Error: Table not found in catalog" << endl;
        return;
    }
    Table table = *tempTable;
    
    this->columnCount = table.columnCount;
    uint maxRowCount = table.maxRowsPerBlock;
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);

    ifstream fin(pageName, ios::in);
    if (!fin.is_open()) {
        cerr << "Error: Could not open file " << pageName << endl;
        return;
    }

    this->rowCount = table.rowsPerBlockCount[pageIndex];
    string line;
    for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++) {
        getline(fin, line);
        stringstream ss(line);
        string value;
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++) {
            if (getline(ss, value, ' ')) {
                this->rows[rowCounter][columnCounter] = stoi(value);
            }
        }
    }
    fin.close();
}

Page::Page(string tableName, int pageIndex, int is_matrix)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);

    // Check if matrix exists
    Matrix* matrixPtr = matrixCatalogue.getmatrix(tableName);
    if (!matrixPtr) {
        cerr << "Error: Matrix not found in catalogue." << endl;
        return;
    }
    Matrix matrix = *matrixPtr;

    this->columnCount = matrix.columnCount;
    uint maxRowCount = matrix.maxRowsPerBlock;

    // Ensure columnCount and maxRowCount are valid
    if (columnCount <= 0 || maxRowCount <= 0) {
        cerr << "Error: Invalid matrix dimensions (columns: " << columnCount 
             << ", max rows per block: " << maxRowCount << ")." << endl;
        return;
    }

    // Initialize rows
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);

    // Open file and check if successful
    ifstream fin(pageName, ios::in);
    if (!fin.is_open()) {
        cerr << "Error: Unable to open file " << pageName << endl;
        return;
    }

    // Ensure pageIndex is within bounds
    if (pageIndex >= matrix.rowsPerBlockCount.size()) {
        cerr << "Error: pageIndex out of bounds for rowsPerBlockCount." << endl;
        return;
    }

    this->rowCount = matrix.rowsPerBlockCount[pageIndex];

    int number;
    for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            if (!(fin >> number)) {
                cerr << "Error: Not enough data in file or incorrect format at row " 
                     << rowCounter << ", column " << columnCounter << "." << endl;
                return;
            }
            this->rows[rowCounter][columnCounter] = number;
        }
    }

    fin.close();
}


/**
 * @brief Get row from page indexed by rowIndex
 * 
 * @param rowIndex 
 * @return vector<int> 
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

Page::Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    this->columnCount = rows[0].size();
    this->pageName = "../data/temp/"+this->tableName + "_Page" + to_string(pageIndex);
}

/**
 * @brief writes current page contents to file.
 * 
 */
void Page::writePage()
{
    logger.log("Page::writePage");
    ofstream fout(this->pageName, ios::trunc);
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
}

void Page::updateRow(int rowIndex, vector<int> newRow) {
    if (rowIndex >= this->rows.size()) {
        cout << "Error: Row index out of bounds." << endl;
        return;
    }
    this->rows[rowIndex] = newRow;
}

vector<vector<int>> Page::getAllRows() {
    logger.log("Page::getAllRows");
    return this->rows;
}

int Page::getrowcount(){
    logger.log("Page::getRowCount");
    return this->rowCount;
}