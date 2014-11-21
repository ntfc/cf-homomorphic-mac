#include <stdio.h>
#include "io.h"
#include "logger.h"

cf_errno cf_mpi_read(cf_mpi_t *mpi, const char *s) {
  return cf_mpi_import_char(mpi, s);
}

cf_errno cf_mpis_read(cf_mpi_t **mpis, size_t *n_mpis, const char *fn) {
  FILE *fp = NULL;
  char line[LINE_LEN];
  size_t val_len;
  cf_mpi_t msg;
  cf_errno cf_err = E_NOERR;
  *n_mpis = 0;
  
  // is char* fn a file or a string?
  if(file_exists(fn) == E_NOERR) {
    fp = fopen(fn, "r");
    if(fp == NULL)
      return E_ERRNO_ERROR;
    while(fgets(line, LINE_LEN, fp) != NULL) {
      val_len = strlen(line);
      if(line[val_len - 1] == '\n')
        line[--val_len] = '\0';
      if(val_len > 0) {
        cf_err = cf_mpi_import_char(&msg, line);
        if(cf_err != E_NOERR)
          goto cleanup;
        if((*n_mpis) == 0) {
          *mpis = calloc(1, sizeof(cf_mpi_t));
          //(*dest)[0] = gcry_mpi_copy(msg);
          cf_mpi_copy(&((*mpis)[0]), msg);
        }
        else {
          *mpis = realloc(*mpis, sizeof(cf_mpi_t) * (*(n_mpis) + 1));
          assert(*mpis);
          //(*dest)[*nwritten] = gcry_mpi_copy(msg);
          cf_mpi_copy(&((*mpis)[*n_mpis]), msg);
        }
        (*n_mpis)++;
        //gcry_mpi_release(msg);
        cf_mpi_free(msg);
      }
    }
    fclose(fp);
  }
  else { // maybe fn is a hex string
    *mpis = calloc(1, sizeof(cf_mpi_t));
    if((*mpis) == NULL) return E_NO_MEM;
    cf_err = cf_mpi_import_char(&((*mpis)[0]), fn);
    if(cf_err == E_NOERR)
      *n_mpis = 1;
  }
cleanup:
  if(cf_err != E_NOERR) {
    if((*mpis) != NULL) {
      while((*n_mpis) > 0) {
        cf_mpi_free((*mpis)[--(*n_mpis)]);
      }
      free(*mpis);
      *mpis = NULL;
    }
  }
  return cf_err;
}

//! @brief Write a MPI to a FILE.
void cf_mpi_write(FILE *fp, const cf_mpi_t mpi) {
  if(mpi != NULL && mpi->_alloced > 0)
    gmp_fprintf(fp, "0x%Zx", mpi->mpi);
  else
    gmp_fprintf(fp, "(null)");
}

//! @brief Write an array of MPIs to a FILE.
void cf_mpis_write(FILE *fp, const cf_mpi_t *mpis, size_t n) {
  size_t i;
  if(n > 0) {
    for(i = 0; i < n; i++) {
      cf_mpi_write(fp, mpis[i]);
      fprintf(fp, "\n");
    }
  }
}

//! @brief Dumps a MPI into \c stderr.
void cf_mpi_dump(const cf_mpi_t mpi) {
  cf_mpi_write(stderr, mpi);
}

//! @brief Dumps an array of MPIs into \c stderr.
void cf_mpis_dump(const cf_mpi_t *mpis, size_t n_mpis) {
  size_t i;
  fprintf(stderr, "{\n");
  for(i = 0; i < n_mpis; i++) {
    fprintf(stderr, "  ");
    cf_mpi_dump(mpis[i]);
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "}");
}

static cf_errno _line_parse_var_val(char **var, char **val, char *buf) {
  size_t buflen = strlen(buf);
  if(buf[buflen - 1] == '\n')
    buf[--buflen] = '\0';
  if(buflen > 0) {
    char tokens[] = ": ";
    *var = strtok(buf, tokens);
    *val = strtok(NULL, tokens);
    if(*var == NULL || strlen(*var) != 1)
      return E_FILE_FORMAT;
    if(*val == NULL || strlen(*val) < 1)
      return E_FILE_FORMAT;
    return E_NOERR;
  }
  return E_FILE_FORMAT;
}

cf_errno cf_keys_write(const char *fn, const cf_keys_t keys) {
  char *fn_ek = NULL, *fn_sk = NULL;
  FILE *fp_ek = NULL, *fp_sk = NULL;
  cf_errno cf_err = E_NOERR;
  size_t n = 0;
  if(keys == NULL)
    return E_NO_KEYPAIR;
  if(fn != NULL) { // create file names
    n = strlen(fn) + 1;
    fn_ek = (char*)calloc(sizeof(char), n + 3); // ".ek"
    fn_sk = (char*)calloc(sizeof(char), n + 3); // ".sk"
    strncpy(fn_ek, fn, n - 1);
    strncpy(fn_sk, fn, n - 1);
    strncat(fn_ek, ".ek", 3);
    strncat(fn_sk, ".sk", 3);
    
    fp_ek = fopen(fn_ek, "w");
    fp_sk = fopen(fn_sk, "w");
  }
  else { // write to stdout
    fp_ek = stdout;
    fp_sk = stdout;
  }
  
  if(!fp_ek || !fp_sk) {
    cf_err = E_ERRNO_ERROR;
    goto cleanup;
  }
  cf_err = cf_key_ek_write(fp_ek, keys->ek);
  
  if(cf_err == E_NOERR)
    cf_err = cf_key_sk_write(fp_sk, keys->sk);
cleanup:
  if(fn != NULL) {
    free(fn_ek);
    fclose(fp_ek);
    free(fn_sk);
    fclose(fp_sk);
  }
  return cf_err;
}

cf_errno cf_key_sk_write(FILE *fp, const cf_key_sk_t sk) {
  cf_errno cf_err = E_NOERR;
  
  if(sk == NULL)
    return E_NO_SK_KEY;
  
  fprintf(fp, "k: ");
  cf_mpi_write(fp, SK_K(sk));
  fprintf(fp, "\nx: ");
  cf_mpi_write(fp, SK_X(sk));
  fprintf(fp, "\np: ");
  cf_mpi_write(fp, SK_P(sk));
  fprintf(fp, "\n");
  if(SK_G(sk) != NULL) {
    fprintf(fp, "g: ");
    cf_mpi_write(fp, SK_G(sk));
    fprintf(fp, "\n");
  }
    
  return cf_err;
}

cf_errno cf_key_ek_write(FILE *fp, const cf_key_ek_t ek) {
  cf_errno cf_err = E_NOERR;
  if(ek == NULL)
    return E_NO_EK_KEY;
    
  fprintf(fp, "p: ");
  cf_mpi_write(fp, EK_P(ek));
  fprintf(fp, "\n");
  if(ek->d > 0 && ek->h != NULL) {
    fprintf(fp, "h d:%u\n", ek->d);
    cf_mpis_write(fp, EK_H(ek), ek->d);
  }

  return cf_err;
}

cf_errno cf_key_sk_read(cf_key_sk_t *key, const char *fn) {
  FILE *fp = NULL;
  char line[LINE_LEN];
  char *var = NULL, *val = NULL;
  size_t var_len;
  cf_errno cf_err = E_NOERR;

  fp = fopen(fn, "r");
  if(fp == NULL) {
    return E_ERRNO_ERROR;
  }
  (*key) = (cf_key_sk_t)malloc(sizeof(struct cf_key_sk_s));
  if((*key) == NULL) {
    fclose(fp);
    cf_err = E_NO_MEM;
    return cf_err;
  }
  (*key)->p = NULL;
  (*key)->g = NULL;
  (*key)->x = NULL;
  (*key)->k = NULL;
  while(fgets(line, LINE_LEN, fp) != NULL) {
    // ignore comments and new lines
    if(line[0] != '#' && line[0] != CR && line[0] != LF) {
      cf_err = _line_parse_var_val(&var, &val, line);
      if(cf_err == E_NOERR) {
        var_len = strlen(var);
        if(strncmp(var, "k", var_len) == 0) {
          if((cf_err = cf_mpi_import_char(&((*key)->k), val)) != E_NOERR)
            break;
        }
        else if(strncmp(var, "x", var_len) == 0) {
          if((cf_err = cf_mpi_import_char(&((*key)->x), val)) != E_NOERR)
            break;
        }
        else if(strncmp(var, "p", var_len) == 0) {
          if((cf_err = cf_mpi_import_char(&((*key)->p), val)) != E_NOERR)
            break;
        }
        else if(strncmp(var, "g", var_len) == 0) {
          if((cf_err = cf_mpi_import_char(&((*key)->g), val)) != E_NOERR)
            break;
        }
        else
          cf_err = E_FILE_FORMAT;
      }
      else
        break;
    }    
  }
  fclose(fp);

  if(cf_err != E_NOERR) {
    cf_err = E_FILE_FORMAT;
  }
  return cf_err;
}

cf_errno cf_key_ek_read(cf_key_ek_t *key, const char *fn) {
  FILE *fp = NULL;
  char line[LINE_LEN];
  char *var = NULL, *val = NULL;
  size_t var_len;
  cf_errno cf_err = E_NOERR;
  int reading_h = 0;
  size_t d = 0, line_len, d_written = 0;
  char tokens[] = ":";
  
  fp = fopen(fn, "r");
  if(fp == NULL)
    return E_ERRNO_ERROR;
    
  (*key) = (cf_key_ek_t)malloc(sizeof(struct cf_key_ek_s));
  (*key)->h = NULL;
  (*key)->p = NULL;

  while(fgets(line, LINE_LEN, fp) != NULL) {
    line_len = strlen(line);
    // strip '\n'
    if(line[line_len - 1] == '\n')
      line[--line_len] = '\0';
    // ignore comments and new lines
    if(line[0] != '#' && line[0] != CR && line[0] != LF) {
      if(strncmp(line, "h d:", 4) == 0) {
        // the last h was not completely read, so throw error
        if(reading_h == 1) {
          cf_err = E_FILE_FORMAT;
          goto cleanup;
        }
        // last iteration we read d+1 h
        assert(d+1 == d_written || d == 0);
        d_written = 0;
        val = strtok(line, tokens);
        val = strtok(NULL, tokens);
        cf_err = str_to_size(&d, val, 10);
        if(cf_err != E_NOERR) {
          goto cleanup;
        }
        reading_h = 1;
        assert(d > 0);
        (*key)->h = calloc(d, sizeof(cf_mpi_t));
        (*key)->d = d;
      }
      else if(reading_h) {
        // Read mpi
        cf_err = cf_mpi_import_char(&((*key)->h[d_written]), line);
        if(cf_err != E_NOERR) {
          goto cleanup;
        }
        d_written++;
      }
      else {
        cf_err = _line_parse_var_val(&var, &val, line);
        if(cf_err == E_NOERR) {
          var_len = strlen(var);
          if(strncmp(var, "p", var_len) == 0) {
            if( (cf_err = cf_mpi_import_char((&(*key)->p), val)) != E_NOERR) {
              goto cleanup;
            }
          }
        }
        else {
          goto cleanup;
        }
      }
      if(d_written == (d + 1)) {
        reading_h = 0; // finished reading h
      }
    }
  }
  //if(d == (uint)-1)
  //  cf_err = E_FILE_FORMAT;
  assert(cf_err == E_NOERR);
cleanup:
  fclose(fp);
  if(cf_err != E_NOERR) {
    cf_err = E_FILE_FORMAT;
  }
  
  return cf_err;
}

void cf_key_ek_dump(const cf_key_ek_t ek) {
  fprintf(stderr, "ek {\n");
  if(ek != NULL) {
    fprintf(stderr, "  p: ");
    cf_mpi_dump(EK_P(ek));
    fprintf(stderr, "\n");
    if(ek->d > 0 && ek->h != NULL) {
      fprintf(stderr, "  h %u: ", ek->d);
      cf_mpis_dump(ek->h, ek->d);
      fprintf(stderr, "\n");
    }
  }
  else {
    fprintf(stderr, "  (null)");
  }
  fprintf(stderr, "}");
}

void cf_key_sk_dump(const cf_key_sk_t sk) {
  fprintf(stderr, "sk {\n");
  if(sk != NULL) {
    fprintf(stderr, "  k: ");
    cf_mpi_dump(SK_K(sk));
    fprintf(stderr, "\n  p: ");
    cf_mpi_dump(SK_P(sk));
    fprintf(stderr, "\n  x: ");
    cf_mpi_dump(SK_X(sk));
    fprintf(stderr, "\n");
    if(SK_G(sk) != NULL) {
      fprintf(stderr, "  g:  ");
      cf_mpi_dump(SK_G(sk));
      fprintf(stderr, "\n");
    }
  }
  else {
    fprintf(stderr, "  (null)");
  }
  fprintf(stderr, "}");
}

void cf_keys_dump(const cf_keys_t keys) {
  if(keys != NULL) {
    fprintf(stderr, "nbits: %u\n", keys->lambda);
    cf_key_ek_dump(keys->ek);
    fprintf(stderr, "\n");
    cf_key_sk_dump(keys->sk);
  }
  else {
    fprintf(stderr, "(null)");
  }
}

cf_errno cf_tags_read(cf_tag_t **tags, size_t *n_tags, const char *fn) {
  FILE *fp = NULL;
  size_t d = 0, line_len, d_written = 0;
  int reading_tag = 0;
  char line[LINE_LEN];
  cf_tag_t tmp_tag = NULL;
  char *val = NULL;
  char tokens[] = ":";
  cf_errno cf_err = E_NOERR;
  
  fp = fopen(fn, "r");
  if(fp == NULL) {
    return E_ERRNO_ERROR;
  }
  
  *n_tags = 0;
  while(fgets(line, LINE_LEN, fp) != NULL) {
    line_len = strlen(line);
    // strip '\n'
    if(line[line_len - 1] == '\n')
      line[--line_len] = '\0';
    // line has content, so parse it
    //if(line[0] != LF || line[0] != CR || line[0] != '#') {
    if(line_len > 0 && (line[0] != LF || line[0] != CR || line[0] != '#')) {
      // read tag degree
      if(strncmp(line, "tag d:", 6) == 0) {
        // the last tag was not completely read, so throw error
        if(reading_tag == 1) {
          cf_err = E_FILE_FORMAT;
          break;
        }
        // last iteration we read d+1 tags
        assert(d+1 == d_written || d == 0);
        d_written = 0;
        val = strtok(line, tokens);
        val = strtok(NULL, tokens);
        cf_err = str_to_size(&d, val, 10);
        if(cf_err != E_NOERR)
          break;
        reading_tag = 1;
        //d = strtoul(val, NULL, 10);
        cf_tag_new(&tmp_tag, d);
      }
      else if(reading_tag) {
        // read tag
        cf_err = cf_mpi_import_char(&(tmp_tag->y[d_written]), line);
        d_written++;
        assert(cf_err == E_NOERR);
      }
      // save tag
      if(d_written == (d + 1)) {
        reading_tag = 0;
        if((*n_tags) == 0) {
          (*tags) = calloc(1, sizeof(cf_tag_t));
        }
        else {
          (*tags) = realloc(*tags, sizeof(cf_tag_t) * (*n_tags + 1));
        }
        (*tags)[*n_tags] = cf_tag_copy(tmp_tag);
        cf_tag_free(tmp_tag);
        (*n_tags)++;
      }
    }
  }
  // cleanup
  if(cf_err != E_NOERR) {
    if((*tags) != NULL) {
      cf_tags_free(*tags, *n_tags);
    }
    cf_tag_free(tmp_tag);
    *tags = NULL;
    *n_tags = 0;
  }
  
  fclose(fp);
  
  return cf_err;
}

cf_errno cf_tag_write(const char *fn, const cf_tag_t tag) {
  cf_errno cf_err = E_NOERR;
  size_t i;
  FILE *fp = stdout;
  if(fn != NULL) {
    fp = fopen(fn, "w");
    if(fp == NULL) return E_ERRNO_ERROR;
  }
  
  fprintf(fp, "tag d:%zu\n", CF_TAG_DEGREE(tag));
  for(i = 0; i < tag->n; i++) {
    cf_mpi_write(fp, tag->y[i]);
    fprintf(fp, "\n");
  }
  
  if(fn != NULL)
    fclose(fp);
  return cf_err;
}

cf_errno cf_tags_write(const char *fn, const cf_tag_t *tags, size_t taglen) {
  FILE *fp = stdout;
  cf_errno cf_err = E_NOERR;
  size_t i;
  
  if(fn != NULL) {
    fp = fopen(fn, "w");
    if(fp == NULL) return E_ERRNO_ERROR;
  }
    
  for(i = 0; i < taglen; i++) {
    fprintf(fp, "tag d:%zu\n", CF_TAG_DEGREE(tags[i]));
    cf_mpis_write(fp, tags[i]->y, tags[i]->n);
  }

  if(fn != NULL)
    fclose(fp);
  return cf_err;
}

void cf_tag_dump(const cf_tag_t tag) {
  size_t i;
  fprintf(stderr, "tag {\n");
  if(tag == NULL) {
    fprintf(stderr, "  (null)");
  }
  else {
    fprintf(stderr, "  degree: %zu\n", CF_TAG_DEGREE(tag));
    fprintf(stderr, "  coeffs {\n    ");
    for(i = 0; i < (tag->n - 1); i++) {
      fprintf(stderr, "y%zu: ", i);
      cf_mpi_dump(tag->y[i]);
      fprintf(stderr, ", ");
    }
    fprintf(stderr, "y%zu: ", i);
    cf_mpi_dump(tag->y[i]);
    fprintf(stderr, "\n  }");
  }
  fprintf(stderr, "\n}");
}

void cf_tags_dump(const cf_tag_t *tags, size_t n_tags) {
  size_t i;
  for(i = 0; i < n_tags; i++) {
    cf_tag_dump(tags[i]);
    fprintf(stderr, "\n");
  }
}

cf_errno file_exists(const char *fn) {
  if(access(fn, F_OK) == -1)
    return E_ERRNO_ERROR;
  return E_NOERR;
}
