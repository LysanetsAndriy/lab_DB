#ifndef LAB_DB_MASTER_QUARTER_H
#define LAB_DB_MASTER_QUARTER_H

#include <vector>
#include <cstdint>

struct Quarter
{
    uint32_t id = 0;
    uint32_t area = 0;

    char coordinates[60] = {};
};

bool insert_m(Quarter quarter);
bool get_m(int id, std::vector<std::string>& columns);
bool del_m(int id);
bool update_m(int id);
void ut_m();
int calc_m();

#endif //LAB_DB_MASTER_QUARTER_H
