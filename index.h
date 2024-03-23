
#ifndef LAB_DB_INDEX_H
#define LAB_DB_INDEX_H

#include <stdint.h>
#include <fstream>
;
struct index
{
    uint32_t id = 0;
    uint32_t record_id = 0;
    bool is_deleted = false;

    int64_t first_department_id = -1;

    uint32_t department_number = 0;
};

bool insert_index(index newIndex, int position = -1);
bool save_index_table();


#endif //LAB_DB_INDEX_H
