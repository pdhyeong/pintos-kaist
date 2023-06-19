/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "include/userprog/process.h"
#include "include/threads/mmu.h"
#include "userprog/syscall.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset) {
	
	void *init_addr = addr;
	while (length > 0)
	{
		size_t page_read_bytes = length < PGSIZE ? length : PGSIZE;

		struct image *aux = (struct image *)malloc(sizeof(struct image));

		aux->file = file;
		aux->offset = offset;
		aux->read_bytes = page_read_bytes;
		if (!vm_alloc_page_with_initializer(VM_FILE, addr,
											writable, lazy_load_segment, aux))
			return NULL;
		
		/* Advance. */
		length -= page_read_bytes;
		addr += PGSIZE;
		offset += page_read_bytes;
	}
	return init_addr;
}

/* Do the munmap */
void
do_munmap (void *addr) {
	struct page *page = spt_find_page(&thread_current()->spt,addr);
	if(page == NULL){
		return;
	}
	struct thread *cur_thread = thread_current();
	struct image *file_info = page->uninit.aux;

	if(file_info->file == NULL){
		return;
	}
	while(page != NULL){
		if(pml4_is_dirty(cur_thread->pml4,addr)){
			lock_acquire(&filesys_lock);
			file_seek(file_info->file,file_info->offset);
			file_write(file_info->file,page->frame->kva,file_info->read_bytes);
			lock_release(&filesys_lock);
			pml4_set_dirty(cur_thread->pml4,addr,false);
		}
		pml4_clear_page(cur_thread->pml4,addr);
		addr += PGSIZE;
		page = spt_find_page(&thread_current()->spt,addr);
	}
}