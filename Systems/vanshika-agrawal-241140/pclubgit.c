#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "pclubgit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (pclubgit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 */

/* pclubgit init
 *
 * - Create .pclubgit directory
 * - Create empty .pclubgit/.index file
 * - Create .pclubgit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int pclubgit_init(void) {
  fs_mkdir(".pclubgit");

  FILE* findex = fopen(".pclubgit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".pclubgit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* pclubgit add <filename>
 * 
 * - Append filename to list in .pclubgit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int pclubgit_add(const char* filename) {
  FILE* findex = fopen(".pclubgit/.index", "r");
  FILE *fnewindex = fopen(".pclubgit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".pclubgit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".pclubgit/.newindex", ".pclubgit/.index");

  return 0;
}


/* pclubgit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int pclubgit_rm(const char* filename) {
  FILE* findex = fopen(".pclubgit/.index", "r");
  FILE *fnewindex = fopen(".pclubgit/.newindex", "w");
  char file[FILENAME_SIZE];
  int found = 0;
  while(fgets(file, sizeof(file), findex)) {
    strtok(file, "\n");
    if (strcmp(file, filename) != 0) {
      fprintf(fnewindex, "%s\n", file);
    }
    else {
      found = 1;
    }
  }
  if(found==0){
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    fclose(findex);
    fclose(fnewindex);
    fs_rm(".pclubgit/.newindex");
    return 1;
  }
  fclose(findex);
  fclose(fnewindex);
  fs_mv(".pclubgit/.newindex", ".pclubgit/.index");

  return 0;
}

/* pclubgit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_pclub = "GO PCLUB!";

int is_commit_msg_ok(const char* msg) {
  if(strstr(msg, go_pclub) != NULL) {
    return 0;
  }
  else{
    fprintf(stderr, "ERROR: Message must contain %s \n", go_pclub);
    return 1;
  }
}

void next_commit_id(char* commit_id) {
  int i = COMMIT_ID_BYTES - 1;
  while (i >= 0) {
    if (commit_id[i] == '6') {
      commit_id[i] = '1';
      break;
    } 
    else if (commit_id[i] == '1') {
      commit_id[i] = 'c';
      break;
    }
    else if(commit_id[i] == '0'){
      commit_id[i]='6';
      break;
    }
    else{
      commit_id[i]='0';
      i--;
    }
  }
  write_string_to_file(".pclubgit/.prev", commit_id);
  
}

int pclubgit_commit(const char* msg) {
  if (is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain %s \n", go_pclub);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".pclubgit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);
  read_string_from_file(".pclubgit/.prev", commit_id, COMMIT_ID_SIZE);
  fs_mkdir(strcat(".pclubgit/", commit_id));
  fs_cp(".pclubgit/.index", strcat(strcat(".pclubgit/", commit_id), "/.index"));
  fs_cp(".pclubgit/.prev", strcat(strcat(".pclubgit/", commit_id), "/.prev"));
  write_string_to_file(strcat(strcat(".pclubgit/", commit_id), "/.msg"), msg);
  FILE* findex = fopen(".pclubgit/.index", "r");
  char file[FILENAME_SIZE];
  while(fgets(file, sizeof(file), findex)) {
    strtok(file, "\n");
    fs_cp(file, strcat(strcat(".pclubgit/", commit_id), file));
  }
  fclose(findex);
  
  printf("None\n");
  return 0;
}

/* pclubgit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int pclubgit_status() {
  printf("Tracked files:\n");
  FILE* file_index = fopen(".pclubgit/.index", "r");
  char tracked_files[FILENAME_SIZE];
  while(fgets(tracked_files, sizeof(tracked_files), file_index)) {
    printf("%s", tracked_files);
  }
  fclose(file_index);
  return 0;
}

/* pclubgit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */

int pclubgit_log() {
  printf("\n");
  char filename[FILENAME_SIZE];
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".pclubgit/.prev", commit_id, COMMIT_ID_SIZE);
  while (strcmp(commit_id, "0000000000000000000000000000000000000000") != 0) {
    char msg[MSG_SIZE];
    read_string_from_file(strcat(strcat(".pclubgit/", commit_id), "/.msg"), msg, MSG_SIZE);
    printf("commit %s\n", commit_id);
    printf("%s\n", msg);
  }


  return 0;
}
