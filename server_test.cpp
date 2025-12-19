#include <iostream>
#include <queue>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm> 
#include "json.hpp"  
#include "httplib.h" 

using json = nlohmann::json;
using namespace std;

// --- RESTAURANT DATA STRUCTURES & GLOBALS ---

// Order structure
struct Order {
    int id;
    vector<string> items;
    string customer;
    int totalPrepTime; // total preparation time in minutes
};

// Global Data Structures
queue<Order> orderQueue;        // Orders waiting (Queue: FIFO) 
map<string, list<string>> feedbacks; // Customer feedback (Map: Customer Name -> List of Comments) 
list<string> menu;              // Menu items (List)
map<string, int> prepTimes;     // Preparation time per item (Map)

int nextOrderId = 1;
const int CANCEL_WINDOW = 5; // minutes allowed for cancellation

// Calculate waiting time (sum of all pending orders' prep times)
int calculateWaitingTime() {
    int total = 0;
    queue<Order> temp = orderQueue; 
    while (!temp.empty()) {
        total += temp.front().totalPrepTime;
        temp.pop();
    }
    return total;
}

// Function to initialize the Menu
void initializeMenu() {
    menu.push_back("Chicken Biryani"); prepTimes["Chicken Biryani"] = 20;
    menu.push_back("Beef Burger");     prepTimes["Beef Burger"] = 15;
    menu.push_back("Masala Fries");    prepTimes["Masala Fries"] = 10;
    menu.push_back("Mint Lemonade");   prepTimes["Mint Lemonade"] = 5;
}

// Function to convert Menu data to JSON for the Webpage
json getMenuAsJson() {
    json menuJson;
    int id = 1;
    for (const auto& item : menu) {
        menuJson.push_back({
            {"id", id++},
            {"name", item},
            {"prepTime", prepTimes[item]}
        });
    }
    return menuJson;
}

// --- END OF RESTAURANT DATA STRUCTURES ---


int main() {
    initializeMenu(); 

    httplib::Server svr;
    
    // Set up the path to serve static files (HTML, CSS, JS)
    const char* frontend_path = "./frontend";
    
    if (!svr.set_base_dir(frontend_path)) {
        std::cerr << "FATAL CRASH: Failed to set base directory to '" << frontend_path << "'. Ensure the 'frontend' folder is in the same directory as server_test.exe" << std::endl;
        return 1;
    }

    // --- 1. GET ROUTE: /menu ---
    svr.Get("/menu", [](const httplib::Request& req, httplib::Response& res) {
        json menuData = getMenuAsJson();
        // Correct set_content: two arguments (content string, MIME type string)
        res.set_content(menuData.dump(), "application/json"); 
    });

    // --- 2. POST ROUTE: /order ---
    svr.Post("/order", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            string customer = data["customer"].get<string>();
            vector<int> dish_ids = data["dish_ids"].get<vector<int>>();
            
            vector<string> selectedItems;
            int totalPrep = 0;
            
            int id_counter = 1;
            for (const auto& item : menu) {
                if (find(dish_ids.begin(), dish_ids.end(), id_counter) != dish_ids.end()) {
                    selectedItems.push_back(item);
                    totalPrep += prepTimes[item];
                }
                id_counter++;
            }

            if (selectedItems.empty()) {
                res.status = 400; // Bad Request
                // FIX APPLIED: Added the "application/json" argument
                res.set_content(R"({"error": "No valid items selected."})", "application/json"); 
                return;
            }

            int currentWait = calculateWaitingTime();
            int totalWaitTime = currentWait + totalPrep;

            Order o{nextOrderId++, selectedItems, customer, totalPrep};
            orderQueue.push(o); 

            json response = {
                {"id", o.id},
                {"customer", customer},
                {"items", selectedItems},
                {"prepTime", totalPrep},
                {"queueTime", currentWait},
                {"totalWaitTime", totalWaitTime},
                {"cancelWindow", CANCEL_WINDOW}
            };
            
            // FIX APPLIED: Added the "application/json" argument
            res.set_content(response.dump(), "application/json"); 

        } catch (const exception& e) {
            res.status = 400; // Bad Request
            res.set_content("{\"error\": \"Invalid JSON or data format.\"}", "application/json"); 
            std::cerr << "Error processing POST /order: " << e.what() << std::endl;
        }
    });

    // --- 3. POST ROUTE: /feedback ---
    svr.Post("/feedback", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            string customer = data["customer"].get<string>();
            string comment = data["comment"].get<string>();

            feedbacks[customer].push_back(comment); 

            // Correct set_content: two arguments (content string, MIME type string)
            res.set_content("{\"status\": \"Feedback received.\"}", "application/json");
            std::cout << "POST /feedback fulfilled for customer: " << customer << std::endl;

        } catch (const exception& e) {
            res.status = 400; // Bad Request
            res.set_content("{\"error\": \"Invalid feedback data.\"}", "application/json");
            std::cerr << "Error processing POST /feedback: " << e.what() << std::endl;
        }
    });


    std::cout << "\nStarting server on http://localhost:8081... " << std::endl;

    if (!svr.listen("localhost", 8081)) { // <-- PORT CHANGED TO 8081
        std::cerr << "CRITICAL ERROR: Could not start server on port 8081 or crashed immediately. Check port availability." << std::endl;
        return 1;
    }
    return 0;
}