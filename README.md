File Manager Emulator

File Manager Emulator (FME) emulates the details of creating, removing, copying and moving files and directories. FME shall be capable to read and execute a batch file with different kind of commands. After the batch file execution it shall generate and print out formatted directory structure or an error message if something went wrong to standard output. Note that program should do nothing with the real file structure on local hard drives and shall only emulate these activities. Your goal is to write such File Manager Emulator.

FME Commands                   

    md – creates a directory.

Command format: md <path>

Notes: md should not create any intermediate directories in the path.

Examples:
a)      md /Test – creates a directory called Test in the root directory.
b)      md /Dir1/Dir2/NewDir – creates a subdirectory “NewDir” if directory “/Dir1/Dir2” exists.

    mf – creates a file.

Command format: mf <path>

Notes: if such file already exists with the given path then FME should continue to the next command in the batch file without any error rising.

Examples:
mf /Dir2/Dir3/file.txt – creates a file named file.txt in “/Dir2/Dir3” subdirectory.

    rm – removes a file or a directory with all its contents.

Command format: rm <path>

Examples:
a)      rm /Dir2/Dir3 – removes the directory “/Dir2/Dir3”.
b)      rm /Dir2/Dir3/file.txt – removes the file “file.txt” from the directory “/Dir2/Dir3”.

    cp – copy an existed directory/file to another location.

Command format: cp <source> <destination>

Notes: Program should copy directory with all its content. Destination path should not contain any file name except base-name otherwise FME should raise error (Base-name of “/dir/file.txt” is “file.txt”).

Examples:
a)      cp /Dir2/Dir3 /Dir1 – copies directory Dir3 in /Dir2 to /Dir1.
b)      cp /Dir2/Dir3/file.txt /Dir1 – copies file “file.txt” from /Dir2/Dir3 to /Dir1.
c)      cp /Dir2/Dir3/file.txt /Dir1/newfile.txt – copies file “file.txt” from /Dir2/Dir3 to /Dir1 and renames it to “newfile.txt”.

    mv – moves an existing directory/file to another location

Command format: mv <source> <destination>

Notes: Program should move directory with all its content.

Additional Implementation Notes
1.      You should use std C++ (program will be compiled and tested in Linux OS).
2.      Initially file system contains only the root directory marked as “/”.
3.      Commands, file and directory names are case sensitive.
4.      Any action shall not change the current directory.
5.      In case of any error occurs, the program shall stop and output a descriptive error message beginning with “ERROR: ”.
6.      In case of no errors the program should print out directory tree in any readable form (file/directory names should be organized in alphabetical ascending order). For example, as follows:

 

/

| _Dir1

|   |_Dir2

|   |  |_Dir3

|   |  |_readme.txt

|   |_EDir4

|   |  |_temp.dat
