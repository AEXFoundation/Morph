#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include "filters.h"

namespace fs = std::filesystem;

std::vector<std::string> splitCommand(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    
    for (char c : input) {
        if (c == '"') {
            inQuotes = !inQuotes;
        }
        else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}

void printHelp() {
    std::cout << "\n=== Morph - RAM-Based Image Processing Engine ===\n" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  -i @\"path\"              Load image(s) to RAM from file or folder" << std::endl;
    std::cout << "  @i                      List all images currently in RAM" << std::endl;
    std::cout << "  @i grayscale <percent>  Apply grayscale filter (in RAM)" << std::endl;
    std::cout << "  @i grayscale <percent> <filename>  Apply grayscale to specific image" << std::endl;
    std::cout << "  preview                 Save current RAM images to Morph/output" << std::endl;
    std::cout << "  preview <filename>      Save specific image to Morph/output" << std::endl;
    std::cout << "  -o @\"path\"              Export images from RAM and clear" << std::endl;
    std::cout << "  -o keep @\"path\"         Export images from RAM but keep in RAM" << std::endl;
    std::cout << "  help                    Show this help message" << std::endl;
    std::cout << "  exit                    Exit program\n" << std::endl;
}

int main() {
    Pipeline pipeline;
    bool running = true;

    std::cout << "\n=== Morph - RAM-Based Image Processing ===\n" << std::endl;
    std::cout << "Type 'help' for commands\n" << std::endl;

    while (running) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;

        auto tokens = splitCommand(input);
        if (tokens.empty()) continue;

        // Lowercase the command only
        std::string cmd = tokens[0];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "exit" || cmd == "quit") {
            // Show RAM usage before exit
            size_t ram_bytes = pipeline.getRAMUsage();
            if (ram_bytes > 0) {
                double ram_mb = ram_bytes / (1024.0 * 1024.0);
                std::cout << "\nClearing " << ram_mb << " MB from RAM..." << std::endl;
            }
            running = false;
        }
        else if (cmd == "help") {
            printHelp();
        }
        else if (cmd == "-i" && tokens.size() >= 2) {
            std::string path = tokens[1];
            if (path[0] == '@') {
                pipeline.addInput(path.substr(1));
            }
            else {
                std::cerr << "Use -i @\"path\" to load images to RAM" << std::endl;
            }
        }
        else if (cmd == "@i") {
            if (tokens.size() == 1) {
                // List images in RAM
                pipeline.listInput();
            }
            else if (tokens.size() >= 2) {
                std::string filter = tokens[1];
                std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
                
                if (filter == "grayscale") {
                    std::string percent = (tokens.size() >= 3) ? tokens[2] : "100";
                    std::string target = (tokens.size() >= 4) ? tokens[3] : "";
                    
                    // Remove % symbol if present
                    size_t pos = percent.find('%');
                    if (pos != std::string::npos) {
                        percent.erase(percent.begin() + pos);
                    }
                    
                    double val = 100.0;
                    try {
                        val = std::stod(percent);
                    }
                    catch (const std::invalid_argument& e) {
                        std::cerr << "Invalid percentage value: " << percent << std::endl;
                        continue;
                    }
                    
                    pipeline.applyGrayscale(target, val);
                }
                else {
                    std::cerr << "Unknown filter: " << filter << std::endl;
                }
            }
        }
        else if (cmd == "preview") {
            // Save preview to Morph/output folder
            std::string target = (tokens.size() >= 2) ? tokens[1] : "";
            pipeline.savePreview(target);
        }
        else if (cmd == "-o" && tokens.size() >= 2) {
            // Parse -o command with flexible argument order
            std::string path;
            bool clearRAM = true; // default
            std::string target;
            
            for (size_t i = 1; i < tokens.size(); i++) {
                std::string token = tokens[i];
                std::string token_lower = token;
                std::transform(token_lower.begin(), token_lower.end(), token_lower.begin(), ::tolower);
                
                if (token[0] == '@') {
                    path = token.substr(1);
                }
                else if (token_lower == "keep") {
                    clearRAM = false;
                }
                else if (token_lower == "clear") {
                    clearRAM = true;
                }
                else {
                    // Assume it's a target filename
                    target = token;
                }
            }
            
            if (path.empty()) {
                std::cerr << "Use -o @\"path\" [keep/clear] [filename] to export" << std::endl;
            }
            else {
                pipeline.exportOutput(path, clearRAM, target);
            }
        }
        else if (cmd == "list") {
            // Legacy command support
            pipeline.listInput();
        }
        else {
            std::cerr << "Unknown command. Type 'help' for available commands." << std::endl;
        }
    }

    std::cout << "\nGoodbye!" << std::endl;
    return 0;
}
