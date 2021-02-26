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
	
	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
        char *image_file = NULL;
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
	
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("cannot opendir, error occured");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/
	Image *givenImgFile = read_image(image_file);
	struct dirent *dp;
    CompRecord CRec;
	strcpy(CRec.filename, "");
	CRec.distance = FLT_MAX;
	int haveSeen = -1; // Indicator that indicates if we have seen any files yet or not 


	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			perror("problem with stat struct");
			exit(1);
		} 

		if(S_ISDIR(sbuf.st_mode)) {
			CompRecord temporary;
			temporary = process_dir(path, givenImgFile, -1);
			if (haveSeen == -1)
			{
				strcpy(CRec.filename, temporary.filename);
                CRec.distance = temporary.distance;
				haveSeen = 1;
			}

			if ((haveSeen != -1) && temporary.distance < CRec.distance)
			{
				CRec.distance = temporary.distance;
				strcpy(CRec.filename, temporary.filename); 
			}
		}

		
	}
	//Here we just deallocate
    //The memory that we had allocated before
    //And we do this in order to
    //Prevent memory leak and 
    //Out of memo error
	free(givenImgFile->p);
    free(givenImgFile);

        printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);
	
	return 0;
}
