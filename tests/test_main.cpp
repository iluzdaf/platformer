#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>

#include "test_helpers/gl_context.hpp"

int main(int argc, char *argv[])
{
#ifndef SKIP_OPENGL_TESTS
    initOpenGLForTests();
#endif
    return Catch::Session().run(argc, argv);
}