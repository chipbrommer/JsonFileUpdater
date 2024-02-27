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

// Function to load JSON file
nlohmann::json LoadJsonFile(const std::string& filename) 
{
    std::ifstream file(filename);
    nlohmann::json jsonData;
    file >> jsonData;
    return jsonData;
}

// Function to validate boolean input
bool ValidateBooleanInput(const std::string& input) 
{
    std::string lowercaseInput;
    for (char c : input) 
    {
        lowercaseInput += tolower(c);
    }
    return lowercaseInput == "true" || lowercaseInput == "false";
}

int main() 
{
    bool exitRequsted = false;

    std::vector<std::string> fileList = { 
        "C:\\Users\\chipb\\Desktop\\updater_settings.json", 
        "C:\\Users\\chipb\\Desktop\\updater_settings.json", 
        "C:\\Users\\chipb\\Desktop\\updater_settings.json" 
    };

    std::cout << "Available files:" << '\n';
    int index = 1;
    for (const auto& file : fileList) 
    {
        std::cout << "\t[" << index++ << "] " << file << std::endl;
    }

    std::string textFileIndex;
    int fileIndex;
    while (!exitRequsted)
    {
        std::cout << "Enter the number of the file you want to load: ";
        std::cin >> textFileIndex;

        if (textFileIndex == "-x") { exitRequsted = true; break; }

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

    std::string selectedFile = fileList[fileIndex - 1];
    std::cout << "You selected: " << selectedFile << '\n';

    // Load JSON file
    nlohmann::json configFile = LoadJsonFile(selectedFile);

    // Prompt user for input for each key
    for (auto it = configFile.begin(); it != configFile.end(); ++it) 
    {
        std::string key = it.key();
        std::string valueType;

        // Determine the type of the value
        if (it.value().is_string())                 { valueType = "string";     }
        else if (it.value().is_number_integer())    { valueType = "integer";    }
        else if (it.value().is_number_float())      { valueType = "double";     }
        else if (it.value().is_boolean())           { valueType = "boolean";    }
        else if (it.value().is_array())             { valueType = "array";      }
        else if (it.value().is_object())            { valueType = "object";     }
        else if (it.value().is_null())              { valueType = "null";       }
        else                                        { valueType = "unknown";    }

        std::cout << "Enter value for " << key << " (" << valueType << "): ";

        //while (true)
        //{
        //    if (valueType == "string") 
        //    {
        //        std::string value;
        //        std::cin >> value;
        //        configFile[key] = value;
        //        break;
        //    }
        //    else if (valueType == "integer") 
        //    {
        //        int value;
        //        std::cin >> value;
        //        configFile[key] = value;
        //        break;
        //    }
        //    else if (valueType == "double") 
        //    {
        //        double value;
        //        std::cin >> value;
        //        configFile[key] = value;
        //        break;
        //    }
        //    else if (valueType == "boolean") 
        //    {
        //        std::string value;
        //        std::cin >> value;

        //        if (ValidateBooleanInput(value)) 
        //        {
        //            configFile[key] = (value == "true");
        //            break;
        //        }
        //        else 
        //        {
        //            std::cout << "Invalid input for boolean. Please enter 'true' or 'false'." << '\n';
        //        }
        //    }
        //    
        //}
    
        // Repeat until valid input is provided
        while (!exitRequsted) {
            std::string input;
            std::cin >> input;

            // Check for flags
            if (input == "-n") { break; }
            if (input == "-x") { exitRequsted = true; break; }

            // Check input
            if (valueType == "string") 
            {
                configFile[key] = input;
                break;
            }
            else if (valueType == "integer") 
            {
                int value;
                try 
                {
                    value = stoi(input);
                    configFile[key] = value;
                    break;
                }
                catch (const std::invalid_argument& e) 
                {
                    std::cout << "Invalid input. Please enter an integer value." << '\n';
                }
            }
            else if (valueType == "double") 
            {
                double value;
                try 
                {
                    value = stod(input);
                    configFile[key] = value;
                    break;
                }
                catch (const std::invalid_argument& e) 
                {
                    std::cout << "Invalid input. Please enter a double value." << '\n';
                }
            }
            else if (valueType == "boolean") 
            {
                if (ValidateBooleanInput(input)) 
                {
                    configFile[key] = (input == "true");
                    break; // Exit loop if input is valid
                }
                else 
                {
                    std::cout << "Invalid input for boolean. Please enter 'true' or 'false'." << '\n';
                    std::cout << "Enter value for " << key << " (" << valueType << ") or enter \"-n\" to skip: ";
                }
            }
            else if (valueType == "array" || valueType == "object" || valueType == "null") {
                std::cout << "Invalid value type for this input. Please enter a different value or enter \"-n\" to skip." << '\n';
                std::cout << "Enter value for " << key << " (" << valueType << ") or enter \"-n\" to skip: ";
            }
        }
    
        // Break out of for loop
        if (exitRequsted) { break; }
    }

    // Save updated JSON to file
    std::ofstream outputFile(selectedFile);
    outputFile << std::setw(4) << configFile << std::endl;
    std::cout << "Changes saved to " << selectedFile << std::endl;

    return 0;
}
