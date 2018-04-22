/**
 * @file   maFilter.c
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief   A filter to provide Low Pass Finite Impulse Response smoothing of data
 * Filter is single channel only 
 */

#include <linux/init.h>
#include <linux/module.h> 
#include <linux/device.h> 
#include <linux/kernel.h>
#include <linux/fs.h> 
#include <linux/uaccess.h>
#include<linux/slab.h>

#include "dataStore.h"
#include "strNumConv.h"
#include "movingAverageFilter.h"
#include"ma_ioctl.h"

#define  DEVICE_NAME "MAFilter"   
#define  CLASS_NAME  "maf"        

MODULE_LICENSE("GPL");           
MODULE_AUTHOR("Pooh Cook");  
MODULE_DESCRIPTION("A simple Moving Average Filter for no particular use");  
MODULE_VERSION("0.1");            

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  MAFClass  = NULL;     ///< The device-driver class struct pointer
static struct device* MAFDevice = NULL;     ///< The device-driver device struct pointer

#define DATA_BUFFFER_SIZE   10000
static struct dataStore* dStore;

#define DEFAULT_MOV_AVG_SIZE 5
static struct movingAverageFilter* movAvgFilter;
        
int dataBuffer[DATA_BUFFFER_SIZE];



// forward definition of device operations
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long    dev_ioctl (struct file *, unsigned int, unsigned long);

// implement basic file operation for a Linux Character Device Driver
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
   .unlocked_ioctl = dev_ioctl,
};

/** @brief The LKM initialization function
 *  @return returns 0 if successful
 */
static int __init maf_init(void){
   printk(KERN_INFO "MAFilter: Initializing the Device Driver\n");

   // register major number
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "MAFilter failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "MAFilter: registered major number %d\n", majorNumber);

   // Register the device class
   MAFClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(MAFClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(MAFClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "MAFilter: device class successfully registered\n");

   // Register the device driver
   MAFDevice = device_create(MAFClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(MAFDevice)){               
      class_destroy(MAFClass);           
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(MAFDevice);
   }
   
   // Allocate data storage buffer
   dStore = CreateDataStore(DATA_BUFFFER_SIZE);
   
   movAvgFilter = CreateMovAvgFilter(DEFAULT_MOV_AVG_SIZE);
    
   printk(KERN_INFO "MAFilter: allocated Data buffer size=%d \n", dStore->buff_size-1); 
   
   printk(KERN_INFO "MAFilter: device class successfully created \n"); 
   
   return 0;
}

/** @brief The LKM cleanup function
 */
static void __exit maf_exit(void){
    
   FreeMovAvgFilter(movAvgFilter);
   FreeDataStore(dStore);  
   device_destroy(MAFClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(MAFClass);                          // unregister the device class
   class_destroy(MAFClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "MAFilter: Goodbye from the MAFilter!\n");
}

/** @brief The device open function 
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "MAFilter: Device has been opened  \n");
   return 0;
}

/** @brief This function is called whenever device is being read from user space. 
 *  
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
    
   int result, read_count;
   char* lxBuffer;
      
   read_count = RetriveNumbers(dStore, dataBuffer, DATA_BUFFFER_SIZE);  
   
      
   lxBuffer = (char*)kmalloc(len, GFP_KERNEL);
   result = ConvertToString(dataBuffer, read_count, lxBuffer, len);
   error_count = copy_to_user(buffer, lxBuffer, strlen(lxBuffer));
   kfree(lxBuffer);
   
   // HACK: if dev_read provided too small of a buffer then data would be lost here
   // routine should check for read_count > result and return the data back to the 
   // the head of the store
   
   if (error_count==0){            // if true then have success
      printk(KERN_INFO "MAFilter: Sent %d characters to the user\n", result);
      return 0;  
   }
   else {
      printk(KERN_INFO "MAFilter: Failed to send %d numbers to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

/** @brief This function is called whenever the device is being written to from user space 
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   
    int error_count = 0;
    int wr_count;
    char* lxBuffer;
    
    // HACK: This does not handle case where input len is greater than DATA_BUFFFER_SIZE
    // In such case, the rule of loosing data from the tail will be lost and 
    // data will be lost from the head
    lxBuffer = (char*)kmalloc(len, GFP_KERNEL);
    error_count = copy_from_user(lxBuffer, buffer, len);
    wr_count = ConvertToIntArray(lxBuffer, dataBuffer, DATA_BUFFFER_SIZE);
    DoMovAvgOnValues(movAvgFilter, dataBuffer, wr_count);
    StoreNumbers(dStore, dataBuffer, wr_count);
    kfree(lxBuffer);
    
    printk(KERN_INFO "MAFilter: Received %d numbers from the user\n", wr_count);
    return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "MAFilter: Device successfully closed\n");
   return 0;
}

static long dev_ioctl (struct file *filep, unsigned int cmd, unsigned long arg){
   printk(KERN_INFO "MAFilter: Device control recieved cmd(%x) arg(%lu)\n", cmd, arg ); 
   switch( cmd){
       case MAF_SET_FILTER_SIZE:
          printk(KERN_INFO "MAFilter: Reset Filter buffer size to: %lu\n",  arg );  
           
       default:
           break;
   };
   
   
   return 0;
}


// Register the init and exit funtions
module_init(maf_init);
module_exit(maf_exit);
