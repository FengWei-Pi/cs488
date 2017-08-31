// #include <stdlib.h>
// #include <OpenAL/al.h>
// #include <OpenAL/alc.h>
// #include <AL/alut.h>
// #include <iostream>
//
// // g++ testal.cpp -lalut -framework OpenAL -o testal
//
// int main(int argc, char** argv) {
//   alutInit (&argc, argv);
//   ALuint stepBuffer = alutCreateBufferFromFile("./Assets/heavy-footstep-mono.wav");
//   ALenum error = alutGetError();
//   if (ALUT_ERROR_NO_ERROR != error) {
//     std::cerr << alutGetErrorString(error) << std::endl;
//     exit(EXIT_FAILURE);
//   }
//   ALuint source;
//   alGenSources (1, &source);
//   alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
//   alSourcef(source, AL_ROLLOFF_FACTOR, 1);
//   alSourcef(source, AL_REFERENCE_DISTANCE, 6);
//   alSourcef(source, AL_MAX_DISTANCE, 15);
//   alSourcei (source, AL_BUFFER, stepBuffer);
//   ALfloat x = 0;
//   while (true) {
//     std::cerr << "x: " << x << std::endl;
//     alSource3f(source, AL_POSITION, x, 0, 0);
//     alSourcePlay (source);
//     alutSleep (0.1);
//     alSourceStop(source);
//
//     x -= 0.2;
//   }
//   alutExit ();
//   return EXIT_SUCCESS;
// }
