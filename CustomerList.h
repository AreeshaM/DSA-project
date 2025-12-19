#ifndef CUSTOMERLIST_H
#define CUSTOMERLIST_H

#include "Customer.h"
#include <iostream>

class CustomerList {
private:
    Customer* head;
    int next_id;

public:
    // Constructor to initialize the list
    CustomerList() : head(nullptr), next_id(1) {}

    // Destructor (important to free memory)
    ~CustomerList() {
        Customer* current = head;
        Customer* next_node = nullptr;
        while (current != nullptr) {
            next_node = current->next;
            delete current; // Free memory of the node
            current = next_node;
        }
        head = nullptr;
    }

    // Method to add a new customer (node) to the beginning of the list
    Customer* addCustomer(const std::string& name, const std::string& email, const std::string& phone) {
        // 1. Create a new Customer node
        Customer* newCustomer = new Customer(next_id++, name, email, phone);

        // 2. Add the new node to the front (head) of the list
        newCustomer->next = head;
        head = newCustomer;

        std::cout << "DEBUG: Added Customer ID: " << newCustomer->id << ", Name: " << newCustomer->name << std::endl;
        
        return newCustomer;
    }

    // A simple method to display the list (for debugging)
    void displayList() const {
        Customer* current = head;
        std::cout << "\n--- Current Customer List ---" << std::endl;
        while (current != nullptr) {
            std::cout << "ID: " << current->id 
                      << ", Name: " << current->name 
                      << ", Email: " << current->email << std::endl;
            current = current->next;
        }
        std::cout << "---------------------------" << std::endl;
    }
};

#endif // CUSTOMERLIST_H