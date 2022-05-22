.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
  li $v0, 4
  la $a0, _prompt
  syscall
  li $v0, 5
  syscall
  jr $ra

write:
  li $v0, 1
  syscall
  li $v0, 4
  la $a0, _ret
  syscall
  move $v0, $0
  jr $ra

main:
  subu $sp, $sp, 56
  sw $ra, 52($sp)
  sw $fp, 48($sp)
  addi $fp, $sp, 56
  la $t0, -16($fp)
  li $t1, 0
  li $t8, 4
  mul $t2, $t1, $t8
  add $t1, $t0, $t2
  move $t2,$t1
  li $t3, 1
  sw $t3,0($t2)
  la $t4, -16($fp)
  li $t5, 0
  li $t8, 4
  mul $t6, $t5, $t8
  add $t5, $t4, $t6
  lw $t6, 0($t5)
  move $a0, $t6
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  sw $t1, -20($fp)
  sw $t1, -24($fp)
  sw $t2, -28($fp)
  sw $t2, -32($fp)
  sw $t3, -36($fp)
  sw $t5, -40($fp)
  sw $t5, -44($fp)
  sw $t6, -48($fp)
  sw $t6, -52($fp)
  li $t1, 0
  lw $ra, 52($sp)
  lw $fp, 48($sp)
  addi $sp, $sp, 56
  move $v0, $t1
  jr, $ra
  sw $t1, -56($fp)
