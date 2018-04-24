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
#include <linux/slab.h>

#include "dataStore.h"
#include "strNumConv.h"
#include "movingAverageFilter.h"
#include "ma_ioctl.h"

#define  DEVICE_NAME "MAFilter"   
#define  CLASS_NAME  "maf"        

MODULE_LICENSE("GPL");           
MODULE_AUTHOR("Pooh Cook");  
MODULE_DESCRIPTION("A simple Moving Average Filter for no particular use");  
MODULE_VERSION("0.1");            

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  MAFClass  = NULL;     ///< The device-driver class struct pointer
static struct device* MAFDevice = NULL;     ///< The device-driver device struct pointer

// HACK:  this is mutex precludes concurrent usage of the Device driver 
// trouble is, it's not clear why 2 users would want to use this driver 
// concurrently so it's also unclear how this should actually be handled
static bool use_mutex = false;
static DEFINE_MUTEX(dev_mutex);  /// A macro that is used to declare a new mutex that is visible in this file
                                     /// results in a semaphore variable dev_mutex with value 1 (unlocked)
                                     

#define DATA_BUFFFER_SIZE   10000
static struct dataStore* dStore = NULL;     // circular buffer for storing filtered data. If stored data 
                                            // exceeds the size of this buffer, data will be evacuated (and lost) 
                                            // from the head of the queue i.e loss the old history and hang onto the
                                            // to the more recent.

#define DEFAULT_MOV_AVG_SIZE 5
static struct movingAverageFilter* movAvgFilter = NULL;  // filter that store the needed filtering history 
        
int dataBuffer[DATA_BUFFFER_SIZE];  // working buffer for dev_read and dev_write. Its declared the 
                                    // same size as the data store to simplify read and write code 


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

/** @brief method to dispose and create new moving average filter
 * 
 */
static void allocate_new_movAvgFilter(int size){
    if( movAvgFilter != NULL) FreeMovAvgFilter(movAvgFilter);
    movAvgFilter = CreateMovAvgFilter(size);  
    
}

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
   
   if( use_mutex) mutex_init(&dev_mutex);       /// Initialize the mutex lock dynamically at runtime
   
   // Allocate data storage buffer
   dStore = CreateDataStore(DATA_BUFFFER_SIZE);
   
   allocate_new_movAvgFilter(DEFAULT_MOV_AVG_SIZE);
    
   printk(KERN_INFO "MAFilter: allocated Data buffer size=%d \n", dStore->buff_size-1); 
   
   printk(KERN_INFO "MAFilter: device class successfully created \n"); 
   
   return 0;
}

/** @brief The LKM cleanup function
 */
static void __exit maf_exit(void){
   
    if( use_mutex) mutex_destroy(&dev_mutex);        /// destroy the dynamically-allocated mutex
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
    if(use_mutex && !mutex_trylock(&dev_mutex)){    /// Try to acquire the mutex 
      printk(KERN_ALERT "MAFilter: Device in use by another process");
      return -EBUSY;
   }
    
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
   int write_count, read_count;
   char* lxBuffer;
   int char_count;

   read_count = RetriveNumbers(dStore, dataBuffer, DATA_BUFFFER_SIZE);  
      
   lxBuffer = (char*)kmalloc(len, GFP_KERNEL);
   write_count = ConvertToString(dataBuffer, read_count, lxBuffer, len);
   char_count = strlen(lxBuffer)+1;
   error_count = copy_to_user(buffer, lxBuffer, char_count);
   kfree(lxBuffer);
   
   // if dev_read provided too small of a buffer then push back non returned data
   if( read_count > write_count){
       PushBackNumbers(dStore, dataBuffer+write_count, read_count - write_count);
   }
   
   if (error_count==0){    // if no errors then success
      printk(KERN_INFO "MAFilter: dev_read sent %d numbers \n", write_count);
      return char_count;  
   }
   else {
      printk(KERN_INFO "MAFilter: dev_read failed to send %d numbers to the user\n", error_count);
      return -EFAULT; // Failed -- return a bad address message (i.e. -14)
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
    
    
    // NOTE: in the case where input len is greater than DATA_BUFFFER_SIZE
    // data will be lost from the head of the store to preserve the most recently written data 
    //  this rule is encoded in the DataStore
    lxBuffer = (char*)kmalloc(len+1, GFP_KERNEL);
    error_count = copy_from_user(lxBuffer, buffer, len);
    lxBuffer[len]= 0;
    wr_count = ConvertToIntArray(lxBuffer, dataBuffer, DATA_BUFFFER_SIZE);
    DoMovAvgOnValues(movAvgFilter, dataBuffer, wr_count);
    StoreNumbers(dStore, dataBuffer, wr_count);
    kfree(lxBuffer);
    
    
    printk(KERN_INFO "MAFilter: dev_write received %d numbers from the user\n", wr_count);
    return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
    if( use_mutex) mutex_unlock(&dev_mutex);          /// Releases the mutex
    printk(KERN_INFO "MAFilter: Device successfully closed\n");
    return 0;
}

/** @brief The device control messages are handled here when ioctl is called by 
 *  the userspace program
 *  
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param cmd  The command being issued
 *  @param arg  The argument associated with the command
 */
static long dev_ioctl (struct file *filep, unsigned int cmd, unsigned long arg){
    printk(KERN_INFO "MAFilter: Device control recieved cmd(%x) arg(%lu)\n", cmd, arg ); 
    switch( cmd){
       case MAF_SET_FILTER_SIZE:
          if( arg > 0){
            allocate_new_movAvgFilter(arg);
            printk(KERN_INFO "MAFilter: Reset Filter buffer size to: %lu\n",  arg ); 
          }
          break;
          
       default:
           break;
    };
   
    return 0;
}


// Register the init and exit funtions
module_init(maf_init);
module_exit(maf_exit);
