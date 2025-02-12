all: read_fat32

read_fat32:
	g++ -Wall src/read_fat32.cc -o read_fat32

clean:
	rm -f read_fat32
