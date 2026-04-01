#include "db_schema.h"
#include <fstream>
using namespace std;

bool Actions::DELETE_ROW(bool has_where, const WhereClause& w) {
    auto colDefs = load_col_defs();
    auto hdr = load_grand_header();
    int  rowBytes = row_size(colDefs);

    fstream dataStream(string(DEFAULT_TABLE) + "_data_book.bin",
        ios::binary | ios::in | ios::out);
    if (!dataStream.is_open()) throw runtime_error("Cannot open data file");

    int delCount = 0;
    for (int idx = 0; idx < hdr.total_rows; idx++) {
        long long pos = row_offset(colDefs, idx);
        dataStream.seekg(pos);
        char flag; dataStream.read(&flag, 1);
        if (flag) continue;

        vector<char> buffer(rowBytes);
        dataStream.read(buffer.data(), rowBytes);

        bool toDelete = true;
        if (has_where) {
            vector<string> valuesVec;
            int offset = 0;
            for (auto& cd : colDefs) {
                if (cd.type == COL_INT) {
                    int v = 0; memcpy(&v, buffer.data() + offset, 4);
                    valuesVec.push_back(to_string(v));
                }
                else {
                    valuesVec.push_back(string(buffer.data() + offset));
                }
                offset += cd.size;
            }
            toDelete = match_where(colDefs, valuesVec, w);
        }

        if (toDelete) {
            dataStream.seekp(pos);
            char del = 1; dataStream.write(&del, 1);
            delCount++;
        }
    }

    cout << delCount << " row(s) deleted.\n";
    return delCount > 0;
}
