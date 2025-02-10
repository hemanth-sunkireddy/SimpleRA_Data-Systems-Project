## Explanation of each command Implementation
### SOURCE
Code for source command working is present in the `server.cpp` itself.  the logic for implementation is as follows : 
- it will first detect the first word in the input query.
- if it is a `SOURCE` command, then it will check for the second input parameter.
- if it has .ra extension, then it opens the file.
- it now gets each line and starts executing them. [this happens in a loop].
- after execution of all the commands, it will close the file and again asks for command as how simpleRA works.

### LOAD

### PRINT

### EXPORT

### ROTATE
- `SYNTACTIC CHECK` : first we will check whether there are any errors in the provided syntax. you can find the code in the `rotate.cpp` file.
- `SEMANTIC CHECK` : here we will check whether our matrix is present or not in the `matrixCatalogue`. there is a function called `isMatrix()` which returns true if the matrix is present in the catalogue and false if it is not present. you can find the code in the `rotate.cpp` file.
- `ACTUAL EXECUTION` : 
    ```cpp
    void executeROTATE_MATRIX() {
        Matrix *matrix = matrixCatalogue.getmatrix(parsedQuery.rotateRelationName);
        matrix->rotate();  // In-place rotation
        matrixCatalogue.updateMatrix(matrix);  // Ensure the catalogue reflects the updated matrix, if needed
        return;
    }
    ```
    - it will first get the matrix with the help of `getmatrix` functiom in `matrixCatalogue`.
    - now it uses the `rotate()` function present in matrix class.
    - finally it updates the matrix data in the matrix catalogue.

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

    - above code helps in rotating the matrix.
    - it uses `get_element`() and `set_element`() functions which gives and sets values from and to the page files of the matrix.
    - they actually create an abstraction that matrix is in single page.
    - but each time you call the function, it traverse through the entire page and gets the data.
    ```cpp
    void Matrix::set_element(int row, int col, int value) {
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
    ```
    - the set function also each and every time, it traverse over the pages and sets the data.
    - for reflecting the updated values in the code, the set function actually deletes the previously stored page data in `buffermanager` and adds the newly edited page each time.

### CROSSTRANSPOSE

### CHECK ANTI-SYMMETRY


## Assumptions


## Contribution of Each Member
