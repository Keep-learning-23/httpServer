target = test
depend = test_main.o

$(target) : $(depend)
	g++ -o $(target) $(depend)
$(depend) : test_main.cpp
	g++ -c test_main.cpp -o $(depend)

clean :
	rm -rf $(target) $(depend)