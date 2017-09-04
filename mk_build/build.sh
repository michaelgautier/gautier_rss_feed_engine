reset 

g++ -std=c++14 -c -fPIC -g -I../src/ -o icmw.o ../src/icmw.cxx
g++ -std=c++14 -c -fPIC -g -I../src/ -I/usr/include/libxml2 -o gautier_rss_model.o ../src/gautier_rss_model.cxx
g++ -std=c++14 -c -fPIC -g -I../src/ -o gautier_rss.o ../src/main.cxx

g++ -g -I../src/ -I/usr/include/libxml2 -lxml2 -lsqlite3 -lfltk -o gautier_rss gautier_rss_model.o gautier_rss.o icmw.o

