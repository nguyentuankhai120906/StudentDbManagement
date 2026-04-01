﻿#include "db_schema.h"
#include <fstream>
#include <iomanip>
using namespace std;

void Actions::SELECT(bool has_where, const WhereClause& w) {
    auto colDefs = load_col_defs();
    auto hdr = load_grand_header();
    int  rowBytes = row_size(colDefs);

    ifstream dataStream(string(DEFAULT_TABLE) + "_data_book.bin", ios::binary);
    if (!dataStream.is_open()) throw runtime_error("Cannot open data file");

    // Header bảng
    for (auto& c : colDefs) cout << setw(14) << left << c.name;
    cout << "\n" << string(14 * colDefs.size(), '-') << "\n";

    int foundCount = 0;
    for (int r = 0; r < hdr.total_rows; r++) {
        char flag;
        dataStream.read(&flag, 1);

        vector<char> buffer(rowBytes);
        dataStream.read(buffer.data(), rowBytes);
        if (flag) continue; // deleted

        // Decode từng cột
        vector<string> vals;
        int offset = 0;
        for (auto& c : colDefs) {
            if (c.type == COL_INT) {
                int v = 0; memcpy(&v, buffer.data() + offset, 4);
                vals.push_back(to_string(v));
            }
            else {
                vals.push_back(string(buffer.data() + offset));
            }
            offset += c.size;
        }

        if (!has_where || match_where(colDefs, vals, w)) {
            for (auto& v : vals) cout << setw(14) << left << v;
            cout << "\n";
            foundCount++;
        }
    }

    cout << foundCount << " row(s) found.\n";
}
