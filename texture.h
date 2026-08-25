
#include <glad/glad.h>

class Texture{
    public:
        unsigned int ID;
        int width, height, channels;

        Texture( const char *file_name, GLenum format, int flip=0 );
};
