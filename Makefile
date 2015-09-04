format:
	clang-format-3.5 -i -style=Google*.h *.cpp *.hpp
	git status

restore:
	git checkout -- *.h *.hpp *.cpp
	git status

test:
	Default/errorcodeNX