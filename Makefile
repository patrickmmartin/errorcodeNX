format:
	clang-format-3.5 -i  *.h *.cpp *.hpp # was -style=Webkit
	git status

format.win:
	"c:\Program Files (x86)\LLVM\bin\clang-format.exe" -i  *.h *.cpp *.hpp 
	git status

restore:
	git checkout -- *.h *.hpp *.cpp
	git status

test:
	Default/errorcodeNX