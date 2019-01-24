# MapReduce
---
Parallel algorythm for indexing words from a list of files.

## Details
---
Made using Visual Studio with C/C++ and MPI.

## Implementation
---
Algorythm has a MASTER process and a few WORKERS.

The MASTER process reads the file names from folder and sends them to workers.

Indexing algorythm with two phases:

1. Map phase   
The WORKER receives the file name, opens it and parses it. It adds the words found into a Hash table.
The Hash table has 500 records and the dispersion function calculates the sum of ascii codes. It adds all the words from file into the Hash table and it saves the words with frequencyes into a temporary file. 
After that it tells to MASTER that it finished the processing.

1. Reduce phase  
The MASTER adds the processed words (from workers when finished) to it's Hash table, where would be stored all the words from parsed files.
The MASTER then can find or display all words and it's occurencies.

## Example result
---
1. Input  
Text with English words.

1. Output  
<antiquity, {16.txt:4,25.txt:3}>
<possesses, {21.txt:11,14.txt:1,5.txt:2,24.txt:5,15.txt:1,7.txt:2,25.txt:58}>
<intellectionem, {21.txt:2}>
<contrasedative, {12.txt:1}>
<faded, {17.txt:1,12.txt:5,20.txt:2,13.txt:7,18.txt:1,10.txt:2,2.txt:1,15.txt:6,19.txt:3}>
<injurious, {23.txt:1,22.txt:1,14.txt:2,16.txt:1,24.txt:3,25.txt:2}>
<supposing, {23.txt:1,22.txt:1,21.txt:5,14.txt:3,5.txt:1,24.txt:8,7.txt:8,25.txt:6,19.txt:1}>
<ourselves, {6.txt:8,23.txt:10,22.txt:10,17.txt:2,12.txt:3,21.txt:2,14.txt:29,5.txt:18,16.txt:4,20.txt:2,11.txt:6,1.txt:7,13.txt:1,18.txt:7,24.txt:29,3.txt:2,10.txt:23,2.txt:13,15.txt:11,7.txt:22,25.txt:132,19.txt:19}>
<positions, {6.txt:2,17.txt:3,11.txt:1,1.txt:2,18.txt:2,24.txt:2,3.txt:2,2.txt:4,25.txt:2,19.txt:9}>

## License
---
MIT License