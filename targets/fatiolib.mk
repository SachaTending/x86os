fatiolib = drivers/fatiolib

CFLAGS += -I $(fatiolib)/include/
OBJ += $(fatiolib)/fat_access.o $(fatiolib)/fat_cache.o $(fatiolib)/fat_filelib.o $(fatiolib)/fat_format.o
OBJ += $(fatiolib)/fat_misc.o $(fatiolib)/fat_string.o $(fatiolib)/fat_table.o $(fatiolib)/fat_write.o