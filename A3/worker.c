#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"


/*
 * Read an image from a file and create a corresponding 
 * image struct 
 */

Image* read_image(char *filename)
{
        FILE *image_file = NULL;
        char header[10];
        int arr_length;
        Pixel *tempo_pointer = NULL;
        image_file = fopen(filename, "r");
        //Checking the retun value of fopen() to see if it opened successfully or otherwise
        if ( image_file == NULL)
        {

                fprintf(stderr, "Couldn't open the file, an error occured HAHHAHAH\n");
                exit(1);
        }
        //Checking if file starts with a magic number
        fscanf(image_file, "%9s", header);
        if (strcmp(header, "P3") != 0)
        {
                return NULL;
        }
        //End of the error checking
        Image *img = (Image *)malloc(sizeof(Image));
        fscanf(image_file, "%d %d", &(img->width), &(img->height));
        fscanf(image_file, "%d", &(img->max_value));
        arr_length = (img->width)*(img->height);
        tempo_pointer = (Pixel *)malloc(sizeof(Pixel)*arr_length); //Do array of structs end with character delimeter ? No i guess
        int red, green, blue;
        int i = 0; //Counter to access array
        while(arr_length > i)
        {       
                fscanf(image_file, "%d %d %d", &red, &green, &blue);
                tempo_pointer[i].red = red;
                tempo_pointer[i].green = green;
                tempo_pointer[i].blue = blue;
                i++;
        }
        img->p = tempo_pointer;
        int result = fclose(image_file);
        if (result  != 0)
        {
                fprintf(stderr, "Couldn't close the file, an error occured\n");
                exit(1);
        }
        return img;
}

/*
 * Print an image based on the provided Image struct 
 */

void print_image(Image *img){
        printf("P3\n");
        printf("%d %d\n", img->width, img->height);
        printf("%d\n", img->max_value);
       
        for(int i=0; i<img->width*img->height; i++)
           printf("%d %d %d  ", img->p[i].red, img->p[i].green, img->p[i].blue);
        printf("\n");
}

/*
 * Compute the Euclidian distance between two pixels 
 */
float eucl_distance (Pixel p1, Pixel p2) {
        return sqrt( pow(p1.red - p2.red,2 ) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
 * Compute the average Euclidian distance between the pixels 
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) 
{       
        // printf("Here\n");
        Image *img2 = read_image(filename);
        //img2 is not and image file
        if (img2 == NULL)
        {
                return FLT_MAX; //Should this be here ??
        }
        int num_of_pixels1 = ((img1->width)*(img1->height)); //Used for iterating throught the picture
        float num_of_pixels2 = (float)((img1->width)*(img1->height));
        float eucl_dist;
        float total_eucl_dist = 0;
        if ((img1->height != img2->height) || (img1->width != img2->width))
        {
                free(img2->p);
                free(img2);
                return FLT_MAX;
        }
        for (int i = 0; i < num_of_pixels1; i++)
        {
                eucl_dist = eucl_distance(img1->p[i], img2->p[i]);
                total_eucl_dist += eucl_dist;
        }
        total_eucl_dist = ((total_eucl_dist)/(num_of_pixels2));
        free(img2->p);
        free(img2);
       return total_eucl_dist;
}
/*
 * This helper function checks if the file is regular or a directory
 * return non-zero if regular
 * return zero otherwise
 */
// int isDir(char *filename)
// {
//         struct stat target_file;
//         stat(filename, &target_file);
//         return S_ISREG(target_file.st_mode);
// }

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it 
* - for each file read the image in it 
* - compare the image read to the image passed as parameter 
* - keep track of the image that is most similar 
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
 CompRecord process_dir(char *dirname, Image *img, int out_fd){
        
        DIR* dirPointer = opendir(dirname); //The directory
        struct dirent *dp = NULL;//Files in the Master directory
        CompRecord CRec;
        if (dirPointer == NULL)
        {
                perror("cannot oped dir");

                exit(1);

        }
        //Initializing CRec here
        // CRec.distance = FLT_MAX;
	// strcpy(CRec.filename, "");
        int haveSeenSoFar = -10;
        while ((dp = readdir(dirPointer)) != NULL)
        {
                char *currentFile;
                currentFile = dp->d_name;

                if (strcmp(currentFile, ".") == 0 || strcmp(currentFile, "..") == 0 || strcmp(currentFile, ".svn") == 0)
		{
			continue;
		}

                char currFilePath[PATHLENGTH];
		strncpy(currFilePath, dirname, PATHLENGTH);
		strncat(currFilePath, "/", PATHLENGTH - strlen(currFilePath) - 1);
		strncat(currFilePath, dp->d_name, PATHLENGTH - strlen(currFilePath) - 1);

                struct stat ourStatSoFar;
                if (stat(currFilePath, &ourStatSoFar) == -1)
		{
			perror("the problem is with our stat struct");
			exit(1);
		}

                if (S_ISREG(ourStatSoFar.st_mode))
		{
			float ValRightNow = 0.0;
                        
                        ValRightNow = compare_images(img, currFilePath);
                        if (haveSeenSoFar < 0) 
                        {
                                CRec.distance = ValRightNow;

				strcpy(CRec.filename, currentFile);
                                haveSeenSoFar = 1;

                        }
                        
			if (haveSeenSoFar > 0 && ValRightNow < CRec.distance)
			{

				CRec.distance = ValRightNow;

				strcpy(CRec.filename, currentFile);

			}
		}
                if (out_fd != -1)
	        {
		        if (write(out_fd, &CRec, sizeof(CRec)) == -1)
		        {
		        	perror("problem with writing to the pipe");

			        exit(1);
		        }
	        }
                

        }

        return CRec;
}




