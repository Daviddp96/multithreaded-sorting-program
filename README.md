MP4: Multithreaded Sorting Program
CS 241, Spring 2012
Due: Mar 2, Friday 11:59 PM
PURPOSE
More and more programs today are being programmed as multithread applications. This MP is to help you understand how a multithreaded program works, what the advantage of multithreading is in comparison to a single-threaded program, and the pitfalls that occur while designing a program to work in a parallel manner. You will become familiar with various pthread library functions during the MP(e.g., for thread creation/join, synchronization among multiple threads, etc.).

You are tasked with writing an application which will sort its input in a parallel fashion. Initially, you assume that the entire data set has been given to you in the form of separate independent files. You will proceed sorting these files by spawning a worker thread to sort each file. After the worker threads complete their execution, you will merge the files into one, by taking two files at a time. In the end you will have one large file which contains all the input data sorted. This method significantly improves the execution time, as the the threads tend to work in parallel.

BACKGROUND
The pthread (POSIX thread) libraries are a standard based thread API for C/C++. It allows one to spawn a new concurrent process flow. It is most effective on multi-processor or multi-core systems where the process flow can be scheduled to run on another processor thus gaining speed through parallel or distributed processing. Threads require less overhead than "forking" or spawning a new process because the system does not initialize a new system virtual memory space and environment for the process.

TASK
[Part 1]
The main thread of the program reads the input file names from the command line. Upon reading the file names, the main thread creates one worker thread for each file and passes the file name or the handle of the file to the worker thread. You may assume that all file names are valid and distinct. You should not make any assumptions on the total number of files.

[Part 2]
The file names referred to in [Part 1] are files containing a list of random numbers. Each line will contain a single integer, the integer will fit within a standard C signed `int`, and the total number of lines will fit within a standard C `unsigned int`.

While the main thread continues to spawn worker threads, the worker threads in the meanwhile read the data from the named file, sort the data in increasing numeric order (e.g., 1 is before 6, and 6 is before 14, etc), then writes the sorted sequence of numbers out to another file. The file should be named .sorted (i.e. file1.txt.sorted or file2.txt.sorted). Note that it does not modify the original file, but stores the result in a new file.

As with the input file, the sorted file is a text file with one number per line. You are required to use the C standard function qsort() for sorting. If you are not familiar with qsort(), please use "man qsort" for details. Having finished writing the sorted file, the worker thread terminates. Before termination, each file prints the number of integers it has sorted and the name of the file it writes to in the following format:

This worker thread writes XXXXX lines to "YYYYY".
This can be accomplished by the following printf() command:

printf("This worker thread writes %d lines to \"%s\".\n", ...);
[Part 3]
The multiple sorted files created in [Part 2] are now to be merged into a single sorted file. You should take two consecutively listed files (depending on the order in which the file names were entered on the command line) and merge them maintaining the sorted order of integers. This should be done by a new thread. Do this merge step iteratively until you have a single file left. When merging the two files, if a number appears in both files, it should appear in the result file only once. You should delete all the intermediate files that you create during this procedure. (The system file tmpfile() will be helpful here; it returns a file pointer to a temporary created file with "w+" access permissions and ensures that the file is deleted at the end of the program. Please refer to man pages for more details.) This single file should contain all the numbers in sorted order from different files and must be named "sorted.txt". If you choose not to use tmpfile(), all files except the orignal input files, the sorted versions of all the orignal input files, and the final sorted.txt file must be deleted.

At each particular level of the merging tree (shown in the figure below), remember to wait for all the threads of the previous level to return before spawning new ones. The merge thread should display the total lines in each of the two files being merged together and the total number of lines after the merge.

Merged XXXXX lines and YYYYY lines into ZZZZZ lines.
...which can be accomplished with the following printf() command:

printf("Merged %d lines and %d lines into %d lines.\n", ...);

[Example Run]
Let us consider an example demonstrating this problem

[Part 1] User enters the following file names at the command line:
./mp4 a1.txt a2.txt a3.txt a4.txt a5.txt a6.txt a7.txt
[Part 2] Sort the numbers contained in each of the files to form the following new files, without altering the original files:
THIS IS A LISTING, NOT OUTPUT.
"a1.txt.sorted" is a sorted copy of "a1.txt"
"a2.txt.sorted" is a sorted copy of "a2.txt"
"a3.txt.sorted" is a sorted copy of "a3.txt"
"a4.txt.sorted" is a sorted copy of "a4.txt"
"a5.txt.sorted" is a sorted copy of "a5.txt"
"a6.txt.sorted" is a sorted copy of "a6.txt"
"a7.txt.sorted" is a sorted copy of "a7.txt"
And each thread prints the total number of integers encountered as they terminate:
This worker thread writes XXXXX lines to "YYYYY".
Therefore your output will be similar to
This worker thread writes 10 lines to "a7.txt.sorted".
This worker thread writes 20 lines to "a3.txt.sorted".
This worker thread writes 30 lines to "a2.txt.sorted".
This worker thread writes 40 lines to "a4.txt.sorted".
This worker thread writes 100000 lines to "a5.txt.sorted".
This worker thread writes 2000000 lines to "a6.txt.sorted".
This worker thread writes 30000000 lines to "a1.txt.sorted".
It's important to note that worker threads may exit in a different order than the order they were created. This is because they're running as threads, in parallel. Therefore, small files will sort very quickly while multi-million line files may take a few seconds to sort.
[Part 3] Start merging the files, while still maintaining the sorted order:


Each merge thread will display the name of the files merged and the name of the new file created with the total number of lines in it. Therefore, your output will look similar to

Merged 100 lines and 1000 lines into 1050 lines.
Merged 10000 lines and 300 lines into 10300 lines.
Merged 10 lines and 800 lines into 801 lines.
Merged 1050 lines and 10300 lines into 10345 lines.
Merged 801 lines and 1 lines into 802 lines.
Merged 10345 lines and 802 lines into 11111 lines.
At the end of the execution of the program, your directory must only contain the following NEWLY created files:

a1.txt.sorted, a2.txt.sorted, ..., a7.txt.sorted, and sorted.txt
As well as the unmodified orignal files:
a1.txt, a2.txt, ..., a7.txt
NOTES
Start with the provided file, mp4.c, and do not rename the file. You may add any functions you'd like to mp4.c, but please do not create any other code files for this MP.

You may find these commands useful in this MP: ...for file I/O: fopen(), fscanf(), fprintf(), tmpfile(), fclose() ...for reading in int`s: scanf(), atoi() ...for threads: pthread_create(), pthread_join()

You are required to use qsort() to sort the input files. Note that merging two sorted files does not require a call to qsort(). (This is the second warning!)

The input files will all be in the format generated by our provided program `gen`. To run our provided `gen` program:

%> make gen
%> ./gen 1250000 >file1.txt
notice that `gen` takes in a single argument: the number of unsorted numbers you would like printed to stdout. You can save those numbers into a file by redirecting your output to a file (as shown above).
You can assume all input files will be generated with `gen`. As such, the file will display a number as text (ASCII) followed by a newline.

Each worker thread should print out its string right before it terminates. Since threads are being ran in parallel, the shorter files will tend to rise to the top while longer tasks will be the last ones to finish.

COMPILING AND RUNNING MP4
To compile, run the following commands from a command prompt on a Linux machine:

%> make clean
%> make
To run the executable,

%> ./mp4 [ ...]
