#include "global.h"

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table()
{
    logger.log("Table::Table");
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
    if (strncmp(tableName.c_str(), "temp/", 5) == 0) {
        this->sourceFileName = "../data/" + tableName + ".csv";
        this->tableName = tableName.substr(5);
    } else {
        this->sourceFileName = "../data/" + tableName + ".csv";
        this->tableName = tableName;
    }
}
struct MyPair {
    vector<int> row;
    int blockNum;
    int boundary;
    int cursorIndex;
    
    MyPair(vector<int> r, int b, int bound, int c) 
        : row(r), blockNum(b), boundary(bound), cursorIndex(c) {}
};

struct CompareByFirstElement {
    bool operator()(const MyPair &lhs, const MyPair &rhs) const {
        for (int i = 0; i < columnIndexes.size(); i++) {
            if (lhs.row[columnIndexes[i]] != rhs.row[columnIndexes[i]]) {
                if (sortValues[i] == 0)
                    return lhs.row[columnIndexes[i]] > rhs.row[columnIndexes[i]];
                else if (sortValues[i] == 1)
                    return lhs.row[columnIndexes[i]] < rhs.row[columnIndexes[i]];
            }
        }
        return false;
    }
};

static bool sortComparator(const vector<int> &a, const vector<int> &b) {
    logger.log("Inside Sort Comp");
    for (int i = 0; i < columnIndexes.size(); i++) {
        logger.log("SORT COL INDEX: " + to_string(columnIndexes[i]));
        if (a[columnIndexes[i]] != b[columnIndexes[i]]) {
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
    this -> sourceFileName = "../data/" + matrixname + ".csv";
    this -> matrixname = matrixname;
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
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
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

void Table::deleteTable() {
    logger.log("Table::deleteTable - Start");

    // Delete the associated file
    string filePath = this->sourceFileName;
    if (remove(filePath.c_str()) == 0) {
        logger.log("Deleted file: " + filePath);
    } else {
        logger.log("Failed to delete file: " + filePath);
    }

    // Delete the table object itself
    delete this;

    logger.log("Table::deleteTable - End");
}

bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this -> sourceFileName, ios::in);
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
    while(getline(s, elem, ','))
    {
        elem.erase(std::remove_if(elem.begin(), elem.end(), ::isspace), elem.end());
        count += 1;
    }
    this -> columnCount = count;
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
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}
void Table::sortTable(bool makePermanent) {
    logger.log("Table::sortTable");
    
    // Initialize sorting parameters
    sortValues.resize(parsedQuery.sortStrategy.size(), 0);
    columnIndexes.resize(parsedQuery.sortStrategy.size());
    
    // Store sorting order
    for (int i = 0; i < parsedQuery.sortStrategy.size(); i++) {
        if (parsedQuery.sortStrategy[i] == DESC)
            sortValues[i] = 1;
    }
    
    // Store column indices to sort by
    for (int i = 0; i < parsedQuery.sortColumns.size(); i++) {
        columnIndexes[i] = this->getColumnIndex(parsedQuery.sortColumns[i]);
    }
    
    // Sort each page individually first
    for (int pageIndex = 0; pageIndex < this->blockCount; pageIndex++) {
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
    if (makePermanent) {
        this->makePermanent();
    }
}

void Table::externalSort() {
    logger.log("Table::externalSort");
    
    int K = BLOCK_COUNT - 1;  // K-way merge
    int mergeIterations = ceil(log(this->blockCount) / log(K));
    
    for (int iteration = 0; iteration < mergeIterations; iteration++) {
        int size = pow(K, iteration);
        int clusters = ceil((double)this->blockCount / size);
        
        for (int cluster = 0; cluster < clusters; cluster++) {
            int start = cluster * size;
            int end = min(start + size - 1, (int)this->blockCount - 1);
            
            // Create output buffer for merged data
            vector<vector<int>> outputBuffer;
            outputBuffer.reserve(this->maxRowsPerBlock);
            
            // Create priority queue for K-way merge
            priority_queue<MyPair, vector<MyPair>, CompareByFirstElement> pq;
            vector<Cursor> cursors;
            
            // Initialize cursors for each block in the cluster
            for (int i = start; i <= end; i++) {
                Cursor cursor(this->tableName, i);
                cursors.push_back(cursor);
                
                // Get first row from each block
                vector<int> row = cursor.getNext();
                if (!row.empty()) {
                    pq.push({row, i, end, cursors.size() - 1});
                }
            }
            
            // Merge blocks
            int outputPageIndex = 0;
            while (!pq.empty()) {
                MyPair top = pq.top();
                pq.pop();
                
                outputBuffer.push_back(top.row);
                
                // If output buffer is full, write it to disk
                if (outputBuffer.size() == this->maxRowsPerBlock) {
                    bufferManager.writePage(this->tableName, outputPageIndex, outputBuffer, outputBuffer.size());
                    outputBuffer.clear();
                    outputPageIndex++;
                }
                
                // Get next row from the block that provided the top element
                vector<int> nextRow = cursors[top.cursorIndex].getNext();
                if (!nextRow.empty()) {
                    pq.push({nextRow, top.blockNum, top.boundary, top.cursorIndex});
                }
            }
            
            // Write remaining rows in output buffer
            if (!outputBuffer.empty()) {
                bufferManager.writePage(this->tableName, outputPageIndex, outputBuffer, outputBuffer.size());
            }
        }
    }
}



void Matrix::updateStatistics(vector<int> row)
{
    this -> rowCount++;
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
    for (auto col : this->columns)
    {
        if (col == columnName)
        {
            return true;
        }
    }
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

    //print headings
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
    uint count = min((long long)PRINT_COUNT, this -> rowCount);

    Cursor cursor(this->matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this -> writeRow(row, cout);
    }
    printRowCount(this -> rowCount);
}

void Matrix::rotate() {
    // cout << "GET ELEM : " << this->get_element(2,2) << endl;
    // this->set_element(2,2,69);
    // cout << "GET ELEM after set : " << this->get_element(2,2) << endl;
    if (this->rowCount != this->columnCount) {
        // In-place rotation is not supported for non-square matrices
        std::cerr << "In-place rotation is only supported for square matrices." << std::endl;
        return;
    }

    int N = this->rowCount;  // Since it's a square matrix, rowCount == columnCount

    // Rotate the matrix layer by layer
    for (int layer = 0; layer < N / 2; ++layer) {
        int first = layer;
        int last = N - 1 - layer;
        
        for (int i = first; i < last; ++i) {
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
    uint count = min((long long) PRINT_COUNT, this -> rowCount);

    Cursor cursor(this -> matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count && rowCounter < i; rowCounter++)
    {
        row = cursor.getNext();
    }
    row = cursor.getNext();

    // now we need to get the jth element from the row.
    return row[j];
}

void Matrix::set_element(int row, int col, int value) {
    // Ensure row and col are within bounds
    if (row >= this->rowCount || col >= this->columnCount || row < 0 || col < 0) {
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
            cursor->nextPage(cursor->pageIndex+1);
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
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
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
    if(!this->isPermanent())
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
    if (this -> sourceFileName == "../data/" + this -> matrixname + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload(){
    logger.log("Table::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

void Matrix::unload(){
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
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName)
            return columnCounter;
    }
}

void Table::groupBy() {
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
    this->sortTable(false);  // Don't make sorting permanent
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
    while (!row.empty()) {
        int groupValue = row[groupIndex];
        int aggregateValue = row[aggregateIndex];
        int havingaggregateValue = row[havingaggregateIndex];

        if (groupValue != currentGroupValue) {
            // Process the previous group
            if (!isFirstGroup) {
                if (havingaggregateFunction == AVG && havingaggregateCount > 0) {
                    havingaggregateResult = havingaggregateResult / havingaggregateCount;
                }
                
                cout << "\nProcessing group with value: " << currentGroupValue << endl;
                cout << "Having aggregate result: " << havingaggregateResult << endl;
                cout << "Having aggregate count: " << havingaggregateCount << endl;
                
                bool havingConditionMet = false;
                if (havingaggregateCount > 0) {
                    if (binOp == EQUAL) {
                        havingConditionMet = (havingaggregateResult == attributeValue);
                    } else if (binOp == GREATER_THAN) {
                        havingConditionMet = (havingaggregateResult > attributeValue);
                    } else if (binOp == GEQ) {
                        havingConditionMet = (havingaggregateResult >= attributeValue);
                    }
                }

                if (havingConditionMet) {
                    cout << "Having condition met for group " << currentGroupValue << endl;
                    vector<int> resultRow;
                    
                    if (aggregateFunction == MIN || aggregateFunction == MAX || aggregateFunction == SUM) {
                        resultRow = {currentGroupValue, aggregationResult};
                    } else if (aggregateFunction == COUNT) {
                        resultRow = {currentGroupValue, aggregateCount};
                    } else if (aggregateFunction == AVG && aggregateCount > 0) {
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
            aggregationResult = aggregateValue;  // Initialize with first value
            aggregateCount = 1;
            havingaggregateCount = 1;
            havingaggregateResult = havingaggregateValue;
            isFirstGroup = false;
        } else {
            // Update aggregation results for current group
            if (aggregateFunction == MIN) {
                aggregationResult = min(aggregationResult, aggregateValue);
            } else if (aggregateFunction == MAX) {
                aggregationResult = max(aggregationResult, aggregateValue);
            } else if (aggregateFunction == SUM) {
                aggregationResult += aggregateValue;
            } else if (aggregateFunction == COUNT) {
                aggregateCount++;
            } else if (aggregateFunction == AVG) {
                aggregationResult += aggregateValue;
                aggregateCount++;
            }

            // Update having clause results
            if (havingaggregateFunction == MIN) {
                havingaggregateResult = min(havingaggregateResult, havingaggregateValue);
            } else if (havingaggregateFunction == MAX) {
                havingaggregateResult = max(havingaggregateResult, havingaggregateValue);
            } else if (havingaggregateFunction == SUM) {
                havingaggregateResult += havingaggregateValue;
            } else if (havingaggregateFunction == COUNT) {
                havingaggregateCount++;
            } else if (havingaggregateFunction == AVG) {
                havingaggregateResult += havingaggregateValue;
                havingaggregateCount++;
            }
        }

        // Write page if full
        if (currRow == groupedTable->maxRowsPerBlock) {
            cout << "Writing page " << pageCounter << " with " << currRow << " rows" << endl;
            bufferManager.writePage(groupedTable->tableName, pageCounter, pageData, currRow);
            pageCounter++;
            currRow = 0;
            pageData.clear();
        }

        row = cursor.getNext();
    }

    // Process the last group
    if (!isFirstGroup) {
        if (havingaggregateFunction == AVG && havingaggregateCount > 0) {
            havingaggregateResult = havingaggregateResult / havingaggregateCount;
        }
        
        cout << "\nProcessing final group with value: " << currentGroupValue << endl;
        cout << "Having aggregate result: " << havingaggregateResult << endl;
        cout << "Having aggregate count: " << havingaggregateCount << endl;
        
        bool havingConditionMet = false;
        if (havingaggregateCount > 0) {
            if (binOp == EQUAL) {
                havingConditionMet = (havingaggregateResult == attributeValue);
            } else if (binOp == GREATER_THAN) {
                havingConditionMet = (havingaggregateResult > attributeValue);
            } else if (binOp == GEQ) {
                havingConditionMet = (havingaggregateResult >= attributeValue);
            }
        }

        if (havingConditionMet) {
            cout << "Having condition met for final group " << currentGroupValue << endl;
            vector<int> resultRow;
            
            if (aggregateFunction == MIN || aggregateFunction == MAX || aggregateFunction == SUM) {
                resultRow = {currentGroupValue, aggregationResult};
            } else if (aggregateFunction == COUNT) {
                resultRow = {currentGroupValue, aggregateCount};
            } else if (aggregateFunction == AVG && aggregateCount > 0) {
                resultRow = {currentGroupValue, aggregationResult / aggregateCount};
            }
            
            cout << "Adding final result row: [" << resultRow[0] << ", " << resultRow[1] << "]" << endl;
            pageData.push_back(resultRow);
            currRow++;
            totalRow++;
        }
    }

    // Write remaining data
    if (!pageData.empty()) {
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
    for (int i = 0; i < pageCounter; i++) {
        ifstream inputFile("../data/temp/" + groupedTable->tableName + "_Page" + to_string(i));
        string line;
        while (getline(inputFile, line)) {
            istringstream iss(line);
            string word;
            bool firstWord = true;
            while (iss >> word) {
                if (!firstWord) {
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