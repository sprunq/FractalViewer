OBJS = Source.o Fractal.o
CXX = g++
CXXFLAGS = -std=c++14 -fopenmp 
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network
LDFLAGS = -fopenmp

fviewer: $(OBJS)
	$(CXX) -o fractalviewer.out $(OBJS) $(LDLIBS) $(LDFLAGS)

Source.o: Source.cpp ArialFont.h Fractal.cpp

clean:
	$(RM) fractalviewer.out $(OBJS)
