#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
using namespace std;

constexpr const char* DEFAULT_TABLE = "default";

enum ColType : int { COL_INT = 0, COL_STRING = 1 };

struct ColDef {
    char    name[32];
    ColType type;
    int     size;
    ColDef() : type(COL_INT), size(4) { memset(name, 0, sizeof(name)); }
};

struct GrandHeader {
    char signature[20];
    int  num_columns;
    int  total_rows;
    GrandHeader() : num_columns(0), total_rows(0) {
        // safe portable init
        std::memset(signature, 0, sizeof(signature));
        const char sig[] = "index_book";
        std::memcpy(signature, sig, std::min<size_t>(sizeof(sig) - 1, sizeof(signature) - 1));
        signature[sizeof(signature) - 1] = '\0';
    }
};

struct WhereClause { string column, oper, value; };

// helpers
ColDef         parse_col_def(const string& token);
GrandHeader    load_grand_header();
vector<ColDef> load_col_defs();
int            row_size(const vector<ColDef>& cols);
bool           match_where(const vector<ColDef>& cols,
    const vector<string>& vals,
    const WhereClause& w);
long long      row_offset(const vector<ColDef>& cols, int row_idx);

class Actions {
public:
    // NEW_TABLE: nhận list col def, xoá trắng 2 file và tạo lại
    static bool NEW_TABLE(const vector<string>& col_defs);
    static bool ADD_ROW(const vector<string>& values);
    static void SELECT(bool has_where, const WhereClause& w);
    // DELETE không WHERE → xoá tất cả row
    static bool DELETE_ROW(bool has_where, const WhereClause& w);
};
