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

choose:
  subu $sp, $sp, 24
  sw $ra, 20($sp)
  sw $fp, 16($sp)
  addi $fp, $sp, 24
  sw $a0, -12($fp)
  sw $a1, -16($fp)
  bgt $a0,$a1,label1
  j label2
label1:
  lw $t1, -12($fp)
  move $t0,$t1
  lw $ra, 20($sp)
  lw $fp, 16($sp)
  addi $sp, $sp, 24
  move $v0, $t0
  jr, $ra
  sw $t1, -12($fp)
  sw $t0, -20($fp)
  j label3
label2:
  lw $t1, -16($fp)
  move $t0,$t1
  lw $ra, 20($sp)
  lw $fp, 16($sp)
  addi $sp, $sp, 24
  move $v0, $t0
  jr, $ra
  sw $t1, -16($fp)
  sw $t0, -24($fp)
label3:
main:
  subu $sp, $sp, 36
  sw $ra, 32($sp)
  sw $fp, 28($sp)
  addi $fp, $sp, 36
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $t0, $v0
  sw $t0, -12($fp)
  lw $t1, -12($fp)
  move $t0,$t1
  sw $t0, -16($fp)
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $t0, $v0
  sw $t1, -12($fp)
  sw $t0, -20($fp)
  lw $t1, -20($fp)
  move $t0,$t1
  lw $t2, -16($fp)
  move $a0, $t2
  move $a1, $t0
  sw $t2, -16($fp)
  sw $t1, -20($fp)
  sw $t0, -24($fp)
  jal choose
  addi $sp, $sp, 0
  move $t0, $v0
  sw $t0, -28($fp)
  lw $t1, -28($fp)
  move $t0,$t1
  move $a0, $t0
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  sw $t1, -28($fp)
  sw $t0, -32($fp)
  li $t0, 0
  lw $ra, 32($sp)
  lw $fp, 28($sp)
  addi $sp, $sp, 36
  move $v0, $t0
  jr, $ra
  sw $t0, -36($fp)
