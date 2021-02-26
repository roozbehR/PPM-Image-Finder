#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"

int main(int argc, char **argv) {

    //We need the following for processing purposes
    //As a result we have declared these variables
    //And pointers
	char ch;
	char path[PATHLENGTH]; //This is used to save the path of the file as we iterate through the directory
	char *startdir = "."; //Initializing our CRec data structure before any itration
    char *image_file = NULL; //Points to the master directory which we will use
	char howManyPath[PATHLENGTH];
    int ourCounter; //Used asa  counter when doing our iterations
    struct dirent *dirPointerCounter;  //Same purpose as the line above
	ourCounter = 0;
	


	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

        if (optind != argc-1) {
	     fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
        } else
             image_file = argv[optind];

    //We need drip
    //To be able to
    //Work with the direcory (master Directory)
    //And being able
    //To iterate through the directory
    //And look at, process, the desired (Image) files
	
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("cannot opendir, error occured");
		exit(1);
	} 
	
	DIR *counterDirectory = NULL;
	counterDirectory = opendir(startdir);
	if (counterDirectory == NULL)
	{
		perror("cannot opendir, error happened");
		exit(1);
	}

	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/
		

	while((dirPointerCounter = readdir(counterDirectory)) != NULL) {

		if(strcmp(dirPointerCounter->d_name, ".") == 0 || 
		   strcmp(dirPointerCounter->d_name, "..") == 0 ||
		   strcmp(dirPointerCounter->d_name, ".svn") == 0){
			continue;
		}
		strncpy(howManyPath, startdir, PATHLENGTH);
		strncat(howManyPath, "/", PATHLENGTH - strlen(howManyPath) - 1);
		strncat(howManyPath, dirPointerCounter->d_name, PATHLENGTH - strlen(howManyPath) - 1);

		struct stat howManysbuf;
		if(stat(howManyPath, &howManysbuf) == -1) {
			perror("error with our stat struct");
			exit(1);
		} 

		//Checks if it is a directory or not
        //If it is it calls increments the counter 
        //Otherwise it doesn't do anything
		if(S_ISDIR(howManysbuf.st_mode)) {
                       ourCounter = ourCounter + 1;
		}
		
	}
	int k; //Used as a vraibale to iterate when we go through the loop
	k = 0; //Initialized to zero
	struct dirent *pointerToDir;
    CompRecord CRec;
    Image *ourImage = NULL; 
	ourImage = read_image(image_file);
	CRec.distance = FLT_MAX;
    strcpy(CRec.filename, "");
    int fd[2]; //The array that is used to write and read from the pipe and communication between the processes
    int fds[ourCounter];
	CompRecord ourTemporarystruct; //This is used as a dummy CRec to save the closest image at each point
    int dummyReturn = 0; //Initialized to zero
    
while ((pointerToDir = readdir(dirp)) != NULL)
    {

        if (strcmp(pointerToDir->d_name, ".") == 0 ||
            strcmp(pointerToDir->d_name, "..") == 0 ||
            strcmp(pointerToDir->d_name, ".svn") == 0)
        {
            continue;
        }
        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path) - 1);
        strncat(path, pointerToDir->d_name, PATHLENGTH - strlen(path) - 1);

        struct stat sbuf;
		int result_of_stat = stat(path, &sbuf);
        if (result_of_stat == -1)
        {
            perror("stat");

            exit(1);
            
        }
        //Checks if it is a directory or not
        //If it is it calls the fcn 
        //Otherwise it ignores it
        if (S_ISDIR(sbuf.st_mode))
        {
            if (pipe(fd) == -1)
            { 
                perror("pipe");

                exit(1);

            }
            fds[k] = fd[0];

            k = k + 1;

            int resultOfFork;

			resultOfFork = fork();
			
            if (resultOfFork < 0)
            { 
                perror("fork");

                exit(1);

            }
            else if (resultOfFork > 0)

            {

                close(fd[1]);

            }


            else
            { 

                close(fd[0]);

                process_dir(path, ourImage, fd[1]);

                free(ourImage->p);

                free(ourImage);

                close(fd[1]);

                return dummyReturn;

            }
        }
    }
    k = k - 1;
    for (int l = k; l != -1; l--)
    {

        while (read(fds[l], &ourTemporarystruct, sizeof(ourTemporarystruct)) > 0)
        {
            
            if (CRec.distance > ourTemporarystruct.distance)
            {
                
                CRec = ourTemporarystruct;
            
            }
        }
        close(fds[k]);
    }
    //Here we just deallocate
    //The memory that we had allocated before
    //And we do this in order to
    //Prevent memory leak and 
    //Out of memo error
    free(ourImage->p);
    free(ourImage);

    printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);


    return dummyReturn;
}