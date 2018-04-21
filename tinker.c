/**
 * @file   tinker.c
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief  A Linux user space program that communicates with the MAFilter.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. 
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   printf("Starting to tinker...\n");
   
   // Open the device with read/write access
   fd = open("/dev/MAFilter", O_RDWR);             
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   printf("Type in a short string to send to the kernel module:\n");
   
   // Read in a string (with spaces)
   scanf("%[^\n]%*c", stringToSend);    
   
   // Send the string to the LKM
   printf("Writing message to the device [%s].\n", stringToSend);
   ret = write(fd, stringToSend, strlen(stringToSend)); 
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   }

   //printf("Press ENTER to read back from the device...\n");
   //getchar();

   // Read the response from the LKM
   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH);       
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   
   
   printf("The received message is: [%s]\n", receive);
   printf("End of the program\n");
   return 0;
}
