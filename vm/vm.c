/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include <string.h>
#define ONE_MB (1 << 20) // 1MB
/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
	
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
	list_init(&frame_list);
	start_clock = list_begin(&frame_list);
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
/*
	TODO:
	인자로 전달받은 적절한 초기화 함수를 가져오고 이 함수를 인자로 갖는 uninit_new함수를 호출
	주어진 type의 페이지 생성. 초기화되지 않은 페이지의 swap_in 핸들러는 
	자동적으로 페이지 타입에 맞게 페이지를 초기화하고 주어진 AUX를 인자고 삼는 INIT 함수를 호출함.
	페이지 구조체를 갖게되면 페이지를 spt에 삽입.
*/
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;
	/* Check wheter the upage is already occupied or not. */

err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	/* TODO: Fill this function. */
	// struct page *page = malloc(sizeof(struct page));

}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	bool succ = false;
	/* TODO: Fill this function. */

}
void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame* vm_get_victim(void)
{
    struct list_elem* victim_elem;
    struct frame* victim;
    
    while (true) {
        victim_elem = list_pop_front(&frame_list);
        victim = list_entry(victim_elem, struct frame, frame_elem);

        if (!pml4_is_accessed(thread_current()->pml4, victim->page->va))
            break;

        pml4_set_accessed(thread_current()->pml4, victim->page->va, 0);
        list_push_back(&frame_list, victim_elem);
    }

    return victim;
}


/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame(void)
{
	// TODO: 희생자 올리기
	struct frame *victim UNUSED = vm_get_victim();
	/* TODO: swap out the victim and return the evicted frame. */
	swap_out(victim->page);

	return victim;
}


/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth(void *addr UNUSED)
{

}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool 
vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED,
						 bool user UNUSED, bool write UNUSED, bool not_present UNUSED)
{
	struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	return swap_in (page, frame->kva);

}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init(&spt->vm, vm_hash_func, vm_less_func, NULL);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

void hash_destroy_func(struct hash_elem *e, void *aux){

}
/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	
}

static unsigned vm_hash_func (const struct hash_elem *e, void *aux){
	/* hash_entry()로 element에 대한 page 구조체 검색 */
	/* hash_int()를 이용해서 vm_entry의 멤버 vaddr에 대한 해시값을 구하고 반환*/
	struct page *page = hash_entry(e,struct page,h_elem);

	return hash_int(page->va);
}

static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux){
	/* hast_entry()로 각각의 element에 대한 vm_entry 구조체를 얻은 후 vaddr 비교 (b가 크면 true, a가 크면 false)*/
	struct page *page_a = hash_entry(a,struct page,h_elem);
	struct page *page_b = hash_entry(b,struct page,h_elem);

	return page_a->va < page_b->va;
}