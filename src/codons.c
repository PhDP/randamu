#include "raw/codons.h"
#include "raw/str.h"

void r_codons_init(r_codons* c, const size_t nmemb) {
  c->codons = (uint32_t*)malloc(nmemb * sizeof(uint32_t));
  c->nmemb = nmemb;
}

void r_codons_random_init(r_codons* c, const size_t nmemb, sfmt_t* rng) {
  r_codons_init(c, nmemb);
  r_codons_random_fill(c, rng);
}

void r_codons_random_fill(r_codons* c, sfmt_t* rng) {
  size_t i = 0;
  for (; i < c->nmemb; ++i) {
    c->codons[i] = sfmt_genrand_uint32(rng);
  }
}

void r_codons_cpy(r_codons* dest, const r_codons* src) {
  dest->codons = (uint32_t*)realloc(dest->codons, src->nmemb);
  dest->nmemb = src->nmemb;
  memcpy((void*)dest->codons, (void*)src->codons, dest->nmemb * sizeof(uint32_t));
}

static char* gen(const r_codons* c, size_t* idx, const r_grammar* g, const size_t current, size_t* wrap, const size_t max_wrap) {
  if (*idx >= c->nmemb) {
    (*wrap)++;
    if (*wrap > max_wrap) {
      return NULL;
    }
    *idx = 0;
  }
  size_t length = 0;
  size_t capacity = 63;
  char* s = (char*)malloc(capacity + 1);

  // This is actually ever-so-slightly biaised unless the grammar divides 2**32 perfectly.
  const size_t alt = g->rules[current].nmemb > 1? c->codons[*idx] % g->rules[current].nmemb : 0;
  (*idx) += g->rules[current].nmemb > 1;

  // Loop over the elements of the selected alternative:
  size_t e = 0;
  for (; e < g->rules[current].alts[alt].nmemb; ++e) {
    if (r_surrounded(g->rules[current].alts[alt].elems[e], '<', '>')) {
      const int64_t rule_index = r_grammar_rule_idx(g, g->rules[current].alts[alt].elems[e]);
      if (rule_index < 0) {
        free(s);
        return NULL;
      }
      char* res = gen(c, idx, g, rule_index, wrap, max_wrap);
      if (!res) {
        free(s);
        return NULL;
      }
      r_strcat(&s, &length, &capacity, res);
      free(res);
    } else {
      r_strcat(&s, &length, &capacity, g->rules[current].alts[alt].elems[e]);
    }
  }
  return s;
}

char* r_codons_generate(const r_codons* c, const r_grammar* g, const size_t max_wrap) {
  size_t idx = 0;
  size_t wrap = 0;
  return gen(c, &idx, g, 0, &wrap, max_wrap);
}

void r_codons_show(const r_codons* c, FILE* out) {
  if (c->nmemb) {
    fprintf(out, "%u", c->codons[0]);
    size_t i = 1;
    for (; i < c->nmemb; ++i) {
      fprintf(out, "-%u", c->codons[i]);
    }
  }
}

void r_codons_free(r_codons* c) {
  if (c && c->codons) {
    free(c->codons);
    c->nmemb = 0;
  }
}

