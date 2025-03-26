
# Phase - 2

## Sort:
* Each page is sorted individually before merging.
* Sorting supports multiple columns with ascending or descending order.
* External merge sort is used to handle large datasets.
* A priority queue is used for efficient K-way merging.
* Sorted data is written back to disk and can be made permanent.

### Synatic parser
* Checks Query Structure: Ensures "BY" is the third word and "IN" is present in the query.

* Extracts Table and Columns: Identifies the table name from the second token and extracts sorting columns until it encounters "IN".

* Validates Sorting Strategy: Collects sorting strategies (ASC or DESC) and verifies that the number of strategies matches the number of columns.

* Updates Parsed Query: If all checks pass, it updates parsedQuery with the extracted table name, columns, and sorting strategies.

* Handles Errors: If any required keyword is missing or there is a mismatch in column and strategy counts, it prints an error and returns false.

### Semantic Parser
* Checks Table Existence: Ensures the table to be sorted exists and that a result table with the same name does not already exist.

* Retrieves Table Schema: Fetches the table from tableCatalogue and prints its column names for reference.

* Validates Column Names: Ensures all specified sorting columns exist in the table.

* Handles Semantic Errors: If the table or any column is missing, it prints an error message and returns false.

* Confirms Execution Readiness: If all checks pass, it returns true, allowing the sorting operation to proceed.

---

### IMPLEMENTATION OF GROUP BY:
## GROUP BY
The `GROUP BY` command implements grouping operations with aggregation functions and HAVING clauses and returns the table containing the grouping attribute and the aggregated values. this table is stored in result_table.

**Syntax:**
```
<result_table> <- GROUP BY <grouping_attribute> FROM <table> HAVING <aggregate_function>(<attribute>) <operator> <value> RETURN <aggregate_function>(<attribute>)
```

**Implementation Details:**

1. **Syntactic Parsing:**
```cpp
bool syntacticParseGROUP_BY() {
    // Validates query length and structure
    if (tokenizedQuery.size() != 13) {
        cout << "SYNTAX ERROR [Query length is not correct. please see grammer]" << endl;
        return false;
    }
    
    // Extracts components:
    // - Result table name
    // - Grouping attribute
    // - Source table
    // - HAVING clause (aggregate function, attribute, operator, value)
    // - RETURN clause (aggregate function and attribute)
}
```

2. **Semantic Parsing:**
```cpp
bool semanticParseGROUP_BY() {
    // Validates:
    // - Result table doesn't already exist
    // - Source table exists
    // - All referenced columns exist in source table
}
```

3. **Execution Process:**

a) **Initial Setup:**
- Creates result table with columns for grouping attribute and aggregated value
- Sorts input table by grouping attribute for efficient processing

b) **Group Processing:**
```cpp
void Table::groupBy() {
    // Initialize variables for tracking groups
    int currentGroupValue = -1;
    int havingaggregateResult = 0;
    int havingaggregateCount = 0;
    int aggregationResult = 0;
    int aggregateCount = 0;
    
    // Process rows sequentially
    while (!row.empty()) {
        if (groupValue != currentGroupValue) {
            // Process previous group
            // Apply HAVING clause
            // Write results if conditions met
        } else {
            // Update aggregation results for current group
        }
    }
}
```

c) **Aggregation Functions:**
- Supports MIN, MAX, SUM, COUNT, and AVG
- Maintains running totals for both HAVING and RETURN clauses
- Handles special cases for AVG (division by count)

d) **HAVING Clause:**
- Evaluates conditions using specified operators (EQUAL, GREATER_THAN, GEQ)
- Only includes groups that satisfy the HAVING condition
- Supports comparison with numeric values

e) **Result Storage:**
- Writes results in blocks using BufferManager
- Creates CSV file with headers
- Updates table catalog with new table metadata

**Block Access:**
- Uses cursor-based iteration for efficient memory usage
- Processes data in blocks to handle large datasets
- Maintains sorted order for efficient grouping

**Error Handling:**
- Validates query syntax and structure
- Checks for existence of tables and columns
- Handles edge cases in aggregation functions
- Manages memory constraints through block-based processing

---
## ORDER BY
* The orderBy function retrieves all rows from the specified table using a cursor, storing them in a vector of vectors. It then identifies the index of the column used for sorting to ensure accurate ordering.

* The rows are sorted in ascending or descending order based on the sorting strategy, using the sort function with a lambda comparator that compares the values in the specified column.

* The sorted rows are written to a new table and saved to a CSV file, with column headers included. The new table is also inserted into the table catalog, allowing further operations on the sorted data.

### Syntax parsing
The function checks whether the query has exactly 8 tokens by verifying the size of tokenizedQuery. If the number of tokens is incorrect, it prints a syntax error message and returns false, ensuring that only correctly structured queries proceed further.

### Semantic Parsing
The function ensures that the result table does not already exist in tableCatalogue. 
Checks whether the column exist in the table or not.
If the table name given for the sorted output is already present, it prints a semantic error message and returns false, preventing unintended overwriting of existing tables.
---


## Assumptions
* for group by, the result should be non empty. else you can view it in the csv file but you cannot load or print that table

## Contribution of Each Member
* Everone contributed equally.
