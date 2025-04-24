# Project Phase 3 Report

## 1. SEARCH Command Implementation

### 1.1 Overview

The SEARCH command is implemented as an optimized version of SELECT that leverages indexing for better performance. The command follows this syntax:

```sql
result_table <- SEARCH FROM table_name WHERE column_name bin_op value
```

### 1.2 Implementation Details

The implementation is split into three main components:

1. **Syntactic Parsing**
   - Validates query format: `R <- SEARCH FROM relation_name WHERE column_name bin_op value`
   - Checks for correct number of tokens (9) and proper keywords (FROM, WHERE)
   - Parses and validates the binary operator and the value
   - Supported operators: <, >, <=, >=, ==, !=
   - Value must be an integer

2. **Semantic Parsing**
   - Verifies result relation doesn't already exist
   - Checks source table exists
   - Validates column exists in source table

3. **Execution**
   - Creates a new result table with same schema as source table
   - Checks for existing index on search column
   - If index exists:
     - Uses the index (B+ Tree) to find matching rows
     - Retrieves actual rows using row IDs from index
   - If no index exists:
     - Creates a new B+ Tree index
     - Uses new index for search
     - Falls back to sequential scan if index creation fails
   - Writes matching rows to result table

### 1.3 Performance Optimizations

1. **Index Reuse**
   - Checks for existing indexes before creating new ones
   - Maintains indexes for future queries

2. **Early Termination**
   - For operators like < and <=, stops scanning once a threshold is reached
   - Skips entire leaf nodes in B+ Tree when possible

3. **Memory Management**
   - Uses cursor-based row retrieval to maintain buffer efficiency
   - Only loads necessary blocks into memory

## 2. Choice of Indexing Structure

### 2.1 B+ Tree

We chose B+ Tree as the primary indexing structure for the following reasons:

1. **Range Query Support**
   - Efficient for both equality (==) and range queries (<, >, <=, >=)
   - Well-suited for the supported binary operators

2. **Ordered Structure**
   - Maintains sorted order of keys
   - Enables efficient sequential access
   - Optimizes range-based searches

3. **Balanced Structure**
   - Guarantees O(log n) search time
   - Self-balancing nature maintains performance as data grows

4. **Disk-Friendly**
   - Node size can match disk block size
   - Reduces I/O operations
   - Leaf nodes linked for efficient range queries

### 2.2 B+ Tree Implementation Details

Key features of our B+ Tree implementation:

1. **Node Structure**
   - Internal nodes store keys and child pointers
   - Leaf nodes store **keys and row IDs**
   - Leaf nodes linked for sequential access

2. **Index Management**
   - Persistent storage in '../data/temp' directory
   - Save/load functionality for index persistence
   - Automatic cleanup of unused indexes

3. **Error Handling**
   - Robust error checking and recovery
   - Fallback to sequential scan when necessary

## 3. Assumptions and Constraints

1. **Memory Constraints**
   - Maximum 10 blocks in memory at any time
   - Page size and block management handled by buffer manager

2. **Index Persistence**
   - Indexes stored in '../data/temp' directory
   - Index files named as 'tablename_columnname_bptree.idx'

3. **Performance**
   - Large tables assumed to exceed main memory
   - Indexing improves performance for frequent queries
   - Trade-off between storage space and query speed

4. **Concurrency**
   - Single-user environment assumed
   - No concurrent access handling

## 4. DELETE Command Implementation

The DELETE command hasn't been implemented in the provided codebase. An ideal implementation would:
```sql
DELETE FROM table_name WHERE condition
```
1. Leverage the same indexing structure as SEARCH
2. Update indexes after deletion
3. Handle page/block reorganization
5. Maintain ACID properties

6. **Index Selection**
   - Implement index selection based on query patterns
   - Add support for multiple indexes per table

7. **Memory Optimization**
   - Implement better buffer management for index operations
   - Optimize node size based on disk block size

8. **Functionality Extensions**
   - Add support for compound indexes
   - Implement DELETE command
   - Add support for non-integer data types

9. **Performance Enhancements**
   - Add statistics for query optimization
   - Implement bulk loading for faster index creation
   - Add index compression techniques


   # Insert Operation

### Input Format Example:
```
INSERT INTO table_name (col1=10,col2=10)
```

## 1. **Syntactic Parse Insert** (`syntacticParseInsert()`)

### **Purpose**:
The `syntacticParseInsert` function is responsible for validating the structure and syntax of the `INSERT` SQL query.

### **Key Steps**:
- **Validation of the Syntax**:
  - The function checks if the second token in the query is `INTO`, ensuring that the query follows the correct structure.
  - The function also ensures that at least one `col_name=value` pair is provided, which is a requirement for the `INSERT` query.

- **Handling Missing Values**:
  - If a column name or value is missing, the system does not break the parsing flow. Instead, it skips such key-value pairs.
  - Missing column names will have a default value (0) assigned as per the specification.

- **Trimming Extra Spaces**:
  - Leading and trailing spaces are removed from each key and value to ensure clean parsing even if extra spaces are present in the query.

- **Concatenating Key-Value Pairs**:
  - All parsed key-value pairs are stored in a map (`parsedQuery.insertKeyValue`), which will be used in the later stages of the `INSERT` operation.

---

## 2. **Semantic Parse Insert** (`semanticParseInsert()`)

### **Purpose**:
The `semanticParseInsert` function performs validations related to the table's existence and schema integrity before executing the `INSERT` operation.

### **Key Steps**:
- **Table Validation**:
  - The function checks if the table exists in the catalog using `tableCatalogue.isTable()`. If the table doesn't exist, an error message is displayed.

- **Column Matching**:
  - The function prints out the table's columns and validates that the parsed key-value pairs correspond to valid columns in the table's schema.
  - This ensures that only valid columns are used for insertion.

- **Debugging and Printing**:
  - Debug prints show the parsed key-value pairs and the table's columns, which help verify the current state of the query and aid in debugging.

- **Additional Checks**:
  - Additional checks can be added to validate data types or handle missing columns (default values), further ensuring the integrity of the `INSERT` operation.

---

## 3. **Execute Insert** (`executeINSERT()`)

### **Purpose**:
The `executeINSERT` function is responsible for actually inserting the parsed data into the target table.

### **Key Steps**:
- **Row Construction**:
  - The function creates a new row by iterating over the table's columns and mapping the corresponding values from `parsedQuery.insertKeyValue`.
  - If any columns are missing, the system assigns default values (0 as per the specification).

- **Inserting Row into Table**:
  - The constructed row is inserted into the table using `table->insertRow(row)`, adding the data to the table.

- **Error Handling**:
  - Proper error messages are displayed if the table or any column is invalid, ensuring users are informed of any issues during execution.

---

## **Additional Considerations**:

### **Separation of Concerns**:
- The syntactic parsing, semantic validation, and execution of the `INSERT` operation are modularized into distinct functions, making the code easier to maintain and extend.

### **Extensibility**:
- Future validations, such as data type checks or handling default values, can be added without affecting the core logic, enhancing the flexibility of the system.

---

By separating the concerns into clear stages — syntactic parsing, semantic validation, and execution — the system ensures that the `INSERT` operation is both well-structured and robust, minimizing the risk of errors and ensuring data integrity during insertion.


# Update Operation
```
UPDATE table_name WHERE col op val SET col = val
```

Where `op` can be one of: `==`, `!=`, `<`, `<=`, `>`, `>=`.

### **Key Steps**:
- **Token Count Check**:
  - Ensures the tokenized query contains exactly 8 tokens. Any deviation results in a syntax error.

- **Keyword Verification**:
  - Checks for proper placement of the keywords `WHERE`, `SET`, and `=`.

- **WHERE Clause Parsing**:
  - Scans the `WHERE` condition to identify the comparison operator.
  - Extracts the column name and value to be used in the filter condition.
  - Converts the operator string into an enumerated type (`BinaryOperator`).

- **SET Clause Parsing**:
  - Extracts the column and value that need to be updated.
  - Stores this information in the `parsedQuery.insertKeyValue` map for later use.

- **Error Handling**:
  - Provides meaningful error messages when the syntax does not conform to expectations.

---

## 2. **Semantic Parse Update** (`semanticParseUpdate()`)

### **Purpose**:
This function performs validations to ensure the `UPDATE` query refers to a valid table and uses valid columns.

### **Key Steps**:
- **Table Existence Check**:
  - Uses `tableCatalogue.isTable()` to confirm that the referenced table exists.

- **Pointer Safety**:
  - Retrieves the table pointer and checks if it's not null to avoid runtime issues.

- **Column Validation**:
  - Iterates through the table’s column names to ensure the columns used in `WHERE` and `SET` clauses are valid.
  - (Can be extended to validate column data types and constraints.)

- **Debug Information**:
  - Prints out the column names and parsed key-value pairs for clarity and debugging purposes.

---

## 3. **Execute Update** (`executeUPDATE()`)

### **Purpose**:
The `executeUPDATE` function applies the update operation by modifying the appropriate rows based on the parsed conditions.

### **Key Steps**:
- **Retrieve Target Table**:
  - Gets the table instance from the catalog using the table name.

- **Construct Update Row**:
  - Builds a new row using the values from `parsedQuery.insertKeyValue`.

- **Update Operation**:
  - Calls `table->updateRow(row)` to perform the actual update. (Assumes this method is defined to apply updates based on the WHERE condition and the new value.)

- **Debugging**:
  - Logs and prints useful messages to track the update execution.

---

## **Additional Notes**

### **Modular Design**:
- Breaking down the logic into three clearly defined phases improves readability, maintainability, and debuggability.

### **Extensibility**:
- Future improvements can include:
  - Multiple column updates in the `SET` clause.
  - Support for complex WHERE conditions.
  - Validation of data types and constraints.

### **Robustness**:
- Error handling is integrated at each stage to provide informative feedback and prevent incorrect operations.

---

By implementing a structured flow for parsing and executing `UPDATE` operations, the system ensures only valid updates are applied to consistent and verified data, thus maintaining the database's integrity.
