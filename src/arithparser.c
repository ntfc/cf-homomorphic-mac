#include <ctype.h> // isdigit
#include <regex.h>
#include <stdio.h>
#include "cf.h"
#include "logger.h"

static long line_nr; // line number currently being parsed

static cf_errno _arith_circ_parse_line(arith_circ_t arith, char *line);
static cf_errno _arith_circ_parse_io_line(gate_id_t **inputs, size_t *n_inputs, gate_id_t *gate_id, char *str);
static int _is_arith_circ_line_one_input(char *line);
static int _create_verb_and_rest_strings(char **verb, int *verb_len, char **rest, int *rest_len, const char *line);

//! @note Used only for pinocchio evals
cf_mpi_t* arith_circ_parse_io(const char *fn, size_t *nread) {
  FILE *fp = NULL;
  char *buf = NULL, *mpi_s = NULL, *current_pos = NULL, *line = NULL;
  long file_len = 0L, current_index = 0L, line_len = 0L, line_avail_space = 0L;
  int input_id;
  cf_mpi_t *dest = NULL;
  size_t i;
  int is_new_line;
  
  if(fn == NULL) {
    return NULL;
  }
  fp = fopen(fn, "r");
  if(fp == NULL) {
    return NULL;
  }
  fseek(fp, 0L, SEEK_END);
  file_len = ftell(fp);
  rewind(fp);
  
  buf = calloc(file_len + 1, sizeof(char));
  if(buf == NULL) {
    //LOG_PRINT("calloc error, all hell breaks loose\n");
    assert(1 == 0);
  }
  
  i = fread(buf, file_len, 1, fp);
  assert(i > 0);
  current_pos = buf;  
  *nread = 0;  
  while(*current_pos) {
    // copy line to buffer line
    current_index = 0L;
    is_new_line = 0;
    line = calloc(LINE_LEN_MIN, sizeof(char));
    line_len = LINE_LEN_MIN; // rename to line_avail_space;
    line_avail_space = line_len;
    while(*current_pos) {
      if(!is_new_line) {
        if(*current_pos == CR || *current_pos == LF) {
          is_new_line = 1;
        }
      }
      else if(*current_pos != CR && *current_pos != LF)
        break;
      if(line_avail_space == 1) { // Only one position left in the buffer => realloc
        // available space in line after realloc
        line_avail_space = line_len + 1;
        line_len *= 2;
        line = realloc(line, sizeof(char) * line_len);
        if(line == NULL) {
          //LOG_PRINT("Something went wrong with realloc..\n");
          fclose(fp); free(buf);
          return NULL;
        }
      }
      line[current_index++] = *current_pos++;
      line_avail_space--;
    }
    line[current_index] = '\0';
    // parse line
    if(line[0] != '#') {
      
      // read I/O value
      mpi_s = calloc(line_len, sizeof(char));
      if(sscanf(line, "%d %s", &input_id, mpi_s) < 0) {
        break;
      }
      if(*nread == 0) {
        dest = calloc(1, sizeof(cf_mpi_t));
        cf_mpi_import_char(&(dest[0]), mpi_s);
      }
      else {
        dest = realloc(dest, sizeof(cf_mpi_t) * ((*nread) + 1));
        cf_mpi_import_char(&(dest[*nread]), mpi_s);
      }
      if(mpi_s != NULL) {
        free(mpi_s);
        mpi_s = NULL;
      }
      (*nread)++;
    }
    free(line);
  }
  
  free(buf);
  fclose(fp);
  
  return dest;
}

//! @note Used only for pinocchio evals
void arith_circ_write_io(const char *fn, const cf_mpi_t *a, size_t len) {
  assert(a);
  assert(len > 0);
  FILE *fp = NULL;
  
  fp = fopen(fn, "w");
  assert(fp);

  size_t i;
  for(i = 0; i < len; i++) {
    fprintf(fp, "%zu ", i);
    cf_mpi_write(fp, a[i]);
    fprintf(fp, "\n");
  }

  fclose(fp);
}

/**
 * @param[out] arith  Pointer to arithmetic circuit.
 * @param[in]  fn     File name string.
 * 
 * @retval #E_NOERR
 * @retval #E_ERRNO_ERROR
 * @retval #E_NO_MEM
 */
cf_errno arith_circ_parse(arith_circ_t *arith, const char *fn) {
  cf_errno cf_err = E_NOERR;
  FILE *fp = NULL;
  long file_len;
  char *file_content = NULL;
  char *current_pos = NULL, *line = NULL;
  int is_new_line = -1;
  long current_index = 0L, line_len = 0L;
  long line_avail_space = 0L;
  size_t i;
  
  if(fn == NULL) return E_NO_ARITH_FILE;
  if(arith == NULL) return E_NO_ARITH;
  fp = fopen(fn, "r");
  if(fp == NULL) {
    return E_ERRNO_ERROR;
  }

  fseek(fp, 0L, SEEK_END);
  file_len = ftell(fp);
  rewind(fp);
  
  file_content = calloc(file_len + 1, sizeof(char));
  if(file_content == NULL) {
    //LOG_PRINT("calloc error, all hell breaks loose\n");
    fclose(fp);
    return E_NO_MEM;
  }

  i = fread(file_content, file_len, 1, fp);
  assert(i > 0);
  current_pos = file_content;
  line_nr = 0L;

  arith_circ_init(arith);
  if(*arith == NULL) {
    cf_err = E_NO_MEM;
  }
  
  if(cf_err == E_NOERR) {
    (*arith)->fn = calloc(strlen(fn) + 1, sizeof(char));
    strncpy((*arith)->fn, fn, strlen(fn));

    while(*current_pos) {
      // copy line to buffer line
      current_index = 0L;
      is_new_line = 0;
      line = calloc(LINE_LEN_MIN, sizeof(char));
      line_len = LINE_LEN_MIN; // rename to line_avail_space;
      line_avail_space = line_len;
      while(*current_pos) {
        if(!is_new_line) {
          if(*current_pos == CR || *current_pos == LF) {
            is_new_line = 1;
          }
        }
        else if(*current_pos != CR && *current_pos != LF)
          break;
        if(line_avail_space == 1) { // Only one position left in the buffer => realloc
          // available space in line after realloc
          line_avail_space = line_len + 1;
          line_len *= 2;
          line = realloc(line, sizeof(char) * line_len);
          if(line == NULL) {
            //LOG_PRINT("Something went wrong with realloc..\n");
            cf_err = E_NO_MEM;
            break;
          }
        }
        line[current_index++] = *current_pos++;
        line_avail_space--;
      }
      if(cf_err != E_NOERR)
        break;
        
      line[current_index] = '\0';
      line_nr++;
      // parse line
      if(line[0] != '#') {
        cf_err = _arith_circ_parse_line(*arith, line);
        if(cf_err != E_NOERR) {
          //LOG_PRINT("error parsing line %ldd.\n", line_nr);
          free(line);
          break;
        }
      }
      free(line);
    }
  }
  
  if(cf_err == E_NOERR) {
    /*if(line_nr < 0 && arith_circ_n_gates(*arith) + 2 != (size_t)line_nr) {
      cf_err = E_FILE_FORMAT;
      LOG_PRINT("error parsing the arith circ in '%s'. please double check that the file is ok.\n", fn);
      arith_circ_free(*arith);
      *arith = NULL;
    }*/
    if((*arith)->t_sort == NULL) {
      cf_err = arith_circ_topology_sort(*arith);
      assert(cf_err == E_NOERR);
    }
  }
  else {
    arith_circ_free(*arith);
    *arith = NULL;
  }
  free(file_content);
  fclose(fp);
  
  return cf_err;
}

static int _is_arith_circ_line_one_input(char *line) {
  int one_input = 0;
  char *aux1 = NULL, *aux2 = NULL;
  aux1 = line;
  if(strstr(aux1, "input") != NULL) {
    if( (aux2 = strtok(aux1, "#")) != NULL) {
      if( (aux2 = strtok(NULL, "#")) != NULL) {
        aux2 = strtok(aux2, " "); // remove possible leading space from aux
        // if the first 9 letters of aux are 'one-input', then this is
        // a special case o an one-input
        if(aux2 && strncmp(aux2, "one-input", 9) == 0)
          one_input = 1;
      }
    }
  }
  return one_input;
}

static int _create_verb_and_rest_strings(char **verb, int *verb_len, char **rest, int *rest_len, const char *line) {
  int extra_spaces = -1;
  int i;
  int rest_start = -1, rest_end = -1;
  int line_len = -1;
  //  strchr returns the first occurrent of a ' ', there is, the position+1 of
  //  the last character of the verb (so we need to alloc verb_len + 1
  //  characters)
  *verb_len = strchr(line, ' ') - line; // 'add' => verb_len = 3
  // count number of extra spaces between verb and rest
  for(extra_spaces = 0, i = *verb_len; line[i] == ' '; extra_spaces++, i++)
    ;
  //start of rest
  rest_start = *verb_len + extra_spaces;

  // find out where rest ends
  line_len = strlen(line);
  for(i = line_len; i > rest_start; i--) {
    if((line[i] == '>') || (isdigit(line[i]))) {
      // i is the actual position of the end of rest
      break;
    }
  }
  rest_end = i;
  *rest_len = rest_end - rest_start + 1;

  // init verb and rest strings
  // add the '\0' to the end, hence the +1
  *verb = calloc(sizeof(char), *verb_len + 1);
  *rest = calloc(sizeof(char), *rest_len + 1);
  memcpy(*verb, line, *verb_len);
  memcpy(*rest, (line + rest_start), *rest_len);
  
  return 1;
}

static cf_errno _arith_circ_parse_line(arith_circ_t arith, char *line) {
  int one_input = 0;
  char *aux = NULL;
  char *verb = NULL, *rest = NULL;
  size_t input_id;
  int verb_len = -1, rest_len = -1;
  size_t tmp = (size_t)-1;
  type_e type = NONE;
  size_t gate_n_inputs = (size_t)-1;
  gate_id_t *inputs_ids = NULL;
  gate_id_t gate_id;
  cf_mpi_t const_val;
  //size_t n_written = 0;
  gate_t gate = NULL;
  cf_errno cf_err = E_NOERR;

  // if the current line is an input line, check if it is an one-input line!
  one_input = _is_arith_circ_line_one_input(line);
  aux = line;
  line = strtok(aux, "#"); // remove possible comments
  line = strtok(line, "\n");

  _create_verb_and_rest_strings(&verb, &verb_len, &rest, &rest_len, line);
  
  if(strncmp(verb, "output", 6) == 0) {
    // ignore
    free(rest);
    free(verb);
    return E_NOERR;
  }
  if(strncmp(verb, "total", 5) == 0) {
    if( (cf_err = str_to_size(&tmp, rest, 10)) != E_NOERR) {
      LOG_PRINT("error reading the total number of gates at line %ld\n", line_nr);
      free(verb);
      free(rest);
      return cf_err;
    }
    arith_circ_init_gates(arith, tmp);
  }
  else {
    if(strncmp(verb, "input", 5) == 0) {
      type = INPUT;
      if(one_input == 1)
        type = ONEINPUT;
      // read gate id
      if( (cf_err = str_to_size(&tmp, rest, 10)) != E_NOERR) {
        LOG_PRINT("error reading input id '%s' at line %ld\n", rest, line_nr);
        free(rest);
        free(verb);
        return cf_err;
      }
      gate_n_inputs = 0;
      inputs_ids = NULL;
    }
    else if(strncmp(verb, "add", 3) == 0) {
      type = ADD;
    }
    else if(strncmp(verb, "mul", 3) == 0) {
      type = MUL;
    }
    else if(strncmp(verb, "const-mul-", 10) == 0) {
      type = CONSTMUL;
      //cf_err = char_to_mpi(&const_val, &n_written, verb+10);
      cf_err = cf_mpi_import_char(&const_val, verb+10);
      if(cf_err != E_NOERR) {
        LOG_PRINT("error reading const value from '%s' at line %ld\n", verb+10, line_nr);
        free(rest);
        free(verb);
        return cf_err;
      }
      // FIXME: not needed?
      /*if(n_written != (size_t)(verb_len - 10)) {
        LOG_PRINT("error: did not read the constant value correcly.\n");
        free(rest);
        free(verb);
        return E_AC_FILE_FORMAT;
      }*/
    }
    else {
      LOG_PRINT("error: invalid gate type '%s'\n", verb);
      free(rest);
      free(verb);
      return E_INVAL_GATE_TYPE;
    }
    if(type != INPUT && type != ONEINPUT) {
      // parse I/O line. tmp is the gate_id
      if((cf_err = _arith_circ_parse_io_line(&inputs_ids, &gate_n_inputs, &gate_id, rest)) != E_NOERR) {
        //LOG_PRINT("error: couldn't parse I/O of ADD/MUL/CONSTMUL gate at line %ld\n", line_nr);
        return cf_err;
      }
    }
    if((type == INPUT || type == ONEINPUT) && gate_n_inputs != 0) {
      LOG_PRINT("error: wrong number of inputs for INPUT gate at line %ld\n", line_nr);
      return E_GATE_INPUTS;
    }
    if((type == CONSTMUL) && gate_n_inputs != 1) {
      LOG_PRINT("error: wrong number of inputs for CONSTMUL gate at line %ld\n", line_nr);
      return E_GATE_INPUTS;
    }
    if((type == ADD || type == MUL) && gate_n_inputs != 2) {
      LOG_PRINT("error: wrong number of inputs for ADD or MUL gate at line %ld\n", line_nr);
      return E_GATE_INPUTS;
    }
    // create and insert gate
    gate = gate_create(type, inputs_ids, gate_n_inputs, const_val);
    for(input_id = 0; input_id < gate_n_inputs; input_id++) {
      // add the edges to the graph
      // FIXME: this if is useless right?
      /*if(type == INPUT || type == ONEINPUT) {
        cf_err = graph_add_edge(arith->graph, inputs_ids[input_id], tmp);
        assert(cf_err == E_NOERR);
      }*/
      //else {
        assert(type != INPUT && type != ONEINPUT);
        cf_err = graph_add_edge(arith->graph, inputs_ids[input_id], gate_id);
        assert(cf_err == E_NOERR);
        // now we can set id = gate_id
        tmp = gate_id;
      //}
    }
    
    cf_err = arith_circ_add_gate(arith, tmp, gate);
    assert(cf_err == E_NOERR);

    // frees
    gate_free(gate);
    gate = NULL;
    if(inputs_ids != NULL) {
      free(inputs_ids);
      inputs_ids = NULL;
      gate_n_inputs = 0;
    }
    gate_id = 0;
    if(type == CONSTMUL) {
      cf_mpi_free(const_val);
    }
    type = NONE;
  }

  free(rest);
  free(verb);
  return cf_err;
}

static cf_errno _arith_circ_parse_io_line(gate_id_t **inputs, size_t *n_inputs, gate_id_t *gate_id, char *str) {
  regex_t re;
  //int err = -1;
  size_t n_groups, n_matched;
  regoff_t start, end;
  char *match = NULL;
  size_t in_len = (size_t)-1, out_len = (size_t)-1;
  size_t in_id;
  regmatch_t *groups = NULL;
  char *tmp = NULL;
  char regex[] = "in +([0-9]*) +<([0-9 ]*)> +out +([0-9]*) +<([0-9 ]*)> *";
  cf_errno cf_err = E_NOERR;

  regex_err = regcomp(&re, regex, REG_EXTENDED);
  if(IS_REGEX_ERROR) {
    LOG_PRINT("error compiling regular expression for line %ld\n", line_nr);
    return E_REGEX_ERROR;
  }

  n_groups = re.re_nsub + 1;
  groups = malloc(n_groups * sizeof(regmatch_t));
  if((regex_err = regexec(&re, str, n_groups, groups, 0)) == REG_NOMATCH) {
    LOG_PRINT("error: no match found for regex '%s' at line %ld\n", regex, line_nr);
    regfree(&re);
    return E_REGEX_ERROR;
  }
  
  for(n_matched = 0; n_matched < n_groups; n_matched++) {
    if(groups[n_matched].rm_so == (-1)) {
      // first invalid group
      //LOG_PRINT("warning: first invalid group is %zu\n", n_matched);
      break;
    }
    // dont know why, but in the 0 match is the original string
    if(n_matched != 0) {
      start = groups[n_matched].rm_so;
      end = groups[n_matched].rm_eo;

      // copy matched string to a string
      match = calloc(sizeof(char), end-start+1);
      memcpy(match, str+start, end-start);
      switch(n_matched - 1) {
        case 0: // read number of inputs
          cf_err = str_to_size(&in_len, match, 10);
          if(cf_err != E_NOERR) {
            LOG_PRINT("error reading the number of inputs from '%s' at line %ld\n", match, line_nr);
            free(match);
            free(groups);
            regfree(&re);
            return cf_err;
          }
          if(in_len != 1 && in_len != 2) {
            LOG_PRINT("error: wrong number of inputs. should have 1 or 2, not %zu, at line %ld\n", in_len, line_nr);
            free(match);
            free(groups);
            regfree(&re);
            return E_GATE_INPUTS;
          }
          *inputs = calloc(in_len, sizeof(gate_id_t));
          *n_inputs = in_len;
          break;
        case 1: // read input ids, one at a time
          tmp = match;
          tmp = strtok(tmp, " ");
          if( (cf_err = str_to_size(&in_id, tmp, 10)) != E_NOERR) {
            LOG_PRINT("error: cannot read input id from '%s' at line %ld\n", tmp, line_nr);
            free(match);
            free(groups);
            regfree(&re);
            return cf_err;
          }
          (*inputs)[0] = in_id;
          if(in_len > 1) {
            tmp = strtok(NULL, " ");
            if( (cf_err = str_to_size(&in_id, tmp, 10)) != E_NOERR) {
              LOG_PRINT("error: cannot read input id from '%s' at line %ld\n", tmp, line_nr);
              free(match);
              free(groups);
              regfree(&re);
              return cf_err;
            }
            (*inputs)[1] = in_id;
          }
          break;
        case 2: // read number of outputs
          if( (cf_err = str_to_size(&out_len, match, 10)) != E_NOERR) {
            LOG_PRINT("error: cannot read number of outputs from '%s' at line %ld\n", match, line_nr);
            free(match);
            free(groups);
            regfree(&re);
            return cf_err;
          }
          if(out_len != 1) {
            //LOG_PRINT("error: only gates with 1 output are supported\n");
            // TODO: free's
            return E_GATE_OUTPUT;
          }
          break;
        case 3: // read output id
          if( (cf_err = str_to_size(gate_id, match, 10)) != E_NOERR) {
            LOG_PRINT("error: cannot read gate id from '%s' at line %ld\n", match, line_nr);
            free(match);
            free(groups);
            regfree(&re);
            return cf_err;
          }
          break;
        default:
          LOG_PRINT("couldn't not find behavior for match %zu at line %ld\n", n_matched, line_nr);
          cf_err = E_AC_FILE_FORMAT;
          break;
      }
      free(match);
    }
  }
  free(groups);
  regfree(&re);
  return cf_err;
}
