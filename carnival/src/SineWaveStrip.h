// ---------- Mesh Container ----------
typedef struct {
    GLuint vao;
    GLuint vbo_pos;
    GLuint vbo_nrm;
    GLuint ebo;
    GLsizei indexCount;

    float* positions;
    float* normals;
} SineWaveMesh;


void drawSineWaveMesh(void);
