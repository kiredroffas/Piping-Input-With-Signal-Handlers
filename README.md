# Piping-Input-With-Signal-Handlers
* This C program, targeted at the Linux platform, spawns a child process with a pipe shared between the parent and child processes. The parent reads from stdin, then writes to the pipe, which the child then reads from. There are no required command line arguments.
* To run this program you can either compile warn.c manually, or use the 'make all' and 'make run' commands from the Makefile.
* Upon execution of the program it will wait for a ctrl+c to be entered, upon entering ctrl+c you can set the optional delay and string and then press enter, at which point it will print your entered string every delay seconds (delay defaults to 5 seconds if no delay is entered). 
* If you wish to change the delay interval/string, simply ctrl+c and enter new arguments. 
* If you wish to quit the program, ctrl+c, and then enter 'exit', which will exit both the child and parent processes. 
* If at any point the program is not exiting, you can ctrl+z to stop the program, and type 'ps' to see current processes, type 'kill -9 __' where __ is the process id of the process your trying to kill, there should be a parent and child process you have to kill.
* User must enter a string or delay/string when they ctrl+c, if just a delay integer is entered output does not work properly.
* A DEBUG flag is included to see the appropraite variables being set within the program, and can be set by setting DEBUG to 1 within warn.c (initially 0).
* Error catching is implemented with the approriate errno messages on the pipe(),fork(),and kill() processes. Other implemented error catching includes: when user enters a negative delay interval (turns to positive) and when user enters 0 as the delay interval (defaults to 5).
## Example
![Alt text](/screenshot/sc1.png?raw=true "sc1")
## Process Pseudocode
* Parent Process: 
    * Intercept SIGINT, when a SIGINT is detected by parent promt user for single line text message.
        * Send signal to stop child process printing while reading from user
    * Once message recieved from user, parent will send (write) the message to the child via the shared pipe, and send a SIGFPE signal to the child to get its attention.
        * Send signal to resume child process printing (after new string has been sent to child)
    * Repeat this behavior for any SIGINT recieved, until an 'exit' message is recieved from user.
		* Upon reading 'exit', parent waits for child process to terminate, and then parent terminates.
* Child Process:  
    * Ignore SIGINT.
    * When a SIGFPE is detected, read a single line message (512 bytes or less) from the shared pipe.
        * The first token of the message may optionally be a integer representing delay time in seconds.
        * If first token is integer, parse integer to be new delay, parse string without the integers.
        * If first token is not an integer, parse entire original read string, set delay to a default of 5 seconds. 
    * Once read message is parsed, print the message every delay seconds to stdout.
        * Repeat printing message until the next SIGFPE is intercepted and a new message is read from the parent (printing will be paused, then resumed with new input).
