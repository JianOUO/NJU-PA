STUID = SA25225171
STUNAME = Jin An

# DO NOT modify the following code!!!
# -q: quiet mode 不显示常规的提交成功消息等输出
# --author: set the author of the commit 格式： 名称 <邮箱>
# --no-verify: bypass pre-commit hooks 确保提交成功
# --allow-empty: allow empty commits 即使工作区没有变化也能创建提交
GITFLAGS = -q --author='tracer-ics2025 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
# 定义一个函数，用于提交代码
# -: 忽略错误 @: 不显示执行的命令本身（静默执行）
# git add $(NEMU_HOME)/.. -A：添加 NEMU 项目根目录的所有变更
# -A: 添加所有文件 --ignore-errors: 忽略添加文件时可能出现的错误
# test: 条件判断命令，-e 选项用于检查文件是否存在
# ( ... )：在子 shell 中执行括号内的命令
# test -e .git/index.lock：检查 git 索引锁是否存在,如果存在，等待 0.1 秒后重试，直到锁被释放,防止多个 Git 操作同时进行导致冲突
# > $(1)
# echo "> $(1)": $(1)是函数参数, 即提交信息msg echo $(STUID) $(STUNAME) && uname -a && uptime：附加信息，包括学号、姓名、系统信息和运行时间
# -F - 从标准输入读取提交信息,对应上一行的echo
# sync：确保所有文件系统的缓冲区数据都写入磁盘，防止提交信息丢失
define git_commit
	-@git add $(NEMU_HOME)/.. -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(echo "> $(1)" && echo $(STUID) $(STUNAME) && uname -a && uptime) | git commit -F - $(GITFLAGS)
	-@sync
endef

# _default：默认目标，当直接运行 make 时执行
# 提示信息：告诉用户需要在子项目目录中运行 make
_default:
	@echo "Please run 'make' under subprojects."

submit:
	git gc
	STUID=$(STUID) STUNAME=$(STUNAME) bash -c "$$(curl -s http://why.ink:8080/static/submit.sh)"

# .PHONY: 声明 default 和 submit 是伪目标，不是实际的文件名，总是执行对应的命令
.PHONY: default submit
