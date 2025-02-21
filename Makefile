all: read_fat32 format_fat32

read_fat32: src/read_fat32.cc
	g++ -Wall src/read_fat32.cc -o read_fat32

format_fat32: src/format_fat32.cc
	g++ -Wall src/format_fat32.cc -o format_fat32

clean:
	rm -f read_fat32 format_fat32
