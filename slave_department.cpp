#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

#include "master_quarter.h"
#include "slave_department.h"
#include "globals.h"
#include "index.h"

using namespace std;

bool Read_slave(Department& record, fstream& file, const streampos& pos)
{
    if (!file)
        return false;

    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(Department));

    return !file.fail();
}

bool Write(const Department& record, fstream& file, const streampos& pos)
{
    if (!file)
        return false;

    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Department));
    file.flush();

    return !file.fail();
}

int64_t find_last_slave_record(uint32_t master_id)
{
    int64_t last_record_id = -1;
    for (const auto& index : index_slave_file_loaded)
    {
        if (!index.is_deleted && index.next == -1 && index.master_id == master_id)
        {
            last_record_id = index.record_id;
            break;
        }
    }
    return last_record_id;
}

bool insert_s(Department department, uint32_t master_id)
{
    if (!slaveFile) {
        cerr << "Error: Slave file not open for writing." << endl;
        return false;
    }

    // Check if master_id is valid and not deleted
    bool valid_master_id = false;
    for (const index &idx : index_file_loaded)
    {
        if (idx.record_id == master_id && !idx.is_deleted)
        {
            valid_master_id = true;
            break;
        }
    }

    if (!valid_master_id)
    {
        cerr << "Error: Invalid or deleted master record ID." << endl;
        return false;
    }

    // Generating a unique ID for the department
    department.id = department_pos;

    // Check if there is any record with is_deleted = 1
    bool inserted = false;
    for (int i = 0; i < index_slave_file_loaded.size(); ++i)
    {
        if (index_slave_file_loaded[i].is_deleted)
        {
            // Found a deleted record, insert the new record at this position
            slaveFile.seekp(0 + 2 * sizeof(uint32_t) + i * sizeof(Department));
            if (!Write(department, slaveFile, 0 + 2 * sizeof(uint32_t) + i * sizeof(Department)))
            {
                cerr << "Error writing to slave file." << endl;
                return false;
            }

            index_slave_file_loaded[i].record_id = department_pos;
            index_slave_file_loaded[i].is_deleted = false;
            index_slave_file_loaded[i].master_id = master_id;
            index_slave_file_loaded[i].next = -1;

            index_file_loaded[master_id].department_number++;

            // Set the next pointer for the previous record if applicable
            int64_t next = index_file_loaded[master_id].first_department_id;
            int64_t prev = -1;
            while (next != -1)
            {
                prev = next;
                next = index_slave_file_loaded[next].next;
            }
            if (prev != -1)
            {
                index_slave_file_loaded[prev].next = department.id;
            }
            else
            {
                // If there was no previous record, update the first_department_id
                index_file_loaded[master_id].first_department_id = department.id;
            }
            inserted = true;
            cout << slaveFile.is_open() << endl;
            break;
        }
    }

    if (!inserted)
    {
        // No record with is_deleted = 1 found, insert at the end
        slaveFile.seekp(0 + 2 * sizeof(uint32_t) + department_num * sizeof(Department)); // Move to the end to append
        if (!Write(department, slaveFile, slaveFile.tellp()))
        {
            cerr << "Error writing to slave file." << endl;
            return false;
        }

        // Change the number of slave records in the slave file
        slaveFile.seekp(0); // Move to the start to change
        slaveFile.write(reinterpret_cast<const char*>(&++department_num), sizeof(uint32_t));
        slaveFile.flush();

        // Increment department_number for the corresponding master record
        for(int i = 0; i < quarter_num; ++i)
        {
            if(index_file_loaded[i].record_id == master_id && !index_file_loaded[i].is_deleted)
            {
                if (index_file_loaded[i].department_number == 0)
                {
                    index_file_loaded[i].department_number++;
                    index_file_loaded[i].first_department_id = department.id;
                }
                else
                {
                    index_file_loaded[i].department_number++;
                    // Search for the last element in the linked list
                    uint32_t last_element_id = index_file_loaded[i].first_department_id;
                    while (true)
                    {
                        bool found_next = false;
                        for (int j = 0; j < index_slave_file_loaded.size(); ++j)
                        {
                            if (index_slave_file_loaded[j].id == last_element_id && index_slave_file_loaded[j].next != -1)
                            {
                                last_element_id = index_slave_file_loaded[j].next;
                                found_next = true;
                                break;
                            }
                        }
                        if (!found_next)
                        {
                            // Insert the new slave record after the last element
                            for (int k = 0; k < index_slave_file_loaded.size(); ++k)
                            {
                                if (index_slave_file_loaded[k].id == last_element_id)
                                {
                                    // Update the next pointer of the last element to point to the new record
                                    index_slave_file_loaded[k].next = department.id;
                                    // Set the next pointer of the new record to -1 (indicating end of the list)
                                    // Write the updated record to the index_slave file
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        index_slave newIndex;
        newIndex.id = index_slave_num;
        newIndex.record_id = department_pos;
        newIndex.master_id = master_id;
        newIndex.next = -1;
        index_slave_file_loaded.push_back(newIndex);
        index_slave_num++;
    }

    // Change the last index of slave records in the slave file
    uint32_t last_department_pos = department_pos;
    slaveFile.seekp(0 + sizeof(uint32_t)); // Move to the start to change
    slaveFile.write(reinterpret_cast<const char*>(&last_department_pos), sizeof(uint32_t));
    slaveFile.flush();
    cout << slaveFile.is_open() << endl;
    if (slaveFile.fail())
    {
        cerr << "Error writing to slave file." << endl;
        return false;
    }

    // Informing the user about successful insertion
    cout << "Slave record inserted successfully with ID: " << department.id << endl;
    department_pos++;
    cout << slaveFile.is_open() << endl;

    return true;
}

void ut_s()
{
    cout << "Slave Table:" << endl;

    // Display column headers for index_slave table fields
    cout << setw(7) << "IndexID" << "|"
         << setw(8) << "RecordID" << "|"
         << setw(7) << "Deleted" << "|"
         << setw(5) << "Next" << "|"
         << setw(8) << "MasterID" <<"|"
         << setw(2) << "ID" <<"|"
         << setw(3) << "Age" << "|"
         << setw(6) << "Height" << "|"
         << setw(8) << "Diameter" << "|"
         << setw(7) << "Density" << "|"
         << setw(8) << "WoodType" << endl;

    // Iterate through the index_slave vector to retrieve all records
    for (const index_slave& idx_s : index_slave_file_loaded) {
        // Retrieve the corresponding record from the Department file
        streampos pos = 0 + 2 * sizeof(uint32_t) + idx_s.id * sizeof(Department);
        slaveFile.seekg(pos);
        Department currentDepartment;
        slaveFile.read(reinterpret_cast<char*>(&currentDepartment), sizeof(Department));

        // Display the record in a table format
        cout << setw(7) << idx_s.id << "|"
             << setw(8) << idx_s.record_id << "|"
             << setw(7) << (idx_s.is_deleted ? "true" : "false") << "|"
             << setw(5) << idx_s.next << "|"
             << setw(8) << idx_s.master_id << "|"
             << setw(2) << currentDepartment.id << "|"
             << setw(3) << currentDepartment.age << "|"
             << setw(6) << currentDepartment.height << "|"
             << setw(8) << currentDepartment.diameter << "|"
             << setw(7) << currentDepartment.density << "|"
             << setw(8) << currentDepartment.main_wood_type << endl;
    }
}

bool del_s(int id)
{
    // Search for the record ID in the index_slave file
    int pos = -1;
    for (int i = 0; i < department_num; ++i)
    {
        if (index_slave_file_loaded[i].record_id == id && !index_slave_file_loaded[i].is_deleted)
        {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        cerr << "Slave record with ID " << id << " not found or already deleted." << endl;
        return false;
    }

    // Mark the record as deleted
    index_slave_file_loaded[pos].is_deleted = true;
    cout << "Slave Record with ID " << id << " deleted." << endl;

    // If the deleted record was the first_department_id in any master record,
    // update the first_department_id field in the corresponding master record
    uint32_t master_id = index_slave_file_loaded[pos].master_id;
    if (index_file_loaded[master_id].first_department_id == id)
    {
        // Update first_department_id to the next record in the linked list
        index_file_loaded[master_id].first_department_id = index_slave_file_loaded[pos].next;
        index_file_loaded[master_id].department_number--;
    }
    else
    {
        // Update the next pointer of the previous record in the linked list
        int64_t next = index_file_loaded[master_id].first_department_id;
        while (next != -1)
        {
            if (index_slave_file_loaded[next].next == id)
            {
                index_slave_file_loaded[next].next = index_slave_file_loaded[pos].next;
                index_file_loaded[master_id].department_number--;
                break;
            }
            next = index_slave_file_loaded[next].next;
        }
    }

    return true;
}

bool get_s(int id, vector<string>& columns)
{
    if (!slaveFile) {
        cerr << "Error: Slave file not open for reading." << endl;
        return false;
    }

    // Search for the record ID in the index_slave vector
    int pos = -1;
    for (int i = 0; i < department_num; ++i)
    {
        if (index_slave_file_loaded[i].record_id == id && !index_slave_file_loaded[i].is_deleted)
        {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        cerr << "Record with ID " << id << " not found or already deleted." << endl;
        return false;
    }

    // Now, index_slave_file_loaded[pos].record_id contains the record ID in the slave file
    // We need to read the corresponding record from the slave file

    // Calculate the position of the record in the slave file
    streampos posInSlave = 0 + 2 * sizeof(uint32_t) + pos * sizeof(Department);

    // Read the record from the slave file
    slaveFile.seekg(posInSlave);
    Department currentDepartment;
    slaveFile.read(reinterpret_cast<char*>(&currentDepartment), sizeof(Department));

    // Output the requested columns
    if (columns.empty()) {
        // If no columns specified, output all columns
        cout << "id: " << currentDepartment.id << "\n";
        cout << "age: " << currentDepartment.age << "\n";
        cout << "height: " << currentDepartment.height << "\n";
        cout << "diameter: " << currentDepartment.diameter << "\n";
        cout << "density: " << currentDepartment.density << "\n";
        cout << "main_wood_type: " << currentDepartment.main_wood_type << "\n";
    } else {
        // Output the specified columns
        for (const string& column : columns) {
            if (column == "id")
                cout << "id: " << currentDepartment.id << "\n";
            else if (column == "age")
                cout << "age: " << currentDepartment.age << "\n";
            else if (column == "height")
                cout << "height: " << currentDepartment.height << "\n";
            else if (column == "diameter")
                cout << "diameter: " << currentDepartment.diameter << "\n";
            else if (column == "density")
                cout << "density: " << currentDepartment.density << "\n";
            else if (column == "main_wood_type")
                cout << "main_wood_type: " << currentDepartment.main_wood_type << "\n";
            else
                cout << "No column with name \"" << column << "\" in this table!\n";
            // Add more columns as needed
        }
    }

    return true;
}

int calc_s()
{
    if (!slaveFile) {
        cerr << "Error: Slave file not open for reading." << endl;
        return -1;
    }

    // Count the number of valid records in the slave file
    int validRecordCount = 0;
    for (const index_slave& idx : index_slave_file_loaded) {
        if (!idx.is_deleted) {
            validRecordCount++;
        }
    }

    return validRecordCount;
}

bool update_s(int id)
{
    if (!slaveFile) {
        cerr << "Error: Slave file not open for writing." << endl;
        return false;
    }

    // Search for the record ID in the index_slave vector
    index_slave currentIndex;
    bool found = false;
    for (int i = 0; i < index_slave_file_loaded.size(); i++)
    {
        if (index_slave_file_loaded[i].record_id == id && !index_slave_file_loaded[i].is_deleted)
        {
            found = true;
            currentIndex = index_slave_file_loaded[i];
            break;
        }
    }

    if (!found)
    {
        cerr << "Record with ID " << id << " not found or already deleted." << endl;
        return false;
    }

    // Now, currentIndex.record_id contains the record ID in the slave file
    // We need to read the corresponding record from the slave file

    // Calculate the position of the record in the slave file
    streampos pos = 0 + 2 * sizeof(uint32_t) + (currentIndex.id) * sizeof(Department);

    // Read the record from the slave file
    slaveFile.seekg(pos);
    Department currentDepartment;
    slaveFile.read(reinterpret_cast<char*>(&currentDepartment), sizeof(Department));

    // Output the requested columns
    cout << "Enter new age: ";
    cin >> currentDepartment.age;
    cout << "Enter new height: ";
    cin >> currentDepartment.height;
    cout << "Enter new diameter: ";
    cin >> currentDepartment.diameter;
    cout << "Enter new density: ";
    cin >> currentDepartment.density;
    cout << "Enter new main wood type: ";
    cin.ignore(); // Ignore newline character from previous input
    cin.getline(currentDepartment.main_wood_type, sizeof(currentDepartment.main_wood_type));

    // Write the updated record back to the slave file
    slaveFile.seekp(pos);
    slaveFile.write(reinterpret_cast<const char*>(&currentDepartment), sizeof(Department));
    slaveFile.flush();

    cout << "Record with ID " << id << " updated successfully." << endl;

    return true;
}












