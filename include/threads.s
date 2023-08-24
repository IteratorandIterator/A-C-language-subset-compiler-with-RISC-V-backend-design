.section .text
.global __create_threads
.global __join_threads


# sys_clone = 220
sys_clone = 120
clone_vm = 256
sigchld = 17
__create_threads:
	addi	sp,sp,-32
	sd	s1,8(sp)
	sd	ra,24(sp)
	sd	s0,16(sp)
	addiw	s1,a0,-1
	blez	s1,.L4
	li	s0,0
.L3:
    li  a7,sys_clone
	li	a4,0
	li	a3,0
	li	a2,0
    mv  a1,sp
	# li	a0,(clone_vm | sigchld)
	li	a0,1
	ecall
	bnez	a0,.L5
	addiw	s0,s0,1
	bne	s1,s0,.L3
	ld	ra,24(sp)
	ld	s0,16(sp)
	mv	a0,s1
	ld	s1,8(sp)
	addi	sp,sp,32
	jr	ra
.L5:
	mv	s1,s0
	ld	ra,24(sp)
	ld	s0,16(sp)
	mv	a0,s1
	ld	s1,8(sp)
	addi	sp,sp,32
	jr	ra
.L4:
	ld	ra,24(sp)
	ld	s0,16(sp)
	li	s1,0
	mv	a0,s1
	ld	s1,8(sp)
	addi	sp,sp,32
	jr	ra
	.size	__create_threads, .-__create_threads


sys_waitid = 95
sys_exit = 93
p_all = 0
wexited = 4
__join_threads:
	addi	sp,sp,-16
	sd	s0,0(sp)
	sd	ra,8(sp)
	addiw	a1,a1,-1
	mv	s0,a0
	bne	a1,a0,.L6
	bnez	s0,.L7
.L1:
	ld	ra,8(sp)
	ld	s0,0(sp)
	addi	sp,sp,16
	jr	ra
.L6:
    li  a7,sys_waitid
	li	a3,wexited
	li	a2,0
	li	a1,0
	li	a0,p_all
	ecall
	beqz	s0,.L1
.L7:
    li  a7,sys_exit
	li	a0,0
	ecall
	.size	__join_threads, .-__join_threads

#  .section .text
#  .global __create_threads
#  .global __join_threads

# __create_threads:
# 	addi	sp,sp,-48
# 	sd	ra,40(sp)
# 	sd	s0,32(sp)
# 	addi	s0,sp,48
# 	mv	a5,a0
# 	sw	a5,-36(s0)
# 	lw	a5,-36(s0)
# 	addiw	a5,a5,-1
# 	sw	a5,-36(s0)
# 	lw	a5,-36(s0)
# 	sext.w	a5,a5
# 	bgtz	a5,.L2
# 	li	a5,0
# 	j	.L3
# .L2:
# 	sw	zero,-24(s0)
# 	j	.L4
# .L6:
# 	li  a6,0
# 	li	a4,0
# 	li	a3,0
# 	li	a2,0
# 	mv	a1,sp
# 	li	a0,1
# 	li	a7,120
# 	ecall
# 	mv	a5,a0
# 	sw	a5,-20(s0)
# 	lw	a5,-20(s0)
# 	mv	a0,a5
# 	# li	a0,0
# 	call	putint@plt
# 	li	a0,10
# 	call	putch@plt
# 	lw	a5,-20(s0)
# 	sext.w	a5,a5
# 	beqz	a5,.L5
# 	lw	a5,-24(s0)
# 	j	.L3
# .L5:
# 	lw	a5,-24(s0)
# 	addiw	a5,a5,1
# 	sw	a5,-24(s0)
# .L4:
# 	lw	a4,-24(s0)
# 	lw	a5,-36(s0)
# 	sext.w	a4,a4
# 	sext.w	a5,a5
# 	blt	a4,a5,.L6
# 	lw	a5,-36(s0)
# .L3:
# 	mv	a0,a5
# 	ld	ra,40(sp)
# 	ld	s0,32(sp)
# 	addi	sp,sp,48
# 	jr	ra
# 	.size	__create_threads, .-__create_threads

# sys_waitid = 280
# sys_exit = 1
# p_all = 0
# wexited = 4
# __join_threads:
# 	addi	sp,sp,-16
# 	sd	s0,0(sp)
# 	sd	ra,8(sp)
# 	addiw	a1,a1,-1
# 	mv	s0,a0
# 	bne	a1,a0,.L61
# 	bnez	s0,.L71
# .L11:
# 	ld	ra,8(sp)
# 	ld	s0,0(sp)
# 	addi	sp,sp,16
# 	jr	ra
# .L61:
#     li  a7,sys_waitid
# 	li	a3,wexited
# 	li	a2,0
# 	li	a1,0
# 	li	a0,p_all
# 	ecall
# 	beqz	s0,.L11
# .L71:
#     li  a7,sys_exit
# 	li	a0,0
# 	ecall
# 	.size	__join_threads, .-__join_threads

# .globl main
# .text

# main:
# addi sp,sp,-16
# li	x10,7
# # call putint
# li	a0,4
# call __create_threads
# # li	a0,8
# call putint
# li	a0,3
# li	a1,4
# # call __join_threads
# # li	x10,7
# # call putint
# addi sp,sp,16
# ret
