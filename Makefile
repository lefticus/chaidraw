INCLUDEDIR=-I../chaiscript/trunk/include/

chaidraw:
	g++ main.cpp  -lSDL -lboost_thread-mt ${INCLUDEDIR}
