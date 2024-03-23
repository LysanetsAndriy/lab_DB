#ifndef LAB_DB_SLAVE_DEPARTMENT_H
#define LAB_DB_SLAVE_DEPARTMENT_H

#include <cstdint>

using namespace std;

struct Department
{
    uint32_t id = 0;
    uint32_t age  = 0;
    uint32_t height = 0;
    uint32_t diameter = 0;
    uint32_t density = 0;

    char main_wood_type[25] = {};


};

int64_t find_last_slave_record(uint32_t master_id);
bool Read_slave(Department& record, fstream& file, const streampos& pos);
bool Write(const Department& record, fstream& file, const streampos& pos);
bool SetNextPtr(fstream& file, const streampos& record_pos, const streampos& next_record_pos);
bool AddNode(const Department& record, fstream& file, const streampos& pos, const streampos& prev_record_pos = -1);

bool insert_s(Department department, uint32_t master_id);
void ut_s();
bool del_s(int id);
bool get_s(int id, vector<string>& columns);
int calc_s();
bool update_s(int id);


#endif //LAB_DB_SLAVE_DEPARTMENT_H
