#include "tableCatalogue.h"

using namespace std;

enum QueryType
{
    CLEAR,
    CROSS,
    DISTINCT,
    EXPORT,
    EXPORT_MATRIX,
    INDEX,
    JOIN,
    LIST,
    LOAD,
    LOAD_MATRIX,
    PRINT,
    GROUP_BY,
    ORDERBY,
    PRINT_MATRIX,
    PROJECTION,
    RENAME,
    SELECTION,
    SORT,
    SOURCE,
    CROSSTRANSPOSE,
    ROTATE_MATRIX,
    CHECKANTISYM,
    UNDETERMINED
};

enum BinaryOperator
{
    LESS_THAN,
    GREATER_THAN,
    LEQ,
    GEQ,
    EQUAL,
    NOT_EQUAL,
    NO_BINOP_CLAUSE
};

enum SortingStrategy
{
    ASC,
    DESC,
    NO_SORT_CLAUSE
};

enum SelectType
{
    COLUMN,
    INT_LITERAL,
    NO_SELECT_CLAUSE
};

enum Aggregate
{
    NO_AGGREGATE_FUNCTION,
    COUNT,
    SUM,
    AVG,
    MIN,
    MAX
};

class ParsedQuery
{

public:
    QueryType queryType = UNDETERMINED;

    string clearRelationName = "";

    string crossResultRelationName = "";
    string crossFirstRelationName = "";
    string crossSecondRelationName = "";

    string distinctResultRelationName = "";
    string distinctRelationName = "";

    string exportRelationName = "";

    IndexingStrategy indexingStrategy = NOTHING;
    string indexColumnName = "";
    string indexRelationName = "";

    BinaryOperator joinBinaryOperator = NO_BINOP_CLAUSE;
    string joinResultRelationName = "";
    string joinFirstRelationName = "";
    string joinSecondRelationName = "";
    string joinFirstColumnName = "";
    string joinSecondColumnName = "";

    string loadRelationName = "";

    string printRelationName = "";
    string rotateRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";

    // GROUP BY REQUIREMENTS
    string groupByResultRelationName = "";
    string groupAttribute = "";
    string groupRelation = "";
    string havingAttribute = "";
    string havingAggregate = "";
    string returnAggregate = "";
    string returnAttribute = "";
    int havingValue = 0;
    Aggregate havingAgg = NO_AGGREGATE_FUNCTION;
    Aggregate returnAgg = NO_AGGREGATE_FUNCTION;
    BinaryOperator groupBinaryOperator = NO_BINOP_CLAUSE;

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;

    SortingStrategy sortingStrategy = NO_SORT_CLAUSE;
    string sortResultRelationName = "";
    string sortColumnName = "";
    vector<string> sortColumns;
    vector<SortingStrategy> sortStrategy;
    string sortRelationName = "";

    string sourceFileName = "";

    string Matrix1 = "";
    string Matrix2 = "";

    string orderResultRelation = "";
    string orderRelationName = "";
    string orderAttribute = "";

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseEXPORT_MATRIX();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParseLOAD_MATRIX();
bool syntacticParsePRINT();
bool syntacticParsePRINT_MATRIX();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();
bool syntacticParseCROSSTRANSPOSE();
bool syntacticParseROTATE_MATRIX();
bool syntacticParseCHECKANTISYM();
bool syntacticParseGROUP_BY();
bool syntacticParseORDERBY();

bool isFileExists(string tableName);
bool isQueryFile(string fileName);
