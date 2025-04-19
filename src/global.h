#include"executor.h"
#include <sys/stat.h>

extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint PRINT_COUNT;
extern vector<string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern TableCatalogue tableCatalogue;
extern MatrixCatalogue matrixCatalogue;
extern BufferManager bufferManager;

/**
 * @brief Helper function to evaluate binary operations
 * 
 * @param value1 the first value
 * @param value2 the second value
 * @param op the binary operator
 * @return true if the condition is satisfied
 * @return false otherwise
 */
inline bool evaluateBinOp(int value1, int value2, BinaryOperator op) {
    switch (op) {
        case LESS_THAN:
            return value1 < value2;
        case GREATER_THAN:
            return value1 > value2;
        case LEQ:
            return value1 <= value2;
        case GEQ:
            return value1 >= value2;
        case EQUAL:
            return value1 == value2;
        case NOT_EQUAL:
            return value1 != value2;
        default:
            return false;
    }
}