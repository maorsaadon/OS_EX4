Operating Systems - Assignment 4
By Maor Saadon and Matan Adar 
ID 318532421 & ID 209321552

------- Introduction -------

Welcome to the Reactor project! The Reactor is a lightweight and efficient event-driven network programming framework implemented in C. 
It is designed to handle multiple client connections concurrently using the reactor pattern. 
This project provides a basic implementation of a reactor that allows you to build scalable and responsive network applications.
The reactor pattern is a well-known design pattern for building event-driven systems. 
It involves a central event loop that efficiently manages and dispatches events from various sources, such as network sockets. 
The Reactor project embraces this pattern and provides a simple yet powerful framework for building network applications with high performance and concurrency.
Key features of the Reactor project include:
1. Concurrent Connection Handling: 
    The Reactor allows you to accept and manage multiple client connections simultaneously. It efficiently handles incoming data and events from multiple clients without blocking or slowing down the server.
2. Event-driven Architecture: 
    The Reactor follows an event-driven architecture, where events are dispatched to appropriate handlers based on their types. This enables efficient utilization of system resources and ensures a highly responsive and scalable application.
3. Graceful Shutdown: 
    The Reactor provides mechanisms for gracefully shutting down the server. It ensures that all client connections are properly closed, resources are released, and the server exits cleanly.

The Reactor project strives for simplicity, efficiency, and ease of use. It provides a flexible framework that you can extend and customize according to your specific application requirements.

---- Reactor ----

Reactor is a simple event-driven network programming framework in C. 
It allows you to handle multiple client connections concurrently using the reactor pattern. 
This project provides a basic implementation of a reactor that can accept client connections, receive and process data from clients, and handle server shutdown gracefully.

------- Requirements -------

* Linux machine (Ubuntu 22.04 LTS recommended)
* GNU C Compiler (GCC)
* Make (For Makefile)

------- Building -------
In order to run this project follow this steps:  
  1. Download zip from our reposetory : push on code -> Download zip. 
     Or Cloning the reposetory to local machine: git clone https://github.com/maorsaadon/OS_Ex4.git
  2. Extract all on yor computer.   
  3. Open the file in the workspace - Ubuntu 20.04 LTS.  

------- Running -------
   1. Export shared libraries: export LD_LIBRARY_PATH="."  
   2. Execute "make all"
   3. Run the server: ./reactor-server
   4. The server is now running and listening for incoming client connections on port 5000.
   5. To connect to the server, use a TCP client (e.g., Telnet) and connect to localhost on port 5000.
   6. You can send messages from the client, and the server will receive and display them.
   7. To stop the server, press Ctrl+C or send a termination signal to the server process.

