/*
 * Compatibility shims for building the MPSS 3.8.6 host modules on
 * Linux 6.x (tested: 6.1 stable).  Included into every object via Kbuild.
 *
 * Only APIs that were removed or renamed without a semantic change are
 * mapped here; anything that changed behaviour is fixed at the call site.
 */
#ifndef _MIC_COMPAT_LINUX6_H_
#define _MIC_COMPAT_LINUX6_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
#include <linux/pci.h>
#include <linux/dma-mapping.h>

/* include/linux/pci-dma-compat.h was removed in 5.18 */
#ifndef PCI_DMA_BIDIRECTIONAL
#define PCI_DMA_BIDIRECTIONAL	DMA_BIDIRECTIONAL
#define PCI_DMA_TODEVICE	DMA_TO_DEVICE
#define PCI_DMA_FROMDEVICE	DMA_FROM_DEVICE
#define PCI_DMA_NONE		DMA_NONE
#endif

static inline dma_addr_t
pci_map_single(struct pci_dev *hwdev, void *ptr, size_t size, int direction)
{
	return dma_map_single(&hwdev->dev, ptr, size,
			      (enum dma_data_direction)direction);
}

static inline void
pci_unmap_single(struct pci_dev *hwdev, dma_addr_t dma_addr, size_t size,
		 int direction)
{
	dma_unmap_single(&hwdev->dev, dma_addr, size,
			 (enum dma_data_direction)direction);
}

static inline dma_addr_t
pci_map_page(struct pci_dev *hwdev, struct page *page, unsigned long offset,
	     size_t size, int direction)
{
	return dma_map_page(&hwdev->dev, page, offset, size,
			    (enum dma_data_direction)direction);
}

static inline void
pci_unmap_page(struct pci_dev *hwdev, dma_addr_t dma_address, size_t size,
	       int direction)
{
	dma_unmap_page(&hwdev->dev, dma_address, size,
		       (enum dma_data_direction)direction);
}

static inline int
pci_map_sg(struct pci_dev *hwdev, struct scatterlist *sg, int nents,
	   int direction)
{
	return dma_map_sg(&hwdev->dev, sg, nents,
			  (enum dma_data_direction)direction);
}

static inline void
pci_unmap_sg(struct pci_dev *hwdev, struct scatterlist *sg, int nents,
	     int direction)
{
	dma_unmap_sg(&hwdev->dev, sg, nents,
		     (enum dma_data_direction)direction);
}

static inline void
pci_dma_sync_single_for_cpu(struct pci_dev *hwdev, dma_addr_t dma_handle,
			    size_t size, int direction)
{
	dma_sync_single_for_cpu(&hwdev->dev, dma_handle, size,
				(enum dma_data_direction)direction);
}

static inline void
pci_dma_sync_single_for_device(struct pci_dev *hwdev, dma_addr_t dma_handle,
			       size_t size, int direction)
{
	dma_sync_single_for_device(&hwdev->dev, dma_handle, size,
				   (enum dma_data_direction)direction);
}

static inline int
pci_dma_mapping_error(struct pci_dev *pdev, dma_addr_t dma_addr)
{
	return dma_mapping_error(&pdev->dev, dma_addr);
}

static inline int pci_set_dma_mask(struct pci_dev *dev, u64 mask)
{
	return dma_set_mask(&dev->dev, mask);
}

static inline int pci_set_consistent_dma_mask(struct pci_dev *dev, u64 mask)
{
	return dma_set_coherent_mask(&dev->dev, mask);
}
#endif /* >= 5.18 */

/* ioremap_nocache() removed in 5.6; ioremap() is uncached on x86 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#ifndef ioremap_nocache
#define ioremap_nocache(addr, size) ioremap(addr, size)
#endif
#endif

/* random_ether_addr() removed in 5.15 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
#include <linux/etherdevice.h>
#ifndef random_ether_addr
#define random_ether_addr(addr) eth_random_addr(addr)
#endif
#endif

/* PDE_DATA() renamed in 5.17 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
#include <linux/proc_fs.h>
#ifndef PDE_DATA
#define PDE_DATA(inode) pde_data(inode)
#endif
#endif

/*
 * (smp_)read_barrier_depends() removed in 5.9: READ_ONCE() now provides
 * the dependency ordering on every architecture, and it was a no-op on
 * x86 anyway.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
#ifndef read_barrier_depends
#define read_barrier_depends()		do { } while (0)
#endif
#ifndef smp_read_barrier_depends
#define smp_read_barrier_depends()	do { } while (0)
#endif
#endif

#endif /* _MIC_COMPAT_LINUX6_H_ */
