/////////////////////////////////////////////////////////////////////////////////
// @file            main.cpp
// @brief           Configuration File Updater main entry point
// @author          Chip Brommer
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"
//
/////////////////////////////////////////////////////////////////////////////////

constexpr int MAJOR_VERSION = 0;
constexpr int MINOR_VERSION = 0;
constexpr int BUILD_VERSION = 1;

static bool g_exitRequested = false;

/// @brief Function declaration for UpdateJsonArray
/// @param array - json array
/// @param arrayName - name of the key for the array
static void UpdateJsonArray(nlohmann::json& array, const std::string& arrayName);

/// @brief Load the json file into a json object
/// @param filename - filename/path to be loaded
/// @return - json object of the file contents
static nlohmann::json LoadJsonFile(const std::string& filename)
{
    std::ifstream file(filename);
    nlohmann::json jsonData;
    file >> jsonData;
    return jsonData;
}

/// @brief Function to validate boolean input
/// @param input - checks string input for valid mapping to true/false
/// @return - true if the string is a bool value "true" or "false", false if not. 
static bool ValidateBooleanInput(const std::string& input)
{
    std::string lowercaseInput;
    for (char c : input) 
    {
        lowercaseInput += tolower(c);
    }
    return lowercaseInput == "true" || lowercaseInput == "false";
}

/// @brief Function to update JSON object
/// @param object - json object
/// @param objectName - optional - name of the object key
static void UpdateJsonObject(nlohmann::json& object, const std::string& objectName = "")
{
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        // Return if exit requested - At top for recursive catch
        if (g_exitRequested)
        {
            return;
        }

        // Get the key name and determine the type of the value
        std::string key = it.key();
        std::string valueType;

        if (it.value().is_string()) { valueType = "string"; }
        else if (it.value().is_number_integer()) { valueType = "integer"; }
        else if (it.value().is_number_float()) { valueType = "double"; }
        else if (it.value().is_boolean()) { valueType = "boolean"; }
        else if (it.value().is_array()) { 
            UpdateJsonArray(object[key], key); continue; }
        else if (it.value().is_object()) { UpdateJsonObject(object[key], key); continue; } // Recursively update the object
        else if (it.value().is_null()) { continue; }
        else { continue; }

        // Display info to user for item they're updating
        if (objectName == "")
            std::cout << "Enter value for " << key << " (" << valueType << "): ";
        else
            std::cout << "Enter value for " << key << " within <" << objectName << "> (" << valueType << "): ";

        while (!g_exitRequested)
        {
            // Get the input
            std::string input;
            std::cin >> input;

            // Check for accepted flags
            if (input == "-n") { break; }
            if (input == "-x") { g_exitRequested = true; break; }

            // Parse input based on the object type. 
            if (valueType == "string")
            {
                object[key] = input;
                break;
            }
            else if (valueType == "integer")
            {
                try
                {
                    int value = std::stoi(input);
                    object[key] = value;
                    break;
                }
                catch (const std::invalid_argument& e)
                {
                    std::cout << "Invalid input. Please enter an integer value." << std::endl;
                }
            }
            else if (valueType == "double")
            {
                try
                {
                    double value = std::stod(input);
                    object[key] = value;
                    break;
                }
                catch (const std::invalid_argument& e)
                {
                    std::cout << "Invalid input. Please enter a double value." << std::endl;
                }
            }
            else if (valueType == "boolean")
            {
                if (input == "true" || input == "false")
                {
                    object[key] = (input == "true");
                    break;
                }
                else
                {
                    std::cout << "Invalid input for boolean. Please enter 'true' or 'false'." << std::endl;
                }
            }
        }
    }
}

/// @brief Function declaration for UpdateJsonArray
/// @param array - json array
/// @param arrayName - name of the key for the array
static void UpdateJsonArray(nlohmann::json& array, const std::string& arrayName)
{
    for (size_t i = 0; i < array.size(); ++i) 
    {
        std::string valueType;
        if (array[i].is_string())               { valueType = "string";     }
        else if (array[i].is_number_integer())  { valueType = "integer";    }
        else if (array[i].is_number_float())    { valueType = "double";     }
        else if (array[i].is_boolean())         { valueType = "boolean";    }
        else if (array[i].is_array())           { UpdateJsonArray(array[i], arrayName);  continue;   }
        else if (array[i].is_object())          { std::string name = arrayName + "[" + std::to_string(i) + "]"; UpdateJsonObject(array[i], name); continue; }
        else if (array[i].is_null())            { continue; }
        else { continue;}

        std::cout << "Enter value for element " << i << " in <" << arrayName << "> (" << valueType << "): ";

        while (!g_exitRequested) 
        {
            std::string input;
            std::cin >> input;

            if (input == "-n") { break; }
            if (input == "-x") { g_exitRequested = true; break; }

            if (valueType == "string") 
            {
                array[i] = input;
                break;
            }
            else if (valueType == "integer") 
            {
                try 
                {
                    int value = std::stoi(input);
                    array[i] = value;
                    break;
                }
                catch (const std::invalid_argument& e) 
                {
                    std::cout << "Invalid input. Please enter an integer value." << std::endl;
                }
            }
            else if (valueType == "double") 
            {
                try 
                {
                    double value = std::stod(input);
                    array[i] = value;
                    break;
                }
                catch (const std::invalid_argument& e) 
                {
                    std::cout << "Invalid input. Please enter a double value." << std::endl;
                }
            }
            else if (valueType == "boolean") 
            {
                if (input == "true" || input == "false") 
                {
                    array[i] = (input == "true");
                    break;
                }
                else 
                {
                    std::cout << "Invalid input for boolean. Please enter 'true' or 'false'." << std::endl;
                }
            }
        }
    }
}

/// @brief main working function. Program entry point. 
/// @return 1
int main() 
{
    // Hard coded file paths for now.
    std::vector<std::string> fileList = { 
        "C:\\Users\\chipb\\Desktop\\updater_settings.json", 
        "C:\\Users\\chipb\\Desktop\\updater_settings.json", 
        "C:\\Users\\chipb\\Desktop\\updater_settings.json" 
    };

    // Display title 
    std::string title = "\n=========================\n";
    title            += "   Config File Updater   \n";
    title            += "=========================\n\n";
    std::cout << title;

    // List the available files
    std::cout << "Available files:" << '\n';
    int index = 1;
    for (const auto& file : fileList) 
    {
        std::cout << "\t[" << index++ << "] " << file << std::endl;
    }

    // Capture file selection and make sure its within bounds.
    // accepts '-x' to close the program. 
    std::string textFileIndex;
    int fileIndex = 0;
    while (!g_exitRequested)
    {
        std::cout << "Enter the number of the file you want to load: ";
        std::cin >> textFileIndex;

        if (textFileIndex == "-x") { g_exitRequested = true; break; }

        // Validate user input
        try {
            fileIndex = stoi(textFileIndex);
            if (fileIndex >= 1 && fileIndex <= fileList.size())
            {
                // Input is valid, exit loop
                break;
            }
            else
            {
                std::cout << "Invalid index. Please enter a number between 1 and " << fileList.size() << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error converting input." << '\n';
        }
        
    }

    // Notify the selection and available flags. 
    std::string selectedFile = fileList[fileIndex - 1];
    std::cout << "You selected: " << selectedFile << '\n';
    std::cout << "\nEnter '-n' to skip any item.\nEnter '-x' to exit the update and save any edits.\n\n";

    // Load JSON file
    nlohmann::json configFile = LoadJsonFile(selectedFile);

    // Call update function
    UpdateJsonObject(configFile);

    // Save updated JSON to file
    std::ofstream outputFile(selectedFile);
    outputFile << std::setw(4) << configFile << std::endl;
    std::cout << "Changes saved to " << selectedFile << std::endl;

    return 0;
}