format:
	clang-format-3.5 -i  *.h *.cpp *.hpp # was -style=Webkit
	git status

restore:
	git checkout -- *.h *.hpp *.cpp
	git status

test:
	Default/errorcodeNX