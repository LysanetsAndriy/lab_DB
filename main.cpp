#include <iostream>
#include <fstream>
#include <vector>
#include <cerrno>
#include <cstring>

#include "slave_department.h"
#include "master_quarter.h"
#include "index.h"
#include "index_slave.h"
#include "globals.h"

using namespace std;

fstream masterFile, slaveFile, indexFile, indexSlaveFile;
uint32_t quarter_num = 0;
uint32_t quarter_pos = 0;
uint32_t index_num = 0;
uint32_t department_num = 0;
uint32_t department_pos = 0;
uint32_t index_slave_num = 0;
vector<index> index_file_loaded = {};
vector<index_slave> index_slave_file_loaded = {};

bool Read_master(Quarter& record, fstream& file, const streampos& pos);

vector<string> split(string str, string token);
void process_command(vector<string> command);
void initialize_file(const string& filename);
void save_global_vars();

int main()
{
    vector<string> filenames = {"master.bin", "slave.bin", "index.bin", "index_slave.bin"};

    for (const auto& filename : filenames)
    {
        initialize_file(filename);
    }

    string s = "";
    while(s != "exit")
    {
        getline(cin, s);
        vector<string> command = split(s, " ");
        process_command(command);
    }


    save_index_table();
    save_index_slave_table();
    save_global_vars();

    masterFile.close();
    slaveFile.close();
    indexFile.close();
    indexSlaveFile.close();
    return 0;
}


//=======================FUNCTIONS========================================
vector<string> split(string str, string token)
{
    vector<string>result;
    while(str.size())
    {
        int index = str.find(token);
        if(index!=string::npos)
        {
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)
                result.push_back(str);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

void initialize_file(const string& filename)
{
    fstream temp(filename, ios::binary | ios::in | ios::out);

    if (!temp.is_open())
    {
        temp.close();
        temp.open(filename, ios::binary | ios::in | ios::out | ios::trunc);

        if (!temp) {
            cerr << "Unable to create file " << filename << ": " << strerror(errno) << endl;
            return;
        }

        if (filename == "master.bin" || filename == "slave.bin")
        {
            // Initialize with count of records (set to 0)
            uint32_t count = 0;
            temp.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
            temp.flush();
            // Initialize with the last id(set to 0)
            temp.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
            temp.flush();
        }
        if (filename == "index.bin" || "index_slave.bin")
        {
            // Initialize with count of records (set to 0)
            uint32_t count = 0;
            temp.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
            temp.flush();
        }

    }

    if (filename == "master.bin")
    {
        masterFile = move(temp);

        // read number of quarters(masters)
        masterFile.seekg(0);
        masterFile.read(reinterpret_cast<char*>(&quarter_num), sizeof(uint32_t));
        masterFile.seekg(0 + sizeof(uint32_t));
        masterFile.read(reinterpret_cast<char*>(&quarter_pos), sizeof(uint32_t));
    }
    else if (filename == "slave.bin")
    {
        slaveFile = move(temp);

        // read number of departments(slaves)
        slaveFile.seekg(0);
        slaveFile.read(reinterpret_cast<char*>(&department_num), sizeof(uint32_t));
        slaveFile.seekg(0 + sizeof(uint32_t));
        slaveFile.read(reinterpret_cast<char*>(&department_pos), sizeof(uint32_t));
    }
    else if (filename == "index.bin" )
    {
        indexFile = move(temp);

        // read number of indexes
        indexFile.seekg(0);
        indexFile.read(reinterpret_cast<char*>(&index_num), sizeof(uint32_t));

        index tmp_index;
        for (int i = 0; i < index_num; i++)
        {
            indexFile.read(reinterpret_cast<char*>(&tmp_index), sizeof(index));
            index_file_loaded.push_back(tmp_index);
        }
    }
    else if (filename == "index_slave.bin" )
    {
        indexSlaveFile = move(temp);

        // read number of indexes
        indexSlaveFile.seekg(0);
        indexSlaveFile.read(reinterpret_cast<char*>(&index_slave_num), sizeof(uint32_t));

        index_slave tmp_index;
        for (int i = 0; i < index_slave_num; i++)
        {
            indexSlaveFile.read(reinterpret_cast<char*>(&tmp_index), sizeof(index_slave));
            index_slave_file_loaded.push_back(tmp_index);
        }
    }
}

void save_global_vars()
{
    // Save quarter_num and quarter_pos to "master.bin"
    if (masterFile)
    {
        masterFile.seekp(0); // Move to the start to change
        masterFile.write(reinterpret_cast<const char*>(&quarter_num), sizeof(uint32_t));
        masterFile.flush();

        masterFile.seekp(0+sizeof(uint32_t)); // Move to the start to change
        masterFile.write(reinterpret_cast<const char*>(&quarter_pos), sizeof(uint32_t));
        masterFile.flush();
    }


    // Save index_num to "index.bin"
    if (indexFile)
    {
        indexFile.seekp(0); // Move to the start to change
        indexFile.write(reinterpret_cast<const char*>(&index_num), sizeof(uint32_t));
        indexFile.flush();
    }

    // Save quarter_num and quarter_pos to "slave.bin"
    if (slaveFile)
    {
        slaveFile.seekp(0); // Move to the start to change
        slaveFile.write(reinterpret_cast<const char*>(&department_num), sizeof(uint32_t));
        slaveFile.flush();

        slaveFile.seekp(0+sizeof(uint32_t)); // Move to the start to change
        slaveFile.write(reinterpret_cast<const char*>(&department_pos), sizeof(uint32_t));
        slaveFile.flush();
    }

    // Save index_slave_num to "index_slave.bin"
    if (indexSlaveFile)
    {
        indexSlaveFile.seekp(0); // Move to the start to change
        indexSlaveFile.write(reinterpret_cast<const char*>(&index_slave_num), sizeof(uint32_t));
        indexSlaveFile.flush();
    }
}

void process_command(vector<string> command)
{
    if (command[0] == "get-m")
    {
        if (command.size() < 2)
        {
            cerr << "Error: Missing quarter ID." << endl;
            return;
        }

        // Parse quarter ID
        int id = stoi(command[1]);

        // Parse columns
        vector<string> columns;
        for (int i = 2; i < command.size(); ++i)
            columns.push_back(command[i]);

        // Call get_m function
        get_m(id, columns);
    }
    else if (command[0] == "get-s")
    {
        if (command.size() < 2)
        {
            cerr << "Error: Missing department ID." << endl;
            return;
        }

        // Parse quarter ID
        int id = stoi(command[1]);

        // Parse columns
        vector<string> columns;
        for (int i = 2; i < command.size(); ++i)
            columns.push_back(command[i]);

        // Call get_m function
        get_s(id, columns);
    }
    else if (command[0] == "del-m")
    {
        if (command.size() < 2)
        {
            cerr << "Error: Missing quarter ID." << endl;
            return;
        }

        // Parse quarter ID
        int id = stoi(command[1]);

        // Call del_m function
        del_m(id);

    }
    else if (command[0] == "del-s")
    {
        if (command.size() < 2)
        {
            cerr << "Error: Missing quarter ID." << endl;
            return;
        }

        // Parse quarter ID
        int id = stoi(command[1]);

        // Call del_m function
        del_s(id);
    }
    else if (command[0] == "update-m")
    {
        if (command.size() < 2)
        {
            cerr << "Error: Missing quarter ID." << endl;
            return;
        }

        // Parse quarter ID
        int id = stoi(command[1]);

        // Call update_m function
        update_m(id);
    }
    else if (command[0] == "update-s")
    {
        if (command.size() < 2)
        {
            cerr << "Error: Missing department ID." << endl;
            return;
        }

        // Parse quarter ID
        int id = stoi(command[1]);

        // Call update_m function
        update_s(id);
    }
    else if (command[0] == "insert-m")
    {
        Quarter newQuarter;
        cout << "Enter area: ";
        cin >> newQuarter.area;
        cout << "Enter coordinates: ";
        cin.ignore(); // Ignore newline character from previous input
        cin.getline(newQuarter.coordinates, sizeof(newQuarter.coordinates));

        // Inserting the new quarter record
        if (!insert_m(newQuarter))
        {
            cerr << "Master record was NOT inserted." << endl;
        }
    }
    else if (command[0] == "insert-s")
    {
        uint32_t master_id = -1;
        Department newDepartment;
        cout << "Enter age: ";
        cin >> newDepartment.age;
        cout << "Enter height: ";
        cin >> newDepartment.height;
        cout << "Enter diameter: ";
        cin >> newDepartment.diameter;
        cout << "Enter density: ";
        cin >> newDepartment.density;
        cout << "Enter master ID: ";
        cin >> master_id;
        cout << "Enter main wood type: ";
        cin.ignore(); // Ignore newline character from previous input
        cin.getline(newDepartment.main_wood_type, sizeof(newDepartment.main_wood_type));

        // Inserting the new department record
        if (!insert_s(newDepartment, master_id))
        {
            cerr << "Slave record was NOT inserted." << endl;
        }
        cout << slaveFile.is_open() << endl;

    }
    else if (command[0] == "calc-m")
    {
        cout << calc_m() << '\n';
    }
    else if (command[0] == "calc-s")
    {
        cout << calc_s() << '\n';
    }
    else if (command[0] == "ut-m")
    {
        ut_m();
    }
    else if (command[0] == "ut-s")
    {
        ut_s();
    }

}


















