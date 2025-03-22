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
void Table::sortTable() {
    cout << "Inside Table class sort function" << endl;

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

    long long pageDataRows = this->maxRowsPerBlock;
    long long cnt = 0;
    vector<vector<int>> pagesData;
    Cursor cursor = this->getCursor();
    int pageIndex = 0;

    // Read each row and sort them page-wise
    while (true) {
        vector<int> dataRow = cursor.getNext();
        if (dataRow.empty()) break;

        pagesData.push_back(dataRow);
        cnt++;

        if (cnt == pageDataRows) {
            cout << "Sorting page " << pageIndex << endl;
            sort(pagesData.begin(), pagesData.end(), sortComparator);
            bufferManager.writePage(this->tableName, pageIndex, pagesData, cnt);
            cnt = 0;
            pageIndex++;
            pagesData.clear();
        }
    }

    // Write the remaining data if not empty
    if (!pagesData.empty()) {
        sort(pagesData.begin(), pagesData.end(), sortComparator);
        bufferManager.writePage(this->tableName, pageIndex, pagesData, cnt);
    }

    // cout << "Calling External Sort" << endl;
    this->externalSort();
    this->makePermanent();
}

void Table::externalSort() {
    // cout << "Performing External Sort using K-way merge" << endl;
    
    int K = BLOCK_COUNT - 1;
    int mergeIterations = ceil(log(this->blockCount) / log(K)) - 1;
    // cout << "Total Iterations: " << mergeIterations << endl;

    vector<Cursor> pageCursor;

    for (int i = 0; i <= mergeIterations; i++) {
        // cout << "Iteration: " << i + 1 << endl;
        int size = pow(K, i);
        int clusters = ceil((double)this->blockCount / size);
        // cout << "Total Clusters: " << clusters << endl;

        int s = 0, e = size - 1;
        vector<pair<int, int>> clusterData(clusters);
        for (int j = 0; j < clusters; j++) {
            clusterData[j] = {s, e};
            s += size;
            e = min(e + size, (int)this->blockCount - 1);
        }

        int globalPageIndex = 0;
        for (int j = 0; j < ceil((double)clusters / K); j++) {
            int start = j * size * K;
            vector<pair<int, int>> boundaries;
            while (start < min((int)this->blockCount, K * size * (j + 1))) {
                boundaries.push_back({start, min(start + size - 1, (int)this->blockCount - 1)});
                start += size;
            }

            vector<vector<int>> mergeData;
            for (int m = 0; m < boundaries.size(); m += K) {
                priority_queue<MyPair, vector<MyPair>, CompareByFirstElement> pq;
                vector<pair<Cursor, int>> cursorVec;

                for (int r = m; r <= min(m + K - 1, (int)boundaries.size() - 1); r++) {
                    Cursor cursor(this->tableName, boundaries[r].first);
                    cursorVec.push_back({cursor, boundaries[r].first});
                }

                for (int c = 0; c < cursorVec.size(); c++) {
                    vector<int> data = cursorVec[c].first.getNextPageRow();
                    if (!data.empty()) {
                        pq.push({data, cursorVec[c].first.pageIndex, cursorVec[c].second, c});
                    }
                }

                while (!pq.empty()) {
                    MyPair element = pq.top();
                    pq.pop();
                    mergeData.push_back(element.row);

                    int cursorIndex = element.cursorIndex;
                    vector<int> newRow = cursorVec[cursorIndex].first.getNextPageRow();
                    if (!newRow.empty()) {
                        pq.push({newRow, cursorVec[cursorIndex].first.pageIndex, element.boundary, cursorIndex});
                    }

                    if (mergeData.size() == (int)this->maxRowsPerBlock) {
                        cout << "Writing to temp file: " << globalPageIndex << endl;
                        ofstream fout("../data/temp/" + this->tableName + "Dummy" + to_string(globalPageIndex));
                        for (const auto& row : mergeData) {
                            for (size_t z = 0; z < row.size(); z++) {
                                if (z != 0) fout << " ";
                                fout << row[z];
                            }
                            fout << endl;
                        }
                        fout.close();
                        mergeData.clear();
                        globalPageIndex++;
                    }
                }
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

    // First sort the table by the grouping attribute
    logger.log("External sorting for GROUP BY");
    parsedQuery.sortStrategy.clear();
    parsedQuery.sortStrategy.push_back(ASC);
    parsedQuery.sortColumns.clear();
    parsedQuery.sortColumns.push_back(groupingAttribute);
    string tempTableName = "temp_GroupBy";
    Table *sortedTable = new Table(tempTableName, this->columns);  

    // Copy all rows to the temporary table
    Cursor cursor = this->getCursor();
    vector<int> row = cursor.getNext();
    logger.log("Copying rows to the temp_groupBy table");
    while (!row.empty()) {
        sortedTable->writeRow(row);
        row = cursor.getNext();
    }
    logger.log("after copying");

    // Load and sort the temporary table
    parsedQuery.loadRelationName = "temp/" + tempTableName;
    logger.log("Loading temp table: " + parsedQuery.loadRelationName);
    cout << "Loading temp table: " << parsedQuery.loadRelationName << endl;
    executeLOAD();
    sortedTable = tableCatalogue.getTable(tempTableName);  // Remove temp/ prefix when getting table
    if (!sortedTable) {
        cout << "Error: Failed to get sorted table from catalogue" << endl;
        return;
    }
    cout << "Successfully loaded temp table. Row count: " << sortedTable->rowCount << endl;

    Cursor cursor2 = sortedTable->getCursor();
    vector<int> row2 = cursor2.getNext();
    while (!row2.empty()) {
        cout << "ROW : ";
        for (int i = 0; i < row2.size(); i++) {
            cout << row2[i] << " ";
        }
        cout << endl;
        row2 = cursor2.getNext();
    }
    sortedTable->sortTable();
    logger.log("before sorting");

    // Create the result table with appropriate columns
    vector<string> header = {groupingAttribute, stringaggregateFunction + "(" + aggregateAttribute + ")"};
    Table *groupedTable = new Table(newTableName, header);
    logger.log("Create a new table for the result");
    
    // Initialize variables for grouping
    int currentGroupValue = -1;
    int havingaggregateResult = 0;
    int havingaggregateCount = 0;
    int aggregationResult = 0;
    int aggregateCount = 0;

    // Get column indices for grouping and aggregate attributes
    int groupIndex = sortedTable->getColumnIndex(groupingAttribute);
    int havingaggregateIndex = sortedTable->getColumnIndex(havingaggregateAttribute);
    int aggregateIndex = sortedTable->getColumnIndex(aggregateAttribute);
    logger.log("Get column indices for groupingAttribute and aggregateAttribute");
    
    // Process the sorted data
    cursor = sortedTable->getCursor();
    row = cursor.getNext();
    logger.log("Performing GROUP BY and Aggregation");
    int currRow = 0;
    long long totalRow = 0;
    int pageCounter = 0;
    vector<vector<int>> pageData;

    while (true) {
        if (row.empty()) {
            // Process the last group
            if (havingaggregateFunction == AVG && havingaggregateCount > 0)
                havingaggregateResult /= havingaggregateCount;
            if (currentGroupValue != -1 && havingaggregateCount > 0 && 
                ((binOp == EQUAL && attributeValue == havingaggregateResult) ||
                (binOp == GREATER_THAN && attributeValue < havingaggregateResult) ||
                (binOp == GEQ && attributeValue <= havingaggregateResult))) {
                
                vector<int> resultRow;
                if (aggregateFunction == MIN || aggregateFunction == MAX || aggregateFunction == SUM) {
                    resultRow = {currentGroupValue, aggregationResult};
                } else if (aggregateFunction == COUNT) {
                    resultRow = {currentGroupValue, aggregateCount};
                } else if (aggregateFunction == AVG) {
                    resultRow = {currentGroupValue, aggregationResult / aggregateCount};
                }
                
                pageData.push_back(resultRow);
                currRow++;
                totalRow++;
            }
            break;
        }

        int groupValue = row[groupIndex];
        int aggregateValue = row[aggregateIndex];
        int havingaggregateValue = row[havingaggregateIndex];

        if (groupValue != currentGroupValue) {
            // Process the previous group
            if (havingaggregateFunction == AVG && havingaggregateCount > 0)
                havingaggregateResult /= havingaggregateCount;
            if (currentGroupValue != -1 && havingaggregateCount > 0 && 
                ((binOp == EQUAL && attributeValue == havingaggregateResult) ||
                (binOp == GREATER_THAN && attributeValue < havingaggregateResult) ||
                (binOp == GEQ && attributeValue <= havingaggregateResult))) {
                
                vector<int> resultRow;
                if (aggregateFunction == MIN || aggregateFunction == MAX || aggregateFunction == SUM) {
                    resultRow = {currentGroupValue, aggregationResult};
                } else if (aggregateFunction == COUNT) {
                    resultRow = {currentGroupValue, aggregateCount};
                } else if (aggregateFunction == AVG) {
                    resultRow = {currentGroupValue, aggregationResult / aggregateCount};
                }
                
                pageData.push_back(resultRow);
                currRow++;
                totalRow++;
            }
            
            // Start new group
            currentGroupValue = groupValue;
            aggregationResult = 0;
            aggregateCount = 0;
            havingaggregateCount = 0;
            havingaggregateResult = 0;
        }

        // Update aggregation results
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
            havingaggregateResult++;
        } else if (havingaggregateFunction == AVG) {
            havingaggregateResult += havingaggregateValue;
            havingaggregateCount++;
        }

        // Write page if full
        if (currRow == groupedTable->maxRowsPerBlock) {
            bufferManager.writePage(groupedTable->tableName, pageCounter, pageData, currRow);
            pageCounter++;
            currRow = 0;
            pageData.clear();
        }

        row = cursor.getNext();
    }

    // Write remaining data
    if (!pageData.empty()) {
        bufferManager.writePage(groupedTable->tableName, pageCounter, pageData, currRow);
        pageCounter++;
    }
    logger.log("pages created");
    
    // Initialize the result table properly
    groupedTable->rowCount = totalRow;
    groupedTable->columnCount = 2;
    groupedTable->blockCount = pageCounter;
    groupedTable->sourceFileName = "../data/" + groupedTable->tableName + ".csv";
    
    // Insert the table into the catalog
    tableCatalogue.insertTable(groupedTable);
    
    // Write the header to the CSV file
    ofstream outputFile(groupedTable->sourceFileName, ios::out);
    outputFile << header[0] << ", " << header[1] << endl;
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
                    outputFile << ", ";  
                }
                outputFile << word;
                firstWord = false;
            }
            outputFile << endl;
        }
        inputFile.close();
    }
    outputFile.close();
    
    logger.log("After make permanent");
    sortedTable->unload();
    sortedTable->deleteTable();
    logger.log("Table::GroupBy - End");
}