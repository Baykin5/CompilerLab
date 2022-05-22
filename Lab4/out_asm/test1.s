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
  subu $sp, $sp, 44
  sw $ra, 40($sp)
  sw $fp, 36($sp)
  addi $fp, $sp, 44
  li $t0, 0
  sw $t0, -12($fp)
  li $t0, 1
  sw $t0, -16($fp)
  li $t0, 0
  sw $t0, -20($fp)
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $t0, $v0
  sw $t0, -24($fp)
  lw $t1, -24($fp)
  move $t0,$t1
  sw $t0, -28($fp)
  sw $t1, -24($fp)
label1:
  lw $t0, -20($fp)
  lw $t1, -28($fp)
  sw $t0, -20($fp)
  sw $t1, -28($fp)
  blt $t0,$t1,label2
  j label3
label2:
  lw $t0, -12($fp)
  lw $t1, -16($fp)
  add $t2, $t0, $t1
  move $t0,$t2
  sw $t0, -36($fp)
  move $a0, $t1
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  sw $t0, -12($fp)
  sw $t1, -16($fp)
  sw $t2, -32($fp)
  lw $t1, -16($fp)
  move $t0,$t1
  sw $t0, -12($fp)
  lw $t0, -36($fp)
  move $t1,$t0
  sw $t1, -16($fp)
  lw $t1, -20($fp)
  addi $t2, $t1, 1
  move $t1,$t2
  sw $t1, -20($fp)
  sw $t0, -36($fp)
  sw $t2, -40($fp)
  j label1
label3:
  li $t0, 0
  lw $ra, 40($sp)
  lw $fp, 36($sp)
  addi $sp, $sp, 44
  move $v0, $t0
  jr, $ra
  sw $t0, -44($fp)
