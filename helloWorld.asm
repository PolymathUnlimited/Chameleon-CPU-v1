// hello world program

console = 0xfeff

	LOD !(message % 256)
	STO str_ptr
	LOD !(message / 256)
	STO str_ptr + 1
	JSR write_str
	HLT

// subroutine to output a string to the console
str_ptr: .reserve 2
write_str:
	LOD str_ptr
	STO lod_inst + 1
	LOD str_ptr + 1
	STO lod_inst + 2
lod_inst:
	LOD 0
	ADD !0
	BNZ continue_write_str
	RSR
continue_write_str:
	STO console
	LOD str_ptr
	ADD !1
	STO str_ptr
	BNC write_str_no_carry
	LOD str_ptr + 1
	ADD !1
	STO str_ptr + 1
write_str_no_carry:
	JMP write_str

message:	.string "Hello world!" // hello world string


