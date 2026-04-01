#include "db_schema.h"
#include <fstream>
using namespace std;

bool Actions::NEW_TABLE(const vector<string>& col_defs) {
    vector<ColDef> colDefsVec;
    for (auto& t : col_defs) colDefsVec.push_back(parse_col_def(t));

    // Clear and write index file
    ofstream indexStream(string(DEFAULT_TABLE) + "_index_book.bin",
        ios::binary | ios::out | ios::trunc);
    if (!indexStream.is_open()) throw runtime_error("Cannot create index file");

    GrandHeader hdr;
    hdr.num_columns = (int)colDefsVec.size();
    hdr.total_rows = 0;
    indexStream.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    for (auto& c : colDefsVec)
        indexStream.write(reinterpret_cast<const char*>(&c), sizeof(ColDef));
    indexStream.close();

    // Clear data file
    ofstream dataStream(string(DEFAULT_TABLE) + "_data_book.bin",
        ios::binary | ios::out | ios::trunc);
    if (!dataStream.is_open()) throw runtime_error("Cannot create data file");
    dataStream.close();

    cout << "Table reset with " << colDefsVec.size() << " column(s).\n";
    return true;
}
