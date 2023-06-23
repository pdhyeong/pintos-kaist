/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "userprog/process.h"
#include "userprog/syscall.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);
void exit_do_munmap(struct supplemental_page_table *cur_spt);

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
	if(page == NULL){
		return false;
	}
	struct info *aux = (struct info *)page->uninit.aux;

	struct file *file = aux->file;
	off_t offset = aux->offset;
	size_t page_read_bytes = aux->read_bytes;
	size_t page_zero_bytes = PGSIZE - page_read_bytes;

	file_seek(file, offset);

	if (file_read(file, kva, page_read_bytes) != (int)page_read_bytes)
	{
		return false;
	}

	memset(kva + page_read_bytes, 0, page_zero_bytes);
	return true;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;

	if(page == NULL){
		return false;
	}
	struct info *aux = page->uninit.aux;

	if (pml4_is_dirty(thread_current()->pml4, page->va))
	{
		lock_acquire(&filesys_lock);
		file_seek(aux->file, aux->offset);
		file_write(aux->file, page->va, aux->read_bytes);
		lock_release(&filesys_lock);
		pml4_set_dirty(thread_current()->pml4, page->va, false);
	}
	pml4_clear_page(thread_current()->pml4, page->va);
	return true;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
	free(page->frame);
}

/* Do the mmap */
/* Do the mmap */
static bool
lazy_mmap(struct page *page, void *aux)
{
	/* project 3 virtual memory */
	struct frame *load_frame = page->frame;
	struct file_page *file_page UNUSED = &page->file;

	file_page->file = ((struct info *)aux)->file;
	file_page->ofs = ((struct info *)aux)->offset;
	file_page->page_read_bytes = ((struct info *)aux)->read_bytes;

	file_seek(file_page->file, file_page->ofs);

	if (file_read(file_page->file, load_frame->kva, file_page->page_read_bytes) != (int)file_page->page_read_bytes)
	{
		palloc_free_page(load_frame->kva);
		free(aux);
		return false;
	}

	memset(load_frame->kva + file_page->page_read_bytes, 0, PGSIZE - file_page->page_read_bytes);
	free(aux);
	return true;
}
// void *
// do_mmap(void *addr, size_t length, int writable,
// 		struct file *file, off_t offset)
// {
// 	size_t read_bytes = length;
// 	void *init_addr = addr;
// 	while (read_bytes > 0)
// 	{
// 		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;

// 		struct info *aux = (struct info *)malloc(sizeof(struct info));
// 		aux->file = file;
// 		aux->offset = offset;
// 		aux->read_bytes = page_read_bytes;

// 		if (!vm_alloc_page_with_initializer(VM_FILE, addr, writable, lazy_load_segment, aux))
// 		{
// 			return;
// 		}

// 		read_bytes -= page_read_bytes;
// 		addr += PGSIZE;
// 		offset += page_read_bytes;
// 	}
// 	return init_addr;
// }

void *
do_mmap(void *addr, size_t length, int writable,
		struct file *file, off_t offset)
{
	size_t read_bytes = length;
	void *init_addr = addr;
	while (read_bytes > 0)
	{
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;

		if (spt_find_page(&thread_current()->spt, addr))
		{
			while (init_addr < addr)
			{
				struct page *page = spt_find_page(&thread_current()->spt, addr);
				spt_remove_page(&thread_current()->spt, page);
				init_addr += PGSIZE;
			}
		}
		struct info *aux = (struct info *)malloc(sizeof(struct info));
		aux->file = file;
		aux->offset = offset;
		aux->read_bytes = page_read_bytes;

		if (!vm_alloc_page_with_initializer(VM_FILE, addr, writable, lazy_mmap, aux))
		{
			free(aux);
			while (init_addr < addr)
			{
				struct page *page = spt_find_page(&thread_current()->spt, addr);
				spt_remove_page(&thread_current()->spt, page);
				init_addr += PGSIZE;
			}
		}

		read_bytes -= page_read_bytes;
		addr += PGSIZE;
		offset += page_read_bytes;
	}
	return init_addr;
}
/* Do the munmap */
void do_munmap(void *addr)
{
	struct thread *cur = thread_current();
	struct page *page = spt_find_page(&cur->spt, addr);
	if (page == NULL)
		return;
	if (!page->file.file)
		return;
	while (page != NULL)
	{
		if (pml4_is_dirty(cur->pml4, addr))
		{
			lock_acquire(&filesys_lock);
			file_seek(page->file.file, page->file.ofs);
			file_write(page->file.file, page->frame->kva, page->file.page_read_bytes);
			lock_release(&filesys_lock);
			pml4_set_dirty(cur->pml4, addr, false);
		}
		pml4_clear_page(cur->pml4, addr);
		addr += PGSIZE;
		page = spt_find_page(&cur->spt, addr);
	}
}

void exit_do_munmap(struct supplemental_page_table *cur_spt){
   struct hash_iterator hash_iter;
	struct page *page_exit;
   	struct supplemental_page_table *spt = cur_spt;
	hash_first (&hash_iter, &spt->vm);
	while (hash_next (&hash_iter)) {
		page_exit = hash_entry (hash_cur (&hash_iter), struct page, h_elem);
		if(page_exit->operations->type == VM_FILE)
			do_munmap(page_exit->va);
	}
}