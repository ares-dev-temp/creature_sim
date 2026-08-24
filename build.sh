mkdir -p build

clang -c glad.c -I./include -o glad.o

clang++ *.cpp glad.o \
-I./include \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lglfw \
-lcjson \
-framework Cocoa \
-framework OpenGL \
-framework IOKit \
-framework CoreVideo \
-o build/app
