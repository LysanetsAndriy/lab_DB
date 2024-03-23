#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

#include "master_quarter.h"
#include "globals.h"
#include "index.h"

using namespace std;

bool insert_m(Quarter quarter)
{
    if (!masterFile) {
        cerr << "Error: Master file not open for writing." << endl;
        return false;
    }

    // Generating a unique ID for the quarter
    quarter.id = quarter_pos;
    // Check if there is any record with is_deleted = 1
    bool inserted = false;
    for (int i = 0; i < index_file_loaded.size(); ++i)
    {
        if (index_file_loaded[i].is_deleted)
        {
            // Found a deleted record, insert the new record at this position
            masterFile.seekp(0 + 2 * sizeof(uint32_t) + i * sizeof(Quarter));
            masterFile.write(reinterpret_cast<const char*>(&quarter), sizeof(Quarter));
            masterFile.flush();

            index_file_loaded[i].record_id = quarter_pos;
            index_file_loaded[i].is_deleted = false;
            index_file_loaded[i].first_department_id = -1;
            index_file_loaded[i].department_number = 0;

            inserted = true;
            break;
        }
    }

    if (!inserted)
    {
        // No record with is_deleted = 1 found, insert at the end
        masterFile.seekp(0 + 2 * sizeof(uint32_t) + quarter_num * sizeof(Quarter)); // Move to the end to append
        masterFile.write(reinterpret_cast<const char*>(&quarter), sizeof(Quarter));
        masterFile.flush();

        // Change the number of master records in the master file
        masterFile.seekp(0); // Move to the start to change
        masterFile.write(reinterpret_cast<const char*>(&++quarter_num), sizeof(uint32_t));
        masterFile.flush();

        index newIndex;
        newIndex.id = index_num;
        newIndex.record_id = quarter_pos;
        index_file_loaded.push_back(newIndex);
        index_num++;
    }

    // Change the the last index of master records in the master file
    masterFile.seekp(0 + sizeof(uint32_t)); // Move to the start to change
    masterFile.write(reinterpret_cast<const char*>(&quarter_pos), sizeof(uint32_t));
    masterFile.flush();

    if (masterFile.fail())
    {
        cerr << "Error writing to master file." << endl;
        return false;
    }

    // Informing the user about successful insertion
    cout << "Master record inserted successfully with ID: " << quarter.id << endl;
    quarter_pos++;
    return true;
}

bool get_m(int id, vector<string>& columns)
{
    if (!masterFile) {
        cerr << "Error: Master file not open for reading." << endl;
        return false;
    }

    // Search for the record ID in the index vector
    index currentIndex;
    bool found = false;
    for (int i = 0; i < index_file_loaded.size(); i++)
    {
        if (index_file_loaded[i].record_id == id && !index_file_loaded[i].is_deleted)
        {
            found = true;
            currentIndex = index_file_loaded[i];
            break;
        }
    }

    if (!found) {
        cerr << "Record with ID " << id << " not found or already deleted." << endl;
        return false;
    }

    // Now, currentIndex.record_id contains the record ID in the master file
    // We need to read the corresponding record from the master file

    // Calculate the position of the record in the master file
    streampos pos = 0+2*sizeof(uint32_t) + (currentIndex.id) * sizeof(Quarter);

    // Read the record from the master file
    masterFile.seekg(pos);
    Quarter currentQuarter;
    masterFile.read(reinterpret_cast<char*>(&currentQuarter), sizeof(Quarter));

    // Output the requested columns
    if (columns.empty()) {
        // If no columns specified, output all columns
        cout << "id: " << currentQuarter.id << "\n";
        cout << "area: " << currentQuarter.area << "\n";
        cout << "coordinates: " << currentQuarter.coordinates << "\n";
    } else {
        // Output the specified columns
        for (const string& column : columns) {
            if (column == "id")
                cout << "id: " << currentQuarter.id << "\n";
            else if (column == "area")
                cout << "area: " << currentQuarter.area << "\n";
            else if (column == "coordinates")
                cout << "coordinates: " << currentQuarter.coordinates << "\n";
            else
                cout << "No column with name \"" << column << "\" in this table!\n";
            // Add more columns as needed
        }
        cout << endl;
    }

    return true;
}

bool del_m(int id)
{
    // Search for the record ID in the index file
    int pos = 0;
    index currentIndex;
    bool found = false;
    for (int i = 0; i < index_file_loaded.size(); i++)
    {
        if (index_file_loaded[i].record_id == id && !index_file_loaded[i].is_deleted)
        {
            found = true;
            currentIndex = index_file_loaded[i];
            pos = i;
            break;
        }
    }

    if (!found) {
        cerr << "Record with ID " << id << " not found or already deleted." << endl;
        return false;
    }

    // Mark the record as deleted
    index_file_loaded[pos].is_deleted = true;

    cout << "Master Record with ID " << id << " deleted." << endl;

    // Delete associated slave records
    int64_t next = index_file_loaded[pos].first_department_id;
    int64_t index_id = next;
    while(next != -1)
    {
        for(int i = 0; i < department_num; i++)
        {
            if(!index_slave_file_loaded[i].is_deleted && index_slave_file_loaded[i].record_id == next)
            {
                index_id = i;
                break;
            }
        }
        // Mark the slave record as deleted
        if(index_slave_file_loaded[index_id].is_deleted == false)
        {
            index_slave_file_loaded[index_id].is_deleted = true;
            cout << "Slave record with ID " << index_slave_file_loaded[index_id].record_id << " deleted." << endl;
            next = index_slave_file_loaded[index_id].next;
            index_file_loaded[pos].department_number--;
        }
    }

    return true;
}

void ut_m()
{
    cout << "Master Table:" << endl;

    // Display column headers for index table fields
    cout << std::setw(8) << "IndexID" << "|"
         << std::setw(8) << "RecordID" << "|"
         << std::setw(8) << "Deleted" << "|"
         << std::setw(8) << "SlaveID" << "|"
         << std::setw(8) << "Slave#" << "|"
         << std::setw(8) << "MasterID" << "|"
         << std::setw(5) << "Area" << "|"
         << std::setw(10) << "Coordinates" << endl;

    // Iterate through the index file to retrieve all records
    for (const index& idx : index_file_loaded) {
        // Retrieve the corresponding record from the master file
        streampos pos = 0 + 2 * sizeof(uint32_t) + idx.id * sizeof(Quarter);
        masterFile.seekg(pos);
        Quarter currentQuarter;
        masterFile.read(reinterpret_cast<char*>(&currentQuarter), sizeof(Quarter));

        // Display the record in a table format
        cout << std::setw(8) << idx.id << "|"
             << std::setw(8) << idx.record_id << "|"
             << std::setw(8) << (idx.is_deleted ? "true" : "false") << "|"
             << std::setw(8) << idx.first_department_id << "|"
             << std::setw(8) << idx.department_number << "|"
             << std::setw(8) << currentQuarter.id << "|"
             << std::setw(5) << currentQuarter.area << "|"
             << std::setw(10) << currentQuarter.coordinates << endl;
    }
}


int calc_m()
{
    if (!masterFile) {
        cerr << "Error: Master file not open for reading." << endl;
        return -1;
    }

    // Count the number of valid records in the master file
    int validRecordCount = 0;
    for (const index& idx : index_file_loaded) {
        if (!idx.is_deleted) {
            validRecordCount++;
        }
    }

    return validRecordCount;
}


bool update_m(int id)
{
    if (!masterFile) {
        cerr << "Error: Master file not open for writing." << endl;
        return false;
    }

    // Search for the record ID in the index vector
    index currentIndex;
    bool found = false;
    for (int i = 0; i < quarter_num; i++)
    {
        if (index_file_loaded[i].record_id == id && !index_file_loaded[i].is_deleted)
        {
            found = true;
            currentIndex = index_file_loaded[i];
            break;
        }
    }

    if (!found)
    {
        cerr << "Record with ID " << id << " not found or already deleted." << endl;
        return false;
    }

    // Now, currentIndex.record_id contains the record ID in the master file
    // We need to read the corresponding record from the master file

    // Calculate the position of the record in the master file
    streampos pos = 0 + 2 * sizeof(uint32_t) + (currentIndex.id) * sizeof(Quarter);

    // Read the record from the master file
    masterFile.seekg(pos);
    Quarter currentQuarter;
    masterFile.read(reinterpret_cast<char*>(&currentQuarter), sizeof(Quarter));

    // Output the requested columns
    cout << "Enter new area: ";
    cin >> currentQuarter.area;
    cout << "Enter new coordinates: ";
    cin.ignore(); // Ignore newline character from previous input
    cin.getline(currentQuarter.coordinates, sizeof(currentQuarter.coordinates));

    // Write the updated record back to the master file
    masterFile.seekp(pos);
    masterFile.write(reinterpret_cast<const char*>(&currentQuarter), sizeof(Quarter));
    masterFile.flush();

    cout << "Record with ID " << id << " updated successfully." << endl;

    return true;
}
