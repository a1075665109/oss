all: oss user

clean:
	-rm oss logFile user

dt:
	gcc -o oss.c oss 
	gcc -o user.c user
