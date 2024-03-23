#include <iostream>
#include <fstream>
#include <vector>

#include "index.h"
#include "globals.h"

using namespace std;


bool save_index_table()
{
    if (!indexFile.is_open()) {
        cerr << "Unable to open Index file for writing." << endl;
        return false;
    }
    // Write index_num and index_pos at the beginning of the file
    indexFile.seekp(0);
    indexFile.write(reinterpret_cast<const char*>(&index_num), sizeof(uint32_t));

    // Write index_file_loaded to the file
    for (const index& idx : index_file_loaded) {
        indexFile.write(reinterpret_cast<const char*>(&idx), sizeof(index));
    }

    return true;
}
