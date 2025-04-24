#include "global.h"
#include <cctype>  // for isdigit
#include <string>
#include <iostream>
#include <typeinfo>
#include <cctype>  // for isdigit
#include <string>
#include <iostream>

/**
 * @brief Destructor for Table class
 */
Table::~Table() {
    // cout << "DEBUG: Table destructor called for " << this->tableName << endl;
    
    // First, set the legacy bPlusTreeIndex to nullptr to avoid double deletion
    // since it points to the same object as one of the indices
    bPlusTreeIndex = nullptr;
    
    // Clean up all indices in the map
    // cout << "DEBUG: Cleaning up " << indices.size() << " indices" << endl;
    for (auto& pair : indices) {
        if (pair.second != nullptr) {
            if (pair.second->bPlusTreeIndex != nullptr) {
                // cout << "DEBUG: Deleting B+ tree index for column " << pair.first << endl;
                delete pair.second->bPlusTreeIndex;
                pair.second->bPlusTreeIndex = nullptr;
            }
            delete pair.second;
            pair.second = nullptr;
        }
    }
    
    indices.clear();
    // cout << "DEBUG: Table destructor completed for " << this->tableName << endl;
}

/**
 * @brief Build an index on the specified column
 * 
 * @param columnName the name of the column to index
 * @return true if index is built successfully
 * @return false otherwise
 */
bool Table::buildIndex(string columnName) {
    logger.log("Table::buildIndex");
    cout << "Building B+ tree index on " << this->tableName << "." << columnName << endl;
    
    if (!this->isColumn(columnName)) {
        cout << "Error: Column " << columnName << " does not exist in table " << this->tableName << endl;
        return false;
    }
    
    // Mark this table as being indexed to prevent its pages from being evicted
    bufferManager.markTableAsBeingIndexed(this->tableName);
    
    // Load all pages into memory before starting to build the index
    // This helps prevent page eviction issues during index building
    // cout << "DEBUG: Pre-loading table pages into memory" << endl;
    try {
        // Create a cursor to iterate through the table
        Cursor cursor = this->getCursor();
        vector<int> row = cursor.getNext();
        
        // Just iterate through all rows to load pages into memory
        while (!row.empty()) {
            row = cursor.getNext();
        }
        
        // cout << "DEBUG: Pre-loaded all table pages into memory" << endl;
    } catch (const exception& e) {
        cout << "WARNING: Exception during page pre-loading: " << e.what() << endl;
        // Continue anyway, this is just an optimization
    }
    
    // cout << "DEBUG: Checking if index already exists for column " << columnName << endl;
    
    // Check if we already have an index on this column in our map
    auto it = indices.find(columnName);
    if (it != indices.end()) {
        // cout << "DEBUG: Found existing index for column " << columnName << endl;
        
        // We already have an index for this column
        IndexInfo* indexInfo = it->second;
        
        // If the index exists but is not a B+ tree, rebuild it
        if (indexInfo->strategy != BTREE) {
            cout << "Rebuilding index as B+ tree..." << endl;
        } else {
            // If we already have a B+ tree index on this column, check if it's loaded
            if (indexInfo->bPlusTreeIndex != nullptr) {
                cout << "B+ tree index already loaded and ready to use" << endl;
                
                // Update legacy fields for backward compatibility
                this->indexed = true;
                this->indexedColumn = columnName;
                this->indexingStrategy = BTREE;
                // Just point to the same object, don't create a new reference
                this->bPlusTreeIndex = indexInfo->bPlusTreeIndex;
                
                return true;
            } else {
                // Try to load the existing index from disk
                cout << "Loading existing B+ tree index from disk..." << endl;
                try {
                    // cout << "DEBUG: Creating new BPlusTree object for " << this->tableName << "." << columnName << endl;
                    indexInfo->bPlusTreeIndex = new BPlusTree(4, this->tableName, columnName);
                    // cout << "DEBUG: BPlusTree object created successfully" << endl;
                    
                    if (indexInfo->bPlusTreeIndex->loadFromDisk()) {
                        cout << "Successfully loaded B+ tree index from disk" << endl;
                        
                        // Update legacy fields for backward compatibility
                        this->indexed = true;
                        this->indexedColumn = columnName;
                        this->indexingStrategy = BTREE;
                        // Just point to the same object, don't create a new reference
                        this->bPlusTreeIndex = indexInfo->bPlusTreeIndex;
                        
                        return true;
                    } else {
                        cout << "Failed to load B+ tree index from disk, rebuilding..." << endl;
                        delete indexInfo->bPlusTreeIndex;
                        indexInfo->bPlusTreeIndex = nullptr;
                        // Continue to rebuild the index
                    }
                } catch (const exception& e) {
                    cout << "ERROR: Exception while loading B+ tree index: " << e.what() << endl;
                    if (indexInfo->bPlusTreeIndex != nullptr) {
                        delete indexInfo->bPlusTreeIndex;
                        indexInfo->bPlusTreeIndex = nullptr;
                    }
                    // Continue to rebuild the index
                }
            }
        }
    } else {
        // cout << "DEBUG: No existing index found for column " << columnName << endl;
        
        // Create a new index info object
        IndexInfo* newIndexInfo = new IndexInfo(columnName, BTREE);
        indices[columnName] = newIndexInfo;
        // cout << "DEBUG: Created new index info for column " << columnName << endl;
    }
    
    // Get the column index
    // cout << "DEBUG: Getting column index for " << columnName << " in table " << this->tableName << endl;
    int columnIndex = this->getColumnIndex(columnName);
    // cout << "DEBUG: Column index is " << columnIndex << endl;
    
    try {
        // Get the index info (either existing or newly created)
        IndexInfo* indexInfo = indices[columnName];
        
        // Clean up any existing B+ tree index
        if (indexInfo->bPlusTreeIndex != nullptr) {
            // cout << "DEBUG: Cleaning up existing B+ tree index for column " << columnName << endl;
            delete indexInfo->bPlusTreeIndex;
            indexInfo->bPlusTreeIndex = nullptr;
        }
        
        // Create a new B+ tree index (order = 4 for simplicity, can be adjusted)
        // cout << "DEBUG: Creating new BPlusTree object for " << this->tableName << "." << columnName << endl;
        indexInfo->bPlusTreeIndex = new BPlusTree(4, this->tableName, columnName);
        // cout << "DEBUG: BPlusTree object created successfully" << endl;
        
        // Create a cursor to iterate through the table
        // cout << "DEBUG: Creating cursor to iterate through table" << endl;
        Cursor cursor = this->getCursor();
        vector<int> row = cursor.getNext();
        int rowCounter = 0;
        
        // For each row, extract the value of the indexed column and insert it into the B+ tree
        // cout << "DEBUG: Starting to process rows for B+ tree index" << endl;
        while (!row.empty()) {
            if (columnIndex >= row.size()) {
                cout << "ERROR: Column index " << columnIndex << " is out of bounds for row with size " << row.size() << endl;
                delete indexInfo->bPlusTreeIndex;
                indexInfo->bPlusTreeIndex = nullptr;
                return false;
            }
            
            try {
                int value = row[columnIndex];
                
                // Insert the value and row number into the B+ tree
                // cout << "DEBUG: Inserting value " << value << " at row " << rowCounter << " into B+ tree" << endl;
                indexInfo->bPlusTreeIndex->insert(value, rowCounter);
                
                row = cursor.getNext();
                rowCounter++;
                
                // Print progress every 100 rows
                if (rowCounter % 100 == 0) {
                    // cout << "DEBUG: Processed " << rowCounter << " rows so far" << endl;
                }
            } catch (const exception& e) {
                cout << "ERROR: Exception while inserting into B+ tree: " << e.what() << endl;
                
                // Continue with the next row instead of aborting the entire indexing process
                row = cursor.getNext();
                rowCounter++;
            }
        }
        
        cout << "Processed " << rowCounter << " rows for B+ tree index" << endl;
        
        // Save the B+ tree to disk
        // cout << "DEBUG: Saving B+ tree to disk" << endl;
        if (indexInfo->bPlusTreeIndex->saveToDisk()) {
            // Update the index info
            indexInfo->strategy = BTREE;
            
            // Update legacy fields for backward compatibility
            this->indexed = true;
            this->indexedColumn = columnName;
            this->indexingStrategy = BTREE;
            // Just point to the same object, don't create a new reference
            this->bPlusTreeIndex = indexInfo->bPlusTreeIndex;
            
            cout << "B+ tree index built successfully on " << this->tableName << "." << columnName << endl;
            // cout << "DEBUG: Table now has " << indices.size() << " indices" << endl;
            
            // Unmark this table as being indexed
            bufferManager.unmarkTableAsBeingIndexed(this->tableName);
            
            return true;
        }
        
        cout << "Error: Failed to build B+ tree index" << endl;
        delete indexInfo->bPlusTreeIndex;
        indexInfo->bPlusTreeIndex = nullptr;
        
        // Unmark this table as being indexed
        bufferManager.unmarkTableAsBeingIndexed(this->tableName);
        
        return false;
    } catch (const exception& e) {
        cout << "ERROR: Exception while building B+ tree index: " << e.what() << endl;
        
        // Clean up in case of exception
        auto it = indices.find(columnName);
        if (it != indices.end() && it->second != nullptr) {
            if (it->second->bPlusTreeIndex != nullptr) {
                delete it->second->bPlusTreeIndex;
                it->second->bPlusTreeIndex = nullptr;
            }
        }
        
        // Unmark this table as being indexed
        bufferManager.unmarkTableAsBeingIndexed(this->tableName);
        
        return false;
    }
}

/**
 * @brief Search the index for rows matching the condition
 * 
 * @param columnName the name of the column to search
 * @param value the value to search for
 * @param op the binary operator for comparison
 * @return vector<int> a vector of row numbers that match the condition
 */
vector<int> Table::searchIndexed(string columnName, int value, BinaryOperator op) {
    logger.log("Table::searchIndexed");
    cout << "Searching index on " << this->tableName << "." << columnName << " for value " << value << endl;
    
    vector<int> matchingRows;
    
    if (!this->isIndexed(columnName)) {
        cout << "Error: Table " << this->tableName << " is not indexed on column " << columnName << endl;
        return matchingRows;
    }
    
    // First check in our indices map
    auto it = indices.find(columnName);
    if (it != indices.end() && it->second != nullptr) {
        IndexInfo* indexInfo = it->second;
        
        // Check if we have a B+ tree index
        if (indexInfo->strategy == BTREE) {
            try {
                // If the B+ tree index is not loaded, load it
                if (indexInfo->bPlusTreeIndex == nullptr) {
                    // cout << "DEBUG: Loading B+ tree index from disk for column " << columnName << endl;
                    indexInfo->bPlusTreeIndex = new BPlusTree(4, this->tableName, columnName);
                    
                    if (!indexInfo->bPlusTreeIndex->loadFromDisk()) {
                        cout << "Error: Failed to load B+ tree index from disk, rebuilding index..." << endl;
                        
                        // Rebuild the index
                        if (!this->buildIndex(columnName)) {
                            cout << "Error: Failed to rebuild B+ tree index" << endl;
                            return matchingRows;
                        }
                    }
                }
                
                // Make sure the index is loaded and valid
                if (indexInfo->bPlusTreeIndex == nullptr) {
                    cout << "Error: B+ tree index is null after loading/rebuilding" << endl;
                    return matchingRows;
                }
                
                // Update legacy bPlusTreeIndex for backward compatibility
                // Just point to the same object, don't create a new reference
                this->bPlusTreeIndex = indexInfo->bPlusTreeIndex;
                
                // Use the B+ tree to search for matching rows
                cout << "Using B+ tree index for search" << endl;
                matchingRows = indexInfo->bPlusTreeIndex->search(value, op);
                cout << "B+ tree search found " << matchingRows.size() << " matching rows" << endl;
            } catch (const exception& e) {
                cout << "ERROR: Exception while searching B+ tree index: " << e.what() << endl;
                
                // Clean up in case of exception
                if (indexInfo->bPlusTreeIndex != nullptr) {
                    delete indexInfo->bPlusTreeIndex;
                    indexInfo->bPlusTreeIndex = nullptr;
                }
                
                // Fall back to sequential scan
                cout << "Falling back to sequential scan..." << endl;
                int columnIndex = this->getColumnIndex(columnName);
                Cursor cursor = this->getCursor();
                vector<int> row = cursor.getNext();
                int rowCounter = 0;
                
                while (!row.empty()) {
                    if (columnIndex < row.size()) {
                        int rowValue = row[columnIndex];
                        if (evaluateBinOp(rowValue, value, op)) {
                            matchingRows.push_back(rowCounter);
                        }
                    }
                    row = cursor.getNext();
                    rowCounter++;
                }
            }
        } else {
            // Fall back to the old index implementation
            cout << "Using legacy index for search" << endl;
            
            string indexTableName = this->tableName + "_" + columnName + "_index";
            Table* indexTable = tableCatalogue.getTable(indexTableName);
            
            if (!indexTable) {
                cout << "Error: Index table not found" << endl;
                return matchingRows;
            }
            
            // Load all index data into memory
            vector<pair<int, int>> indexData;
            try {
                Cursor cursor = indexTable->getCursor();
                vector<int> indexRow = cursor.getNext();
                
                while (!indexRow.empty()) {
                    if (indexRow.size() >= 2) {
                        indexData.push_back(make_pair(indexRow[0], indexRow[1]));
                    }
                    indexRow = cursor.getNext();
                }
            } catch (const exception& e) {
                cout << "ERROR: Exception while loading index data: " << e.what() << endl;
                return matchingRows;
            }
            
            // Sort the index data by value (should already be sorted, but just to be safe)
            sort(indexData.begin(), indexData.end());
            
            // For EQUAL operator, we can use binary search for better performance
            if (op == EQUAL) {
                // Perform binary search
                int left = 0;
                int right = indexData.size() - 1;
                
                while (left <= right) {
                    int mid = left + (right - left) / 2;
                    int midValue = indexData[mid].first;
                    
                    if (midValue == value) {
                        // Found a match, add it to the result
                        matchingRows.push_back(indexData[mid].second);
                        
                        // Check for more matches to the left
                        int leftPtr = mid - 1;
                        while (leftPtr >= 0 && indexData[leftPtr].first == value) {
                            matchingRows.push_back(indexData[leftPtr].second);
                            leftPtr--;
                        }
                        
                        // Check for more matches to the right
                        int rightPtr = mid + 1;
                        while (rightPtr < indexData.size() && indexData[rightPtr].first == value) {
                            matchingRows.push_back(indexData[rightPtr].second);
                            rightPtr++;
                        }
                        
                        break;
                    } else if (midValue < value) {
                        left = mid + 1;
                    } else {
                        right = mid - 1;
                    }
                }
            } else {
                // For other operators, we need to scan the index
                for (const auto& entry : indexData) {
                    int indexValue = entry.first;
                    int rowNumber = entry.second;
                    
                    if (evaluateBinOp(indexValue, value, op)) {
                        matchingRows.push_back(rowNumber);
                    }
                    
                    // Optimization: If we've passed the value for certain operators, we can stop
                    if ((op == LESS_THAN || op == LEQ) && indexValue > value) {
                        break;
                    }
                }
            }
        }
    } else if (this->indexed && this->indexedColumn == columnName) {
        // For backward compatibility, use the legacy fields
        // cout << "DEBUG: Using legacy index fields for column " << columnName << endl;
        
        // Check if we have a B+ tree index
        if (this->indexingStrategy == BTREE) {
            try {
                // If the B+ tree index is not loaded, load it
                if (bPlusTreeIndex == nullptr) {
                    cout << "Loading B+ tree index from disk..." << endl;
                    bPlusTreeIndex = new BPlusTree(4, this->tableName, columnName);
                    
                    if (!bPlusTreeIndex->loadFromDisk()) {
                        cout << "Error: Failed to load B+ tree index from disk, rebuilding index..." << endl;
                        
                        // Rebuild the index
                        if (!this->buildIndex(columnName)) {
                            cout << "Error: Failed to rebuild B+ tree index" << endl;
                            return matchingRows;
                        }
                    }
                }
                
                // Make sure the index is loaded and valid
                if (bPlusTreeIndex == nullptr) {
                    cout << "Error: B+ tree index is null after loading/rebuilding" << endl;
                    return matchingRows;
                }
                
                // Use the B+ tree to search for matching rows
                cout << "Using B+ tree index for search" << endl;
                matchingRows = bPlusTreeIndex->search(value, op);
                cout << "B+ tree search found " << matchingRows.size() << " matching rows" << endl;
            } catch (const exception& e) {
                cout << "ERROR: Exception while searching B+ tree index: " << e.what() << endl;
                if (bPlusTreeIndex != nullptr) {
                    delete bPlusTreeIndex;
                    bPlusTreeIndex = nullptr;
                }
                // Fall back to sequential scan
                cout << "Falling back to sequential scan..." << endl;
                int columnIndex = this->getColumnIndex(columnName);
                Cursor cursor = this->getCursor();
                vector<int> row = cursor.getNext();
                int rowCounter = 0;
                
                while (!row.empty()) {
                    if (columnIndex < row.size()) {
                        int rowValue = row[columnIndex];
                        if (evaluateBinOp(rowValue, value, op)) {
                            matchingRows.push_back(rowCounter);
                        }
                    }
                    row = cursor.getNext();
                    rowCounter++;
                }
            }
        } else {
            // Fall back to the old index implementation
            cout << "Using legacy index for search" << endl;
            
            string indexTableName = this->tableName + "_" + columnName + "_index";
            Table* indexTable = tableCatalogue.getTable(indexTableName);
            
            if (!indexTable) {
                cout << "Error: Index table not found" << endl;
                return matchingRows;
            }
            
            // Load all index data into memory
            vector<pair<int, int>> indexData;
            try {
                Cursor cursor = indexTable->getCursor();
                vector<int> indexRow = cursor.getNext();
                
                while (!indexRow.empty()) {
                    if (indexRow.size() >= 2) {
                        indexData.push_back(make_pair(indexRow[0], indexRow[1]));
                    }
                    indexRow = cursor.getNext();
                }
            } catch (const exception& e) {
                cout << "ERROR: Exception while loading index data: " << e.what() << endl;
                return matchingRows;
            }
            
            // Sort the index data by value (should already be sorted, but just to be safe)
            sort(indexData.begin(), indexData.end());
            
            // For EQUAL operator, we can use binary search for better performance
            if (op == EQUAL) {
                // Perform binary search
                int left = 0;
                int right = indexData.size() - 1;
                
                while (left <= right) {
                    int mid = left + (right - left) / 2;
                    int midValue = indexData[mid].first;
                    
                    if (midValue == value) {
                        // Found a match, add it to the result
                        matchingRows.push_back(indexData[mid].second);
                        
                        // Check for more matches to the left
                        int leftPtr = mid - 1;
                        while (leftPtr >= 0 && indexData[leftPtr].first == value) {
                            matchingRows.push_back(indexData[leftPtr].second);
                            leftPtr--;
                        }
                        
                        // Check for more matches to the right
                        int rightPtr = mid + 1;
                        while (rightPtr < indexData.size() && indexData[rightPtr].first == value) {
                            matchingRows.push_back(indexData[rightPtr].second);
                            rightPtr++;
                        }
                        
                        break;
                    } else if (midValue < value) {
                        left = mid + 1;
                    } else {
                        right = mid - 1;
                    }
                }
            } else {
                // For other operators, we need to scan the index
                for (const auto& entry : indexData) {
                    int indexValue = entry.first;
                    int rowNumber = entry.second;
                    
                    if (evaluateBinOp(indexValue, value, op)) {
                        matchingRows.push_back(rowNumber);
                    }
                    
                    // Optimization: If we've passed the value for certain operators, we can stop
                    if ((op == LESS_THAN || op == LEQ) && indexValue > value) {
                        break;
                    }
                }
            }
        }
    }
    
    cout << "Found " << matchingRows.size() << " matching rows" << endl;
    return matchingRows;
}

/**
 * @brief Check if the table is indexed on the specified column
 * 
 * @param columnName the name of the column to check
 * @return true if the table is indexed on the column
 * @return false otherwise
 */
bool Table::isIndexed(string columnName) {
    logger.log("Table::isIndexed");
    
    // cout << "DEBUG: Checking if table " << this->tableName << " is indexed on column " << columnName << endl;
    
    // First check in our indices map
    auto it = indices.find(columnName);
    if (it != indices.end() && it->second != nullptr) {
        // cout << "DEBUG: Found index for column " << columnName << " in indices map" << endl;
        return true;
    }
    
    // For backward compatibility, also check the legacy fields
    bool legacyResult = this->indexed && this->indexedColumn == columnName;
    
    // cout << "DEBUG: Legacy index check result: " << (legacyResult ? "YES" : "NO") << endl;
    
    return legacyResult || (it != indices.end());
}

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table()
{
    logger.log("Table::Table");
    this->indexed = false;
    this->indexedColumn = "";
    this->indexingStrategy = NOTHING;
}
vector<int> sortValues, columnIndexes;
/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName
 */
Table::Table(string tableName)
{
    logger.log("Table::Table");
    this->indexed = false;
    this->indexedColumn = "";
    this->indexingStrategy = NOTHING;
    if (strncmp(tableName.c_str(), "temp/", 5) == 0)
    {
        this->sourceFileName = "../data/" + tableName + ".csv";
        this->tableName = tableName.substr(5);
    }
    else
    {
        this->sourceFileName = "../data/" + tableName + ".csv";
        this->tableName = tableName;
    }
}
struct MyPair
{
    vector<int> row;
    int blockNum;
    int boundary;
    int cursorIndex;

    MyPair(vector<int> r, int b, int bound, int c)
        : row(r), blockNum(b), boundary(bound), cursorIndex(c) {}
};

struct CompareByFirstElement
{
    bool operator()(const MyPair &lhs, const MyPair &rhs) const
    {
        for (int i = 0; i < columnIndexes.size(); i++)
        {
            if (lhs.row[columnIndexes[i]] != rhs.row[columnIndexes[i]])
            {
                if (sortValues[i] == 0)
                    return lhs.row[columnIndexes[i]] > rhs.row[columnIndexes[i]];
                else if (sortValues[i] == 1)
                    return lhs.row[columnIndexes[i]] < rhs.row[columnIndexes[i]];
            }
        }
        return false;
    }
};

static bool sortComparator(const vector<int> &a, const vector<int> &b)
{
    logger.log("Inside Sort Comp");
    for (int i = 0; i < columnIndexes.size(); i++)
    {
        logger.log("SORT COL INDEX: " + to_string(columnIndexes[i]));
        if (a[columnIndexes[i]] != b[columnIndexes[i]])
        {
            if (sortValues[i] == 0)
                return a[columnIndexes[i]] < b[columnIndexes[i]];
            else if (sortValues[i] == 1)
                return a[columnIndexes[i]] > b[columnIndexes[i]];
        }
    }
    return false;
}

Matrix::Matrix(string matrixname)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixname + ".csv";
    this->matrixname = matrixname;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName
 * @param columns
 */
Table::Table(string tableName, vector<string> columns)
{
    logger.log("Table::Table");
    this->indexed = false;
    this->indexedColumn = "";
    this->indexingStrategy = NOTHING;
    
    // Ensure the temp directory exists
    string tempDir = "../data/temp";
    struct stat info;
    if (stat(tempDir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        // cout << "DEBUG: Creating temp directory: " << tempDir << endl;
        system(("mkdir -p " + tempDir).c_str());
    }
    
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    
    // Initialize distinctValuesInColumns and distinctValuesPerColumnCount
    unordered_set<int> dummy;
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    
    // Make sure we can open the file for writing
    ofstream testFile(this->sourceFileName);
    if (!testFile) {
        cout << "ERROR: Failed to create file at " << this->sourceFileName << endl;
        throw runtime_error("Failed to create file: " + this->sourceFileName);
    }
    testFile.close();
    
    this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded
 * @return false if an error occurred
 */
bool Table::load()
{
    logger.log("Table::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

void Table::deleteTable()
{
    logger.log("Table::deleteTable - Start");

    // Delete the associated file
    string filePath = this->sourceFileName;
    if (remove(filePath.c_str()) == 0)
    {
        logger.log("Deleted file: " + filePath);
    }
    else
    {
        logger.log("Failed to delete file: " + filePath);
    }

    // Delete the table object itself
    delete this;

    logger.log("Table::deleteTable - End");
}

bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnCount(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file.
 *
 * @param line
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine)
{
    logger.log("Table::extractColumnNames");
    unordered_set<string> columnNames;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (columnNames.count(word))
            return false;
        columnNames.insert(word);
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

bool Matrix::extractColumnCount(string firstLine)
{
    logger.log("Matrix::extractColumnCount");
    unordered_set<string> first_row;
    string elem;
    stringstream s(firstLine);
    int count = 0;
    while (getline(s, elem, ','))
    {
        elem.erase(std::remove_if(elem.begin(), elem.end(), ::isspace), elem.end());
        count += 1;
    }
    this->columnCount = count;
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify()
{
    logger.log("Table::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;
    unordered_set<int> dummy;
    dummy.clear();
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    getline(fin, line);
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock)
        {
            bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter)
    {
        bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, elem;
    vector<int> row(this->columnCount, 0);
    vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, elem, ','))
                return false;
            row[columnCounter] = stoi(elem);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock)
        {
            bufferManager.writePage(this->matrixname, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter)
    {
        bufferManager.writePage(this->matrixname, this->blockCount, rowsInPage, pageCounter);
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    // this->distinctValuesInColumns.clear();
    return true;
}
/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row
 */
void Table::updateStatistics(vector<int> row)
{
    this->rowCount++;
    
    // Ensure distinctValuesInColumns and distinctValuesPerColumnCount are properly sized
    if (this->distinctValuesInColumns.size() != this->columnCount) {
        unordered_set<int> dummy;
        this->distinctValuesInColumns.assign(this->columnCount, dummy);
    }
    
    if (this->distinctValuesPerColumnCount.size() != this->columnCount) {
        this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    }
    
    // Update statistics for each column
    for (int columnCounter = 0; columnCounter < this->columnCount && columnCounter < row.size(); columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}
void Table::sortTable(bool makePermanent)
{
    logger.log("Table::sortTable");

    // Initialize sorting parameters
    sortValues.resize(parsedQuery.sortStrategy.size(), 0);
    columnIndexes.resize(parsedQuery.sortStrategy.size());

    // Store sorting order
    for (int i = 0; i < parsedQuery.sortStrategy.size(); i++)
    {
        if (parsedQuery.sortStrategy[i] == DESC)
            sortValues[i] = 1;
    }

    // Store column indices to sort by
    for (int i = 0; i < parsedQuery.sortColumns.size(); i++)
    {
        columnIndexes[i] = this->getColumnIndex(parsedQuery.sortColumns[i]);
    }

    // Sort each page individually first
    for (int pageIndex = 0; pageIndex < this->blockCount; pageIndex++)
    {
        Page page = bufferManager.getPage(this->tableName, pageIndex);
        vector<vector<int>> pageData = page.getAllRows();
        int rowCount = page.getrowcount();

        // Sort the page data
        sort(pageData.begin(), pageData.begin() + rowCount, sortComparator);

        // Write back the sorted page
        bufferManager.writePage(this->tableName, pageIndex, pageData, rowCount);
    }

    // Perform external merge sort
    this->externalSort();

    // Make the sorted table permanent only if requested
    if (makePermanent)
    {
        this->makePermanent();
    }
}

void Table::externalSort()
{
    logger.log("Table::externalSort");

    int K = BLOCK_COUNT - 1; // K-way merge
    int mergeIterations = ceil(log(this->blockCount) / log(K));

    for (int iteration = 0; iteration < mergeIterations; iteration++)
    {
        int size = pow(K, iteration);
        int clusters = ceil((double)this->blockCount / size);

        for (int cluster = 0; cluster < clusters; cluster++)
        {
            int start = cluster * size;
            int end = min(start + size - 1, (int)this->blockCount - 1);

            // Create output buffer for merged data
            vector<vector<int>> outputBuffer;
            outputBuffer.reserve(this->maxRowsPerBlock);

            // Create priority queue for K-way merge
            priority_queue<MyPair, vector<MyPair>, CompareByFirstElement> pq;
            vector<Cursor> cursors;

            // Initialize cursors for each block in the cluster
            for (int i = start; i <= end; i++)
            {
                Cursor cursor(this->tableName, i);
                cursors.push_back(cursor);

                // Get first row from each block
                vector<int> row = cursor.getNext();
                if (!row.empty())
                {
                    pq.push({row, i, end, cursors.size() - 1});
                }
            }

            // Merge blocks
            int outputPageIndex = 0;
            while (!pq.empty())
            {
                MyPair top = pq.top();
                pq.pop();

                outputBuffer.push_back(top.row);

                // If output buffer is full, write it to disk
                if (outputBuffer.size() == this->maxRowsPerBlock)
                {
                    bufferManager.writePage(this->tableName, outputPageIndex, outputBuffer, outputBuffer.size());
                    outputBuffer.clear();
                    outputPageIndex++;
                }

                // Get next row from the block that provided the top element
                vector<int> nextRow = cursors[top.cursorIndex].getNext();
                if (!nextRow.empty())
                {
                    pq.push({nextRow, top.blockNum, top.boundary, top.cursorIndex});
                }
            }

            // Write remaining rows in output buffer
            if (!outputBuffer.empty())
            {
                bufferManager.writePage(this->tableName, outputPageIndex, outputBuffer, outputBuffer.size());
            }
        }
    }
}

void Matrix::updateStatistics(vector<int> row)
{
    this->rowCount++;
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName
 * @return true
 * @return false
 */
bool Table::isColumn(string columnName)
{
    logger.log("Table::isColumn");
    // cout << "DEBUG: Checking if column " << columnName << " exists in table " << this->tableName << endl;
    // cout << "DEBUG: Table has " << this->columnCount << " columns: ";
    for (int i = 0; i < this->columnCount; i++) {
        cout << this->columns[i];
        if (i < this->columnCount - 1) cout << ", ";
    }
    cout << endl;
    
    for (auto col : this->columns)
    {
        if (col == columnName)
        {
            // cout << "DEBUG: Column " << columnName << " found in table " << this->tableName << endl;
            return true;
        }
    }
    
    // cout << "DEBUG: Column " << columnName << " NOT found in table " << this->tableName << endl;
    return false;
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName
 * @param toColumnName
 */
void Table::renameColumn(string fromColumnName, string toColumnName)
{
    logger.log("Table::renameColumn");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (columns[columnCounter] == fromColumnName)
        {
            columns[columnCounter] = toColumnName;
            break;
        }
    }
    return;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print()
{
    logger.log("Table::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    // print headings
    this->writeRow(this->columns, cout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}
void Matrix::print()
{
    logger.log("Matrix :: Print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    Cursor cursor(this->matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}

void Matrix::rotate()
{
    // cout << "GET ELEM : " << this->get_element(2,2) << endl;
    // this->set_element(2,2,69);
    // cout << "GET ELEM after set : " << this->get_element(2,2) << endl;
    if (this->rowCount != this->columnCount)
    {
        // In-place rotation is not supported for non-square matrices
        std::cerr << "In-place rotation is only supported for square matrices." << std::endl;
        return;
    }

    int N = this->rowCount; // Since it's a square matrix, rowCount == columnCount

    // Rotate the matrix layer by layer
    for (int layer = 0; layer < N / 2; ++layer)
    {
        int first = layer;
        int last = N - 1 - layer;

        for (int i = first; i < last; ++i)
        {
            int offset = i - first;

            // Save the top element
            int top = this->get_element(first, i);

            // Left -> Top
            this->set_element(first, i, this->get_element(last - offset, first));

            // Bottom -> Left
            this->set_element(last - offset, first, this->get_element(last, last - offset));

            // Right -> Bottom
            this->set_element(last, last - offset, this->get_element(i, last));

            // Top (saved) -> Right
            this->set_element(i, last, top);
        }
    }
}

int Matrix::get_element(int i, int j)
{
    logger.log("Matrix :: get_element");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    Cursor cursor(this->matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count && rowCounter < i; rowCounter++)
    {
        row = cursor.getNext();
    }
    row = cursor.getNext();

    // now we need to get the jth element from the row.
    return row[j];
}

void Matrix::set_element(int row, int col, int value)
{
    // Ensure row and col are within bounds
    if (row >= this->rowCount || col >= this->columnCount || row < 0 || col < 0)
    {
        cout << "Error: Attempting to access out-of-bounds element." << endl;
        return;
    }

    // Fetch the page/block that contains the element
    int pageIndex = row / this->maxRowsPerBlock;
    int rowInPage = row % this->maxRowsPerBlock;
    string pageName = "../data/temp/" + this->matrixname + "_Page" + to_string(pageIndex);

    // Load the page from buffer (if not already in memory)
    Page page = bufferManager.getPage(this->matrixname, pageIndex, 1);
    vector<int> matrixRow = page.getRow(rowInPage);

    // Update the element in the row
    matrixRow[col] = value;

    // Write the updated row back to the page
    page.updateRow(rowInPage, matrixRow);

    // Optionally mark the page as dirty, if your buffer manager tracks changes
    bufferManager.writePage(this->matrixname, pageIndex, page.getAllRows(), page.getrowcount());
    matrixCatalogue.updateMatrix(this);
    bufferManager.deletePage(pageName);
}

/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Table::getNextPage(Cursor *cursor)
{
    logger.log("Table::getNext");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex + 1);
    }
}

void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");
    if (cursor->pageIndex < this->blockCount - 1)
        cursor->nextPage(cursor->pageIndex + 1);
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent()
{
    logger.log("Table::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    // print headings
    this->writeRow(this->columns, fout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

void Matrix::makePermanent()
{
    logger.log("Table::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixname + ".csv";
    ofstream fout(newSourceFile, ios::out);

    Cursor cursor(this->matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent()
{
    logger.log("Table::isPermanent");
    if (this->sourceFileName == "../data/" + this->tableName + ".csv")
        return true;
    return false;
}

bool Matrix::isPermanent()
{
    logger.log("Matrix :: isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixname + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload()
{
    logger.log("Table::~unload");
    
    // We don't delete bPlusTreeIndex here because it's just a pointer to an index
    // in the indices map, which will be cleaned up in the destructor
    bPlusTreeIndex = nullptr;
    
    // Clean up all indices in the map
    for (auto& pair : indices) {
        if (pair.second != nullptr) {
            if (pair.second->bPlusTreeIndex != nullptr) {
                delete pair.second->bPlusTreeIndex;
                pair.second->bPlusTreeIndex = nullptr;
            }
            delete pair.second;
            pair.second = nullptr;
        }
    }
    
    indices.clear();
    
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

void Matrix::unload()
{
    logger.log("Table::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixname, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 *
 * @return Cursor
 */
Cursor Table::getCursor()
{
    logger.log("Table::getCursor");
    Cursor cursor(this->tableName, 0);
    return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 *
 * @param columnName
 * @return int
 */
int Table::getColumnIndex(string columnName)
{
    logger.log("Table::getColumnIndex");
    // cout << "DEBUG: Getting column index for " << columnName << " in table " << this->tableName << endl;
    // cout << "DEBUG: Table has " << this->columnCount << " columns: ";
    for (int i = 0; i < this->columnCount; i++) {
        cout << this->columns[i];
        if (i < this->columnCount - 1) cout << ", ";
    }
    cout << endl;
    
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName) {
            // cout << "DEBUG: Found column " << columnName << " at index " << columnCounter << endl;
            return columnCounter;
        }
    }
    
    cout << "ERROR: Column " << columnName << " not found in table " << this->tableName << endl;
    return -1; // Return -1 if column not found
}

void Table::groupBy()
{
    logger.log("Table::groupBy");
    string newTableName = parsedQuery.groupByResultRelationName;
    string groupingAttribute = parsedQuery.groupAttribute;
    string havingaggregateAttribute = parsedQuery.havingAttribute;
    Aggregate havingaggregateFunction = parsedQuery.havingAgg;
    BinaryOperator binOp = parsedQuery.groupBinaryOperator;
    int attributeValue = parsedQuery.havingValue;
    string aggregateAttribute = parsedQuery.returnAttribute;
    Aggregate aggregateFunction = parsedQuery.returnAgg;
    string stringaggregateFunction = parsedQuery.returnAggregate;

    cout << "\n=== Starting GROUP BY Operation ===" << endl;
    cout << "Grouping by: " << groupingAttribute << endl;
    cout << "Aggregating: " << aggregateAttribute << " with function: " << stringaggregateFunction << endl;
    cout << "Having clause: " << havingaggregateAttribute << " " << (binOp == EQUAL ? "=" : (binOp == GREATER_THAN ? ">" : ">=")) << " " << attributeValue << endl;

    // First sort the table by the grouping attribute
    logger.log("External sorting for GROUP BY");
    parsedQuery.sortStrategy.clear();
    parsedQuery.sortStrategy.push_back(ASC);
    parsedQuery.sortColumns.clear();
    parsedQuery.sortColumns.push_back(groupingAttribute);

    cout << "\nSorting table by " << groupingAttribute << "..." << endl;
    this->sortTable(false); // Don't make sorting permanent
    this->unload();

    // print the table
    this->print();

    // Create the result table with appropriate columns
    vector<string> header = {groupingAttribute, stringaggregateFunction + "(" + aggregateAttribute + ")"};
    Table *groupedTable = new Table(newTableName, header);
    logger.log("Created new table for result");

    // Initialize variables for grouping
    int currentGroupValue = -1;
    int havingaggregateResult = 0;
    int havingaggregateCount = 0;
    int aggregationResult = 0;
    int aggregateCount = 0;
    bool isFirstGroup = true;

    // Get column indices for grouping and aggregate attributes
    int groupIndex = this->getColumnIndex(groupingAttribute);
    int havingaggregateIndex = this->getColumnIndex(havingaggregateAttribute);
    int aggregateIndex = this->getColumnIndex(aggregateAttribute);

    cout << "\nColumn indices:" << endl;
    cout << "Group index: " << groupIndex << endl;
    cout << "Aggregate index: " << aggregateIndex << endl;
    cout << "Having aggregate index: " << havingaggregateIndex << endl;

    // Process the sorted data
    Cursor cursor = this->getCursor();
    vector<int> row = cursor.getNext();
    logger.log("Starting GROUP BY processing");

    int currRow = 0;
    long long totalRow = 0;
    int pageCounter = 0;
    vector<vector<int>> pageData;

    cout << "\nProcessing rows..." << endl;
    while (!row.empty())
    {
        int groupValue = row[groupIndex];
        int aggregateValue = row[aggregateIndex];
        int havingaggregateValue = row[havingaggregateIndex];

        if (groupValue != currentGroupValue)
        {
            // Process the previous group
            if (!isFirstGroup)
            {
                if (havingaggregateFunction == AVG && havingaggregateCount > 0)
                {
                    havingaggregateResult = havingaggregateResult / havingaggregateCount;
                }

                cout << "\nProcessing group with value: " << currentGroupValue << endl;
                cout << "Having aggregate result: " << havingaggregateResult << endl;
                cout << "Having aggregate count: " << havingaggregateCount << endl;

                bool havingConditionMet = false;
                if (havingaggregateCount > 0)
                {
                    if (binOp == EQUAL)
                    {
                        havingConditionMet = (havingaggregateResult == attributeValue);
                    }
                    else if (binOp == GREATER_THAN)
                    {
                        havingConditionMet = (havingaggregateResult > attributeValue);
                    }
                    else if (binOp == GEQ)
                    {
                        havingConditionMet = (havingaggregateResult >= attributeValue);
                    }
                }

                if (havingConditionMet)
                {
                    cout << "Having condition met for group " << currentGroupValue << endl;
                    vector<int> resultRow;

                    if (aggregateFunction == MIN || aggregateFunction == MAX || aggregateFunction == SUM)
                    {
                        resultRow = {currentGroupValue, aggregationResult};
                    }
                    else if (aggregateFunction == COUNT)
                    {
                        resultRow = {currentGroupValue, aggregateCount};
                    }
                    else if (aggregateFunction == AVG && aggregateCount > 0)
                    {
                        resultRow = {currentGroupValue, aggregationResult / aggregateCount};
                    }

                    cout << "Adding result row: [" << resultRow[0] << ", " << resultRow[1] << "]" << endl;
                    pageData.push_back(resultRow);
                    currRow++;
                    totalRow++;
                }
            }

            // Start new group
            currentGroupValue = groupValue;
            aggregationResult = aggregateValue; // Initialize with first value
            aggregateCount = 1;
            havingaggregateCount = 1;
            havingaggregateResult = havingaggregateValue;
            isFirstGroup = false;
        }
        else
        {
            // Update aggregation results for current group
            if (aggregateFunction == MIN)
            {
                aggregationResult = min(aggregationResult, aggregateValue);
            }
            else if (aggregateFunction == MAX)
            {
                aggregationResult = max(aggregationResult, aggregateValue);
            }
            else if (aggregateFunction == SUM)
            {
                aggregationResult += aggregateValue;
            }
            else if (aggregateFunction == COUNT)
            {
                aggregateCount++;
            }
            else if (aggregateFunction == AVG)
            {
                aggregationResult += aggregateValue;
                aggregateCount++;
            }

            // Update having clause results
            if (havingaggregateFunction == MIN)
            {
                havingaggregateResult = min(havingaggregateResult, havingaggregateValue);
            }
            else if (havingaggregateFunction == MAX)
            {
                havingaggregateResult = max(havingaggregateResult, havingaggregateValue);
            }
            else if (havingaggregateFunction == SUM)
            {
                havingaggregateResult += havingaggregateValue;
            }
            else if (havingaggregateFunction == COUNT)
            {
                havingaggregateCount++;
            }
            else if (havingaggregateFunction == AVG)
            {
                havingaggregateResult += havingaggregateValue;
                havingaggregateCount++;
            }
        }

        // Write page if full
        if (currRow == groupedTable->maxRowsPerBlock)
        {
            cout << "Writing page " << pageCounter << " with " << currRow << " rows" << endl;
            bufferManager.writePage(groupedTable->tableName, pageCounter, pageData, currRow);
            pageCounter++;
            currRow = 0;
            pageData.clear();
        }

        row = cursor.getNext();
    }

    // Process the last group
    if (!isFirstGroup)
    {
        if (havingaggregateFunction == AVG && havingaggregateCount > 0)
        {
            havingaggregateResult = havingaggregateResult / havingaggregateCount;
        }

        cout << "\nProcessing final group with value: " << currentGroupValue << endl;
        cout << "Having aggregate result: " << havingaggregateResult << endl;
        cout << "Having aggregate count: " << havingaggregateCount << endl;

        bool havingConditionMet = false;
        if (havingaggregateCount > 0)
        {
            if (binOp == EQUAL)
            {
                havingConditionMet = (havingaggregateResult == attributeValue);
            }
            else if (binOp == GREATER_THAN)
            {
                havingConditionMet = (havingaggregateResult > attributeValue);
            }
            else if (binOp == GEQ)
            {
                havingConditionMet = (havingaggregateResult >= attributeValue);
            }
        }

        if (havingConditionMet)
        {
            cout << "Having condition met for final group " << currentGroupValue << endl;
            vector<int> resultRow;

            if (aggregateFunction == MIN || aggregateFunction == MAX || aggregateFunction == SUM)
            {
                resultRow = {currentGroupValue, aggregationResult};
            }
            else if (aggregateFunction == COUNT)
            {
                resultRow = {currentGroupValue, aggregateCount};
            }
            else if (aggregateFunction == AVG && aggregateCount > 0)
            {
                resultRow = {currentGroupValue, aggregationResult / aggregateCount};
            }

            cout << "Adding final result row: [" << resultRow[0] << ", " << resultRow[1] << "]" << endl;
            pageData.push_back(resultRow);
            currRow++;
            totalRow++;
        }
    }

    // Write remaining data
    if (!pageData.empty())
    {
        cout << "Writing final page " << pageCounter << " with " << currRow << " rows" << endl;
        bufferManager.writePage(groupedTable->tableName, pageCounter, pageData, currRow);
        pageCounter++;
    }

    cout << "\nTotal rows in result: " << totalRow << endl;
    cout << "Total pages: " << pageCounter << endl;

    // Initialize the result table properly
    groupedTable->rowCount = totalRow;
    groupedTable->columnCount = 2;
    groupedTable->blockCount = pageCounter;
    groupedTable->sourceFileName = "../data/" + groupedTable->tableName + ".csv";

    // Insert the table into the catalog
    tableCatalogue.insertTable(groupedTable);

    // Write the header to the CSV file
    ofstream outputFile(groupedTable->sourceFileName, ios::out);
    outputFile << header[0] << "," << header[1] << endl;
    outputFile.close();

    // Append the data to the CSV file
    outputFile.open(groupedTable->sourceFileName, ios::app);
    for (int i = 0; i < pageCounter; i++)
    {
        ifstream inputFile("../data/temp/" + groupedTable->tableName + "_Page" + to_string(i));
        string line;
        while (getline(inputFile, line))
        {
            istringstream iss(line);
            string word;
            bool firstWord = true;
            while (iss >> word)
            {
                if (!firstWord)
                {
                    outputFile << ",";
                }
                outputFile << word;
                firstWord = false;
            }
            outputFile << endl;
        }
        inputFile.close();
    }
    outputFile.close();

    cout << "\n=== GROUP BY Operation Completed ===" << endl;
    logger.log("Table::GroupBy - End");
}

// join

void Table::joinTables()
{
    logger.log("Table::joinTables - Start");

    // Extract parsed query info
    string newRelationName = parsedQuery.joinResultRelationName;
    string tableName1 = parsedQuery.joinFirstRelationName;
    string tableName2 = parsedQuery.joinSecondRelationName;
    string column1 = parsedQuery.joinFirstColumnName;
    string column2 = parsedQuery.joinSecondColumnName;

    // cout << "Performing HASH JOIN" << endl;
    // cout << "Result Table: " << newRelationName << endl;
    // cout << "Input Table 1: " << tableName1 << ", Column: " << column1 << endl;
    // cout << "Input Table 2: " << tableName2 << ", Column: " << column2 << endl;

    // Fetch tables
    Table *table1 = tableCatalogue.getTable(tableName1);
    Table *table2 = tableCatalogue.getTable(tableName2);

    if (!table1 || !table2)
    {
        cout << "ERROR: One or both input tables not found." << endl;
        return;
    }

    // Get column indices
    int colIndex1 = table1->getColumnIndex(column1);
    int colIndex2 = table2->getColumnIndex(column2);

    if (colIndex1 == -1 || colIndex2 == -1)
    {
        cout << "ERROR: Join column not found in respective table(s)." << endl;
        return;
    }

    // Create result table schema
    vector<string> resultColumns = table1->columns;
    resultColumns.insert(resultColumns.end(), table2->columns.begin(), table2->columns.end());
    Table *resultTable = new Table(newRelationName, resultColumns);

    // Step 1: Build hash table on Table 1
    unordered_map<int, vector<vector<int>>> hashTable;

    Cursor cursor1 = table1->getCursor();
    vector<int> row1 = cursor1.getNext();
    while (!row1.empty())
    {
        int key = row1[colIndex1];
        hashTable[key].push_back(row1);
        row1 = cursor1.getNext();
    }
    // cout << "Hash table built on Table 1 with " << hashTable.size() << " unique keys." << endl;

    // Step 2: Probe hash table using Table 2
    Cursor cursor2 = table2->getCursor();
    vector<int> row2 = cursor2.getNext();

    long long int joinedRows = 0;
    vector<vector<int>> pageBuffer;
    int currRow = 0, pageCounter = 0;

    while (!row2.empty())
    {
        int key = row2[colIndex2];

        if (hashTable.find(key) != hashTable.end())
        {
            for (const vector<int> &matchRow : hashTable[key])
            {
                vector<int> joinedRow = matchRow;
                joinedRow.insert(joinedRow.end(), row2.begin(), row2.end());
                pageBuffer.push_back(joinedRow);
                currRow++;
                joinedRows++;

                if (currRow == resultTable->maxRowsPerBlock)
                {
                    bufferManager.writePage(resultTable->tableName, pageCounter, pageBuffer, currRow);
                    pageCounter++;
                    currRow = 0;
                    pageBuffer.clear();
                }
            }
        }

        row2 = cursor2.getNext();
    }

    // Flush remaining rows
    if (!pageBuffer.empty())
    {
        bufferManager.writePage(resultTable->tableName, pageCounter, pageBuffer, currRow);
        pageCounter++;
    }

    resultTable->rowCount = joinedRows;
    resultTable->columnCount = resultColumns.size();
    tableCatalogue.insertTable(resultTable);

    cout << "Hash join complete. Rows joined: " << joinedRows << endl;
    logger.log("Table::joinTables - End");
}

Table::Table(string tableName, vector<string> columns, bool ORDER_BY_OPERATION, int block_count)
{
    logger.log("Table::Table");
    this->indexed = false;
    this->indexedColumn = "";
    this->indexingStrategy = NOTHING;

    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->blockCount = block_count;
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    this->writeRow<string>(columns);
}

void Table::orderBy()
{
    logger.log("Table::orderBy - Start");
    cout << "ORDER BY STARTED..." << endl;

    string newTableName = parsedQuery.orderResultRelation;
    string oldTableName = parsedQuery.orderRelationName;
    string orderColumn = parsedQuery.orderAttribute;
    bool isDescending = (parsedQuery.sortingStrategy == SortingStrategy::DESC);

    // Retrieve the Table object using oldTableName
    Table *oldTable = tableCatalogue.getTable(oldTableName);
    if (!oldTable)
    {
        cout << "Error: Table not found - " << oldTableName << endl;
        return;
    }

    Table *sortedTable = nullptr;
    try
    {
        sortedTable = new Table(newTableName, oldTable->columns, true, oldTable->columnCount);
        sortedTable->blockCount = oldTable->blockCount;
    }
    catch (exception &e)
    {
        cout << "Exception: " << e.what() << endl;
        return;
    }
    catch (...)
    {
        cout << "Unknown exception occurred" << endl;
        return;
    }

    vector<vector<int>> allRows;
    Cursor cursor = oldTable->getCursor();
    vector<int> row = cursor.getNext();
    while (!row.empty())
    {
        allRows.push_back(row);
        row = cursor.getNext();
    }

    // Get column index to sort by
    int columnIndex = oldTable->getColumnIndex(orderColumn);

    // Sort rows using the specified column and order
    sort(allRows.begin(), allRows.end(), [&](const vector<int> &a, const vector<int> &b)
         { return isDescending ? a[columnIndex] > b[columnIndex] : a[columnIndex] < b[columnIndex]; });

    // Write sorted data into the new table
    for (const auto &sortedRow : allRows)
    {
        sortedTable->writeRow(sortedRow);
    }

    parsedQuery.loadRelationName = newTableName;
    tableCatalogue.insertTable(sortedTable);

    ofstream csvFile("../data/" + newTableName + ".csv");
    for (size_t i = 0; i < oldTable->columns.size(); i++)
    {
        csvFile << oldTable->columns[i];
        if (i < oldTable->columns.size() - 1)
            csvFile << ",";
    }
    csvFile << "\n";
    for (const auto &sortedRow : allRows)
    {
        sortedTable->writeRow(sortedRow);
        for (size_t i = 0; i < sortedRow.size(); i++)
        {
            csvFile << sortedRow[i];
            if (i < sortedRow.size() - 1)
                csvFile << ",";
        }
        csvFile << "\n";
    }
    csvFile.close();
    // sortedTable->makePermanent();
    cout << "ORDER BY COMPLETED" << endl;
    logger.log("Table::orderBy - End");
}



// Helper function to check if a string represents a valid integer
bool isInteger(const std::string& str) {
    // Check if the string is empty
    if (str.empty()) return false;

    // Handle negative numbers
    size_t start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.size() == 1) return false;  // A negative sign alone is not a number
    }

    // Check if all characters (except the optional negative sign) are digits
    for (size_t i = start; i < str.size(); ++i) {
        if (!isdigit(str[i])) {
            return false;  // Non-digit character found
        }
    }

    return true;
}

void Table::insertRow(const vector<string>& rowStrVec) {
    logger.log("Table::insertRow");

    // Step 1: Parse strings to integers and handle invalid/missing values
    vector<int> row;
    // cout << "Parsing input row:\n";
    for (auto val : rowStrVec) {
        if (val.empty()) {
            // cout << " - Empty value, inserting 0\n";
            row.push_back(0);
        } else {
            try {
                int intVal = stoi(val);
                // cout << " - Parsed value: " << intVal << "\n";
                row.push_back(intVal);
            } catch (...) {
                // cout << " - Invalid integer format, inserting 0 for value: " << val << "\n";
                row.push_back(0);
            }
        }
    }

    // cout << "Final parsed row: ";
    // for (auto ro : row) cout << ro << " ";
    // cout << endl;

    // Step 2: Update total row count
    this->rowCount++;
    // cout << "Updated row count: " << this->rowCount << endl;

    // Step 3: Block initialization if this is the first insert
    if (this->rowsPerBlockCount.empty()) {
        this->rowsPerBlockCount.push_back(0);
        this->blockCount = 1;
        // cout << "Initialized first block.\n";
    }

    // Step 4: Determine current block state
    int lastBlockIndex = this->blockCount - 1;
    int rowsInLastBlock = this->rowsPerBlockCount[lastBlockIndex];
    int maxRows = this->maxRowsPerBlock;

    // cout << "Current block index: " << lastBlockIndex << "\n";
    // cout << "Rows in last block: " << rowsInLastBlock << "\n";
    // cout << "Max rows per block: " << maxRows << "\n";

    vector<vector<int>> pageData;

    // Step 5: Load page data if block is not full
    if (rowsInLastBlock < maxRows) {
        Page page = bufferManager.getPage(this->tableName, lastBlockIndex);
        pageData = page.getAllRows();
        // cout << "Loaded existing page data. Page row count: " << pageData.size() << "\n";
    } else {
        // cout << "Last block is full. Preparing to create a new block.\n";
    }

    // Step 6: Print before resizing (if necessary)
    int validRows = this->rowsPerBlockCount[lastBlockIndex];
    // cout << "Valid (already filled) rows in block: " << validRows << "\n";

    if ((int)pageData.size() > validRows) {
        // cout << "Resizing page data from size " << pageData.size() << " to " << validRows << "\n";
        pageData.resize(validRows); // Drop any garbage or extra rows
    }

    // Step 7: Determine whether to insert into new block or current
    if (rowsInLastBlock + 1 > maxRows) {
        // Step 7A: New block needed
        // cout << "New block being created for insertion.\n";
        this->rowsPerBlockCount.push_back(1);
        this->blockCount++;
        vector<vector<int>> newPageData = {row};
        bufferManager.writePage(this->tableName, lastBlockIndex + 1, newPageData, 1);
        // cout << "New page written to block index: " << (lastBlockIndex + 1) << "\n";
    } else {
        // Step 7B: Insert into current block
        Page page = bufferManager.getPage(this->tableName, lastBlockIndex);
        pageData = page.getAllRows();
        if ((int)pageData.size() < validRows + 1) {
            // cout << "Resizing page data to accommodate new row: " << (validRows + 1) << "\n";
            pageData.resize(validRows + 1);
        }

        // cout << "Inserting row at index: " << validRows << " in block: " << lastBlockIndex << "\n";
        pageData[validRows] = row;

        // Debug print updated pageData
        // cout << "Updated Page Data (Block " << lastBlockIndex << "):\n";
        // for (const auto &r : pageData) {
        //     for (int v : r) cout << v << " ";
        //     cout << "\n";
        // }

        // This line wrongly increments blockCount even when not needed (remove it)
        // this->blockCount++;

        // Update internal metadata
        this->rowsPerBlockCount[lastBlockIndex]++;

        // Write back to buffer
        bufferManager.writePage(this->tableName, lastBlockIndex, pageData, validRows + 1);
        // cout << "Page written back to buffer manager.\n";
    }

    // Step 8: Print current B+ Tree Indices
    cout << "Current B+ Tree Indices in Table:\n";
    for (auto& [colName, indexInfo] : this->indices) {
        if (indexInfo->strategy == BTREE && indexInfo->bPlusTreeIndex != nullptr) {
            cout << " - Column: " << colName << ", Index exists\n";
        }
    }

    // Step 9: (Optional) Update B+ Tree Index
    for (auto& [colName, indexInfo] : this->indices) {
        if (indexInfo->strategy == BTREE && indexInfo->bPlusTreeIndex != nullptr) {
            int colIdx = this->getColumnIndex(colName);
            if (colIdx >= 0 && colIdx < (int)row.size()) {
                int key = row[colIdx];
                indexInfo->bPlusTreeIndex->insert(key, this->rowCount - 1);
                cout << "Inserted into B+ Tree Index for " << colName << ": key=" << key << ", row=" << this->rowCount - 1 << "\n";
            }
        }
    }

    // Step 10: Final operations
    cout << "Calling makePermanent...\n";
    // this->makePermanent();

    cout << "Updating statistics...\n";
    this->updateStatistics(row);

    cout << "INSERTION SUCCESS ✅\n";
}

void Table::updateRow(const vector<string>& rowStrVec) {
    logger.log("Table::insertRow");

    // Step 1: Parse strings to integers and handle invalid/missing values
    vector<int> row;
    cout << "Parsing update row:\n";
    for (auto val : rowStrVec) {
        if (val.empty()) {
            cout << " - Empty value, inserting 0\n";
            row.push_back(0);
        } else {
            try {
                int intVal = stoi(val);
                cout << " - Parsed value: " << intVal << "\n";
                row.push_back(intVal);
            } catch (...) {
                cout << " - Invalid integer format, inserting 0 for value: " << val << "\n";
                row.push_back(0);
            }
        }
    }

    cout << "Final parsed update row: ";
    for (auto ro : row) cout << ro << " ";
    cout << endl;


    // Step 8: Print current B+ Tree Indices
    cout << "Current B+ Tree Indices in Table:\n";
    for (auto& [colName, indexInfo] : this->indices) {
        if (indexInfo->strategy == BTREE && indexInfo->bPlusTreeIndex != nullptr) {
            cout << " - Column: " << colName << ", Index exists\n";
        }
    }
    for (auto& [colName, indexInfo] : this->indices) {
        if (indexInfo->strategy == BTREE && indexInfo->bPlusTreeIndex != nullptr) {
            int colIdx = this->getColumnIndex(colName);
            if (colIdx >= 0 && colIdx < (int)row.size()) {
                int searchKey = stoi(parsedQuery.updateWhereValue);
                cout << "Searching in B+ Tree for " << colName << " = " << searchKey << "\n";
    
                vector<int> matchingRowIndices = indexInfo->bPlusTreeIndex->search(searchKey, parsedQuery.updateOperator);
                cout << "Found " << matchingRowIndices.size() << " matching rows:\n";
    
                for (int rowId : matchingRowIndices) {
                    int blockIndex = rowId / this->maxRowsPerBlock;
                    int offsetInBlock = rowId % this->maxRowsPerBlock;
    
                    // Step 1: Fetch the page using getPage
                    Page page = bufferManager.getPage(this->tableName, blockIndex);
                    vector<vector<int>> pageData = page.getAllRows();
    
                    // Step 2: Print current row
                    cout << "Before Update (rowId " << rowId << "): ";
                    for (int val : pageData[offsetInBlock]) cout << val << " ";
                    cout << endl;
    
                    // Step 3: Perform the update
                    pageData[offsetInBlock] = row;
    
                    // Step 4: Write updated page back
                    bufferManager.writePage(this->tableName, blockIndex, pageData, pageData.size());
    
                    cout << "Updated rowId " << rowId << " with new values.\n";
                }
            }
            break; // Only using one B+ Tree index to filter
        }
    }
    

    // Step 10: Final operations
    // cout << "Calling makePermanent...\n";
    // this->makePermanent();

    cout << "Updating statistics...\n";
    this->updateStatistics(row);

    cout << "UPDATE OPERATION done.";
}

