OBJS=pfind.o sfind.o

all: $(OBJS)


# %  模式
# $< 规则的第一个依赖文件	这里是%.c
# $@ 规则的目标文件  	   这里是%.out
# $^ 规则的所有依赖文件    这里包括%.c和它依赖的.h文件
# -c表明只编译不链接, 如果用-o %.c 是编译并链接
%.o: %.c
	gcc -lpthread -g $< -o $@

clean:
	rm -f ./*.out ./*.o