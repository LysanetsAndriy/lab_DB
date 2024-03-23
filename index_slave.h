
#ifndef LAB_DB_INDEX_SLAVE_H
#define LAB_DB_INDEX_SLAVE_H

#include <stdint.h>
#include <fstream>

struct index_slave
{
    uint32_t id = 0;
    uint32_t record_id = 0;
    bool is_deleted = false;
    int64_t next = -1;
    int64_t master_id = -1;
};

bool save_index_slave_table();


#endif //LAB_DB_INDEX_SLAVE_H
