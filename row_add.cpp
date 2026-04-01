#include "db_schema.h"
#include <fstream>
#include <cstring>
using namespace std;

bool Actions::ADD_ROW(const vector<string>& values) {
    auto colDefs = load_col_defs();
    if (values.size() != colDefs.size())
        throw runtime_error("ADD_ROW: expected " + to_string(colDefs.size())
            + " values, got " + to_string(values.size()));

    ofstream dataStream(string(DEFAULT_TABLE) + "_data_book.bin", ios::binary | ios::app);
    if (!dataStream.is_open()) throw runtime_error("Cannot open data file");

    char aliveFlag = 0;
    dataStream.write(&aliveFlag, 1);

    for (int i = 0; i < (int)colDefs.size(); i++) {
        vector<char> buf(colDefs[i].size, 0);
        if (colDefs[i].type == COL_INT) {
            int v = stoi(values[i]);
            memcpy(buf.data(), &v, 4);
        }
        else {
            strncpy_s(buf.data(), colDefs[i].size, values[i].c_str(), _TRUNCATE);
        }
        dataStream.write(buf.data(), colDefs[i].size);
    }
    dataStream.close();

    fstream indexStream(string(DEFAULT_TABLE) + "_index_book.bin",
        ios::binary | ios::in | ios::out);
    GrandHeader hdr;
    indexStream.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    hdr.total_rows++;
    indexStream.seekp(0);
    indexStream.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    indexStream.close();

    cout << "Row added.\n";
    return true;
}
