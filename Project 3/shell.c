#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>
#include<signal.h> 
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h> 
#include<ctype.h>
#include<errno.h>
#define DEBUG 1

//Structures
struct process_def {
   int  process_id; 
   char job_status[30];
   char command[100];
   char job_id; 
   int back_ground; 
} processes[100];
 

//Functions declaration
int coreShell();
int isDir(const char*);
int find(char*,char*);
int removeSpace(char*); 

//Global variables
static int process_counter;
int parent_pid; 
static int current_process_pid;
struct process_def *last_process;


void sigIntHandler(int sig_num)
{
    if(process_counter>0)
    {
        if(last_process->back_ground != 1)
        {
            kill(last_process->process_id, SIGINT);
            strcpy(last_process->job_status, "Complete"); 
            printf("[%d] %d Terminated by signal 2\n", last_process->job_id, last_process->process_id); 
        }
        fflush(stdout);
    }
    
}

void sigStopHandler(int sig_num)
{
    if(process_counter>0)
    {
        int olderrno = errno;
        if(last_process->back_ground != 1)
        {
            kill(last_process->process_id, SIGSTOP);
            strcpy(last_process->job_status, "Stopped");  
        }
        errno = olderrno;  
    }     
}


static int count=0;

int main(int argc, char *argv[])
{
    signal(SIGINT, sigIntHandler);
    signal(SIGTSTP, sigStopHandler);

    parent_pid = getpid();
    coreShell();
    return 0;
}

int coreShell()
{
    char cmd [200] = "";
    char cmd_path[50] = "";
    int file_found = 0;
    char input[256]; 
    printf("> ");

    while (fgets(input, sizeof (input), stdin) != NULL)
    {

        // divide the string into smaller strings based on delim = " "
        const char delim[2] = ";";
        const char command_delim[2] = " ";
        char *token;
        char *arg_list[1000];
        char directory[500];
        //char *cmd_directory;
        char *command;
        char basic_command[200];
        char temp[100];
        int has_value = 0;
        int status;
        int word_counter;
        int background = 0;


       /* get the first token */
        token = strtok(input, delim);                   
        /* walk through other tokens */
        while( token != NULL && !(strlen(token) == 1  && token[0] == '\n') ) 
        {
            removeSpace(token); 
            has_value++;
            // gets rid of the empty space between lines
            input[strlen(input)]=0;
            // gets rid of leading spaces
            input[strcspn(input, "\n")] = '\0'; 

            //this logic should separate the directory from the file name
            //add string parser for the input token into directory and command - probably should be a separate function
            if(token[0]=='/'){
                strcpy(directory, token);
                printf("directory1: %s\n", directory);
                command = strrchr(directory, '/')+1;
                directory[strlen(directory)-strlen(command)]= '\0';
                printf("directory2: %s\n", directory);
                printf("command: %s\n", command);
            }                
                command = token;
                int counter = 0;
                int blank_found_after_first_cmd =0;
                for(int w=0; command[w]!='\0'; w++)
                {   

                    if(command[w]!=' ' && blank_found_after_first_cmd != 2)
                    {   
                        if(blank_found_after_first_cmd == 0)
                            blank_found_after_first_cmd++;
                        basic_command[counter++] =  command[w];   
                    }
                    else if (blank_found_after_first_cmd == 1)
                        blank_found_after_first_cmd++; 
                    basic_command[counter] = '\0';                   
                }
                

            printf("basic command: %s\n", basic_command);
            // Clear arg_list
            for( int c = 0; c<=word_counter; c++)
                arg_list[c]= '\0';
            word_counter = 0;
            int len = strlen(command);
            memcpy(temp, command, len);
            temp[len] = '\0';
            char *token2 = strtok(temp, command_delim); // tokinize string based on " "
            if(token2 != NULL)
            {
                arg_list[word_counter] = token2; // add first token (path) to words array
                word_counter++;
            }   
            while( token2 != NULL) // valid path so get the remaining tokens
            {       
                token2 = strtok(NULL, command_delim);  // remaining tokens (command line options)
                if(token2 != NULL)
                {
                    arg_list[word_counter] = token2; // add remaining tokens to arg_list array
                    word_counter++;
                }
            }
            
            if (has_value == 1)
            {
                if (find("/usr/bin/", basic_command)) 
                {    
                    strcpy(cmd_path, "/usr/bin/");           
                    file_found = 1;
                }

                else if (find("/bin/", basic_command))
                {
                    strcpy(cmd_path, "/bin/");
                    file_found = 1;
                }

                else if (directory == NULL && find("./", basic_command))
                {
                    strcpy(cmd_path, "./");
                    file_found = 1;
                }

                else if (directory != NULL && find(directory, basic_command))
                {
                    strcpy(cmd_path, directory);
                    file_found = 1;
                }
                else if(basic_command[0]== '/')
                {
                    strcpy(cmd_path, "");
                    file_found = 1;
                }
                else
                {
                    strcpy(directory, "./");
                }

            }             
       
            else file_found = 1;
                if (strcmp(token, "exit") == 0) 
                {
                    return 0;
                } 
                else if (strstr(token, "cd") ==  token)
                {   
                    strcpy(directory, &token[3]);
                    removeSpace(directory);
                    if(chdir(directory)<0)
                    {
                        printf("Invalid path.");
                    }
                }
                else if (strstr(token, "bg") == token)
                { 

                    int index=0;
                    if(word_counter==1){
                        printf("Enter a job id.");
                    }
                    else if(*arg_list[1]== '%')
                    {
                        //we should add logic to confirm the first character is actually '%'                 
                        index = atoi(&arg_list[1][1])-1;   
                        current_process_pid = processes[index].process_id;   //change process_counter in this line to the command line argument

                        printf("Current process id we are resuming: %d\n", current_process_pid);
                        if(kill(current_process_pid,SIGCONT) < 0) 
                            printf("fg failed\n");                   
                        strcpy(processes[index].job_status, "Running\0");
                        last_process = &processes[index];
                        last_process->back_ground = 1;
                    }
                }
                else if (strstr(token, "fg") == token)
                {  
                    int index=0;
                    if(word_counter==1){
                        printf("Enter a job id.");
                    }
                    else if(*arg_list[1]== '%')
                    {
                        index = atoi(&arg_list[1][1])-1;   
                        current_process_pid = processes[index].process_id;   //change process_counter in this line to the command line argument

                        printf("Current process id we are resuming: %d\n", current_process_pid);
                        //kill(-(current_process_pid), SIGCONT);
                        if(kill(current_process_pid,SIGCONT) < 0) 
                            printf("fg failed\n");
                        int stat;
                        last_process = &processes[index];
                        last_process->back_ground = 0;
                        waitpid(current_process_pid, &stat, WUNTRACED);
                        strcpy(processes[index].job_status, "Running\0");
                        
                    }
                    
                }
                else if (strstr(token, "jobs") == token)
                {   
                    printf("JobID     PID        Status         CMD    \n");
                    for(int i=0;i<process_counter;i++)
                    {
                        status = kill(processes[i].process_id, 0);
                        if(status == -1)  //job completed - mark complete - not currently resetting any job counters
                                strcpy(processes[i].job_status, "Complete\0");
                        if (strcmp(processes[i].job_status, "Complete"))
                            printf("[%d]     %d      %s       %s   \n",processes[i].job_id, processes[i].process_id, processes[i].job_status, processes[i].command );
                    }
                    
                }
                else if (strstr(token, "kill") == token )
                {   
                    int index=0;
                    if(word_counter==1){
                        printf("Enter a job id.");
                    }
                    else if(*arg_list[1]== '%')
                    {
                        index = atoi(&arg_list[1][1])-1;  
                        current_process_pid = processes[index].process_id;   //change process_counter in this line to the command line argument
                        kill(current_process_pid, SIGKILL);
                        strcpy(processes[index].job_status, "Complete\0");
                        printf("[%d] %d Terminated by signal 15\n", processes[index].job_id, processes[index].process_id);
                    }
                }
            
            else if (strcmp(directory, "./") && file_found == 0)
            {
                printf( "%s: No such file or directory ", token );
            }
                
            else if (file_found == 0)
                printf( "%s: Command not found ", token );
            else if (strcmp(token, "shell") != 0)
            {
                    strcpy(cmd, cmd_path);
                    strcat(cmd, basic_command);    
                    if ((strcmp(arg_list[word_counter-1], "&")==0) || strcmp((((arg_list[word_counter-1])+strlen(arg_list[word_counter-1])-1)), "&")==0)
                       {
                            background = 1;
                            if(strcmp((((arg_list[word_counter-1])+strlen(arg_list[word_counter-1])-1)), "&")==0)
                            {
                            int x = strlen(arg_list[word_counter-1]);
                            *(arg_list[word_counter-1]+ x - 1) = '\0';
                            } 
                       }
                    sigset_t mask, prev;
                    sigemptyset(&mask);
                    sigaddset(&mask, SIGCHLD); // store previous mask
                    sigprocmask(SIG_BLOCK, &mask, &prev); // blocking child terminate (SIGCHLD) signals
                    pid_t pid=fork();
                    if (pid==0) 
                    {                                                   
                        execve(cmd, arg_list, NULL);
                        exit(0); 
                    }
                    else 
                    { 
                        setpgid(pid, pid); 
                        process_counter++;
                        processes[process_counter].process_id = pid;
                        strcpy(processes[process_counter].command, cmd_path);
                        strcat(processes[process_counter].command, command);
                        strcpy(processes[process_counter].job_status, "Running");
                        current_process_pid = processes[process_counter].process_id;
                        processes[process_counter].job_id = process_counter+1;
                        last_process = &processes[process_counter];
                        int stat;
                        sigprocmask(SIG_SETMASK, &prev, NULL);  
                        if (background != 1) 
                        {
                            waitpid(pid,&stat,WUNTRACED);
                        }
                        else 
                        {
                            background = 0;
                        }
                            
                        
                    }    
            }
            else 
                printf("Already in shell");    
            token = strtok(NULL, delim);  
          
        }
        if (has_value == 0)
            printf("> ");
        else printf("\n>");
        has_value = 0;
    }
    return 0;
}


int removeSpace(char *string)
{
    int  a,b;
 
	for(a=0;string[a]==' '||string[a]=='\t';a++);
		
	for(b=0;string[a];a++)
	{
		string[b++]=string[a];
	}
	string[b]='\0';
	for(a=0;string[a]!='\0';a++)
	{
		if(string[a]!=' '&& string[a]!='\t')
				b=a;
	}
	string[b+1]='\0';
    return 0;

}

int find(char *basedir,char *pattern)
{
        DIR *dir;
        int file_found = 0;
        struct dirent *str1;
        dir = opendir(basedir);
        if(dir != NULL)
        {
        while((str1 = readdir(dir)) !=NULL)
        {
            if(strcmp(str1->d_name, ".") == 0 || strcmp(str1->d_name, "..")==0)
            {
                continue;
            }
            char dirpath[300] = "";
            strcat(dirpath, basedir);
            strcat(dirpath, "\\");
            strcat(dirpath, str1->d_name);
            if(isDir(dirpath))
            {
                    if(strcmp(str1->d_name, pattern)==0)                   
                    {
                        file_found++;
                        count++;
                    }   
            }
        else
        {
                if(strcmp(str1->d_name, pattern)==0)
                {
                    file_found++;
                    count++;
                }
        }
            }
            closedir(dir);
        }
            else
            {              
                if(DEBUG)
                {
                    //perror("opendir()");
                }
            }
        return file_found;
        }
        int isDir(const char* file_path)
        {
            struct stat str2;
            stat(file_path, &str2);
            return S_ISDIR(str2.st_mode);
        }
        
