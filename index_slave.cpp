#include <iostream>
#include <fstream>
#include <vector>

#include "index_slave.h"
#include "globals.h"

using namespace std;

bool save_index_slave_table()
{
    if (!indexSlaveFile.is_open()) {
        cerr << "Unable to open Slave Index file for writing." << endl;
        return false;
    }
    // Write index_num and index_pos at the beginning of the file
    indexSlaveFile.seekp(0);
    indexSlaveFile.write(reinterpret_cast<const char*>(&index_slave_num), sizeof(uint32_t));

    // Write index_file_loaded to the file
    for (const index_slave& idx : index_slave_file_loaded) {
        indexSlaveFile.write(reinterpret_cast<const char*>(&idx), sizeof(index_slave));
    }

    return true;
}
