#include "tableCatalogue.h"
// #include "matrixCatalogue.h"

using namespace std;

enum QueryType
{
    CLEAR,
    CROSS,
    DISTINCT,
    EXPORT,
    INDEX,
    JOIN,
    LIST,
    LOAD,
    LOAD_MATRIX,
    TRANSPOSE,
    CHECKSYMMETRY,
    EXPORT_MATRIX,
    COMPUTE,
    PRINT,
    PRINT_MATRIX,
    PROJECTION,
    RENAME,
    RENAME_MATRIX,
    SELECTION,
    SORT,
    SOURCE,
    UNDETERMINED,
    ORDER_BY,
    GROUP_BY
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
    string transposeRelationName = "";

    string printRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";
    string renameToRelationName = "";

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;

    // vector<SortingStrategy> sortingStrategy = {};
    // string sortResultRelationName = "";
    // vector<string> sortColumnNames = {};
    vector<pair<string, SortingStrategy>> sortColumns;
    string sortRelationName = "";

    string sourceFileName = "";

    string orderByResultantRelationName = "";

    string groupByResultantRelationName = "";
    
    string havingAGG = "";
    string returnAGG = "";
    
    string groupByColumn = "";
    string havingColumn = "";
    string returnColumn = "";
    

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParseTRANSPOSE();
bool syntacticParseCHECKSYMMETRY();
bool syntacticParseEXPORTMATRIX();
bool syntacticParseCOMPUTE();
bool syntacticParsePRINT();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();
bool syntacticParseORDERBY();
bool syntacticParseGROUPBY();

bool isFileExists(string tableName);
bool isQueryFile(string fileName);
