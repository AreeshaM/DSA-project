#include <iostream>
#include <queue>
#include <stack>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
using namespace std;

// Order structure
struct Order {
    int id;
    vector<string> items;
    string customer;
    int totalPrepTime; // total preparation time in minutes
};


queue<Order> orderQueue;       // Orders waiting
stack<Order> cancelledOrders;  // Cancelled orders
list<string> menu;             // Menu items
map<string, list<string>> feedbacks; // Customer feedback
map<string, int> prepTimes;    // Preparation time per item

int nextOrderId = 1;
const int CANCEL_WINDOW = 5; // minutes allowed for cancellation

// Calculate waiting time (sum of all pending orders)
int calculateWaitingTime() {
    int total = 0;
    queue<Order> temp = orderQueue;
    while (!temp.empty()) {
        total += temp.front().totalPrepTime;
        temp.pop();
    }
    return total;
}

// Interactive menu selection (multiple items)
void orderFromMenu(string customer) {
    cout << "\n--- Menu ---\n";
    int idx = 1;
    vector<string> itemsList;
    for (auto &item : menu) {
        cout << idx << ". " << item << " (Prep time: " << prepTimes[item] << " mins)\n";
        itemsList.push_back(item);
        idx++;
    }

    int n;
    cout << "How many dishes do you want to order? ";
    cin >> n;

    vector<string> selectedItems;
    int totalPrep = 0;
    for (int i = 0; i < n; i++) {
        int choice;
        cout << "Select dish number " << (i+1) << ": ";
        cin >> choice;
        if (choice < 1 || choice > itemsList.size()) {
            cout << "Invalid choice.\n";
            i--; continue;
        }
        string dish = itemsList[choice-1];
        selectedItems.push_back(dish);
        totalPrep += prepTimes[dish];
    }

    int waitTime = calculateWaitingTime();
    Order o{nextOrderId++, selectedItems, customer, totalPrep};

    cout << "\nYou selected: ";
    for (auto &d : selectedItems) cout << d << " ";
    cout << "| Total prep time: " << totalPrep << " mins\n";
    cout << "👉 Your order will be ready in approximately " << (waitTime + totalPrep) << " minutes.\n";
    cout << "👉 You can cancel this order within " << CANCEL_WINDOW << " minutes.\n";

    char cancelChoice;
    cout << "Do you want to cancel this order? (y/n): ";
    cin >> cancelChoice;
    if (cancelChoice == 'y' || cancelChoice == 'Y') {
        cancelledOrders.push(o);
        cout << "\n❌ Order cancelled.\n";
        return;
    }

    // Confirm order
    orderQueue.push(o);
    cout << "\n✅ Order confirmed! ID: " << o.id << "\n";
    cout << "Your meal is being prepared...\n";

    // Countdown simulation (in seconds for demo)
    int countdown = totalPrep; // treat prepTime as seconds for demo
    cout << "Countdown until ready:\n";
    while (countdown > 0) {
        cout << countdown << " seconds remaining...\r" << flush;
        this_thread::sleep_for(chrono::seconds(1));
        countdown--;
    }
    cout << "\n🍽️ Order " << o.id << " is ready to be delivered!\n";

    // Take feedback after meal
    string comment;
    cout << "\nWe hope you enjoyed your meal! Please give your feedback: ";
    cin.ignore();
    getline(cin, comment);
    feedbacks[customer].push_back(comment);
    cout << "🙏 Thank you for your feedback!\n";
}

int main() {
    // Seed menu
    menu.push_back("Chicken Biryani"); prepTimes["Chicken Biryani"] = 20;
    menu.push_back("Beef Burger");     prepTimes["Beef Burger"] = 15;
    menu.push_back("Masala Fries");    prepTimes["Masala Fries"] = 10;
    menu.push_back("Mint Lemonade");   prepTimes["Mint Lemonade"] = 5;

    string cust;
    cout << "Customer name: ";
    cin >> cust;

    orderFromMenu(cust);

    cout << "\n--- All Feedback ---\n";
    for (auto &entry : feedbacks) {
        cout << "Customer: " << entry.first << "\n";
        for (auto &comment : entry.second) {
            cout << "  - " << comment << "\n";
        }
    }

    cout << "\nGoodbye!\n";
    return 0;
}
