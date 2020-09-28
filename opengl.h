#ifndef OPENGL_H
#define OPENGL_H

// call this once
void gl_init(int width, int height);

// call this every frame
void gl_new_frame();

void gl_update_texture(unsigned char* buffer);

// free everything
void gl_destroy();

#endif // OPENGL_H
