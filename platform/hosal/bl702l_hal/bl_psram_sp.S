#if __riscv_xlen == 64
	#define store_x sd
	#define load_x ld
#elif __riscv_xlen == 32
	#define store_x sw
	#define load_x lw
#else
	#error Assembler did not define __riscv_xlen
#endif

.global update_sp
.global resotre_sp
.extern __psram_taskstack_spn_top
.extern psram_taskstack_spo

/*-----------------------------------------------------------*/

.align 8
.func
update_sp:
	addi sp, sp, -12                /* backup t0/t1/t2 in task stack */
	sw t0, 0(sp)
	sw t1, 4(sp)
	sw t2, 8(sp)

	mv t2, sp                       /* record sp vale of task stack in t2 */

	la t0, psram_taskstack_spo
	load_x sp, __psram_taskstack_spn_top            /* switch to temperay stack */

	addi t2, t2, 12                 /* mark end of task stack in t2 without t0/t1/t2 backup */
	addi t1, t2, 32                 /* copy 8x4 to temporay stack */
	store_x t1, 0(t0)               /* backup (sp - 8) in psram_taskstack_spo */

update_sp_loop:                     /* copy 8x4 from task stack to temporay stack */
	beq t1, t2, update_sp_loop_end
	addi sp, sp, -4
	addi t1, t1, -4
	load_x t0, 0(t1)
	store_x t0, 0(sp)
	j update_sp_loop

update_sp_loop_end:                 /* recover t0/t1/t2 from task stack */
    addi t2, t2, -12
	load_x t0, 0(t2)
	load_x t1, 4(t2)
	load_x t2, 8(t2)
	ret
	.endfunc
/*-----------------------------------------------------------*/

.align 8
.func
resotre_sp:
	addi sp, sp, -12                /* backup t0/t1/t2 in temporay stack */
	sw t0, 0(sp)
	sw t1, 4(sp)
	sw t2, 8(sp)

	mv t2, sp                       /* record sp vale of temporay stack in t2 */

	la t0, psram_taskstack_spo
	load_x sp, 0(t0)                /* switch to task stack */

	addi t2, t2, 12                 /* mark end of temporay stack in t2 without t0/t1/t2 backup */
	load_x t1, __psram_taskstack_spn_top            /* mark top of temporay stack in t1 */

restore_sp_loop:                    /* copy contents in to temporay stack to task stack */
	beq t1, t2, restore_sp_loop_end
	addi sp, sp, -4
	addi t1, t1, -4
	load_x t0, 0(t1)
	store_x t0, 0(sp)
	j update_sp_loop

restore_sp_loop_end:                /* recover t0/t1/t2 from task stack */
    addi t2, t2, -12
	load_x t0, 0(t2)
	load_x t1, 4(t2)
	load_x t2, 8(t2)
	ret
	.endfunc
/*-----------------------------------------------------------*/
