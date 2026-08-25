#define STB_IMAGE_IMPLEMENTATION

#include "texture.h"
#include "libs/stb_image.h"
#include <GLFW/glfw3.h>

Texture::Texture( const char *file_name, GLenum format, int flip ){
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    //Set texture wrapping/filtering methods (on the current bound texture object)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    stbi_set_flip_vertically_on_load( flip );

    //load and generates the texture
    unsigned char *data = stbi_load( file_name, &width, &height, &channels, 0 );

    if(data){
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data );
        glGenerateMipmap(GL_TEXTURE_2D);
    }else
        printf( "Failed to load Texture" );

    stbi_image_free(data);
}

