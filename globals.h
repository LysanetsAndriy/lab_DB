#ifndef LAB_DB_GLOBALS_H
#define LAB_DB_GLOBALS_H

#include <vector>
#include "index.h"
#include "index_slave.h"
;
extern std::fstream masterFile, slaveFile, indexFile, indexSlaveFile;
extern uint32_t quarter_num;
extern uint32_t quarter_pos;
extern uint32_t index_num;
extern uint32_t department_num;
extern uint32_t department_pos;
extern uint32_t index_slave_num;
extern std::vector<index> index_file_loaded;
extern std::vector<index_slave> index_slave_file_loaded;



#endif //LAB_DB_GLOBALS_H
