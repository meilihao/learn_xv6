struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe; // FD_PIPE
  struct inode *ip;  // FD_INODE and FD_DEVICE
  uint off;          // FD_INODE
  short major;       // FD_DEVICE
};

#define major(dev)  ((dev) >> 16 & 0xFFFF)
#define minor(dev)  ((dev) & 0xFFFF)
#define	mkdev(m,n)  ((uint)((m)<<16| (n)))

// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};

// map major device number to device functions.
// 在UNIX系统中，有主设备号(major device number)和从设备号(minor device number)的区分，其中主设备号用来确定设备要使用的驱动程序大类
// 而即便再类似的外部设备，它们的驱动程序也一定会有细微差别，这时候就需要借助从设备号(minor device number)在驱动程序中对特定的设备加以区分和细节处理了
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

extern struct devsw devsw[]; // 定义一个名为 devsw 的外部数组. extern：表示这个数组是在其他源文件中定义的，当前文件只声明它的存在

#define CONSOLE 1
