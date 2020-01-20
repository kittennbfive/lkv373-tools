OBJ_NDS32=anal_nds32.o
OBJ_NDS32+=../../asm/arch/nds32/decode_disassm16.o
OBJ_NDS32+=../../asm/arch/nds32/decode_disassm32.o
OBJ_NDS32+=../../asm/arch/nds32/opc32.o
OBJ_NDS32+=../../asm/arch/nds32/opc16.o
OBJ_NDS32+=../../asm/arch/nds32/sign_extend.o

STATIC_OBJ+=${OBJ_NDS32}
TARGET_NDS32=anal_nds32.${EXT_SO}

ALL_TARGETS+=${TARGET_NDS32}

${TARGET_NDS32}: ${OBJ_NDS32}
	${CC} ${LDFLAGS} ${CFLAGS} -o anal_nds32.${EXT_SO} ${OBJ_NDS32}
