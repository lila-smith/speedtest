/*  cdmacdev.c - The simplest kernel module.

* Copyright (C) 2013 - 2016 Xilinx, Inc
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program. If not, see <http://www.gnu.org/licenses/>.

*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_dma.h>

#include "cdmacdev.h"

/* Standard module information, edit as appropriate */
MODULE_LICENSE("GPL");
MODULE_AUTHOR
    ("sakamoto@icepp.s.u-tokyo.ac.jp");
MODULE_DESCRIPTION
    ("cdmacdev - character device driver for CDMA");

#define DRIVER_NAME "cdmacdev"
#define CHRDEV_NAME "cdmach"
#define DMA_NAME "cdmach0"
//#define BUFFER_SIZE 0x1000
#define BUFFER_SIZE 0x8010
#define BRAM_BASE_ADDR 0xB0010000 // 0xA0003000
#define BRAM_SIZE 0x1000 // 0x1000
#define BRAM_MAX_ADDR ( BRAM_BASE_ADDR + BRAM_SIZE )
// #define DRAM_BASE_ADDR 0x70000000
// #define DRAM_SIZE 0x10000000
// #define DRAM_MAX_ADDR ( DRAM_BASE_ADDR + DRAM_SIZE )
#define CPU_MAX_ADDR 0x10000000

struct cdmacdev_buffer {

    unsigned char * p_bufvmem;
    dma_addr_t m_bufphys;
    int m_bufsize;
    enum cdmacdev_buftype {
        CDMACDEV_BUF_NONE = 0,
        CDMACDEV_BUF_CPU = 1,
        CDMACDEV_BUF_DRAM = 2,
        CDMACDEV_BUF_BRAM = 3
        } m_buftype;
};

struct cdmacdev_channel {

    struct dma_chan * p_dmachan;
    struct device * p_device;
    struct dma_device * p_dmadev;
    dev_t m_devnode;
    struct cdev m_cdev;
    struct class * p_class;
    struct device * p_classdev;
    struct cdmacdev_buffer m_cpubuf;
    struct cdmacdev_buffer m_devbuf;
    unsigned long m_datasize;
    struct dma_async_tx_descriptor * p_descr;
    struct completion m_comp;
    dma_cookie_t m_cookie;
    unsigned long m_direction;

};

static struct cdmacdev_channel cdmachannel;


static int cdmacdev_alloc_buffer( struct cdmacdev_channel * pchan,
    struct cdmacdev_buffer * pbuf ) {

    switch( pbuf -> m_buftype ) {
    case CDMACDEV_BUF_CPU:
        pbuf -> p_bufvmem = dmam_alloc_coherent( pchan -> p_device,
            pbuf -> m_bufsize, & pbuf -> m_bufphys, GFP_KERNEL );
        if( ! pbuf -> p_bufvmem ) {
            dev_err( pchan -> p_device, "failed to allocate memory\n" );
            return -ENOMEM;
        }
        break;
    case CDMACDEV_BUF_BRAM:
        pbuf -> p_bufvmem = ioremap( pbuf -> m_bufphys, pbuf -> m_bufsize );
        break;
    default:
        dev_err( pchan -> p_device, "Unsupported buffer type %d\n", pbuf -> m_buftype );
        return -EINVAL;
    }
    // dev_info( pchan -> p_device, "buffer allocated: type = %d size = %d vmem = 0x%d phys = 0x%llx\n",
    //     pbuf -> m_buftype, pbuf -> m_bufsize, pbuf -> p_bufvmem, pbuf -> m_bufphys );
    dev_info( pchan -> p_device, "buffer allocated: type = %d size = %d phys = 0x%llx\n",
        pbuf -> m_buftype, pbuf -> m_bufsize, pbuf -> m_bufphys );
    return 0;
    }

static int cdmacdev_free_buffer( struct cdmacdev_channel * pchan,
    struct cdmacdev_buffer * pbuf ) {

    // dev_info( pchan -> p_device, "freeing vmem = 0x%x phys = 0x%llx\n",
    //         pbuf -> p_bufvmem, pbuf -> m_bufphys );
    dev_info( pchan -> p_device, "phys = 0x%llx\n",
        pbuf -> m_bufphys );

    switch( pbuf -> m_buftype ) {
    case CDMACDEV_BUF_NONE:     // nothing to do
        break;
    case CDMACDEV_BUF_CPU:
        dmam_free_coherent( pchan -> p_device, pbuf -> m_bufsize,
            pbuf -> p_bufvmem, pbuf -> m_bufphys );
        break;
    case CDMACDEV_BUF_BRAM:
        iounmap( pbuf -> p_bufvmem );
        break;
    default:
        dev_err( pchan -> p_device, "Unsupported buffer type %d\n", pbuf -> m_buftype );
        return -EINVAL;
    }
    pbuf -> m_buftype = CDMACDEV_BUF_NONE;

    return 0;

}

static int cdmacdev_open( struct inode * p_inode, struct file * p_file ) {

    struct cdmacdev_channel * pchan;

    pchan = container_of( p_inode -> i_cdev,
        struct cdmacdev_channel, m_cdev );

    p_file -> private_data = pchan;

    dev_info( pchan -> p_device, "cdmacdev_open called\n" );

    return 0;

}

static int cdmacdev_release( struct inode * p_inode, struct file * p_file ) {

    struct cdmacdev_channel * pchan;

    pchan = ( struct cdmacdev_channel * ) p_file -> private_data;

    dev_info( pchan -> p_device, "cdmacdev_release called\n" );

    cdmacdev_free_buffer( pchan, & pchan -> m_cpubuf );
    cdmacdev_free_buffer( pchan, & pchan -> m_devbuf );

    return 0;

}

static void cdmacdev_setup_buffer( struct cdmacdev_channel * pchan ) {

    int i;
    unsigned char * psrc, * pdst;

    if( pchan -> m_direction == CDMACDEV_DIR_MEM_TO_DEV ) {
        psrc = pchan -> m_cpubuf.p_bufvmem;
        pdst = pchan -> m_devbuf.p_bufvmem;
    }
    else if( pchan -> m_direction == CDMACDEV_DIR_DEV_TO_MEM ) {
        psrc = pchan -> m_devbuf.p_bufvmem;
        pdst = pchan -> m_cpubuf.p_bufvmem;
    }
    else return;

    for( i = 0; i < pchan -> m_datasize; i ++ ) {
        * psrc ++ = ( unsigned char )( i & 0xff );
        * pdst ++ = '\0';
    }

}

static int cdmacdev_compare_buffer( struct cdmacdev_channel * pchan ) {

    int i;
    int diff;
    unsigned char * psrc, * pdst;

    diff = 0;
    psrc = pchan -> m_cpubuf.p_bufvmem;
    pdst = pchan -> m_devbuf.p_bufvmem;

    for( i = 0; i < pchan -> m_datasize; i ++ ) {
        if( * psrc ++ != * pdst ++ ) diff ++;
    }

    return diff;

}


static void cdmacdev_callback( void * completion ) {

    complete( completion );

}

static int cdmacdev_transfer( struct cdmacdev_channel * pchan ) {

    unsigned long timeout = msecs_to_jiffies( 3000 );
    enum dma_status status;
    enum dma_ctrl_flags flags;
    dma_addr_t src, dst;

    flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    if( pchan -> m_direction == CDMACDEV_DIR_DEV_TO_MEM ) {
        src = pchan -> m_devbuf.m_bufphys;
        dst = pchan -> m_cpubuf.m_bufphys;
    }
    else if( pchan -> m_direction == CDMACDEV_DIR_MEM_TO_DEV ) {
        src = pchan -> m_cpubuf.m_bufphys;
        dst = pchan -> m_devbuf.m_bufphys;
    }
    else {
        dev_err( pchan -> p_device, "Invalid transfer directon %lu\n", pchan -> m_direction );
        return -EINVAL;
    }

    pchan -> p_descr = pchan -> p_dmadev -> device_prep_dma_memcpy(
        pchan -> p_dmachan, dst, src, pchan -> m_datasize, flags );
    if( ! pchan -> p_descr ) {
        dev_err( pchan -> p_device, "DMA buffer preparation failure\n" );
        return -EINVAL;
    }

    init_completion( & pchan -> m_comp );
    pchan -> p_descr -> callback = cdmacdev_callback;
    pchan -> p_descr -> callback_param = & pchan -> m_comp;
    pchan -> m_cookie = pchan -> p_descr -> tx_submit( pchan -> p_descr );
    if( dma_submit_error( pchan -> m_cookie ) ) {
        dev_err( pchan -> p_device, "DMA submission failed\n" );
        return -EIO;
    }

    dma_async_issue_pending( pchan -> p_dmachan );

    timeout = wait_for_completion_timeout( & pchan -> m_comp, timeout );

    status = dma_async_is_tx_complete( pchan -> p_dmachan,
        pchan -> m_cookie, NULL, NULL );

    if( timeout == 0 ) {
        dev_err( pchan -> p_device, "DMA timed out\n" );
        return -ETIME;
    } else if( status != DMA_COMPLETE ) {
        dev_err( pchan -> p_device, "DMA transfer failure\n" );
        return -EIO;
    }

    return 0;

}

static int cdmacdev_create_channel( struct platform_device * pdev,
    struct cdmacdev_channel * pchan, char * name ) {

    pchan -> p_dmachan = dma_request_slave_channel(
        & pdev -> dev, name );
    if( IS_ERR( pchan -> p_dmachan ) ) {
        dev_err( & pdev -> dev, "DMA channel request failure\n" );
        // return pchan -> p_dmachan;
        return -EINVAL;
    }

    pchan -> m_cpubuf.m_bufsize = BUFFER_SIZE;
    pchan -> m_cpubuf.m_buftype = CDMACDEV_BUF_NONE;
    pchan -> m_devbuf.m_bufsize = BUFFER_SIZE;
    pchan -> m_devbuf.m_buftype = CDMACDEV_BUF_NONE;
    pchan -> m_datasize = BUFFER_SIZE;
    pchan -> p_dmadev = pchan -> p_dmachan -> device;

    if( ! dma_has_cap( DMA_MEMCPY, pchan -> p_dmadev -> cap_mask ) ) {
        dev_err( & pdev -> dev, "DMA has no memcpy capability\n" );
        return -EINVAL;
    }

    return 0;

}

static int cdmacdev_close_channel( struct cdmacdev_channel * pchan ) {

    cdmacdev_free_buffer( pchan, & pchan -> m_cpubuf );
    cdmacdev_free_buffer( pchan, & pchan -> m_devbuf );
    dma_release_channel( pchan -> p_dmachan );

    return 0;

}
static ssize_t cdmacdev_read( struct file * p_file, char __user * p_buf,
    size_t size, loff_t * pos ) {

    struct cdmacdev_channel * pchan;
    int st;
    size_t tsize;

    pchan = ( struct cdmacdev_channel * ) p_file -> private_data;

    if( pchan -> m_devbuf.m_buftype != CDMACDEV_BUF_BRAM ) {
        cdmacdev_free_buffer( pchan, & pchan -> m_devbuf );
        pchan -> m_devbuf.m_buftype = CDMACDEV_BUF_BRAM;
        st = cdmacdev_alloc_buffer( pchan, & pchan -> m_devbuf );
        if( st < 0 ) return st;
    }
    if( pchan -> m_cpubuf.m_buftype != CDMACDEV_BUF_CPU ) {
        cdmacdev_free_buffer( pchan, & pchan -> m_cpubuf );
        pchan -> m_cpubuf.m_buftype = CDMACDEV_BUF_CPU;
        st = cdmacdev_alloc_buffer( pchan, & pchan -> m_cpubuf );
        if( st < 0 ) return st;
    }

    tsize = size;
    if( tsize > pchan -> m_devbuf.m_bufsize ) {
        tsize = pchan -> m_devbuf.m_bufsize;
        dev_warn( pchan -> p_device, "Data truncated to %ld bytes\n", tsize );
    }
    pchan -> m_datasize = tsize;
    pchan -> m_direction = CDMACDEV_DIR_DEV_TO_MEM;

    st = cdmacdev_transfer( pchan );
    if( st < 0 ) return st;

    st = copy_to_user( p_buf, pchan -> m_cpubuf.p_bufvmem, tsize );
    if( st < 0 ) return st;

    * pos = tsize + 1;

    return tsize;

}

static ssize_t cdmacdev_write( struct file * p_file, const char __user * p_buf,
    size_t size, loff_t * pos ) {

    struct cdmacdev_channel * pchan;
    int st;
    size_t tsize;

    pchan = ( struct cdmacdev_channel * ) p_file -> private_data;

    if( pchan -> m_devbuf.m_buftype != CDMACDEV_BUF_BRAM ) {
        cdmacdev_free_buffer( pchan, & pchan -> m_devbuf );
        pchan -> m_devbuf.m_buftype = CDMACDEV_BUF_BRAM;
        st = cdmacdev_alloc_buffer( pchan, & pchan -> m_devbuf );
        if( st < 0 ) return st;
    }
    if( pchan -> m_cpubuf.m_buftype != CDMACDEV_BUF_CPU ) {
        cdmacdev_free_buffer( pchan, & pchan -> m_cpubuf );
        pchan -> m_cpubuf.m_buftype = CDMACDEV_BUF_CPU;
        st = cdmacdev_alloc_buffer( pchan, & pchan -> m_cpubuf );
        if( st < 0 ) return st;
    }

    tsize = size;
    if( tsize > pchan -> m_devbuf.m_bufsize ) {
        tsize = pchan -> m_devbuf.m_bufsize;
        dev_warn( pchan -> p_device, "Data truncated to %ld bytes\n", tsize );
    }
    pchan -> m_datasize = tsize;
    pchan -> m_direction = CDMACDEV_DIR_MEM_TO_DEV;

    st = copy_from_user( pchan -> m_cpubuf.p_bufvmem, p_buf, tsize );
    if( st < 0 ) return st;

    st = cdmacdev_transfer( pchan );
    if( st < 0 ) return st;

    * pos = tsize + 1;

    return tsize;

}

static long cdmacdev_ioctl( struct file * p_file, unsigned int cmd,
	unsigned long arg ) {

    struct cdmacdev_channel * pchan;
    struct cdmacdev_buffer * pbuf;
    int st;
    unsigned long diff;

    pchan = ( struct cdmacdev_channel * ) p_file -> private_data;

    pr_info("cdmacdev_ioctl called with cmd = 0x%x arg = 0x%lx\n",
	cmd, arg );

    switch( cmd ) {
    case CDMACDEV_IOC_SET_DEV_ADDR:
        pbuf = & pchan -> m_devbuf;
        if( pbuf -> m_buftype != CDMACDEV_BUF_NONE ) {
            st = cdmacdev_free_buffer( pchan, pbuf );
            if( st < 0 ) {
            pbuf -> m_buftype = CDMACDEV_BUF_NONE;
            return st;
            }
        }

        pbuf -> m_buftype = CDMACDEV_BUF_BRAM;
        pbuf -> m_bufphys = arg;

        st = cdmacdev_alloc_buffer( pchan, pbuf );
        if( st < 0 ) return st;
        break;
    case CDMACDEV_IOC_SET_CPU_ADDR:
        pbuf = & pchan -> m_cpubuf;
        if( pbuf -> m_buftype != CDMACDEV_BUF_NONE ) {
            st = cdmacdev_free_buffer( pchan, pbuf );
            if( st < 0 ) {
            pbuf -> m_buftype = CDMACDEV_BUF_NONE;
            return st;
            }
        }
        
        pbuf -> m_buftype = CDMACDEV_BUF_CPU;

        st = cdmacdev_alloc_buffer( pchan, pbuf );
        if( st < 0 ) return st;
        break;
    case CDMACDEV_IOC_GET_DEV_ADDR:
    case CDMACDEV_IOC_GET_CPU_ADDR:
        if( cmd == CDMACDEV_IOC_GET_DEV_ADDR ) pbuf = & pchan -> m_devbuf;
        else pbuf = & pchan -> m_cpubuf;
        st = copy_to_user( ( unsigned long * ) arg, & pbuf -> m_bufphys,
            sizeof( unsigned long ) );
        if( st < 0 ) {
            dev_err( pchan -> p_device, "failed to copy to user memory 0x%lx\n",
            arg );
            return st;
        }
        break;
    case CDMACDEV_IOC_SET_DATA_SIZE:
        if( arg > pchan -> m_devbuf.m_bufsize ||
            arg > pchan -> m_cpubuf.m_bufsize ) {
            dev_err( pchan -> p_device, "data size to large\n" );
            return -EINVAL;
        }
        pchan -> m_datasize = arg;
        break;
    case CDMACDEV_IOC_GET_DATA_SIZE:
        st = copy_to_user( ( unsigned long * ) arg,
            & pchan -> m_datasize,
            sizeof( unsigned long ) );
        if( st < 0 ) {
            dev_err( pchan -> p_device, "failed to copy to user memory 0x%lx\n", arg );
            return st;
        }
        break;
    case CDMACDEV_IOC_FREE_CPUBUF:
        if( pchan -> m_cpubuf.m_buftype == CDMACDEV_BUF_NONE ) break;
        st = cdmacdev_free_buffer( pchan, & pchan -> m_cpubuf );
        if( st < 0 ) return st;
        break;
    case CDMACDEV_IOC_FREE_DEVBUF:
        if( pchan -> m_devbuf.m_buftype == CDMACDEV_BUF_NONE ) break;
        st = cdmacdev_free_buffer( pchan, & pchan -> m_devbuf );
        if( st < 0 ) return st;
        break;
    case CDMACDEV_IOC_SET_DIRECTION:
        if( arg != CDMACDEV_DIR_DEV_TO_MEM && arg != CDMACDEV_DIR_MEM_TO_DEV ) {
            dev_err( pchan -> p_device, "Invalid direction value %ld\n", arg );
            return -EINVAL;
        }
        pchan -> m_direction = arg;
        break;
    case CDMACDEV_IOC_GET_DIRECTION:
        pr_info("cdmacdev_ioctl: direction = 0x%lx\n",
            pchan -> m_direction );
        st = copy_to_user( ( unsigned long * ) arg,
            & pchan -> m_direction,
            sizeof( unsigned long ) );
        if( st < 0 ) {
            pr_err("cdmacdev_ioctl: failed to copy to user memory 0x%lx\n",
            arg );
            return st;
        }
        break;
    case CDMACDEV_IOC_DO_TRANSFER:
        st = cdmacdev_transfer( pchan );
        if( st < 0 ) return st;
        break;
    case CDMACDEV_IOC_FILL_BUFFER:
        cdmacdev_setup_buffer( pchan );
        break;
    case CDMACDEV_IOC_COMPARE_BUFFER:
        diff = (unsigned long) cdmacdev_compare_buffer( pchan );
        pr_info("cdmacdev_ioctl: compare buffer detected %ld differences\n",
            diff );
        st = copy_to_user( ( unsigned long * ) arg,
            & diff, sizeof( unsigned long ) );
        break;
    default:
	    dev_err( pchan -> p_device, "Unsupported IOCTL %d\n", cmd );
	return -EINVAL;
    }

    return 0;

}

static int cdmacdev_mmap( struct file * p_file, struct vm_area_struct * vma ) {

    struct cdmacdev_channel * pchan;
    int st;

    pchan = ( struct cdmacdev_channel * ) p_file -> private_data;

    if( pchan -> m_cpubuf.m_buftype != CDMACDEV_BUF_CPU ) {
        dev_err( pchan -> p_device, "Memory buffer not allocated\n" );
        return -ENOMEM;
    }

    return dma_mmap_coherent( pchan -> p_device, vma,
        pchan -> m_cpubuf.p_bufvmem, pchan -> m_cpubuf.m_bufphys,
        vma -> vm_end - vma -> vm_start );

}

static struct file_operations cdmacdev_fops = {
    .owner = THIS_MODULE,
    .open = cdmacdev_open,
    .release = cdmacdev_release,
    .unlocked_ioctl = cdmacdev_ioctl,
    .read = cdmacdev_read,
    .write = cdmacdev_write,
    .mmap = cdmacdev_mmap,
};

static int cdmacdev_create_cdev( struct cdmacdev_channel * pchan, char * name ) {

    int st;

    st = alloc_chrdev_region( & pchan -> m_devnode, 0, 1, "cdmacdev" );
    if( st ) {
        dev_err( pchan -> p_device, "Failed to allocate cdev region\n" );
        return st;
    }

    cdev_init( & pchan -> m_cdev, & cdmacdev_fops );

    pchan -> m_cdev.owner = THIS_MODULE;

    st = cdev_add( & pchan -> m_cdev, pchan -> m_devnode, 1 );
    if( st ) {
        dev_err( pchan -> p_device, "Failed to add a character device\n" );
        goto error_1;
    }

    pchan -> p_class = class_create( THIS_MODULE, DRIVER_NAME );
    if( IS_ERR( pchan -> p_device -> class ) ) {
        dev_err( pchan -> p_device, "Failed to create class\n" );
        st = -EINVAL;
        goto error_2;
    }

    pchan -> p_classdev = device_create( pchan -> p_class, NULL,
        pchan -> m_devnode, NULL, name );
    if( IS_ERR( pchan -> p_classdev ) ) {
        dev_err( pchan -> p_device, "Failed to create a class device %s\n", name );
        st = -EINVAL;
        goto error_3;
    }

    return 0;

error_3:
    class_destroy( pchan -> p_class );
error_2:
    cdev_del( & pchan -> m_cdev );
error_1:
    unregister_chrdev_region( pchan -> m_devnode, 1 );
    return st;

}



static int cdmacdev_probe(struct platform_device *pdev)
{
    int st;
    
    dev_info( & pdev -> dev, "probe called\n" );
    cdmachannel.p_device = & pdev -> dev;

    st = cdmacdev_create_cdev( & cdmachannel, CHRDEV_NAME );
    if( st ) return st;

    st = cdmacdev_create_channel( pdev, & cdmachannel, DMA_NAME );
    if( st ) return st;

    pr_info( "cdmacdev channel successfully created\n" );
    
	return 0;
}

static int cdmacdev_destroy_cdev( struct cdmacdev_channel * pchan ) {

    if( pchan -> p_classdev ) {
        device_destroy( pchan -> p_class, pchan -> m_devnode );
        class_destroy( pchan -> p_class );
        cdev_del( & pchan -> m_cdev );
        unregister_chrdev_region( pchan -> m_devnode, 1 );
    }
    return 0;

}

static int cdmacdev_remove(struct platform_device *pdev)
{
    dev_info( & pdev -> dev, "remove called\n" );
    cdmacdev_close_channel( & cdmachannel );
    cdmacdev_destroy_cdev( & cdmachannel );
    return 0;
}






static struct of_device_id cdmacdev_of_match[] = {
        { .compatible = "cdmacdev", },
        { /* end of list */ },
};
MODULE_DEVICE_TABLE(of, cdmacdev_of_match);


static struct platform_driver cdmacdev_driver = {
        .driver = {
                .name = DRIVER_NAME,
                .owner = THIS_MODULE,
                .of_match_table = cdmacdev_of_match,
        },
        .probe          = cdmacdev_probe,
        .remove         = cdmacdev_remove,
};

static int __init cdmacdev_init(void)
{
        return platform_driver_register(&cdmacdev_driver);
}


static void __exit cdmacdev_exit(void)
{
        platform_driver_unregister(&cdmacdev_driver);
}

module_init(cdmacdev_init);
module_exit(cdmacdev_exit);