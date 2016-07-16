CXXFLAGS =	-std=c++0x -O2 -g -Wall -fno-strict-aliasing -fmessage-length=0 -static-libgcc -static-libstdc++

MAIN = main.o

OBJS = fooerrors.o\
	   LibA.o\
	   test_error_id.o\
	   test_error_id_tmp.o\
	   test_typed_error.o  

DUMPOBJS =

LIBS =

ifeq ($(OS),Windows_NT)
	TESTS =	Default/errorcodeNX.exe
else
	TESTS =	Default/errorcodeNX
endif

TARGETS = $(TESTS)

$(TESTS): $(MAIN) $(OBJS)
	$(CXX) -o $(TESTS) $(MAIN) $(OBJS) $(LIBS) $(CXXFLAGS)



error_id.o: error_id.hpp
fooerrors.o: error_id.hpp
LibA.o: error_id.hpp
main.o: error_id.hpp
test_error_id.o: error_id.hpp
test_error_id_tmp.o: error_id.hpp
test_typed_error.o: error_id.hpp

all:	$(TARGETS)

clean:
	rm -f $(MAIN) $(OBJS) $(TARGETS)

	
format:
ifeq ($(OS),Windows_NT)
	"c:\Program Files (x86)\LLVM\bin\clang-format.exe" -i fooerrors.cpp\
															LibA.cpp\
															main.cpp\
															test_error_id.cpp\
															test_error_id_tmp.cpp\
															test_typed_error.cpp
		
else
	clang-format-3.5 -i  *.h *.cpp *.hpp # was -style=Webkit
	git status
endif

restore:
	git checkout -- *.h *.hpp *.cpp
	git status


test: $(TESTS)
	./$(TESTS)

