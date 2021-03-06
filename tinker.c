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
#include<sys/ioctl.h>
#include"ma_ioctl.h"


#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM


//  This whole program is one big HACK...  Rather than fix it, I'm going to create 
// something in Java to replace it....
int main(){
   int ret, fd, filterSize;
   char stringToSend[BUFFER_LENGTH];
   printf("Starting to tinker...\n");
   
   // Open the device with read/write access
   fd = open("/dev/MAFilter", O_RDWR);             
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   
   
   printf("Enter Filter size to use:");
   
   // Read in a string (with spaces)
   scanf("%d%*c", &filterSize); 
   
   int res = ioctl(fd, MAF_SET_FILTER_SIZE, filterSize);
   printf("result = %d\n", res);
   
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
   ret = read(fd, receive, 20);       
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   
   printf("The received message1 is: [%s]\n", receive);
   
   printf("Reading from the device...\n");
   ret = read(fd, receive, 20);       
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   
   printf("The received message2 is: [%s]\n", receive);
   
   printf("Reading from the device...\n");
   ret = read(fd, receive, 20);       
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   
   printf("The received message3 is: [%s]\n", receive);
   
   
   
   
   
   printf("End of the program\n");
   return 0;
}
