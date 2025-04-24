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