# C-shell
Kieron Hushon (sc16kh) Operating Systems project 2017
A simple shell in C

To start the shell type 'make' into the terminal to compile the shell.
Then type './sc16kh' to start the shell.

The shell can run programs with the command 'ex'
For example: 'ex atom' will run atom.

The shell can run programs in the background with the command 'exb'
For example: 'exb sleep 5' will run the sleep function in the background for 5 seconds.

The shell supports multi-piping.
For example: 'ex ls | ex head -3 | ex tail -2'

The shell can create a file and redirect output to it.
For example: 'ex ls > files.txt' will create a file called 'files.txt' which lists the files in the directory.

The colour of the output of the shell can be changed with simple commands such as...
'colour green'
'colour red'
'colour blue'
 etc.
