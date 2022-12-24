#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int main(int argc, char* argv[]){
    DIR *dirp;
    struct dirent *dir;
    struct stat stats;
    struct passwd *pw;
    struct group *grp;

    char temp2[512];
    char time_buf[512];
    char buf[512][512]; // buffer to store name of the files and directories
    int n = 0; // number of files and directories
    if(argc <= 2){
        printf("argv[1]: %s", argv[1]);
        dirp = opendir(argv[1]); // open the 1st input directory
    } else if(argc > 2){
        dirp = opendir(argv[2]); // open the 1st input directory
    }
    // if the input path does not exist
    if(dirp == NULL) 
    {
        fprintf(stderr,"Path not found.\n");
        return 1;
    }

    while((dir = readdir(dirp)) != NULL){
            sprintf(buf[n], "%s",dir->d_name); // store the directory name into array as a string
            n++;   
        // fail to open directory
        if (errno){
            printf("error!\n");
        }    
    }

    

    
    char temp[512]; 
    //Sort array 
    for(int i=0; i<n; i++){
        for(int j=0; j<n-1-i; j++){
            if(strcasecmp(buf[j], buf[j+1]) > 0){
                //swap buf[j] and buf[j+1]
                strcpy(temp, buf[j]);
                strcpy(buf[j], buf[j+1]);
                strcpy(buf[j+1], temp);
            }
        }
    }
    
    // if input is only ls
    if(argc <= 2){
    // print array
    for(int k=0; k<n; k++){
        if(buf[k][0] != '.'){
            printf("%s\n", buf[k]);
        }    
    }
    } else{
        // if input is ls -l
        if(strcmp(argv[1], "-l") == 0){
            for(int j=0; j<n; j++){
                if(buf[j][0] != '.'){
                    // building the path
                    strcpy(temp2, argv[2]);
                    strcat(temp2, "/");
                    strcat(temp2, buf[j]);
                    stat(temp2, &stats); // get the file attributes pointed by path and store them in temp2
                    // file permissions
                    printf( (S_ISDIR(stats.st_mode)) ? "d" : "-");
                    printf( (stats.st_mode & S_IRUSR) ? "r" : "-");
                    printf( (stats.st_mode & S_IWUSR) ? "w" : "-");
                    printf( (stats.st_mode & S_IXUSR) ? "x" : "-");
                    printf( (stats.st_mode & S_IRGRP) ? "r" : "-");
                    printf( (stats.st_mode & S_IWGRP) ? "w" : "-");
                    printf( (stats.st_mode & S_IXGRP) ? "x" : "-");
                    printf( (stats.st_mode & S_IROTH) ? "r" : "-");
                    printf( (stats.st_mode & S_IWOTH) ? "w" : "-");
                    printf( (stats.st_mode & S_IXOTH) ? "x" : "-");
                    // get username/ID
                    if( (pw = getpwuid(getuid())) == NULL ) {
                        // fprintf( stderr,
                        // "getpwuid: no password entry\n" );
                        // exit(EXIT_FAILURE);
                        if(pw->pw_name){
                            printf( " %s", pw->pw_name ); // username
                        } else{
                            printf( " %d", pw->pw_uid );  // user ID
                        }
                    }                               
                    // get group name/ID
                    if( ( grp = getgrgid( getgid() ) ) == NULL ) {
                    // fprintf( stderr,
                    //     "getpwuid: no password entry\n" );
                    // exit( EXIT_FAILURE );
                        if(grp->gr_name){
                            printf( " %s", grp->gr_name ); // group name
                        } else{
                            printf( " %d", grp->gr_gid );  // group ID
                        }
                    }                      
                    // size of the file
                    printf(" %zu",stats.st_size);
                    // time of last modification
                    strftime(time_buf, 20, "%b %d %H:%M", localtime(&(stats.st_ctime)));
                    printf(" %s%s", buf[n], time_buf);
                    // filename
                    printf(" %s", buf[j]);
                    printf("\n");
                }
              
            }            
        }
    }
    closedir(dirp);
}
