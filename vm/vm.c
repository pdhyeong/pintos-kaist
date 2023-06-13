/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "hash.h"
#include "vaddr.h"

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
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
									vm_initializer *init, void *aux)
{

	ASSERT(VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current()->spt;
	/* Check wheter the upage is already occupied or not. */
	struct page *find_page = spt_find_page(spt, upage);
	if (find_page == NULL)
	{
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		/*
		(struct page *page, void *va, vm_initializer *init,
		enum vm_type type, void *aux,
		bool (*initializer)(struct page *, enum vm_type, void *))
		*/
		/* TODO: Insert the page into the spt. */
		struct page *new_page = malloc(sizeof(struct page));
		switch (VM_TYPE(type))
		{
		case VM_ANON:
			uninit_new(new_page, upage, init, type, NULL, anon_initializer);
			break;
		case VM_FILE:
			uninit_new(new_page, upage, init, type, NULL, file_backed_initializer);
			break;
		}
		new_page->writable = writable;
		/* TODO: Insert the page into the spt. */
		spt_insert_page(spt, new_page);
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED)
{
	struct page *page = NULL;
	/* TODO: Fill this function. */
	page = malloc(sizeof(struct page));
	page->va = pq_round_down(va);
	struct hash_elem *h = hash_find(&spt->vm, &page->h_elem);
	if (h == NULL)
	{
		free(page);
		return NULL;
	}
	free(page);
	page = hash_entry(h, struct page, h_elem);
	return page;
}

/* Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
					 struct page *page UNUSED)
{
	int succ = false;
	/* TODO: Fill this function. */
	/*없으면 insert 됨 */
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
static struct frame *
vm_get_victim(void)
{
	struct frame *victim = NULL;
	/* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame(void)
{
	struct frame *victim UNUSED = vm_get_victim();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame(void)
{
	struct frame *frame = palloc_get_page(PAL_ZERO);
	if (frame == NULL)
	{
		PANIC("todo");
	}
	/* TODO: Fill this function. */
	ASSERT(frame != NULL);
	ASSERT(frame->page == NULL);
	/*
	palloc_get_page 함수를 호출함으로써 당신의 메모리 풀에서 새로운 물리메모리 페이지를 가져옵니다.
	유저 메모리 풀에서 페이지를 성공적으로 가져오면,
	프레임을 할당하고 프레임 구조체의 멤버들을 초기화한 후 해당 프레임을 반환합니다.
	당신이 frame *vm_get_frame  함수를 구현한 후에는 모든 유저 공간 페이지들을 이 함수를 통해 할당해야 합니다.
	지금으로서는 페이지 할당이 실패했을 경우의 swap out을 할 필요가 없습니다. 일단 지금은 PANIC ("todo")으로 해당 케이스들을 표시해 두십시오.*/
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth(void *addr UNUSED)
{
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
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page(page);
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
	struct page *page = spt_find_page(&thread_current()->spt, va);
	/* TODO: Fill this function */
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
	// 매핑 해주고 테이블에 올리기
	if (spt_insert_page(&thread_current()->spt, page))
	{
		return swap_in(page, frame->kva);
	}
	else
	{
		return false;
	}
}
/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt UNUSED)
{
	hash_init(&spt->vm, vm_hash_func, vm_less_func, NULL);
}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
								  struct supplemental_page_table *src UNUSED)
{
}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED)
{
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}
static unsigned vm_hash_func(const struct hash_elem *e, void *aux)
{
	struct page *curpage = hash_entry(e, struct page, h_elem);
	return hash_int(curpage->va);
}
static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
	struct page *comp_a = hash_entry(a, struct page, h_elem);
	struct page *comp_b = hash_entry(b, struct page, h_elem);

	return comp_a->va < comp_b->va;
}