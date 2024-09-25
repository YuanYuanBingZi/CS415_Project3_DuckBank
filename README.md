# 🦆 Duck Bank - Multithreaded Banking System

> **A scalable and thread-safe online banking system built using `pthread` for handling high volumes of client requests.**

### Overview

Duck Bank is a multithreaded banking system designed to handle hundreds of thousands of requests concurrently, ensuring the correctness and security of client information. This project demonstrates proficiency in **concurrency**, **thread synchronization**, and **file I/O** using **POSIX threads**. It also showcases solutions to complex problems such as **race conditions**, **deadlocks**, and efficient **inter-process communication**.

🚀 **Key Features:**
- Supports **high-volume** banking transactions: transfer, deposit, withdrawal, and balance checks.
- Implements **thread-safe** operations using `pthread_mutex` and `pthread_cond`.
- Periodic **reward updates** for accounts using transaction tracking.
- Uses **shared memory (mmap)** to communicate between two banking processes.
- Designed for **scalability** and **real-time responsiveness**.

---

## 💻 Technologies Used
- **C Programming Language**
- **POSIX Threads (pthreads)** for multithreading
- **Memory Management** with Valgrind
- **File I/O** for transaction processing
- **Inter-Process Communication** using `mmap`
- **Linux Environment** for development and testing

---

## 📂 Project Structure

```bash
├── part1/                  # Single-threaded solution
├── part2/                  # Multi-threaded solution with synchronization
├── part3/                  # Coordinated thread execution using barriers & conditions
├── part4/                  # Inter-process communication (Duck Bank and Puddles Bank)
├── Makefile                # Build system
├── input.txt               # Sample input file with accounts and transactions
└── output/                 # Generated account balance files
```

---

## ⚙️ How to Run

1. **Clone the Repository:**
    ```bash
    git clone https://github.com/YuanYuanBingZi/Duck-Bank.git
    cd Duck-Bank
    ```

2. **Compile the Project:**
    ```bash
    make
    ```

3. **Run the Banking System:**
    ```bash
    ./bank input.txt
    ```

4. **Check the Output:**
    - `output.txt` will contain the final balance of all accounts.
    - Individual account files will be generated in the `output` directory, showing account balances after each update.

---

## 🛠️ Project Features

### 1. **Single-Threaded Implementation (Part 1)**
   - The foundation of the project. Implements a basic banking system that processes transactions one at a time.
   - Ensures **correctness** by guaranteeing that the final state of each account is accurate regardless of transaction order.

### 2. **Multithreaded with Critical Section Protection (Part 2)**
   - Transforms the single-threaded solution into a **thread-safe** multithreaded solution using `pthread_mutex` to manage access to shared resources.
   - Implements **10 worker threads** to divide the load of transaction requests, improving efficiency.
   
### 3. **Coordinated Thread Execution (Part 3)**
   - Worker threads pause after processing a set number of transactions and notify the **bank thread** to update account balances based on custom reward rates.
   - Uses `pthread_cond_wait` and `pthread_cond_broadcast` for thread communication, ensuring real-time updates.

### 4. **Inter-Process Communication (Part 4)**
   - Expands the project to handle accounts in two banks (Duck Bank and Puddles Bank) using **shared memory (`mmap`)** for communication between processes.
   - Each customer has a savings account at Puddles Bank, and balances are synchronized between the two banks.

---

## 🏆 Key Accomplishments

- **Concurrency & Synchronization:** Successfully implemented multithreaded transaction handling, ensuring data integrity even with simultaneous access to shared resources.
- **Thread Communication:** Leveraged condition variables to coordinate between threads and processes, ensuring real-time balance updates.
- **Memory Management:** Ensured memory efficiency by utilizing `mmap` for inter-process communication and avoiding memory leaks, validated using **Valgrind**.
- **Scalability:** Designed to handle large-scale transaction volumes efficiently with scalable solutions for both single-process and multi-process environments.

---

## 🔧 Challenges & Solutions

### 1. **Race Conditions and Deadlocks**
   - **Challenge:** Managing simultaneous access to shared data by multiple threads without causing race conditions or deadlocks.
   - **Solution:** Used `pthread_mutex` to protect critical sections and condition variables (`pthread_cond_wait` and `pthread_cond_broadcast`) to synchronize threads effectively.

### 2. **Efficient Inter-Process Communication**
   - **Challenge:** Keeping account information synchronized between Duck Bank and Puddles Bank across separate processes.
   - **Solution:** Implemented **shared memory (mmap)** to ensure both processes share account details and reward updates in real-time.

---

## 📊 Performance Metrics
- **Transaction Throughput:** Capable of processing **100,000+ transactions** with 10 worker threads under high-load scenarios.
- **Latency:** Balance updates for all accounts completed in **less than 1 second** after crossing the transaction threshold.
- **Memory Efficiency:** No memory leaks or unfreed resources as verified by **Valgrind**.

---

## 📝 Future Improvements

- **Add Error Handling:** Implement error handling for invalid transactions (e.g., incorrect passwords).
- **Improve Scalability:** Explore distributed systems for handling millions of transactions across multiple nodes.
- **UI/UX Enhancements:** Build a simple web interface to visualize account balances and transactions in real-time.

---
