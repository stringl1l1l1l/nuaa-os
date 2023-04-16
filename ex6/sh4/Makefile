OBJS=main.o cmd.o parse.o utest.o

all: $(OBJS)
	gcc -g -o sh4 $(OBJS)

# %  模式
# $< 规则的第一个依赖文件	这里是%.c
# $@ 规则的目标文件  	   这里是%.o
# $^ 规则的所有依赖文件    这里包括%.c和它依赖的.h文件
# -c表明只编译不链接, 如果用-o %.c 是编译并链接
%.o: %.c
	gcc -g -c $< -o $@

# 引入依赖关系
include Makefile.dep

clean:
	rm -f sh4 ./*.o