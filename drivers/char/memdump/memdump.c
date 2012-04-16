#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define MEMDUMP_MAJOR 200
#define MEMDUMP_MINOR 1

static ssize_t memdump_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t memdump_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static loff_t memdump_lseek(struct file *file,loff_t offset, int origin );
static struct class_simple *memdump_class;

unsigned long ofs=0;

static struct file_operations memdump_ops = {
	.owner		= THIS_MODULE,
	.read		= memdump_read,
	.write		= memdump_write,
	.llseek		= memdump_lseek
};

static loff_t memdump_lseek(struct file *file,loff_t offset, int origin ) {
	printk("memdump: seek %d %d\n", offset, origin);
	ofs=offset;
	return offset;
}

static int copy_words(u_int32_t *s, u_int32_t *d, int count)
{
	while (count--) {
		u_int32_t le = readl(s++);
		writel(le, d++);
	};
	return 0;
}
static ssize_t memdump_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long p_a = *ppos+ofs;
	void *mapped_mem = ioremap(p_a, count);
	if (!mapped_mem) {
		return -EINVAL;
	}
	printk("mapped %08lx to %p\n", p_a, mapped_mem);
	char mbuf[count];
	copy_words(mapped_mem, (u_int32_t*)mbuf, count/4);
	iounmap(mapped_mem);
	if (copy_to_user(buf, mbuf, (unsigned long)count)) {
		return -EFAULT;
	}
	*ppos += count;
	return count;
}

static ssize_t memdump_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char mbuf[count];
	if (copy_from_user(mbuf, buf, (unsigned long)count)) {
		return -EFAULT;
	}
	unsigned long p_a = *ppos;
	void *mapped_mem = ioremap(p_a, count);
	if (!mapped_mem) {
		return -EINVAL;
	}
	printk("mapped %08lx to %p\n", p_a, mapped_mem);
	copy_words((u_int32_t*)mbuf, mapped_mem, count/4);
	iounmap(mapped_mem);
	*ppos += count;
	return count;
}

void msm_nand_hack_test(void)
{
	unsigned *pa;

	pa=ioremap(0xA900026C, 4096);	pa[0]=0x00000009;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00020, 4096);	pa[0]=0xAA5400C0;	iounmap(pa);
	pa=ioremap(0xA0A00024, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000031;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00024, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00020, 4096);	pa[0]=0xAA5400C0;	iounmap(pa);
	pa=ioremap(0xA0A00024, 4096);	pa[0]=0x000A7476;	iounmap(pa);
	pa=ioremap(0xA0A00004, 4096);	pa[0]=0x00000000;	iounmap(pa);
	pa=ioremap(0xA0A00008, 4096);	pa[0]=0x00000000;	iounmap(pa);
	pa=ioremap(0xA0A00024, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00004, 4096);	pa[0]=0x00010000;	iounmap(pa);
	pa=ioremap(0xA0A00008, 4096);	pa[0]=0x00000000;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000031;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
	pa=ioremap(0xA0A00010, 4096);	pa[0]=0x00000001;	iounmap(pa);
	pa=ioremap(0xA0A00000, 4096);	pa[0]=0x00000034;	iounmap(pa);
}

static int __init memdump_init(void)
{
	unsigned *pa;

	dev_t dev;
	dev=MKDEV(MEMDUMP_MAJOR, MEMDUMP_MINOR);

	if (register_chrdev(MEMDUMP_MAJOR, "memdump", &memdump_ops)) {
		printk("Failed to register character device %d\n", MEMDUMP_MAJOR);
		return -EBUSY;
	}

	// removing modem memory protection
	pa=ioremap(0xA8240000, 4096);
	printk("Reseting 0xA8240800 (%X) MPU\n", pa);
	if (!pa) {
		printk("unable to realloc 0xA824000");
		return -EBUSY;
	}

	pa[0x200]=0;
	iounmap(pa);

	pa=ioremap(0xA8250000, 4096);
	printk("Reseting 0xA8250800 (%X) MPU\n", pa);
	if (!pa) {
		printk("unable to realloc 0xA825000");
		return -EBUSY;
	}

 	pa[0x200]=0;
 	iounmap(pa);

	pa=ioremap(0xA0B00000, 4096);
	printk("Reseting 0xA0B00000 (%X) MAND MPU\n", pa);
	if (!pa) {
		printk("unable to realloc 0xA0B00000");
		return -EBUSY;
	}

	pa[0]=0;
	iounmap(pa);

	pa=ioremap(0xA0A00000, 4096);
	printk("Nand Config from 0xA0A00000 (%X)\n", pa);
	if (!pa) {
		printk("unable to realloc 0xA825000");
		return -EBUSY;
	}
	printk("NAND_FLASH_CMD = %X, NAND_ADDR_0 = %X, NAND_ADDR_1 = %X, NAND_FLASH_CHIP_SELECT = %X\n", pa[0],pa[1],pa[2],pa[3]);
	printk("NAND_FLASH_STATUS = %X, NANDC_BUFFER_STATUS = %X\n", pa[5],pa[6]);
	printk(" NAND_DEVn_CFG0 = %X, NAND_DEVn_CFG1 = %X\n", pa[8],pa[9]);
	printk(" NAND_DEV1_CFG0 = %X, NAND_DEV1_CFG1 = %X\n", pa[12],pa[13]);
	printk("NAND_FLASH_READ_ID = %X, NAND_FLASH_READ_STATUS =%X\n", pa[16],pa[17]);

	iounmap(pa);

	msm_nand_hack_test();

	return 0;
}

static void __exit memdump_exit(void)
{
	unregister_chrdev(MEMDUMP_MAJOR, "memdump");
}

module_init(memdump_init);
module_exit(memdump_exit);

