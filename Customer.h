#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>

// This struct represents a single customer, which also acts as a node in the Singly Linked List
struct Customer {
    // Data fields to store customer information
    int id;
    std::string name;
    std::string email;
    std::string phone;
    
    // Pointer to the next node in the list
    Customer* next; 

    // Constructor to easily create a new Customer/Node
    Customer(int id, const std::string& name, const std::string& email, const std::string& phone)
        : id(id), name(name), email(email), phone(phone), next(nullptr) {}
};

#endif // CUSTOMER_H
