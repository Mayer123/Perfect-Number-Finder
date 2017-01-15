# Perfect-Number-Finder


Manage's job is to coordinate the computations done by the "compute" processes. It is the first process started, and takes just one argument, the tcp port number to be used for communication. Manage starts up, does any initialization needed and waits for connections. 

Compute's job is to compute perfect numbers. One or more computes can be started on any machine on the network. It takes three command line arguments, a hostname for manage, a port number for manage and the number of threads it should use for computation. Compute connects to the manage process, and sends it a startup request, including the number of threads it will be using. Manage replies with one search range for each thread. Manage only gives out ranges that have not been tested. "Compute" sends manage any perfect number it finds, as it finds it (not at the end of a range). When one of its threads finishes its range, compute requests a new one from manage and gets that same thread to start on the new range. 

Report's job is to query "manage" and print out the perfect numbers found (including which host found each), and for each compute running the hostname it is running on, how many numbers it has tested, and what the current ranges it is working on are. Report's first two arguments are the same as "compute" and identify the node and port of "manage". If invoked with the "-k" switch as a third arguments, it also is used to inform the Manage process to shut down the computation. 

Note: Each compute process will have a sending thread, a receiving thread and a signal thread besides the user-defined number of computing threads. Manage times each compute's last range and adjust the length of the new range for a target of 15 seconds.
