#include <GL/glew.h>
#include "helper.h"

void initShaders() {

  idProgramShader = glCreateProgram();

  idVertexShader   = initVertexShader("flat/shader.vert");
  idFragmentShader = initFragmentShader("flat/shader.frag");

  glAttachShader(idProgramShader, idVertexShader);
  glAttachShader(idProgramShader, idFragmentShader);

  glLinkProgram(idProgramShader);

}

GLuint initVertexShader(const std::string& filename)
{
    std::string shaderSource;

    if (!readDataFromFile(filename, shaderSource)){
        std::cout << "Cannot find file name: " + filename << std::endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    return vs;
}

GLuint initFragmentShader(const std::string& filename)
{
    std::string shaderSource;

    if (!readDataFromFile(filename, shaderSource)) {
        std::cout << "Cannot find file name: " + filename << std::endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    return fs;
}

bool readDataFromFile(const std::string& fileName, std::string &data) {
    std::fstream myfile;

    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open()) {
        std::string curLine;

        while (getline(myfile, curLine)){
            data += curLine;
            if (!myfile.eof())
                data += "\n";
        }

        myfile.close();
    }
    else
        return false;

    return true;
}

void initTexture(char *filename, int *w, int *h, bool isHeightmap)
{
    int width, height;

    unsigned char *raw_image = nullptr;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen( filename, "rb" );
    unsigned long location = 0;
    int i = 0, j = 0;

    if ( !infile )
    {
        printf("Error opening jpeg file %s\n!", filename );
        return;
    }

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error( &jerr );
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress( &cinfo );
    /* this makes the library read from infile */
    jpeg_stdio_src( &cinfo, infile );
    /* reading the image header which contains image information */
    jpeg_read_header( &cinfo, TRUE );
    /* Start decompression jpeg here */
    jpeg_start_decompress( &cinfo );

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc( cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc( cinfo.output_width*cinfo.num_components);
    /* read one scan line at a time */
    while( cinfo.output_scanline < cinfo.image_height )
    {
        jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    if (isHeightmap) {
		printf("Heightmap Texture filename = %s\n", filename);
		glGenTextures(1, &idJpegHeightmap);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idJpegHeightmap);
    }
    else {
		printf("Texture filename = %s\n", filename);
		glGenTextures(1, &idJpegTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, idJpegTexture);
    }
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
	*w = width;
    *h = height;

    glGenerateMipmap(GL_TEXTURE_2D);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    free( row_pointer[0] );
    free( raw_image );
    fclose( infile );

}
