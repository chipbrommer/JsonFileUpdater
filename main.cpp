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
/// @param outputObject - json object to place parsed content into
/// @return - true on good load and parse, else false. 
static bool LoadJsonFile(const std::string& filename, nlohmann::json& outputObject)
{
    std::ifstream file(filename);
    if (!file.is_open()) { std::cout << "Failed to open file: " + filename; return false; }

    nlohmann::json jsonData;
    try 
    { 
        file >> jsonData;
    }
    catch (const std::exception& e) 
    {
        std::cout << "Failed to parse JSON from file: " << filename << '\n';
        std::cout << "Error: " << e.what() << '\n';
        return false;
    }

    outputObject = jsonData;
    return true;
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

/// @brief Validate a string is numeric input only
/// @param integer - bool to indicate if we are checking an integer or a float
/// @param input - string input to be checked
/// @return - true if value, else false
static bool ValidateNumericInput(const bool integer, const std::string& input)
{
    bool validInput = true;

    for (char c : input) 
    {
        if (integer && !std::isdigit(c) && c != '-')
        {
            validInput = false;
            break;
        }
        else if (!std::isdigit(c) && c != '-' && c != '.') 
        {
            validInput = false;
            break;
        }
    }

    return validInput;
}

/// @brief Function to update JSON object
/// @param object - json object
/// @param objectName - optional - name of the object key
static void UpdateJsonObject(nlohmann::json& object, const std::string& objectName = "")
{
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        // Return if exit requested - At top for recursive catch
        if (g_exitRequested) { return; }

        // Get the key name and determine the type of the value
        std::string key = it.key();
        std::string valueType;

        if (it.value().is_string())                 { valueType = "string"; }
        else if (it.value().is_number_integer())    { valueType = "integer"; }
        else if (it.value().is_number_float())      { valueType = "double"; }
        else if (it.value().is_boolean())           { valueType = "boolean"; }
        else if (it.value().is_array())             { UpdateJsonArray(object[key], key);    continue; }
        else if (it.value().is_object())            { UpdateJsonObject(object[key], key);   continue; }
        else if (it.value().is_null())              { continue; }
        else                                        { continue; }

        // Display info to user for item they're updating
        if (objectName == "")
        {
            std::cout << "Item: " << key << " - Current Value (" << valueType << "): " << it.value() << "\n";
            std::cout << "Enter new value: ";
        }
        else
        {
            std::cout << "Object: " << objectName << " - Item: " << key << " - Current Value (" << valueType << "): " << it.value() << "\n";
            std::cout << "Enter new value: ";
        }

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
                if (ValidateNumericInput(true, input))
                {
                    int value = std::stoi(input);
                    object[key] = value;
                    break;
                }
                else
                {
                    std::cout << "Invalid input for integer: " << input << '\n';
                    std::cout << "Enter value for " << key << " (" << valueType << ") or enter \"-n\" to skip: ";
                }
            }
            else if (valueType == "double")
            {
                if (ValidateNumericInput(false,input))
                {
                    double value = std::stod(input);
                    object[key] = value;
                    break;
                }
                else
                {
                    std::cout << "Invalid input for double: " << input << '\n';
                    std::cout << "Enter value for " << key << " (" << valueType << ") or enter \"-n\" to skip: ";
                }
            }
            else if (valueType == "boolean")
            {
                if (ValidateBooleanInput(input))
                {
                    object[key] = (input == "true");
                    break;
                }
                else
                {
                    std::cout << "Invalid input for boolean. Please enter 'true' or 'false'." << '\n';
                    std::cout << "Enter value for " << key << " (" << valueType << ") or enter \"-n\" to skip: ";
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
        if (g_exitRequested) { return; }
        std::string name = arrayName + "[" + std::to_string(i) + "]";
        UpdateJsonObject(array[i], name);
    }
}

/// @brief main working function. Program entry point. 
/// @return 1
int main(int argc, char* argv[])
{
    std::string selectedFile;

    // Display title 
    std::string title = "\n=========================\n";
    title            += "   Config File Updater   \n";
    title            += "=========================\n\n";
    std::cout << title;

    // Only attempt to process the default filepath files if we didnt receive
    // a passed in filepath as an argument
    if (argc <= 1)
    {
        // Vector to hold the list of files
        std::vector<std::string> fileList = {
            "C:\\Users\\chipb\\Desktop\\updater_settings.json",
            "C:\\Users\\chipb\\Desktop\\updater_settings.json",
            "C:\\Users\\chipb\\Desktop\\updater_settings.json"
        };

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
            try 
            {
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
            catch (const std::exception& e)
            {
                std::cerr << "Error converting input: " << e.what() << '\n';
            }

        }

        // Notify the selection and available flags. 
        selectedFile = fileList[fileIndex - 1];
        std::cout << "You selected: " << selectedFile << '\
    }
    else
    {
        selectedFile = argv[1];
    }
    
    std::cout << "\nEnter '-n' to skip any item.\nEnter '-x' to exit the update and save any edits.\n\n";

    try
    {
        // Load JSON file
        nlohmann::json configFile;
        if (LoadJsonFile(selectedFile, configFile) == false) { return -1; }

        // Call update function
        UpdateJsonObject(configFile);

        // Save updated JSON to file
        std::ofstream outputFile(selectedFile);

        if (!outputFile.is_open()) 
        {
            throw std::runtime_error("Failed to open file for saving changes: " + selectedFile);
        }

        outputFile << std::setw(4) << configFile << std::endl;
        std::cout << "\nChanges saved to " << selectedFile << '\n';
        outputFile.close();

        std::string printInput;
        std::cout << "\nEnter 'y' to print the file: ";
        std::cin >> printInput;

        if (printInput == "y")
            std::cout << '\n' << std::setw(4) << configFile << std::endl;
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
