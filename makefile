# super simple makefile
# call it using 'make NAME=name_of_code_file_without_extension'
# (assumes a .cpp extension)
NAME = "a1"

#
# Add $(MAC_OPT) to the compile line for Mac OSX.



MAC_OPT = "-I/opt/X11/include"

all:
	@echo "Compiling..."
	g++ -o $(NAME) $(NAME).cpp -L/usr/X11R6/lib -lX11 -lstdc++ $(MAC_OPT) -Dsina=1 
	#if you want to be able to pass in command line arguments for Frame/second and the speed of ball
	#remove -Dsina=1 from the compiling line


	@echo "Running..."
	./$(NAME)


clean:
	-rm a1