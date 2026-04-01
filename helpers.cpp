#include "db_schema.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
using namespace std;

ColDef parse_col_def(const string& token) {
    ColDef col;
    istringstream ss(token);
    string col_name, type_str;
    ss >> col_name >> type_str;
    strncpy_s(col.name, sizeof(col.name), col_name.c_str(), _TRUNCATE);

    if (type_str == "INT") {
        col.type = COL_INT; col.size = 4;
    }
    else if (type_str.rfind("STRING", 0) == 0) {
        col.type = COL_STRING; col.size = 20;
        size_t lb = type_str.find('['), rb = type_str.find(']');
        if (lb != string::npos && rb != string::npos)
            col.size = stoi(type_str.substr(lb + 1, rb - lb - 1));
    }
    else {
        throw runtime_error("Unknown type: " + type_str);
    }
    return col;
}

long long col_def_offset(int i) {
    return sizeof(GrandHeader) + (long long)i * sizeof(ColDef);
}

int row_size(const vector<ColDef>& cols) {
    int total = 0;
    for (auto& c : cols) total += c.size;
    return total;
}

long long row_offset(const vector<ColDef>& cols, int row_idx) {
    return (long long)row_idx * (1 + row_size(cols));
}

GrandHeader load_grand_header() {
    ifstream fh(string(DEFAULT_TABLE) + "_index_book.bin", ios::binary);
    if (!fh.is_open()) throw runtime_error("Table not found. Run NEW_TABLE first.");
    GrandHeader hdr;
    fh.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    return hdr;
}

vector<ColDef> load_col_defs() {
    auto hdr = load_grand_header();
    ifstream fh(string(DEFAULT_TABLE) + "_index_book.bin", ios::binary);
    fh.seekg(sizeof(GrandHeader));
    vector<ColDef> cols(hdr.num_columns);
    for (auto& c : cols)
        fh.read(reinterpret_cast<char*>(&c), sizeof(ColDef));
    return cols;
}

bool match_where(const vector<ColDef>& cols,
    const vector<string>& row_vals,
    const WhereClause& w)
{
    int ci = -1;
    for (int i = 0; i < (int)cols.size(); i++)
        if (string(cols[i].name) == w.column) { ci = i; break; }
    if (ci < 0) throw runtime_error("WHERE: unknown column: " + w.column);

    auto& op = w.oper;
    if (cols[ci].type == COL_INT) {
        int a = stoi(row_vals[ci]), b = stoi(w.value);
        if (op == "=")  return a == b;
        if (op == "!=") return a != b;
        if (op == ">")  return a > b;
        if (op == "<")  return a < b;
        if (op == ">=") return a >= b;
        if (op == "<=") return a <= b;
    }
    else {
        string a = row_vals[ci], b = w.value;
        if (op == "=")  return a == b;
        if (op == "!=") return a != b;
    }
    throw runtime_error("Unsupported operator: " + op);
}
