/////////////////////////////////////////////////////////////////////////////////
// @file            json_updater.cpp
// @brief           json file updater for console only systems. 
// @author          Chip Brommer
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <filesystem>
#include <vector>
#include "nlohmann/json.hpp"
#include "cxxopts.hpp"

constexpr int MAJOR_VERSION = 0;
constexpr int MINOR_VERSION = 0;
constexpr int BUILD_VERSION = 2;

static bool g_exitRequested = false;
static bool g_saveChanges = true;
static bool g_quitProgram = false;

struct Options {
    std::string filePath;
    std::string dirPath;
    bool showHelp = false;
    bool showVersion = false;
};

/**
 * @brief Trims leading and trailing whitespace from a string.
 * @param str The input string to trim.
 * @return The trimmed string, or empty string if all whitespace.
 */
static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

/**
 * @brief Converts a string to lowercase.
 * @param str The input string to convert.
 * @return The lowercase string.
 */
static std::string to_lower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

/**
 * @brief Normalizes a file path to Unix-style and converts to absolute path.
 * @param path The input path to normalize.
 * @return The normalized absolute path with forward slashes.
 */
static std::string normalizePath(const std::string& path) {
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return std::filesystem::absolute(normalized).string();
}

/**
 * @brief Handles interrupt signals (e.g., Ctrl+C) to exit gracefully.
 * @param signum The signal number received.
 */
static void signalHandler(int signum) {
    std::cerr << "\nInterrupt signal received. Exiting without saving changes.\n";
    g_exitRequested = true;
    g_saveChanges = false;
}

/**
 * @brief Loads a JSON file into a json object.
 * @param filename The path to the JSON file.
 * @param outputObject The json object to store the parsed data.
 * @return True if the file was loaded and parsed successfully, false otherwise.
 */
static bool LoadJsonFile(const std::string& filename, nlohmann::json& outputObject) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file '" << filename << "': " << strerror(errno) << '\n';
        return false;
    }
    try {
        file >> outputObject;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON from file '" << filename << "': " << e.what() << '\n';
        return false;
    }
    return true;
}

/**
 * @brief Detects JSON files in the specified directory.
 * @param dirPath The directory path to scan.
 * @return A vector of absolute paths to JSON files found.
 */
static std::vector<std::string> DetectJsonFiles(const std::string& dirPath) {
    std::vector<std::string> jsonFiles;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                jsonFiles.push_back(normalizePath(entry.path().string()));
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory '" << dirPath << "': " << e.what() << '\n';
    }
    return jsonFiles;
}

/**
 * @brief Prompts the user to select a JSON file from a list.
 * @param jsonFiles The list of JSON file paths to display.
 * @return The selected file path, or empty string if none selected or user quits.
 */
static std::string SelectJsonFile(const std::vector<std::string>& jsonFiles) {
    if (jsonFiles.empty()) {
        std::cerr << "No JSON files found in the directory.\n";
        return "";
    }

    std::cout << "Available JSON files:\n";
    for (size_t i = 0; i < jsonFiles.size(); ++i) {
        std::cout << "  [" << i + 1 << "] " << jsonFiles[i] << '\n';
    }
    std::cout << "Enter the number of the file to edit, '-x' to skip, or '-q' to quit: ";

    while (!g_exitRequested && !g_quitProgram) {
        std::string input;
        std::getline(std::cin, input);
        std::string trimmed = trim(input);

        if (trimmed == "-x") {
            g_exitRequested = true;
            return "";
        }
        if (trimmed == "-q") {
            g_quitProgram = true;
            return "";
        }

        try {
            size_t index = std::stoul(trimmed);
            if (index >= 1 && index <= jsonFiles.size()) {
                return jsonFiles[index - 1];
            }
            else {
                std::cout << "Invalid index. Enter a number between 1 and " << jsonFiles.size() << ", '-x' to skip, or '-q' to quit: ";
            }
        }
        catch (const std::exception& e) {
            std::cout << "Invalid input. Enter a number between 1 and " << jsonFiles.size() << ", '-x' to skip, or '-q' to quit: ";
        }
    }
    return "";
}

/**
 * @brief Parses command-line options using cxxopts.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return An Options struct containing parsed arguments.
 */
static Options parseOptions(int argc, char* argv[]) {
    cxxopts::Options options("json_updater", "JSON File Updater");
    options.add_options()
        ("f,file", "JSON file to update", cxxopts::value<std::string>())
        ("d,directory", "Directory to scan for JSON files", cxxopts::value<std::string>())
        ("h,help", "Display help")
        ("v,version", "Display version");
    auto result = options.parse(argc, argv);
    Options opts;
    if (result.count("help")) opts.showHelp = true;
    if (result.count("version")) opts.showVersion = true;
    if (result.count("file")) opts.filePath = result["file"].as<std::string>();
    if (result.count("directory")) opts.dirPath = result["directory"].as<std::string>();
    else if (!result.unmatched().empty()) opts.dirPath = result.unmatched()[0];
    return opts;
}

/**
 * @brief Updates a JSON array by processing each element.
 * @param array The JSON array to update.
 * @param arrayName The name of the array for display purposes.
 */
static void UpdateJsonArray(nlohmann::json& array, const std::string& arrayName);

/**
 * @brief Updates a JSON object by prompting the user for new values.
 * @param object The JSON object to update.
 * @param objectName The name of the object for display purposes (optional).
 */
static void UpdateJsonObject(nlohmann::json& object, const std::string& objectName = "") {
    for (auto it = object.begin(); it != object.end() && !g_exitRequested; ++it) {
        std::string key = it.key();
        std::string valueType;
        switch (it.value().type()) {
        case nlohmann::json::value_t::string: valueType = "string"; break;
        case nlohmann::json::value_t::number_integer:
        case nlohmann::json::value_t::number_unsigned: valueType = "integer"; break;
        case nlohmann::json::value_t::number_float: valueType = "double"; break;
        case nlohmann::json::value_t::boolean: valueType = "boolean"; break;
        case nlohmann::json::value_t::array: UpdateJsonArray(object[key], key); continue;
        case nlohmann::json::value_t::object: UpdateJsonObject(object[key], key); continue;
        default: continue;
        }
        std::cout << (objectName.empty() ? "Item: " : "Object: " + objectName + " - Item: ")
            << key << " - Current Value (" << valueType << "): " << it.value() << "\n"
            << "Enter new value: ";
        while (!g_exitRequested) {
            std::string input;
            std::getline(std::cin, input);
            std::string trimmed = trim(input);
            if (trimmed == "-n") break;
            if (trimmed == "-s") { g_saveChanges = true; g_exitRequested = true; break; }
            if (trimmed == "-x") { g_saveChanges = false; g_exitRequested = true; break; }
            if (valueType == "string") {
                object[key] = input;
                break;
            }
            else if (valueType == "integer") {
                try {
                    size_t pos;
                    long long value = std::stoll(trimmed, &pos); // Use long long to handle large numbers
                    if (pos == trimmed.size()) {
                        if (value >= 0) {
                            object[key] = static_cast<unsigned long long>(value); // Store as unsigned if positive
                        }
                        else {
                            object[key] = value; // Store as signed if negative
                        }
                        break;
                    }
                }
                catch (...) {}
                std::cout << "Invalid input for integer. Try again: ";
            }
            else if (valueType == "double") {
                try {
                    size_t pos;
                    double value = std::stod(trimmed, &pos);
                    if (pos == trimmed.size()) {
                        object[key] = value;
                        break;
                    }
                }
                catch (...) {}
                std::cout << "Invalid input for double. Try again: ";
            }
            else if (valueType == "boolean") {
                std::string lower = to_lower(trimmed);
                if (lower == "true") {
                    object[key] = true;
                    break;
                }
                else if (lower == "false") {
                    object[key] = false;
                    break;
                }
                std::cout << "Invalid input for boolean. Enter 'true' or 'false': ";
            }
        }
    }
}

/**
 * @brief Updates a JSON array by processing each element.
 * @param array The JSON array to update.
 * @param arrayName The name of the array for display purposes.
 */
static void UpdateJsonArray(nlohmann::json& array, const std::string& arrayName) {
    for (size_t i = 0; i < array.size() && !g_exitRequested; ++i) {
        std::string name = arrayName + "[" + std::to_string(i) + "]";
        UpdateJsonObject(array[i], name);
    }
}

/**
 * @brief Processes a single JSON file, including loading, editing, saving, and printing.
 * @param selectedFile The path to the JSON file to process.
 * @return True if processing was successful, false if file loading failed.
 */
static bool ProcessSingleFile(const std::string& selectedFile) {
    nlohmann::json configFile;
    if (!LoadJsonFile(selectedFile, configFile)) {
        g_exitRequested = false; // Reset for next iteration
        return false;
    }
    std::cout << "\nUpdating file: " << selectedFile << '\n';
    std::cout << "For each item, enter the new value or:\n"
        << "  '-n' to skip\n"
        << "  '-s' to save and exit file\n"
        << "  '-x' to exit file without saving\n\n";
    g_exitRequested = false; // Reset before editing
    g_saveChanges = true; // Reset before editing
    UpdateJsonObject(configFile);
    if (!g_exitRequested || (g_exitRequested && g_saveChanges)) {
        std::ofstream outputFile(selectedFile);
        if (!outputFile.is_open()) {
            std::cerr << "Failed to open file for writing: " << selectedFile << '\n';
            g_exitRequested = false; // Reset for next iteration
            return false;
        }
        outputFile << std::setw(4) << configFile << '\n';
        outputFile.close();
        std::cout << "\nChanges saved to " << selectedFile << '\n';
    }
    else {
        std::cout << "\nExiting file without saving changes.\n";
    }
    std::cout << "\nEnter 'y' to print the file: ";
    std::string printInput;
    std::getline(std::cin, printInput);
    if (trim(printInput) == "y") {
        std::cout << '\n' << std::setw(4) << configFile << '\n';
    }
    g_exitRequested = false; // Reset for next file
    g_saveChanges = true; // Reset for next file
    std::cout << "\nReturning to file selection...\n\n";
    return true;
}

/**
 * @brief Handles file selection and processing loop based on command-line options.
 * @param opts The parsed command-line options.
 * @return True if processing completed successfully, false if an error occurred.
 */
static bool HandleFileSelectionAndProcessing(const Options& opts) {
    while (!g_quitProgram) {
        std::string selectedFile;
        if (!opts.dirPath.empty()) {
            // Scan specified directory for JSON files
            std::string dirPath = normalizePath(opts.dirPath);
            std::cout << "Detecting JSON files in directory: " << dirPath << " ..." << std::endl;
            std::vector<std::string> jsonFiles = DetectJsonFiles(dirPath);
            selectedFile = SelectJsonFile(jsonFiles);
            if (selectedFile.empty()) {
                if (g_quitProgram) break; // User chose to quit
                if (!g_exitRequested) {
                    std::cerr << "No file selected.\n";
                }
                g_exitRequested = false; // Reset for next iteration
                continue;
            }
        }
        else if (!opts.filePath.empty()) {
            selectedFile = normalizePath(opts.filePath);
        }
        else {
            // Default to scanning current directory
            std::cout << "Detecting JSON files in current directory." << std::endl;
            std::vector<std::string> jsonFiles = DetectJsonFiles(".");
            selectedFile = SelectJsonFile(jsonFiles);
            if (selectedFile.empty()) {
                if (g_quitProgram) break; // User chose to quit
                if (!g_exitRequested) {
                    std::cerr << "No file selected.\n";
                }
                g_exitRequested = false; // Reset for next iteration
                continue;
            }
        }

        ProcessSingleFile(selectedFile);
    }
    return true;
}

/**
 * @brief Main entry point for the JSON File Updater program.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    std::cout << "\n=========================\n"
        << "    JSON File Updater   \n"
        << "=========================\n" << std::endl;
    signal(SIGINT, signalHandler);
    try {
        Options opts = parseOptions(argc, argv);
        if (opts.showHelp) {
            std::cout << "JSON File Updater v" << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_VERSION << "\n\n"
                << "Usage: json_updater [options] [directory]\n\n"
                << "Options:\n"
                << "  -f, --file FILE      Specify the JSON file to update\n"
                << "  -d, --directory DIR  Scan directory for JSON files (default: current directory)\n"
                << "  -h, --help           Display this help message\n"
                << "  -v, --version        Display the version\n\n"
                << "When selecting a file:\n"
                << "  Enter a number to edit a file, '-x' to skip, or '-q' to quit the program.\n";
            return 0;
        }
        if (opts.showVersion) {
            std::cout << "JSON File Updater v" << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_VERSION << '\n';
            return 0;
        }

        if (!HandleFileSelectionAndProcessing(opts)) {
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}