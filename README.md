# 🧵 Multithreaded DNS Resolver

A multithreaded DNS resolver written in **C** using the **POSIX pthread library**.  
Developed as part of an **Operating Systems** course to demonstrate **concurrent programming**, **thread synchronization**, and the **producer–consumer pattern**.  
Implemented and tested in a **VMware virtual machine** running **Linux**.

---

## 🧩 Overview

The program creates multiple **requester** and **resolver** threads to perform DNS lookups concurrently:

- **Requester threads**  
  - Read domain names from one or more input files  
  - Place each domain name into a **shared array**  
  - Log every line they read to a requester log file  

- **Resolver threads**  
  - Remove domain names from the shared array  
  - Perform DNS lookups using functions in `util.c`  
  - Log results to the resolver log file as either  
    ```
    <hostname>, <IP address>
    ```  
    or  
    ```
    <hostname>, NOT_RESOLVED
    ```

---

## ⚙️ Implementation

| File | Description |
|------|--------------|
| `util.c`, `util.h` | DNS lookup utility functions (provided by instructor) |
| `array.c`, `array.h` | Thread-safe shared array implementation (**written by me**) |
| `multi-lookup.c`, `multi-lookup.h` | Multithreading logic, synchronization, and program entry point (**written by me**) |

The program uses:
- **POSIX threads (`pthread`)** for concurrency  
- **Mutex locks** and **condition variables** to coordinate thread access  
- **Logging** to separate requester and resolver output files  

---

## 🧮 Compilation

```bash
gcc -o multi-lookup multi-lookup.c array.c util.c -pthread
```

---

## 🚀 Usage

```bash
./multi-lookup <# requesters> <# resolvers> <requester logfile> <resolver logfile> [ <data file> ... ]
```

**Example:**
```bash
./multi-lookup 5 5 serviced.txt resolved.txt input/names*.txt
```

This example runs:
- **5 requester threads**
- **5 resolver threads**
- Requester output → `serviced.txt`
- Resolver output → `resolved.txt`
- Input files → all files matching `input/names*.txt`

---

## 📋 Constraints

- Maximum **10 requester threads**  
- Maximum **10 resolver threads**  
- Maximum **100 input files**

---

## 🧠 Concepts Demonstrated

- Multithreading and synchronization with **pthreads**  
- **Producer–consumer** problem implementation  
- **Mutexes** and **condition variables**  
- **Concurrent I/O** and safe shared memory access  
- **DNS resolution** and basic **network programming**

---

## 🖥️ Development Environment

- **Language:** C (C99)  
- **Compiler:** GCC  
- **Operating System:** Linux  
- **Environment:** VMware Virtual Machine  

---

## 📜 Example Log Output

**Requester Log (`serviced.txt`):**
```
www.example.com
www.google.com
invalid.domain
```

**Resolver Log (`resolved.txt`):**
```
www.example.com, 93.184.216.34
www.google.com, 142.250.72.36
invalid.domain, NOT_RESOLVED
```

---

## 🧾 License

This project was developed for educational purposes as part of an Operating Systems course.  
All code in `array.c`, `array.h`, `multi-lookup.c`, and `multi-lookup.h` is my own work.

---
