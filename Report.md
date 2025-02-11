## Explanation of Each Command Implementation

### SOURCE
The code for the `SOURCE` command is located in `server.cpp`. The logic is as follows:

- Detects the first word in the input query.
- If the command is `SOURCE`, it checks for the second input parameter.
- If the parameter has a `.ra` extension, it opens the file.
- Reads each line and executes commands in a loop.
- After executing all commands, it closes the file and prompts for the next command, similar to how `simpleRA` operates.

---

### LOAD
**Logic:**

The `LOAD MATRIX` command reads a matrix from an input file and stores it in a structured format for efficient retrieval and processing.

1. **Parsing the Input File:**
   - Reads the matrix file line by line.
   - Extracts individual elements and stores them in a 2D vector or list of lists.
   - Writes the matrix into pages.

2. **Handling Large Matrices with Paging:**
   - Divides large matrices into smaller blocks (pages) to fit in memory.
   - Stores each block separately for efficient access.

3. **Creating Matrix Metadata:**
   - Stores metadata such as matrix dimensions, block size, and storage format in the matrix object.
   - Adds this matrix data to the `MatrixCatalogue`.

4. **Storing Data in Blocks:**
   - Splits the matrix into pages, each containing a subset of rows and columns.
   - Writes pages to disk, managed by the `BufferManager`.

5. **Updating Matrix Catalogue:**
   - Registers the matrix in the `MatrixCatalogue` for easy access during queries.

```cpp
class Matrix {
    string sourceFileName = "";
    string matrixname = "";
    long long int columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0;

    bool extractColumnCount(string firstline);
    bool blockify();
    void updateStatistics(vector<int> row);
    Matrix(string matrixname);
    bool load();
    void print();
    void rotate();
    int get_element(int row_i, int col_j);
    void set_element(int row_i, int col_j, int elem);
    void makePermanent();
    bool isPermanent();
    void crossTranspose(Matrix *matrix2);
    void writeRow(vector<T> row, ostream &fout);
};
```

**Page Design:**

- Each **page** contains a subset of matrix rows for memory efficiency.
- **Naming Convention:** `matrixName_PageX_Y`, where `X` is the row index and `Y` is the column index.
- Pages are stored in `../data/temp/` and loaded dynamically to reduce memory overhead.

**Block Access:**

1. **Check if Page Exists in Memory:**
   - If loaded, returns the page; otherwise, loads from disk.
2. **Fetching Data from Disk:**
   - Uses `BufferManager::getPage()`.
3. **Evicting Old Pages:**
   - Removes least recently used pages when memory is full.
4. **Seamless Data Access:**
   - Loads matrix pages as needed for smooth computation.

**Error Handling:**

- **File Not Found:** Logs an error if the matrix file doesn't exist.
- **Malformed Data:** Aborts the load operation for incorrect matrix formats.
- **Insufficient Memory:** Evicts old pages using LRU if too many pages are loaded.
- **Duplicate Matrix:** Prevents duplicate loading or allows overwriting based on settings.

---

### PRINT
The `PRINT MATRIX` command retrieves and displays the matrix stored in paginated form.

1. **Check Matrix Existence:** Verifies if the matrix exists in the `MatrixCatalogue`.
2. **Initialize Cursor:** Creates a cursor to traverse matrix pages.
3. **Iterate Through Pages:** Retrieves pages from the buffer manager.
4. **Print Row-wise:** Prints matrix rows sequentially with correct formatting.
5. **Handle Errors:** Manages missing pages, incorrect references, and empty matrices.

```cpp
void Matrix::print() {
    uint count = min((long long)PRINT_COUNT, this->rowCount);
    Cursor cursor(this->matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++) {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}
```

**Block Access:**

- **Retrieve Pages:** Checks if pages are in memory; if not, loads from disk.
- **Sequential Iteration:** Processes pages in order.
- **Row Access:** Reads rows within pages before moving to the next.
- **Formatted Output:** Prints elements in correct order.

**Error Handling:**

- **Matrix Not Found:** Displays an error message.
- **Page Retrieval Failure:** Skips or reports missing/corrupted pages.
- **Empty Matrix:** Displays a message if no valid data exists.
- **Memory Constraints:** Removes older pages if too many are loaded.

---

### EXPORT
**Logic:**

The `EXPORT MATRIX` command converts an internal matrix to CSV format.

```cpp
void Matrix::makePermanent() {
    logger.log("Table::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixname + ".csv";
    ofstream fout(newSourceFile, ios::out);

    Cursor cursor(this->matrixname, 0, 1);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++) {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}
```

**Steps of Execution:**

1. **Command Invocation:**
   - User enters `EXPORT MATRIX <matrix_name> TO <file_path>`.
   - System verifies if the matrix exists in the `MatrixCatalogue`.

2. **Fetching Matrix Data:**
   - A `Cursor` iterates over matrix pages.

3. **File Creation and Writing Data:**
   - Creates a CSV file and writes matrix values row by row.

4. **Completion and Logging:**
   - Closes the file and logs successful export.

**Block Access:**

- **Sequential Block Access:** Cursor moves through pages efficiently.
- **Optimized Buffer Usage:** Fetches pages into memory only when needed.

**Error Handling:**

- **Matrix Not Found:** Displays an error message.
- **File Creation Failure:** Logs errors for permission or path issues.
- **Incomplete Write:** Ensures the file is closed properly if errors occur.

---

### ROTATE

1. **Syntactic Check:** Validates syntax in `rotate.cpp`.
2. **Semantic Check:** Checks matrix existence in `MatrixCatalogue` using `isMatrix()`.
3. **Execution:**

```cpp
void executeROTATE_MATRIX() {
    Matrix *matrix = matrixCatalogue.getmatrix(parsedQuery.rotateRelationName);
    matrix->rotate();  // In-place rotation
    matrixCatalogue.updateMatrix(matrix);
    return;
}
```

**Rotation Logic:**

```cpp
void Matrix::rotate() {
    int N = this->rowCount;
    for (int layer = 0; layer < N / 2; ++layer) {
        int first = layer;
        int last = N - 1 - layer;
        for (int i = first; i < last; ++i) {
            int offset = i - first;
            int top = this->get_element(first, i);
            this->set_element(first, i, this->get_element(last - offset, first));
            this->set_element(last - offset, first, this->get_element(last, last - offset));
            this->set_element(last, last - offset, this->get_element(i, last));
            this->set_element(i, last, top);
        }
    }
}
```

- Uses `get_element()` and `set_element()` to manipulate matrix values.
- Abstracts matrix as a single page while handling multiple pages.

**Set Element Logic:**

```cpp
void Matrix::set_element(int row, int col, int value) {
    int pageIndex = row / this->maxRowsPerBlock;
    int rowInPage = row % this->maxRowsPerBlock;
    string pageName = "../data/temp/" + this->matrixname + "_Page" + to_string(pageIndex);

    Page page = bufferManager.getPage(this->matrixname, pageIndex, 1);
    vector<int> matrixRow = page.getRow(rowInPage);
    matrixRow[col] = value;
    page.updateRow(rowInPage, matrixRow);

    bufferManager.writePage(this->matrixname, pageIndex, page.getAllRows(), page.getrowcount());
    matrixCatalogue.updateMatrix(this);
    bufferManager.deletePage(pageName);
}
```

- Traverses pages to set data.
- Deletes previous page data in `BufferManager` to reflect updated values.

---

### CROSSTRANSPOSE
*To be added.*

### CHECK ANTI-SYMMETRY
*To be added.*

---

## Assumptions
*To be added.*

## Contribution of Each Member
*To be added.*
Susheel <Koncham Jaali choopinchandi sir>
Hemanth <Dongodu>
Ayush <Good Boy>

