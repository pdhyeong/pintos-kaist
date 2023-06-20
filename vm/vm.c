/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "include/threads/mmu.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void vm_init(void)
{
	vm_anon_init();
	vm_file_init();

#ifdef EFILESYS /* For project 4 */
	pagecache_init();
#endif
	register_inspect_intr();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type(struct page *page)
{
	int ty = VM_TYPE(page->operations->type);
	switch (ty)
	{
	case VM_UNINIT:
		return VM_TYPE(page->uninit.type);
	default:
		return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

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
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
									vm_initializer *init, void *aux)
{

	ASSERT(VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current()->spt;
	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page(spt, upage) == NULL)
	{
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		struct page *p = (struct page *)malloc(sizeof(struct page));
		if (p == NULL)
		{
			return false;
		}
		switch (VM_TYPE(type))
		{
		case VM_ANON:
			uninit_new(p, upage, init, type, aux, anon_initializer);
			break;
		case VM_FILE:
			uninit_new(p, upage, init, type, aux, file_backed_initializer);
			break;
		default:
			break;
		}
		p->writable = writable;
		/* TODO: Insert the page into the spt. */
		return spt_insert_page(spt, p);
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED)
{
	/* TODO: Fill this function. */
	// struct page *page = malloc(sizeof(struct page));
	struct page page;
	// page->va = pg_round_down (va);
	page.va = pg_round_down(va);
	struct hash_elem *founded_hash_elem = hash_find(&spt->vm, &page.h_elem);
	if (founded_hash_elem == NULL)
		return NULL;
	return hash_entry(founded_hash_elem, struct page, h_elem);
}

/* Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
					 struct page *page UNUSED)
{
	bool succ = false;
	/* TODO: Fill this function. */
	if (hash_insert(&spt->vm, &page->h_elem) == NULL)
	{
		succ = true;
	}
	return succ;
}

void spt_remove_page(struct supplemental_page_table *spt, struct page *page)
{
	vm_dealloc_page(page);
	return true;
}

/* Get the struct frame, that will be evicted. */
/*
bool pml4_is_accessed (uint64_t *pml4, const void *upage);
void pml4_set_accessed (uint64_t *pml4, const void *upage, bool accessed);
사용
*/
static struct frame *
vm_get_victim(void)
{
	// //TODO: 희생자 선택
	// struct frame *victim = NULL;
	// /* TODO: The policy for eviction is up to you. */
	// struct thread *cur_thread = thread_current();
	// struct list_elem *e = &victim->f_elem;
	// for (e = list_begin (&frame_list); e != list_end (&frame_list); e = list_next (&frame_list)){
	// 	victim = list_entry(e,struct frame,f_elem);
	// 	if(pml4_is_accessed(cur_thread->pml4,victim->page->va)){
	// 		pml4_set_accessed(cur_thread->pml4,victim->page->va,0);
	// 	}
	// 	else{
	// 		return victim;
	// 	}
	// }
	// for (e = list_begin (&frame_list); e != list_end (&frame_list); e = list_next (&frame_list)){
	// 	victim = list_entry(e,struct frame,f_elem);
	// 	if(pml4_is_accessed(cur_thread->pml4,victim->page->va)){
	// 		pml4_set_accessed(cur_thread->pml4,victim->page->va,0);
	// 	}
	// }
	// return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame(void)
{
	// TODO: 희생자 올리기
	// struct frame *victim UNUSED = vm_get_victim();
	/* TODO: swap out the victim and return the evicted frame. */

	// return swap_out(victim->page);
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame(void)
{
	struct frame *frame = NULL;
	/* TODO: Fill this function. */
	frame = malloc(sizeof(struct frame));
	frame->kva = palloc_get_page(PAL_USER);
	if (frame->kva == NULL)
	{
		frame->page = NULL;
		PANIC("todo");
	}
	frame->page = NULL;
	// list_push_back(&frame_list,&frame->f_elem);
	ASSERT(frame != NULL);
	ASSERT(frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static bool
vm_stack_growth(void *addr UNUSED)
{
	if(vm_alloc_page(VM_ANON, addr, 1)){
		thread_current()->save_stack_bottom -= PGSIZE;
		return true;
	}
	return false;
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp(struct page *page UNUSED)
{
}

/* Return true on success */
bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED,
						 bool user UNUSED, bool write UNUSED, bool not_present UNUSED)
{
	struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */
	if (is_kernel_vaddr(addr) || !addr)
	{
		return false;
	}
	if (not_present)
	{
		if (addr >= USER_STACK - ONE_MB && addr <= USER_STACK && f->rsp - (1<<3) <= addr && thread_current()->save_stack_bottom >= addr)
		{
			addr = thread_current()->save_stack_bottom - PGSIZE;
			vm_stack_growth(addr);
		}
	}
	return vm_claim_page(addr);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void vm_dealloc_page(struct page *page)
{
	destroy(page);
	free(page);
}

/* Claim the page that allocate on VA. */
bool vm_claim_page(void *va UNUSED)
{
	struct page *page = NULL;
	/* TODO: Fill this function */
	page = spt_find_page(&thread_current()->spt, va);
	if (page == NULL)
	{
		return false;
	}
	return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page(struct page *page)
{
	struct frame *frame = vm_get_frame();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	if (install_page(page->va, frame->kva, page->writable))
	{
		return swap_in(page, frame->kva);
	}
	return false;
}

/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt UNUSED)
{
	hash_init(&spt->vm, vm_hash_func, vm_less_func, NULL);
}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED, struct supplemental_page_table *src UNUSED)
{
	struct hash_iterator hash_iter;
	hash_first(&hash_iter, &src->vm);
	bool success = false;
	while (hash_next(&hash_iter))
	{
		struct page *parent_page = hash_entry(hash_cur(&hash_iter), struct page, h_elem);
		success = vm_alloc_page_with_initializer(parent_page->uninit.type, parent_page->va, parent_page->writable, parent_page->uninit.init, parent_page->uninit.aux);
		if (!success)
			return false;
		struct page *child_page = spt_find_page(dst, parent_page->va);
		if (parent_page->frame)
		{
			success = vm_do_claim_page(child_page);
			if (!success)
			{
				return false;
			}
			memcpy(child_page->frame->kva, parent_page->frame->kva, PGSIZE);
		}
	}
	return success;
}

void hash_destroy_func(struct hash_elem *e, void *aux)
{
	struct page *page = hash_entry(e, struct page, h_elem);
	vm_dealloc_page(page);
}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED)
{
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	hash_destroy(&spt->vm, hash_destroy_func);
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux)
{
	/* hash_entry()로 element에 대한 page 구조체 검색 */
	/* hash_int()를 이용해서 vm_entry의 멤버 vaddr에 대한 해시값을 구하고 반환*/
	struct page *p = hash_entry(e, struct page, h_elem);
	return hash_int(p->va);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
	/* hast_entry()로 각각의 element에 대한 vm_entry 구조체를 얻은 후 vaddr 비교 (b가 크면 true, a가 크면 false)*/
	struct page *page_a = hash_entry(a, struct page, h_elem);
	struct page *page_b = hash_entry(b, struct page, h_elem);
	return page_b->va > page_a->va;
}