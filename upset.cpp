
#include <stdio.h>
#include <string.h>

#include "nall/filemap.hpp"
#include "nall/ups.hpp"

#define abort(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while (0)

static int ups_create(char* pristine_path, char* patched_path) {

  if (!nall::file::exists(pristine_path)) abort("%s\n", "Pristine file does not exist");
  if (!nall::file::exists(patched_path)) abort("%s\n", "UPS patch file does not exist");

  nall::string output_path = nall::string(nall::basename(pristine_path), ".ups");

  nall::ups ups_handle;
  nall::filemap pristine_file(pristine_path, nall::filemap::mode::read);
  nall::filemap patched_file(patched_path, nall::filemap::mode::read);

  nall::ups::result result = ups_handle.create(
      pristine_file.data(), pristine_file.size(),
      patched_file.data(), patched_file.size(),
      output_path);

  if(result != nall::ups::result::success) {
    unlink(output_path);
    abort("error creating the UPS patch\n");
  }

  printf("UPS patch created in %s\n", output_path());
  return 0;
}

static int ups_is_good(char* ups_file){
  // TODO : implement !
  return 1;
}

static int ups_apply(char* pristine_path, char* ups_path) {

  if (!nall::file::exists(pristine_path)) abort("%s\n", "Target file does not exist");
  if (!nall::file::exists(ups_path)) abort("%s\n", "UPS patch file does not exist");
  if (!ups_is_good(ups_path)) abort("%s\n", "Invalid UPS patch file");

  nall::string output_path = nall::string(nall::dir(pristine_path), "patched_", nall::notdir(pristine_path));

  unsigned patched_size = 0;

  nall::ups ups_handle;
  nall::filemap pristine_file(pristine_path, nall::filemap::mode::read);
  nall::filemap ups_file(ups_path, nall::filemap::mode::read);

  ups_handle.apply(
      ups_file.data(), ups_file.size(),
      pristine_file.data(), pristine_file.size(),
      0, patched_size);

  nall::file tmp_file;
  if (!tmp_file.open(output_path, nall::file::mode::write)) {
    abort("error opening the output %s\n", output_path);
  }
  tmp_file.seek(patched_size);
  tmp_file.close();

  nall::filemap output_file(output_path, nall::filemap::mode::readwrite);
  nall::ups::result result = ups_handle.apply(
      ups_file.data(), ups_file.size(),
      pristine_file.data(), pristine_file.size(),
      output_file.data(),
      patched_size = output_file.size());

  ups_file.close();
  pristine_file.close();
  output_file.close();

  if(result != nall::ups::result::success) {
    unlink(output_path);
    abort("error applying the patching\n");
  }

  printf("UPS patch applyed in %s\n", output_path());
  return 0;
}

// --------------------------------------------------------------------------------

#define CREATE_FLAG "-c"
#define APPLY_FLAG "-a"

static char* appname = (char*) "upset";

void print_usage(void) {
  printf("Usage:\n");
  printf("  %s %s  pristine_file  patched_file\n", appname, CREATE_FLAG);
  printf("  %s %s  pristine_file  ups_file\n", appname, APPLY_FLAG);
}

int main(int argc, char** argv) {
  if (argc > 0) appname = argv[0];
  if (argc < 3) {
    print_usage();
    return 1;
  }
  if (!strcmp(argv[1], CREATE_FLAG)) return ups_create(argv[2], argv[3]);
  if (!strcmp(argv[1], APPLY_FLAG)) return ups_apply(argv[2], argv[3]);
  fprintf(stderr, "invalid mode '%s'\n", argv[3]);
  print_usage();
  return -1;
}

